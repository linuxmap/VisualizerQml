#include "displayitem.h"
#include <d3d9.h>
#include <D3D11.h>
#include <QQuickWindow>
DisplayItem::DisplayItem(QQuickItem * parent)
	: QQuickFramebufferObject(parent)
	, m_bIsPlaying(false)
	, _isVideoDirty(false)
	, _videoFrameRate(0.0)
	, _avFrame(new Frame)
	, _isYuy2(false)
	, m_target(0, 0, -1)
	, m_syncState(AllNeedsSync)
{
	_avFrame->frame = av_frame_alloc();
	//setFlag(ItemHasContents);
}

DisplayItem::~DisplayItem()
{
	lockFrame();
	if (!_isYuy2)
	{
		av_frame_unref(_avFrame->frame);
		av_frame_free(&(_avFrame->frame));
	}
	delete _avFrame;
	unlockFrame();
}

int DisplayItem::renderFrame(Frame *frame)
{
	if (frame == NULL)//stop render image data 
	{
	}
	lockFrame();
	av_frame_unref(_avFrame->frame);
	if (frame)
		av_frame_ref(_avFrame->frame, frame->frame);
	unlockFrame();
	update();
	return 0;
}

int DisplayItem::renderFrame(AVFrame * frame)
{
	if (frame == NULL)//stop render image data 
	{
	}
	lockFrame();
	if (_isYuy2)
		_avFrame->frame = frame;
	else
	{
		av_frame_unref(_avFrame->frame);
		if (frame)av_frame_ref(_avFrame->frame, frame);
	}
	unlockFrame();
	//update();
	QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
	return 0;
}

int DisplayItem::renderModeChanged(bool isYuy2)
{
	if (_isYuy2 == isYuy2)return 0;
	_isYuy2 = isYuy2;
	lockFrame();
	if (_isYuy2)
		av_frame_free(&_avFrame->frame);
	else
		_avFrame->frame = av_frame_alloc();
	unlockFrame();
	return 0;
}

void DisplayItem::lockFrame(void)
{
	_frameLock.lock();
}

void DisplayItem::unlockFrame(void)
{
	_frameLock.unlock();
}


QQuickFramebufferObject::Renderer * DisplayItem::createRenderer() const
{
	return new OpenGLRenderNode();
}

double DisplayItem::videoFrameRate(void)
{
	return _videoFrameRate;
}

void DisplayItem::setScale(qreal scale)
{
	QQuickItem::setScale(scale);
	hhtdebug("emit scaleChanged");
	emit scaleChanged(scale);
}

void DisplayItem::setEye(const QVector3D &v)
{
	if (m_eye != v) {
		m_eye = v;
		m_syncState |= CameraNeedsSync;
		update();
	}
}

void DisplayItem::setTarget(const QVector3D &v)
{
	if (m_target != v) {
		m_target = v;
		m_syncState |= CameraNeedsSync;
		update();
	}
}

void DisplayItem::setRotation(const QVector3D &v)
{
	if (m_rotation != v) {
		m_rotation = v;
		m_syncState |= RotationNeedsSync;
		update();
	}
}

int DisplayItem::swapSyncState()
{
	int s = m_syncState;
	m_syncState = 0;
	return s;
}


void DisplayItem::sltStreamOpened(void)
{
}

void DisplayItem::sltStreamClosed(void)
{
}
bool DisplayItem::event(QEvent *e)
{
	//qDebug() << e->type();
	//switch (e->type())
	//{
	//case QEvent::FocusOut:
	//case QEvent::FocusIn:
	//	qDebug() << "video update";
	//	update();
	//	break;
	//default:break;
	//}
	return QQuickItem::event(e);
}

