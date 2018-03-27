#include "picrenderer.h"
#include <QPainter>
#include <QPaintEngine>
#include <math.h>

#if VIDEO_RENDER_QSG_FBO

PicRenderer::PicRenderer()
    :m_bNeedUpdate(false)
	,m_pVideoFrame(NULL)
    , m_pYuv422pShaderProgram(NULL)
    , m_pYuy2ShaderProgram(NULL)
    , m_pShaderProgramMap(NULL)
    , m_pCurShaderProgram(NULL)
    , m_pVShaderMap(NULL)
    , m_currVShader(QString(""))
    , m_pFShaderMap(NULL)
    , m_currFShader(QString(""))
    , m_curVideoW(0)
    , m_curVideoH(0)
{
    for (int i = 0;i < AttribNum;i++)
    {
        Attribs[i] = i+3;
    }
    textureUniformY = 0;
    textureUniformU = 0;
    textureUniformV = 0;
    id_y = 0;
    id_u = 0;
    id_v = 0;
    //m_pVSHader = NULL;
    //m_pFSHader = NULL;
    //m_pShaderProgram = NULL;
    m_pTextureY = NULL;
    m_pTextureU = NULL;
    m_pTextureV = NULL;
}

PicRenderer::~PicRenderer()
{
}


void PicRenderer::paintQtLogo()
{
    program1.enableAttributeArray(normalAttr1);
    program1.enableAttributeArray(vertexAttr1);
    program1.setAttributeArray(vertexAttr1, vertices.constData());
    program1.setAttributeArray(normalAttr1, normals.constData());
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    program1.disableAttributeArray(normalAttr1);
    program1.disableAttributeArray(vertexAttr1);
}


void PicRenderer::initialize()
{
   /* initializeOpenGLFunctions();

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    const char *vsrc1 =
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec3 normal;\n"
        "uniform mediump mat4 matrix;\n"
        "varying mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));\n"
        "    float angle = max(dot(normal, toLight), 0.0);\n"
        "    vec3 col = vec3(0.40, 1.0, 0.0);\n"
        "    color = vec4(col * 0.2 + col * 0.8 * angle, 1.0);\n"
        "    color = clamp(color, 0.0, 1.0);\n"
        "    gl_Position = matrix * vertex;\n"
        "}\n";

    const char *fsrc1 =
        "varying mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = color;\n"
        "}\n";

    program1.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vsrc1);
    program1.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fsrc1);
    program1.link();

    vertexAttr1 = program1.attributeLocation("vertex");
    normalAttr1 = program1.attributeLocation("normal");
    matrixUniform1 = program1.uniformLocation("matrix");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    m_fAngle = 0;
    m_fScale = 1;
    createGeometry();*/
    hhtdebug("PicRenderer::initialize");

    initializeOpenGLFunctions();
    //glGenVertexArrays(VaoNum, VAOs);
    //glGenBuffers(VboNum, VBOs);
   //glDisable(GL_DEPTH_TEST);
    //glEnable(GL_DEPTH_TEST);
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
            pProgram->addShader(m_pVShaderMap->value(QString("base")));
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
    m_pTextureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureY->create();
    m_pTextureU->create();
    m_pTextureV->create();

    id_y = m_pTextureY->textureId();
    id_u = m_pTextureU->textureId();
    id_v = m_pTextureV->textureId();

}

void PicRenderer::displayRawData(VideoFrame *frame)
{
    m_pVideoFrame=frame;
}


bool PicRenderer::isNeedUpdate(void)
{
	return m_bNeedUpdate;
}

void PicRenderer::clearUpdateRequest()
{
	m_bNeedUpdate = false;
}

