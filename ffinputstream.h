#pragma once
#include <QtCore>
#include <QQmlEngine>
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/fifo.h"
#include "libavfilter/avfilter.h"
}
#include "ffobject.h"

typedef struct InputFilter {
	AVFilterContext    *filter;
	struct InputStream *ist;
	struct FilterGraph *graph;
	uint8_t            *name;
	enum AVMediaType    type;   // AVMEDIA_TYPE_SUBTITLE for sub2video
	AVFifoBuffer *frame_queue;
	// parameters configured for this input
	int format;
	int width, height;
	AVRational sample_aspect_ratio;
	int sample_rate;
	int channels;
	uint64_t channel_layout;
	AVBufferRef *hw_frames_ctx;
	int eof;
} InputFilter;

class FFHWAccelerator;
class FFInputStream :public FFObject
{
	Q_OBJECT
public:
	FFInputStream(QObject *parent = nullptr);
	~FFInputStream();

	int _fileIndex;
	AVStream *_st;
	int _discard;             /* true if stream data should be discarded */
	int _userSetDiscard;
	int _decodingNeeded;     /* non zero if the packets must be decoded in 'raw_fifo', see DECODING_FOR_* */
#define DECODING_FOR_OST    1
#define DECODING_FOR_FILTER 2
	AVCodecContext *_decCtx;
	AVCodec *_dec;
	AVFrame *_decodedFrame;
	AVFrame *_filterFrame; /* a ref of decoded_frame, to be sent to filters */
	int64_t       _start;     /* time when read started */
							 /* predicted dts of the next packet read for this stream or (when there are
							 * several frames in a packet) of the next frame in current packet (in AV_TIME_BASE units) */
	int64_t       _nextDts;
	int64_t       _dts;       ///< dts of the last packet read for this stream (in AV_TIME_BASE units)
	int64_t       _nextPts;  ///< synthetic pts for the next decode frame (in AV_TIME_BASE units)
	int64_t       _pts;       ///< current pts of the decoded frame  (in AV_TIME_BASE units)
	int           _wrapCorrectionDone;
	int64_t _filterInRescaleDeltaLast;
	int64_t _minPts; /* pts with the smallest value in a current stream */
	int64_t _maxPts; /* pts with the higher value in a current stream */
					 // when forcing constant input framerate through -r,
					 // this contains the pts that will be given to the next decoded frame
	int64_t _cfrNextPts;
	int64_t _nbSamples; /* number of samples in the last decoded audio frame before looping */
	double _tsScale;
	int _sawFirstTs;
	AVDictionary *_decoderOpts;
	AVRational _framerate;               /* framerate forced with -r */
	int _topFieldFirst;
	int _guessLayoutMax;
	int _autoRotate;
	int _fixSubDuration;
	struct { /* previous decoded subtitle and related variables */
		int got_output;
		int ret;
		AVSubtitle subtitle;
	} _prevSub;
	struct sub2video {
		int64_t last_pts;
		int64_t end_pts;
		AVFifoBuffer *sub_queue;    ///< queue of AVSubtitle* before filter init
		AVFrame *frame;
		int w, h;
	} _sub2video;
	int _dr1;
	/* decoded data from this stream goes into all those filters
	* currently video and audio only */
	InputFilter **_filters;
	int        _nbFilters;
	int _reinitFilters;
	/* hwaccel options */
	enum HWAccelID _hwaccelId;
	char  *_hwaccelDevice;
	QString _qsvDevice;
	enum AVPixelFormat _hwaccelOutputFormat;
	/* hwaccel context */
	enum HWAccelID _activeHwaccelId;
	void  *_hwaccelCtx;
	AVBufferRef *_hwDeviceCtx;
	void(*_hwaccelUninit)(AVCodecContext *s);
	int(*_hwaccelGetBuffer)(AVCodecContext *s, AVFrame *frame, int flags);
	int(*_hwaccelRetrieveData)(AVCodecContext *s, AVFrame *frame);
	enum AVPixelFormat _hwaccelPixFmt;
	enum AVPixelFormat _hwaccelRetrievedPixFmt;
	AVBufferRef *_hwFramesCtx;
	/* stats */
	// combined size of all the packets read
	uint64_t _dataSize;
	/* number of packets successfully read for this stream */
	uint64_t _nbPackets;
	// number of frames/samples retrieved from the decoder
	uint64_t _framesDecoded;
	uint64_t _samplesDecoded;
	int64_t *_dtsBuffer;
	int _nbDtsBuffer;
	int _gotOutput;
};
QML_DECLARE_TYPE(FFInputStream)