OpenGLRenderNode::OpenGLRenderNode()
	:m_item(NULL)
	, m_pCurShaderProgram(NULL)
	, m_bInit(false)
	, m_lastColorSpace(-1)
	, m_frameCnt(0)
	, m_beginTime(0)
	, m_endTime(0)
	, _dxvaFrameLocked(false)
	, _oldFrameWidth(-1)
	, _oldFrameHeight(-1)
{
	memset(_vertices, 0, sizeof(_vertices));
	for (int i = 0; i < AttribNum; i++)
	{
		Attribs[i] = i + 3;
	}
	textureUniformY = 0;
	textureUniformU = 0;
	textureUniformV = 0;
	id_y = 0;
	id_u = 0;
	id_v = 0;
	m_pTextureY = NULL;
	m_pTextureU = NULL;
	m_pTextureV = NULL;
	_pD3d11Frame = av_frame_alloc();

}

OpenGLRenderNode::~OpenGLRenderNode() {
	qDebug() << "delete opengl render node";
}


void OpenGLRenderNode::render()
{
	if (!m_bInit)
	{
		init();
		m_bInit = true;
	}
	if (!m_item->_avFrame->frame)return;
	m_item->lockFrame();
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	if (m_item->_avFrame->frame->format == AV_PIX_FMT_DXVA2_VLD)
	{
		dxva2_retrieve_data(m_item->_avFrame->frame);
	}
	else if (m_item->_avFrame->frame->format == AV_PIX_FMT_D3D11)
	{
		d3d11va_retrieve_data(m_item->_avFrame->frame);
	}
	switch (m_item->_avFrame->frame->format)
	{
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
	case AV_PIX_FMT_YUVJ422P:
		m_pCurShaderProgram = m_pShaderProgramMap->value(QString("yuv420p"));
		break;
	case AV_PIX_FMT_YUYV422:
		m_pCurShaderProgram = m_pShaderProgramMap->value(QString("yuy2"));
		break;
	case AV_PIX_FMT_NV12:
		m_pCurShaderProgram = m_pShaderProgramMap->value(QString("nv12"));
		break;
	default:
		goto EXIT;
	}

	if (m_pCurShaderProgram)
	{
		if (!m_pCurShaderProgram->bind())
		{
			hhtdebug("Bind program failed");
		}
		textureUniformY = m_pCurShaderProgram->uniformLocation("tex_y");
		textureUniformU = m_pCurShaderProgram->uniformLocation("tex_u");
		textureUniformV= m_pCurShaderProgram->uniformLocation("tex_v");
		//m_matrixUniform = m_pCurShaderProgram->uniformLocation("matrix");
		//m_opacityUniform = m_pCurShaderProgram->uniformLocation("opacity");
	}
	else
	{
		goto EXIT;
	}

	if (_oldFrameWidth != m_item->_avFrame->frame->width
		|| _oldFrameHeight != m_item->_avFrame->frame->height
		)
	{
		videoResolChanged();
		_oldFrameWidth = m_item->_avFrame->frame->width;
		_oldFrameHeight = m_item->_avFrame->frame->height;
		if (formatChanged())
			m_lastColorSpace = m_item->_avFrame->frame->format;
		else
			goto EXIT;
	}
	else if (m_lastColorSpace != m_item->_avFrame->frame->format)
	{
		if (formatChanged())
			m_lastColorSpace = m_item->_avFrame->frame->format;
		else
			goto EXIT;
	}
	//m_pCurShaderProgram->setUniformValue(m_opacityUniform, GLfloat(m_item->opacity()));
	f->glVertexAttribPointer(Attribs[0], 2, GL_FLOAT, GL_FALSE, 0, _vertices[0]);
	f->glEnableVertexAttribArray(Attribs[0]);
	f->glVertexAttribPointer(Attribs[1], 2, GL_FLOAT, GL_FALSE, 0, _vertices[1]);
	f->glEnableVertexAttribArray(Attribs[1]);
	paintGL();
	m_pCurShaderProgram->release();
	f->glDisableVertexAttribArray(Attribs[0]);
	f->glDisableVertexAttribArray(Attribs[1]);
	if (_dxvaFrameLocked)
	{
		dxva2_free_data(m_item->_avFrame->frame);
	}
EXIT:
	m_item->unlockFrame();
	QQuickWindow *w = m_item->window();
	w->resetOpenGLState();

}