void PicRenderer::paintGL()
{
    if (!m_pVideoFrame)return;
#if 0
        //hhtdebug("Draw background:%d", cnt++);
        static int frameCnt;
        static int beginTime;
        static int endTime;
        if (0 == frameCnt)
        {
            beginTime = g_programTimer.elapsed();
            frameCnt++;
        }
        else if (frameCnt < 100)
            frameCnt++;
        else if (100 == frameCnt)
        {
            frameCnt = 0;
            endTime = g_programTimer.elapsed();
            if (endTime > beginTime)
            {
                float fps = 100.0 / ((endTime - beginTime) / 1000);
                hhtdebug("FPS:%f", fps);
            }
        }
#endif
/*
        static  int i;
        i++;
        unsigned char file[MAX_PATH];
        if (!(i % 100))
        {
            //memset();
            sprintf((char *)(file), "yuv%d.yuv", i / 100);
            hhtdebug("yuv file:%s", file);
            FILE *pfile = fopen((char *)(file), "w");
            fwrite(m_pBufYuy2, 1, 2592 * 1944 * 2, pfile);
            fflush(pfile);
            fclose(pfile);
        }
*/
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id_y);
        switch (m_pVideoFrame->pixFmt)
        {
        case PIX_FMT_YUYV422:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, m_pVideoFrame->width, m_pVideoFrame->height, 0, GL_RG, GL_UNSIGNED_BYTE, m_pVideoFrame->data[0]);
            break;
        case PIX_FMT_YUVJ422P:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                //m_pVideoFrame->width,
                m_pVideoFrame->lineSize[0],
                m_pVideoFrame->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pVideoFrame->data[0]);
            break;
        default:
            break;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glActiveTexture(GL_TEXTURE1);

        glBindTexture(GL_TEXTURE_2D, id_u);
        switch (m_pVideoFrame->pixFmt)
        {
        case PIX_FMT_YUYV422:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pVideoFrame->width / 2, m_pVideoFrame->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pVideoFrame->data[1]);
            break;
        case PIX_FMT_YUVJ422P:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                //m_pVideoFrame->width/2,
                m_pVideoFrame->lineSize[1],
                m_pVideoFrame->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pVideoFrame->data[1]);
            break;
        default:
            break;
        }


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glActiveTexture(GL_TEXTURE2);

        glBindTexture(GL_TEXTURE_2D, id_v);

        switch (m_pVideoFrame->pixFmt)
        {
        case PIX_FMT_YUYV422:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pVideoFrame->width / 2, m_pVideoFrame->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pVideoFrame->data[2]);
            break;
        case PIX_FMT_YUVJ422P:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                //m_pVideoFrame->width / 2,
                m_pVideoFrame->lineSize[2],
                m_pVideoFrame->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pVideoFrame->data[2]);
            break;
        default:
            break;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glUniform1i(textureUniformY, 0);
        glUniform1i(textureUniformU, 1);
        glUniform1i(textureUniformV, 2);


        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}

bool PicRenderer::videoResolChanged()
{
   static const GLfloat  vertices[VaoNum][8] =
   {
	   {
		   -1.0f, -1.0f,
			1.0f, -1.0f,
			1.0f,1.0f,
			-1.0f, 1.0f
	   },
	   {
		   0.0f, 0.0f,
		   0.98f, 0.0f,
			0.98f, 0.98f,
			0.0f, 0.98f
	   }
   };

    glVertexAttribPointer(Attribs[0], 2, GL_FLOAT, GL_FALSE, 0, vertices[0]);
    glEnableVertexAttribArray(Attribs[0]);
    glVertexAttribPointer(Attribs[1], 2, GL_FLOAT, GL_FALSE, 0, vertices[1]);
    glEnableVertexAttribArray(Attribs[1]);
    return true;
}

void PicRenderer::render()
{
    if(!m_pVideoFrame)return;
    if(!m_pVideoFrame->bIsDirty)return;
    m_pVideoFrame->bIsDirty=false;
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
    switch (m_pVideoFrame->pixFmt)
        {
        case PIX_FMT_YUYV422:
            m_pCurShaderProgram = m_pShaderProgramMap->value(QString("yuy2"));
            if (m_pCurShaderProgram)
            {
                if (!m_pCurShaderProgram->bind())
                {
                    hhtdebug("Bind program failed");
                }
                //glUseProgram();
                textureUniformY = m_pCurShaderProgram->uniformLocation("tex_y");
                textureUniformU = m_pCurShaderProgram->uniformLocation("tex_u");
                textureUniformV = m_pCurShaderProgram->uniformLocation("tex_v");

            }
            else return;
            break;
        case PIX_FMT_YUVJ422P:
            m_pCurShaderProgram = m_pShaderProgramMap->value(QString("yuv422p"));
            if (m_pCurShaderProgram)
            {
                if (!m_pCurShaderProgram->bind())
                {
                    hhtdebug("Bind program failed");
                }
                textureUniformY = m_pCurShaderProgram->uniformLocation("tex_y");
                textureUniformU = m_pCurShaderProgram->uniformLocation("tex_u");
                textureUniformV = m_pCurShaderProgram->uniformLocation("tex_v");
            }
            else return;
            break;
        default:
            break;
        }
        if (1)//m_curVideoW != m_pVideoFrame->width || m_curVideoH != m_pVideoFrame->height)
        {
            //hhtdebug("Video resolution changed");
            bool ret=videoResolChanged();
            if (!ret)
            {
                hhtdebug("Change video resolution configuration failed");
                return;
            }
            m_curVideoW = m_pVideoFrame->width;
            m_curVideoH = m_pVideoFrame->height;
        }
		//m_pCurShaderProgram->bind();
        paintGL();
        m_pCurShaderProgram->release();
		glDisableVertexAttribArray(Attribs[0]);
		glDisableVertexAttribArray(Attribs[1]);
		m_bNeedUpdate = true;
}

