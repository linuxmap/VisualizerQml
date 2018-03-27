#pragma once
#include <QObject>
extern "C"
{
#include "libavutil\opt.h"
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libavutil\time.h"
#include "libavdevice\avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/mathematics.h"
#include "libavutil/imgutils.h"
}
#include "common.h"

#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

class muxer : public QObject {
	Q_OBJECT

public:
	typedef struct OutputStream 
	{
		AVStream *st;
		AVCodecContext *enc;
		/* pts of the next frame that will be generated */
		int64_t next_pts;
		int samples_count;
		AVFrame *frame;
		AVFrame *tmp_frame;
		float t, tincr, tincr2;
		struct SwsContext *sws_ctx;
		struct SwrContext *swr_ctx;
	} OutputStream;

public:
	muxer(QObject * parent = Q_NULLPTR);
	~muxer();
	bool startMux(const char *filename);

private:
	void add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
	AVFormatContext *m_pOCtx;
	AVOutputFormat *m_pFormat;
	OutputStream video_st = { 0 }, audio_st = { 0 };
	AVCodec *audio_codec, *video_codec;
};