void OpenGLRenderNode::synchronize(QQuickFramebufferObject *item)
{
	//qDebug() << "synchronize--" << item;
	m_item = static_cast<DisplayItem*>(item);
}

QOpenGLFramebufferObject * OpenGLRenderNode::createFramebufferObject(const QSize &size)
{
	QOpenGLFramebufferObjectFormat format;
	format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
	format.setSamples(0);
	return new QOpenGLFramebufferObject(size, format);
}

bool OpenGLRenderNode::init(void)
{
	m_pVShaderMap = new QMap<QString, QOpenGLShader*>;
	m_pFShaderMap = new QMap<QString, QOpenGLShader*>;
	m_pShaderProgramMap = new QMap<QString, QOpenGLShaderProgram*>;

	QStringList filter;
	QList<QFileInfo> files;

	filter = QStringList("*.vsh");
	files = QDir(":/shaders/").entryInfoList(filter, QDir::Files | QDir::Readable);
	foreach(QFileInfo file, files)
	{
		QOpenGLShader *vShader = new QOpenGLShader(QOpenGLShader::Vertex, NULL);
		bool bCompile = vShader->compileSourceFile(file.absoluteFilePath());
		if (!bCompile)
		{
			hhtdebug("Compile vertex shader code failed ");
		}
		else
			m_pVShaderMap->insert(file.baseName(), vShader);
	}

	filter = QStringList("*.fsh");
	files = QDir(":/shaders/").entryInfoList(filter, QDir::Files | QDir::Readable);
	foreach(QFileInfo file, files)
	{
		QOpenGLShader *fShader = new QOpenGLShader(QOpenGLShader::Fragment, NULL);
		bool bCompile = fShader->compileSourceFile(file.absoluteFilePath());
		if (!bCompile)
		{
			hhtdebug("Compile vertex shader code failed ");
		}
		else
			m_pFShaderMap->insert(file.baseName(), fShader);
	}
	//QList<QOpenGLShader> fshaderList = m_pFShaderMap->values();
	foreach(QOpenGLShader *fshader, m_pFShaderMap->values())
	{
		QOpenGLShaderProgram* pProgram = new QOpenGLShaderProgram;
		pProgram->addShader(fshader);
		QString fshaderName = m_pFShaderMap->key(fshader);
		if (m_pVShaderMap->contains(fshaderName))
		{
			pProgram->addShader(m_pVShaderMap->value(fshaderName));
		}
		else
		{
			pProgram->addShader(m_pVShaderMap->value(QString("vertexshader")));
		}
		bool ret = pProgram->link();
		if (!ret)
		{
			hhtdebug("Link shader program failed ");
		}
		else
		{
			m_pShaderProgramMap->insert(fshaderName, pProgram);
			pProgram->bind();
		}
		pProgram->bindAttributeLocation("vertexIn", Attribs[0]);
		pProgram->bindAttributeLocation("textureIn", Attribs[1]);

		if (m_pShaderProgramMap->size() != 0)
		{
			//m_currShaderProgram = m_pShaderProgramMap->lastKey();

		}
	}
	m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
	m_pTextureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
	m_pTextureV= new QOpenGLTexture(QOpenGLTexture::Target2D);
	m_pTextureY->create();
	m_pTextureU->create();
	m_pTextureV->create();
	id_y = m_pTextureY->textureId();
	id_u = m_pTextureU->textureId();
	id_v = m_pTextureV->textureId();
	return true;
}