void PicRenderer::createGeometry()
{
    vertices.clear();
    normals.clear();

    qreal x1 = +0.06f;
    qreal y1 = -0.14f;
    qreal x2 = +0.14f;
    qreal y2 = -0.06f;
    qreal x3 = +0.08f;
    qreal y3 = +0.00f;
    qreal x4 = +0.30f;
    qreal y4 = +0.22f;

    quad(x1, y1, x2, y2, y2, x2, y1, x1);
    quad(x3, y3, x4, y4, y4, x4, y3, x3);

    extrude(x1, y1, x2, y2);
    extrude(x2, y2, y2, x2);
    extrude(y2, x2, y1, x1);
    extrude(y1, x1, x1, y1);
    extrude(x3, y3, x4, y4);
    extrude(x4, y4, y4, x4);
    extrude(y4, x4, y3, x3);

    const qreal Pi = 3.14159f;
    const int NumSectors = 100;

    for (int i = 0; i < NumSectors; ++i) {
        qreal angle1 = (i * 2 * Pi) / NumSectors;
        qreal x5 = 0.30 * sin(angle1);
        qreal y5 = 0.30 * cos(angle1);
        qreal x6 = 0.20 * sin(angle1);
        qreal y6 = 0.20 * cos(angle1);

        qreal angle2 = ((i + 1) * 2 * Pi) / NumSectors;
        qreal x7 = 0.20 * sin(angle2);
        qreal y7 = 0.20 * cos(angle2);
        qreal x8 = 0.30 * sin(angle2);
        qreal y8 = 0.30 * cos(angle2);

        quad(x5, y5, x6, y6, x7, y7, x8, y8);

        extrude(x6, y6, x7, y7);
        extrude(x8, y8, x5, y5);
    }

    for (int i = 0;i < vertices.size();i++)
        vertices[i] *= 2.0f;
}

void PicRenderer::quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4)
{
    vertices << QVector3D(x1, y1, -0.05f);
    vertices << QVector3D(x2, y2, -0.05f);
    vertices << QVector3D(x4, y4, -0.05f);

    vertices << QVector3D(x3, y3, -0.05f);
    vertices << QVector3D(x4, y4, -0.05f);
    vertices << QVector3D(x2, y2, -0.05f);

    QVector3D n = QVector3D::normal
        (QVector3D(x2 - x1, y2 - y1, 0.0f), QVector3D(x4 - x1, y4 - y1, 0.0f));

    normals << n;
    normals << n;
    normals << n;

    normals << n;
    normals << n;
    normals << n;

    vertices << QVector3D(x4, y4, 0.05f);
    vertices << QVector3D(x2, y2, 0.05f);
    vertices << QVector3D(x1, y1, 0.05f);

    vertices << QVector3D(x2, y2, 0.05f);
    vertices << QVector3D(x4, y4, 0.05f);
    vertices << QVector3D(x3, y3, 0.05f);

    n = QVector3D::normal
        (QVector3D(x2 - x4, y2 - y4, 0.0f), QVector3D(x1 - x4, y1 - y4, 0.0f));

    normals << n;
    normals << n;
    normals << n;

    normals << n;
    normals << n;
    normals << n;
}

void PicRenderer::extrude(qreal x1, qreal y1, qreal x2, qreal y2)
{
    vertices << QVector3D(x1, y1, +0.05f);
    vertices << QVector3D(x2, y2, +0.05f);
    vertices << QVector3D(x1, y1, -0.05f);

    vertices << QVector3D(x2, y2, -0.05f);
    vertices << QVector3D(x1, y1, -0.05f);
    vertices << QVector3D(x2, y2, +0.05f);

    QVector3D n = QVector3D::normal
        (QVector3D(x2 - x1, y2 - y1, 0.0f), QVector3D(0.0f, 0.0f, -0.1f));

    normals << n;
    normals << n;
    normals << n;

    normals << n;
    normals << n;
    normals << n;
}

#endif