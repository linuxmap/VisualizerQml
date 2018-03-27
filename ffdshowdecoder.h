#pragma once
#include "common.h"
extern "C"{
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
#include "libavutil/hwcontext.h"
}

#include "ffinputstream.h"
#include "ffhwaccelerator.h"
typedef struct PixelFormatTag {
	enum AVPixelFormat pix_fmt;
	unsigned int fourcc;
} PixelFormatTag;


class FFDShowDecoder:public FFObject
{
public:
	explicit FFDShowDecoder(QObject *parent=nullptr);
	~FFDShowDecoder();
	static enum AVPixelFormat get_format(AVCodecContext *s, const enum AVPixelFormat *pix_fmts);
	static int get_buffer(AVCodecContext *s, AVFrame *frame, int flags);
	int init(AM_MEDIA_TYPE type);
	void changeResol(long width,long height);
	bool decode(unsigned char *buff,int buffLen);
	bool decode(double SampleTime,unsigned char *buff, int buffLen);
	bool decode(double SampleTime, unsigned char *buff, int buffLen, bool isIFrame);
	int getAVFrame(AVFrame *frame);
#ifdef FFMPEG_HW_ACC
	static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);
#endif
private:
	void deInit();
	int  printCtxOption();
	//enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);
	int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type);
	enum AVPixelFormat find_fmt_by_hw_type(const enum AVHWDeviceType type);
	static enum AVPixelFormat dshow_pixfmt(DWORD biCompression, WORD biBitCount);
	static enum AVPixelFormat avpriv_find_pix_fmt(const PixelFormatTag *tags, unsigned int fourcc);
#if 0
	void avpriv_set_pts_info(AVStream *s, int pts_wrap_bits, unsigned int pts_num, unsigned int pts_den);
#endif
private:
	enum dshowDeviceType {
		VideoDevice = 0,
		AudioDevice = 1,
	};
	FFInputStream *_ist;
	FFHWAccelerator *_hwAccelerator;
	AVCodec *_pDecoder;
	AVCodecContext * _pDecoderCtx;
	AVFrame *m_pFrame;
	AVPacket *m_pPacket;
	int m_vWidth;
	int m_vHeight;
	bool m_bHwAcc;
	/*HWAccelID*/AVHWDeviceType m_accType;
	AVBufferRef *m_pHwDeviceCtx;
#ifdef FFMPEG_HW_ACC
	static enum AVPixelFormat hw_pix_fmt;
#endif
	AVFormatContext *_pFmtCtx;
	//AVBitStreamFilterContext* _h264bsfc;
};