void OpenGLRenderNode::paintGL(void)
{
	if (!m_item->_avFrame)return;
#if 0
	if (0 == m_frameCnt)
	{
		m_beginTime = g_programTimer.elapsed();
		m_frameCnt++;
	}
	else if (m_frameCnt < 100)
		m_frameCnt++;
	else if (100 == m_frameCnt)
	{
		m_frameCnt = 0;
		m_endTime = g_programTimer.elapsed();
		if (m_endTime > m_beginTime)
		{
			float fps = 100.0 / (float(m_endTime - m_beginTime) / 1000.0);
			hhtdebug("FPS:%f", fps);
		}
	}
#endif
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	f->glDepthMask(false);
	f->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	f->glClear(GL_COLOR_BUFFER_BIT);
	//f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	f->glDisable(GL_CULL_FACE);
	f->glDisable(GL_DEPTH_TEST);
	f->glActiveTexture(GL_TEXTURE0);
	f->glBindTexture(GL_TEXTURE_2D, id_y);
	switch (m_item->_avFrame->frame->format)
	{
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
	case AV_PIX_FMT_YUVJ422P:
	case AV_PIX_FMT_NV12:
		f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[0], m_item->_avFrame->frame->height, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[0]);
		break;
	case AV_PIX_FMT_YUYV422:
		f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[0], m_item->_avFrame->frame->height, GL_RG, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[0]);
		break;
	default:
		break;
	}
	f->glUniform1i(textureUniformY, 0);

	f->glActiveTexture(GL_TEXTURE1);
	f->glBindTexture(GL_TEXTURE_2D, id_u);
	switch (m_item->_avFrame->frame->format)
	{
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
		f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[1], m_item->_avFrame->frame->height / 2, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[1]);
		break;
	case AV_PIX_FMT_YUYV422:
		f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[0] / 2, m_item->_avFrame->frame->height, GL_RGBA, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[0]);
		break;
	case AV_PIX_FMT_YUVJ422P:
		f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[1], m_item->_avFrame->frame->height, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[1]);
		break;
	case AV_PIX_FMT_NV12:
		f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[1] / 2, m_item->_avFrame->frame->height / 2, GL_RG, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[1]);
		break;

	default:
		break;
	}
	f->glUniform1i(textureUniformU, 1);

	if (m_item->_avFrame->frame->format != AV_PIX_FMT_YUYV422
		&& m_item->_avFrame->frame->format != AV_PIX_FMT_NV12
		)
	{
		f->glActiveTexture(GL_TEXTURE2);
		f->glBindTexture(GL_TEXTURE_2D, id_v);
		switch (m_item->_avFrame->frame->format)
		{
		case AV_PIX_FMT_YUV420P:
		case AV_PIX_FMT_YUVJ420P:
			f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[2], m_item->_avFrame->frame->height / 2, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[2]);
			break;
		case AV_PIX_FMT_YUVJ422P:
			f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[2], m_item->_avFrame->frame->height, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[2]);
			break;
		case AV_PIX_FMT_NV12:
			f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_item->_avFrame->frame->linesize[1] / 2, m_item->_avFrame->frame->height / 2, GL_RG, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[1]);
			break;
		default:
			break;
		}
		f->glUniform1i(textureUniformV, 2);
	}


	f->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


}

