#pragma once
#include <QQuickItem>
#include "common.h"
#if QT_CONFIG(opengl)
#include "openglrendernode.hpp"
#endif
#if QT_CONFIG(d3d12)
#include "d3d12renderer.h"
#endif
#include "ffplay_qt.h"
#define  D3D9       1
#define	  D3D11       1
class DisplayItem;

class OpenGLRenderNode : public QQuickFramebufferObject::Renderer
{

public:
	OpenGLRenderNode();
	~OpenGLRenderNode();
	void render() Q_DECL_OVERRIDE;
	void synchronize(QQuickFramebufferObject *item) Q_DECL_OVERRIDE;
	QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) Q_DECL_OVERRIDE;
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
	DisplayItem *m_item;
	QOpenGLShaderProgram *m_program = nullptr;
	QOpenGLBuffer *m_vbo = nullptr;
	enum VAO_IDs { Rectangle, Texture, VaoNum };
	enum VBO_IDs { RectBuffer, TexBuffer, VboNum };
	enum Attrib_IDs { RectAttrib = 3, TexAttrib = 4, AttribNum = 2 };

	bool m_bInit;
	int m_lastColorSpace;
	GLfloat  _vertices[VaoNum][8];

	GLuint textureUniformY;
	GLuint textureUniformU;
	GLuint textureUniformV;
	int m_opacityUniform;
	GLuint id_y;
	GLuint id_u;
	GLuint id_v;
	QOpenGLTexture* m_pTextureY;
	QOpenGLTexture* m_pTextureU;
	QOpenGLTexture* m_pTextureV;

	QOpenGLShaderProgram* m_pCurShaderProgram;
	GLuint VAOs[VaoNum];
	GLuint VBOs[VboNum];
	GLuint Attribs[AttribNum];
	QMap<QString, QOpenGLShaderProgram*>  *m_pShaderProgramMap;
	QMap<QString, QOpenGLShader*>  *m_pVShaderMap;
	QMap<QString, QOpenGLShader*>  *m_pFShaderMap;

	int m_frameCnt;
	int m_beginTime;
	int m_endTime;
	bool _dxvaFrameLocked;
	int _oldFrameWidth;
	int _oldFrameHeight;
};

class DisplayItem : public QQuickFramebufferObject, public IFrameRenderer
{
	Q_OBJECT
	Q_PROPERTY(double videoFrameRate READ videoFrameRate NOTIFY videoFrameRateChanged)
	Q_PROPERTY(IFrameRenderer *renderer READ renderer NOTIFY rendererChanged)
		Q_PROPERTY(QVector3D eye READ eye WRITE setEye)
		Q_PROPERTY(QVector3D target READ target WRITE setTarget)
		Q_PROPERTY(QVector3D rotation READ rotation WRITE setRotation)

public:
	DisplayItem(QQuickItem * parent = Q_NULLPTR);
	~DisplayItem();
	Q_INVOKABLE int renderFrame(Frame *frame);
	Q_INVOKABLE int renderFrame(AVFrame *frame);
	Q_INVOKABLE int renderModeChanged(bool isYuy2);
	void lockFrame(void);
	void unlockFrame(void);
	QQuickFramebufferObject::Renderer *createRenderer() const Q_DECL_OVERRIDE;
	double videoFrameRate(void);
	IFrameRenderer * renderer() { return this; };
	void setScale(qreal);
	QVector3D eye() const { return m_eye; }
	void setEye(const QVector3D &v);
	QVector3D target() const { return m_target; }
	void setTarget(const QVector3D &v);

	QVector3D rotation() const { return m_rotation; }
	void setRotation(const QVector3D &v);

	enum SyncState {
		CameraNeedsSync = 0x01,
		RotationNeedsSync = 0x02,
		AllNeedsSync = 0xFF
	};
	int swapSyncState();
	friend class OpenGLRenderNode;
signals:
	void playingStateChanged();
	void scaleChanged(qreal scale);
	void rotationChanged(qreal angle);
	void xChanged(qreal x);
	void yChanged(qreal y);
	void videoFrameRateChanged(void);
	void rendererChanged();
private slots :
	void sltStreamOpened(void);
	void sltStreamClosed(void);
protected:
	bool event(QEvent *e)override;
private:
	bool m_bRenderReady;
	bool m_bIsPlaying;
	bool _isVideoDirty;
	double _videoFrameRate;
	QPointer<QObject> _source;
	bool _isYuy2;
	Frame *_avFrame;
	QMutex _frameLock;
	QVector3D m_eye;
	QVector3D m_target;
	QVector3D m_rotation;
	int m_syncState;

};
