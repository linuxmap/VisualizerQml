#ifndef FBOINSGRENDERER_H
#define FBOINSGRENDERER_H

#include <QtQuick/QQuickFramebufferObject>
#include "common.h"
//#include "cameracapture.h"
#include "picrenderer.h"

#if VIDEO_RENDER_QSG_FBO
class PicRenderer;
class UvcCapture;
class FboInSGRenderer;

class PicInFboRenderer : public QObject,public QQuickFramebufferObject::Renderer
{
	Q_OBJECT

public:
	PicInFboRenderer();
	QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);
	PicRenderer pic;
public slots:
	void render();
	void refresh();
protected:
	void synchronize(QQuickFramebufferObject *item) override;
private:
	FboInSGRenderer *m_pItem;

};


class FboInSGRenderer : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(UvcCapture * cameraCapture READ cameraCapture )
public:
    FboInSGRenderer(QQuickItem *parent = Q_NULLPTR);
    ~FboInSGRenderer();
    Renderer *createRenderer() const;
    void dispRawData(VideoFrame *frame);
    UvcCapture *cameraCapture();
    VideoFrame *m_pFrame;
signals:
	void textureChanged(void);
protected:
    bool event(QEvent *e) override;
private:
    UvcCapture *m_pCamCap;

};

#endif
#endif