bool OpenGLRenderNode::videoResolChanged(void)
{
#if 0
	float frameAsr = float(m_item->_avFrame->frame->width) / float(m_item->_avFrame->frame->height);
	float itemWidth = float(m_item->width());
	float itemHeight = float(m_item->height());
	float itemAsr = itemWidth / itemHeight;
	float adjFrameWidth, adjFrameHeight, scale;
	float x, y, w, h;

#if 0
	if (frameAsr >= itemAsr)
	{
		adjFrameWidth = itemWidth;
		scale = adjFrameWidth / float(_avFrame->frame->width);
		adjFrameHeight = float(_avFrame->frame->height)*scale;
		x = 0;
		y = (itemHeight - adjFrameHeight) / 2;
		w = adjFrameWidth;
		h = adjFrameHeight;
	}
	else
	{
		adjFrameHeight = itemHeight;
		scale = adjFrameHeight / float(_avFrame->frame->height);
		adjFrameWidth = float(_avFrame->frame->width)*scale;
		x = (itemWidth - adjFrameWidth) / 2;
		y = 0;
		w = adjFrameWidth;
		h = adjFrameHeight;
	}
#endif
	qreal newItemHeight = itemWidth / frameAsr;
	m_item->setHeight(newItemHeight);
	x = 0;
	y = 0;
	w = itemWidth;
	h = newItemHeight;


	_vertices[0][0] = x;
	_vertices[0][1] = y;
	_vertices[0][2] = x + w - 1;
	_vertices[0][3] = y;
	_vertices[0][4] = x + w - 1;
	_vertices[0][5] = y + h;
	_vertices[0][6] = x;
	_vertices[0][7] = y + h;

	_vertices[1][2] = float(m_item->_avFrame->frame->width) / float(m_item->_avFrame->frame->linesize[0]) - 0.01;
	_vertices[1][4] = _vertices[1][2];
	_vertices[1][5] = 1;
	_vertices[1][7] = 1;
#endif
	_vertices[0][0] = -1.0;
	_vertices[0][1] = -1.0;
	_vertices[0][2] = 1.0;
	_vertices[0][3] = -1.0;
	_vertices[0][4] = 1.0;
	_vertices[0][5] = 1.0;
	_vertices[0][6] = -1.0;
	_vertices[0][7] = 1.0;

	_vertices[1][2] = float(m_item->_avFrame->frame->width) / float(m_item->_avFrame->frame->linesize[0]) - 0.01;
	_vertices[1][4] = _vertices[1][2];
	_vertices[1][5] = 1;
	_vertices[1][7] = 1;
	return true;

}
bool OpenGLRenderNode::formatChanged(void)
{
	qDebug() << "Format changed";

	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

	f->glActiveTexture(GL_TEXTURE0);
	f->glBindTexture(GL_TEXTURE_2D, id_y);
	switch (m_item->_avFrame->frame->format)
	{
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
	case AV_PIX_FMT_YUVJ422P:
	case AV_PIX_FMT_NV12:
		f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_item->_avFrame->frame->linesize[0], m_item->_avFrame->frame->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[0]);
		break;
	case AV_PIX_FMT_YUYV422:
		f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, m_item->_avFrame->frame->linesize[0], m_item->_avFrame->frame->height, 0, GL_RG, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[0]);
		break;
	default:
		break;
	}

	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	f->glUniform1i(textureUniformY, 0);

	f->glActiveTexture(GL_TEXTURE1);
	f->glBindTexture(GL_TEXTURE_2D, id_u);
	switch (m_item->_avFrame->frame->format)
	{
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
		f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_item->_avFrame->frame->linesize[1], m_item->_avFrame->frame->height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[1]);
		break;
	case AV_PIX_FMT_YUYV422:
		f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_item->_avFrame->frame->linesize[0] / 2, m_item->_avFrame->frame->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[0]);
		break;
	case AV_PIX_FMT_YUVJ422P:
		f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_item->_avFrame->frame->linesize[1], m_item->_avFrame->frame->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[1]);
		break;
	case AV_PIX_FMT_NV12:
		f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, m_item->_avFrame->frame->linesize[1] / 2, m_item->_avFrame->frame->height / 2, 0, GL_RG, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[1]);
		break;
	default:
		break;
	}
	f->glUniform1i(textureUniformU, 1);


	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (m_item->_avFrame->frame->format != AV_PIX_FMT_YUYV422
		&& m_item->_avFrame->frame->format != AV_PIX_FMT_NV12
		&& m_item->_avFrame->frame->format != AV_PIX_FMT_DXVA2_VLD)
	{
		f->glActiveTexture(GL_TEXTURE2);
		f->glBindTexture(GL_TEXTURE_2D, id_v);

		switch (m_item->_avFrame->frame->format)
		{
		case AV_PIX_FMT_YUV420P:
		case AV_PIX_FMT_YUVJ420P:
			f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_item->_avFrame->frame->linesize[2], m_item->_avFrame->frame->height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[2]);
			break;
		case AV_PIX_FMT_YUVJ422P:
			f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_item->_avFrame->frame->linesize[2], m_item->_avFrame->frame->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_item->_avFrame->frame->data[2]);
			break;
		default:
			break;
		}

		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		f->glUniform1i(textureUniformV, 2);

	}

	return true;
}
#ifdef D3D9
int OpenGLRenderNode::dxva2_retrieve_data(AVFrame *frame)
{
	LPDIRECT3DSURFACE9 surface = (LPDIRECT3DSURFACE9)frame->data[3];
	D3DSURFACE_DESC    surfaceDesc;
	D3DLOCKED_RECT     LockedRect;
	HRESULT            hr;

	IDirect3DSurface9_GetDesc(surface, &surfaceDesc);
	hr = IDirect3DSurface9_LockRect(surface, &LockedRect, NULL, D3DLOCK_READONLY);
	if (FAILED(hr)) {
		av_log(NULL, AV_LOG_ERROR, "Unable to lock DXVA2 surface\n");
		return -1;
	}
	else _dxvaFrameLocked = true;
	switch (surfaceDesc.Format)
	{
	case MAKEFOURCC('N', 'V', '1', '2'):
		frame->format = AV_PIX_FMT_NV12;
		break;
	default:
		printf("unknown pix format:0x%x\n", surfaceDesc.Format);
		break;
	}
	frame->data[0] = (uint8_t*)LockedRect.pBits;
	frame->data[1] = (uint8_t*)LockedRect.pBits + LockedRect.Pitch * surfaceDesc.Height;
	frame->data[2] = frame->data[1];
	frame->linesize[0] = LockedRect.Pitch;
	frame->linesize[1] = frame->linesize[0];
	frame->linesize[2] = frame->linesize[0];
#if 0
	static int pktCnt;
	pktCnt++;
	QString file;
	file.append(pktCnt);
	file.append(".nv12");

	FILE *f = fopen(file.toStdString().data(), "w+");
	if (f)
	{

		if ((fwrite(frame->data[0], LockedRect.Pitch * surfaceDesc.Height*1.5, 1, f)) < 0)
		{
			fprintf(stderr, "Failed to dump raw data.\n");
		}
		fclose(f);
	}

#endif
	return 0;
}

