#include "fboinsgrenderer.h"
#include <QtGui/QOpenGLFramebufferObject>

#include <QtQuick/QQuickWindow>
#include <qsgsimpletexturenode.h>

#if VIDEO_RENDER_QSG_FBO
PicInFboRenderer::PicInFboRenderer()
{
	pic.initialize();
}

void PicInFboRenderer::render() 
{
	static int i;
	qDebug() << i;
	if (!m_pItem)return;
	if (!i)
	{
		connect(m_pItem, SIGNAL(textureChanged()), this, SLOT(refresh()), Qt::QueuedConnection);
	}
		i++;

	pic.displayRawData(m_pItem->m_pFrame);
	pic.render();
	QQuickWindow *w = m_pItem->window();
	if (w)
		w->resetOpenGLState();
}

void PicInFboRenderer::refresh()
{
	qDebug() << "Refresh";
	update();
}

QOpenGLFramebufferObject *PicInFboRenderer::createFramebufferObject(const QSize &size)
{
	QOpenGLFramebufferObjectFormat format;
	format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
	format.setSamples(4);
	return new QOpenGLFramebufferObject(size, format);
}

void PicInFboRenderer::synchronize(QQuickFramebufferObject *item) 
{
	hhtdebug("PicRenderer::synchronize");
	qDebug() << item;
	m_pItem = static_cast<FboInSGRenderer *>(item);
	QQuickFramebufferObject::Renderer::synchronize(item);
}


FboInSGRenderer::FboInSGRenderer(QQuickItem *parent)
	:QQuickFramebufferObject(parent)
	, m_pCamCap(NULL)
	, m_pFrame(NULL)
{
	//setAcceptHoverEvents(true);
	//setAcceptedMouseButtons(Qt::AllButtons);
	//setFlag(ItemAcceptsInputMethod, true);
	//setFlag(ItemHasContents);
	m_pCamCap = new UvcCapture(this);
}

FboInSGRenderer::~FboInSGRenderer()
{
	//if (m_pCamCap)m_pCamCap->stop();
	//dispRawData(NULL);
	//QThread::msleep(100);

	//if (m_pCamCap)
	//{
	//	delete m_pCamCap;
	//	m_pCamCap = NULL;
	//}
	m_pFrame = 0;
}

QQuickFramebufferObject::Renderer *FboInSGRenderer::createRenderer() const
{
	return new PicInFboRenderer();
}

void FboInSGRenderer::dispRawData(VideoFrame *frame)
{
	m_pFrame = frame;
	qDebug() << "textureChanged";
	emit textureChanged();
}

UvcCapture *FboInSGRenderer::cameraCapture()
{
	return m_pCamCap;
}



bool FboInSGRenderer::event(QEvent * e)
{
	//qDebug()<<e->type();
	return QQuickFramebufferObject::event(e);
}
#endif



