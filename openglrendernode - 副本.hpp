#pragma once
#include <qsgrendernode.h>
#include <QtOpenGL/QtOpenGL>
#include <QQuickItem>
#include "common.h"

#define FFPLAY
#ifdef FFPLAY
//#include "ffplay_qt.h"
#endif
#if VIDEO_RENDER_QSG_NODE
QT_BEGIN_NAMESPACE

class QQuickItem;
class QOpenGLShaderProgram;
class QOpenGLBuffer;

QT_END_NAMESPACE
#define  D3D9       1
#define	  D3D11       1
class OpenGLRenderNode : public QSGRenderNode, public IFrameRenderer
{

public:
	OpenGLRenderNode(QQuickItem * parent = Q_NULLPTR);
	~OpenGLRenderNode();
#ifdef FFPLAY
	int renderFrame(Frame *frame);
	int renderFrame(AVFrame *frame);
	int renderModeChanged(bool isYuy2);
	void lockFrame(void);
	void unlockFrame(void);
	int refreshFrame(void);
	void setSelfUpdate(bool e);
	bool getSelfUpdate(void);
#endif
	void render(const RenderState *state) override;
	StateFlags changedStates() const override;
	RenderingFlags flags() const override;
	QRectF rect() const override;

private:
	bool init(void);
	void paintGL(void);
	bool videoResolChanged(void);
	bool formatChanged(void);
#ifdef D3D9
	int dxva2_retrieve_data(AVFrame *frame);
	int dxva2_free_data(AVFrame *frame);
#endif
#ifdef D3D11
	int d3d11va_retrieve_data(AVFrame *frame);
	AVFrame *_pD3d11Frame;
#endif
	bool _selfUpdate;
	Frame *_avFrame;
	int _oldFrameWidth;
	int _oldFrameHeight;
	int _oldItemWidth;
	int _oldItemHeight;
	QMutex _frameLock;

	QQuickItem *m_item;
	QOpenGLShaderProgram *m_program = nullptr;
	QOpenGLBuffer *m_vbo = nullptr;
	bool m_bNeedUpdate;
	enum VAO_IDs { Rectangle, Texture, VaoNum };
	enum VBO_IDs { RectBuffer, TexBuffer, VboNum };
	enum Attrib_IDs { RectAttrib = 3, TexAttrib = 4, AttribNum = 2 };

	qreal   m_fAngle;
	qreal   m_fScale;

	bool m_bInit;
	int m_lastColorSpace;
	GLfloat  _vertices[VaoNum][8];

	GLuint textureUniformY; 
	GLuint textureUniformU; 
	GLuint textureUniformV; 
	int m_matrixUniform;
	int m_opacityUniform;
	GLuint id_y; 
	GLuint id_u; 
	GLuint id_v; 
	QOpenGLTexture* m_pTextureY;  
	QOpenGLTexture* m_pTextureU;  
	QOpenGLTexture* m_pTextureV;  
	QOpenGLShaderProgram *m_pYuv422pShaderProgram; 
	QOpenGLShaderProgram *m_pYuy2ShaderProgram; 
	QMap<QString, QOpenGLShaderProgram*>  *m_pShaderProgramMap;
	QOpenGLShaderProgram* m_pCurShaderProgram;
	QMap<QString, QOpenGLShader*>  *m_pVShaderMap;
	QString m_currVShader;
	QMap<QString, QOpenGLShader*>  *m_pFShaderMap;
	QString m_currFShader;

	int m_curVideoW;
	int m_curVideoH;
	GLuint VAOs[VaoNum];
	GLuint VBOs[VboNum];
	GLuint Attribs[AttribNum];
	QOpenGLDebugLogger *m_pLogger;

	int m_frameCnt;
	int m_beginTime;
	int m_endTime;
	bool _dxvaFrameLocked;
	bool _isYuy2;
};
#endif