int OpenGLRenderNode::dxva2_free_data(AVFrame *frame)
{
	LPDIRECT3DSURFACE9 surface = (LPDIRECT3DSURFACE9)frame->data[3];
	IDirect3DSurface9_UnlockRect(surface);
	_dxvaFrameLocked = false;
	return 0;

}

#endif

#ifdef D3D11
int OpenGLRenderNode::d3d11va_retrieve_data(AVFrame * frame)
{
#if 0
	D3D11_TEXTURE2D_DESC textureDesc;
	ID3D11Texture2D * d3d11tex = (ID3D11Texture2D *)frame->data[0];

	if (d3d11tex != nullptr) {
		ID3D11Device *device;
		ID3D11DeviceContext *context;
		d3d11tex->GetDesc(&textureDesc);
		d3d11tex->GetDevice(&device);
		device->GetImmediateContext(&context);

		D3D11_MAPPED_SUBRESOURCE  mapResource;
		ZeroMemory(&mapResource, sizeof(mapResource));
		UINT index = UINT(frame->data[1]);
		HRESULT hr = context->Map(d3d11tex, 0, D3D11_MAP_READ, NULL, &mapResource);

		frame->data[3] = (uint8_t *)mapResource.pData;
		frame->data[4] = frame->data[0] + mapResource.RowPitch*frame->height;
		frame->data[5] = frame->data[1];
		context->Unmap(d3d11tex, 0);
	}
#endif
	int ret = av_hwframe_transfer_data(_pD3d11Frame, frame, 0);
	av_frame_unref(frame);
	av_frame_move_ref(frame, _pD3d11Frame);
	return ret;
}
#endif

