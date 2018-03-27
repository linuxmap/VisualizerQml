#pragma once
#include <QtCore>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
#include "libavdevice/avdevice.h"  
#include "libavfilter/avfilter.h"
#include "libavutil/error.h"
#include "libavutil/mathematics.h"  
#include "libavutil/time.h"  
#include "inttypes.h"
#include "stdint.h"
};
#include "common.h"

class FFDemuxer:public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString url READ url WRITE setUrl)
	Q_PROPERTY(bool isRunning READ isRunning)

public:
	FFDemuxer(QObject *parent=Q_NULLPTR);
	~FFDemuxer();
	QString url();
	int setUrl(const QString url);
	bool isRunning(void);
	Q_INVOKABLE int openUrl(const QString url);
	Q_INVOKABLE int close();

	int videoStreamIdx();
	int audioStreamIdx();
	AVFormatContext *formatContext();

	int readFrame(AVPacket *pkt);

private:
	QString m_url;
	bool m_bIsRunning;
	AVFormatContext * m_pFC;
	AVInputFormat* m_pFormat;
	
	//video param
	int m_vStreamIdx;
	int m_width;
	int m_height;
	double m_frameRate;                                                    
	AVCodecID m_vCodecID;
	AVPixelFormat m_pixelfromat;
	char spspps[100];
	int spspps_size;

	//audio param
	int m_aStreamIdx;
	int m_aChannelCount; 
	int m_aBitsPerSample; 
	int m_aFrequency;     
	AVCodecID m_aCodecID;
	int m_aFrameSize;

};

class FFDecoder :public QObject
{
	Q_OBJECT
public:
	FFDecoder(QObject *parent = Q_NULLPTR, AVFormatContext * fc=NULL);
	~FFDecoder();
	int init(AVFormatContext * fc,int streamIdx);
	int decode(AVPacket *pkt, AVFrame *frame);
	AVCodec *decoder(void);
private:
	AVFormatContext * m_pFC;
	AVCodecContext *m_pCodecCtx;
	AVCodec *m_pCodec;
};




#define avstrerr(s,errno) \
{\
	char errbuf[AV_ERROR_MAX_STRING_SIZE];\
	av_strerror(errno, errbuf, sizeof(errbuf));\
	hhtdebug(s##":%s", errbuf);\
}
