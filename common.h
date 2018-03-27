#pragma once
#define VIDEO_STILL_CAPTURE                   0

#define SAFE_RELEASE(x) {if(NULL != x){x->Release(); x=NULL;}}

#include <stdint.h>
#include <QDebug>

#ifdef __MAIN__
#define GLOBAL 
#else
#define GLOBAL extern
#endif

#include <stdio.h>
#include <QTime>
GLOBAL QTime g_programTimer;
#define hhtdebug(format,...) qDebug("[ %08d ] %s:%04d: " format,g_programTimer.elapsed(),__FILE__,__LINE__,##__VA_ARGS__);

#include <QMutex> 

#include "C:\Work\FFMPEG\src\compat\avisynth\avs\config.h"
#define CONFIG_AVFILTER                     0
#define CONFIG_AVDEVICE                     1
#define CONFIG_RTSP_DEMUXER					1
#define CONFIG_MMSH_PROTOCOL                1

extern "C" {
#include "libavutil/avstring.h"
#include "libavutil/eval.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavutil/time.h"
#include "libavutil/display.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"
}

#if CONFIG_AVFILTER

extern "C" {
# include "libavfilter/avfilter.h"
# include "libavfilter/buffersink.h"
# include "libavfilter/buffersrc.h"
#include "libavutil/display.h"
}
#endif
/* Common struct for handling all types of decoded data and allocated render buffers. */
typedef struct Frame {
	AVFrame *frame;
	AVSubtitle sub;
	int serial;
	double pts;           /* presentation timestamp for the frame */
	double duration;      /* estimated duration of the frame */
	int64_t pos;          /* byte position of the frame in the input file */
	int width;
	int height;
	int format;
	AVRational sar;
	int uploaded;
	int flip_v;
} Frame;

class IFrameRenderer
{
public:
	virtual int renderFrame(Frame *frame) = 0;
	virtual int renderFrame(AVFrame *frame) = 0;
	virtual int renderModeChanged(bool isYuy2)=0;
	virtual void lockFrame(void) = 0;
	virtual void unlockFrame(void) = 0;
	IFrameRenderer() {};
	~IFrameRenderer() {};
};

#define FFMPEG_HW_ACC   1
#define avstrerr(s,errno) \
{\
	char errbuf[AV_ERROR_MAX_STRING_SIZE];\
	av_strerror(errno, errbuf, sizeof(errbuf));\
	hhtdebug(s##":%s", errbuf);\
}



