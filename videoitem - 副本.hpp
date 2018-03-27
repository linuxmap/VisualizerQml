#pragma once
#include <QQuickItem>
#include "common.h"
//#include "ffdemuxingdecoding.h"
#if QT_CONFIG(opengl)
#include "openglrendernode.hpp"
#endif
//#include "audiooutput.h"
#if QT_CONFIG(d3d12)
#include "d3d12renderer.h"
#endif
#ifdef FFPLAY
#include "ffplay_qt.h"
#endif


#if VIDEO_RENDER_QSG_NODE
class VideoItem;
#ifdef FFPLAY
#if 0
class FfplayThread :public QThread
{
	Q_OBJECT
public:
	FfplayThread(Ffplay *ffplay,QObject *parent=nullptr);
	~FfplayThread();
	void run()override;
	void stop(void);
private:
	bool m_bStop;
	VideoItem *m_pDispItem;
	Ffplay *_f;

};
#endif
#else
class FFmpegThread :public QThread
{
	Q_OBJECT
public:
	FFmpegThread();
	~FFmpegThread();
	int openUrl(const QString url);
	int close();
	void run()override;
	void setDispItem(VideoItem *item);
private:
	int viPlay(AVFrame *frame);
	int auPlay(AVFrame *frame);
	bool m_bStop;
	FFDemuxer *m_pDemuxer;
	AVFormatContext *m_formatCtx;
	int m_viStreamIdx;
	int m_auStreamIdx;

	AVPacket *m_pPacket;
	AVFrame *m_pViFrame;
	AVFrame *m_pAuFrame;
	FFDShowDecoder *m_pViDecoder;
	FFDShowDecoder  *m_pAuDecoder;
	VideoItem *m_pDispItem;
	VideoFrame *m_pVideoFrame;
	AudioOutput *m_audioOutput;

};
#endif

class VideoItem : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(QObject* source READ source WRITE setSource)
	Q_PROPERTY(double videoFrameRate READ videoFrameRate NOTIFY videoFrameRateChanged)
	//Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playingStateChanged)
	Q_PROPERTY(bool selfUpdate READ selfUpdate WRITE setSelfUpdate)
	//Q_PROPERTY(QString vcodec READ vcodec WRITE setVCodec)
	Q_PROPERTY(OpenGLRenderNode * renderNode READ renderNode NOTIFY renderNodeChanged)
public:
	VideoItem(QQuickItem * parent = Q_NULLPTR);
	~VideoItem();
	//Q_INVOKABLE int play(void);
	//Q_INVOKABLE int stop(void);
	//Q_INVOKABLE int togglePause(void);
	//Q_INVOKABLE int toggleMute(void);
	//Q_INVOKABLE int updateVolume(int sign, double step);
	//Q_INVOKABLE int stepPlay(void);
#ifdef FFPLAY
	void updateVideo();
	bool selfUpdate(void);
	void setSelfUpdate(bool e);
#endif
	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *nodeData) override;
	double videoFrameRate(void);
	QObject*  source(void) { return _source.data(); }
	int setSource(QObject* source);
	void setScale(qreal);
	//bool isPlaying(void);
	//QString vcodec(void);
	//void setVCodec(const QString c);
	OpenGLRenderNode * renderNode();
signals:
	void playingStateChanged();
	void textureChanged(void);
	void scaleChanged(qreal scale);
	void rotationChanged(qreal angle);
	void xChanged(qreal x);
	void yChanged(qreal y);
	void videoFrameRateChanged(void);
	void renderNodeChanged();

private slots :
	void sltStreamOpened(void);
	void sltStreamClosed(void);
protected:
	bool event(QEvent *e)override;
private:
	static QList<VideoItem*> *_videoItems;
	bool m_bRenderReady;
	bool m_bIsPlaying;
	OpenGLRenderNode *m_pRenderNode;
	QString m_source;
	bool _isVideoDirty;
#ifdef FFPLAY
	//FfplayThread *_ffplayThread;
#else
	FFmpegThread *m_pFFmpegThread;
#endif
#ifdef FFPLAY
	//Ffplay *_ffplay;
	Frame *_avFrame;
	double _videoFrameRate;
#endif
	QPointer<QObject> _source;
};

#endif

