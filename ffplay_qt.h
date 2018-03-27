#pragma once
#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <QtCore>
#include "common.h"
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
#include "libavutil/hwcontext.h"

}
//#include "cameracapture.h"

#undef main
#pragma region type define

#define THREAD_QTHREAD                   1
//#define THREAD_SDL_THREAD                0
//#define RENDER_SDL                        
#define RENDER_OPENGL                       1
//#define FECH_VIDEO_PACKET                  1
class Ffplay;
const char program_name[] = "ffplay";
const int program_birth_year = 2017;

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* Step size for volume control in dB */
#define SDL_VOLUME_STEP (0.75)

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#define CURSOR_HIDE_DELAY 1000000

#define USE_ONEPASS_SUBTITLE_RENDER 1

static unsigned sws_flags = SWS_BICUBIC;

typedef struct MyAVPacketList {
	AVPacket pkt;
	struct MyAVPacketList *next;
	int serial;
} MyAVPacketList;

typedef struct PacketQueue {
	MyAVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	int64_t duration;
	int abort_request;
	int serial;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

#define VIDEO_PICTURE_QUEUE_SIZE 9
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 10
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

typedef struct AudioParams {
	int freq;
	int channels;
	int64_t channel_layout;
	enum AVSampleFormat fmt;
	int frame_size;
	int bytes_per_sec;
} AudioParams;

typedef struct Clock {
	double pts;           /* clock base */
	double pts_drift;     /* clock base minus time at which we updated the clock */
	double last_updated;
	double speed;
	int serial;           /* clock is based on a packet with this serial */
	int paused;
	int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;





typedef struct FrameQueue {
	Frame queue[FRAME_QUEUE_SIZE];
	int rindex;
	int windex;
	int size;
	int max_size;
	int keep_last;
	int rindex_shown;
	SDL_mutex *mutex;
	SDL_cond *cond;
	PacketQueue *pktq;
} FrameQueue;

enum {
	AV_SYNC_AUDIO_MASTER, /* default choice */
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

typedef struct Decoder {
	AVPacket pkt;
	PacketQueue *queue;
	AVCodecContext *avctx;
	int pkt_serial;
	int finished;
	int packet_pending;
	SDL_cond *empty_queue_cond;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
#ifdef THREAD_QTHREAD
	QThread *decodeThread;
#else
	SDL_Thread *decoder_tid;
#endif
} Decoder;
class AudioCbThread;

typedef struct VideoState {
#ifdef THREAD_QTHREAD
	QThread *readThread;
#else
	SDL_Thread *read_tid;
#endif
	AVInputFormat *iformat;
	int abort_request;
	int force_refresh;
	int paused;
	int last_paused;
	int queue_attachments_req;
	int seek_req;
	int seek_flags;
	int64_t seek_pos;
	int64_t seek_rel;
	int read_pause_return;
	AVFormatContext *ic;
	int realtime;

	Clock audclk;
	Clock vidclk;
	Clock extclk;

	FrameQueue pictq;
	FrameQueue subpq;
	FrameQueue sampq;

	Decoder auddec;
	Decoder viddec;
	Decoder subdec;

	int audio_stream;

	int av_sync_type;

	double audio_clock;
	int audio_clock_serial;
	double audio_diff_cum; /* used for AV difference average computation */
	double audio_diff_avg_coef;
	double audio_diff_threshold;
	int audio_diff_avg_count;
	AVStream *audio_st;
	PacketQueue audioq;
	int audio_hw_buf_size;
	uint8_t *audio_buf;
	uint8_t *audio_buf1;
	unsigned int audio_buf_size; /* in bytes */
	unsigned int audio_buf1_size;
	int audio_buf_index; /* in bytes */
	int audio_write_buf_size;
	int audio_volume;
	int muted;
	struct AudioParams audio_src;
#if CONFIG_AVFILTER
	struct AudioParams audio_filter_src;
#endif
	struct AudioParams audio_tgt;
	bool audio_is_ready;
	AudioCbThread *audioCbThread;
	struct SwrContext *swr_ctx;
	int frame_drops_early;
	int frame_drops_late;

	enum ShowMode {
		SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT, SHOW_MODE_NB
	} show_mode;
	int16_t sample_array[SAMPLE_ARRAY_SIZE];
	int sample_array_index;
	int last_i_start;
	RDFTContext *rdft;
	int rdft_bits;
	FFTSample *rdft_data;
	int xpos;
	double last_vis_time;
	SDL_Texture *vis_texture;
	SDL_Texture *sub_texture;
	SDL_Texture *vid_texture;

	int subtitle_stream;
	AVStream *subtitle_st;
	PacketQueue subtitleq;

	double frame_timer;
	double frame_last_returned_time;
	double frame_last_filter_delay;
	int video_stream;
	AVStream *video_st;
	PacketQueue videoq;
	double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
	struct SwsContext *img_convert_ctx;
	struct SwsContext *sub_convert_ctx;
	int eof;

	char *filename;
	int width, height, xleft, ytop;
	int step;

#if CONFIG_AVFILTER
	int vfilter_idx;
	AVFilterContext *in_video_filter;   // the first filter in the video chain
	AVFilterContext *out_video_filter;  // the last filter in the video chain
	AVFilterContext *in_audio_filter;   // the first filter in the audio chain
	AVFilterContext *out_audio_filter;  // the last filter in the audio chain
	AVFilterGraph *agraph;              // audio filter graph
#endif
	int last_video_stream, last_audio_stream, last_subtitle_stream;
	SDL_cond *continue_read_thread;
	Ffplay *ffplay;
	bool eventThreadRun;
	SDL_AudioSpec audioSpec;
} VideoState;
#pragma  endregion



class FfplayReadThread :public QThread
{
	Q_OBJECT

public:
	FfplayReadThread(Ffplay *f,VideoState *arg);
	~FfplayReadThread();
	void run()override;
private:
	VideoState *is;
	Ffplay *ffplay;
#ifdef FECH_VIDEO_PACKET
	FILE *_tempfile;
#endif

};

class FfplayAudioThread :public QThread
{
	Q_OBJECT

public:
	FfplayAudioThread(Ffplay *f, VideoState *arg);
	~FfplayAudioThread();
	void run()override;
private:
	VideoState *is;
	Ffplay *ffplay;
};

class FfplayVideoThread :public QThread
{
	Q_OBJECT

public:
	FfplayVideoThread(Ffplay *f, VideoState *arg);
	~FfplayVideoThread();
	void run()override;
private:
	VideoState *is;
	Ffplay *ffplay;
};

class FfplaySubtitleThread :public QThread
{
	Q_OBJECT

public:
	FfplaySubtitleThread(Ffplay *f, VideoState *arg);
	~FfplaySubtitleThread();
	void run()override;
private:
	VideoState *is;
	Ffplay *ffplay;
};

class AudioCbThread :public QThread
{
public:
	AudioCbThread(VideoState *vs);
	int mixAudio(Uint8 *stream, int len);
	void run();
	~AudioCbThread();
private:
	VideoState *_is;
	Uint8 *_stream;
	int _len;
};
class FfplayEventThread :public QThread
{
public:
	FfplayEventThread(Ffplay *f,QObject *parent=nullptr);
	~FfplayEventThread();
	void run()override;
private:
	Ffplay * _ffplay;
};
class Ffplay :public QObject
{
	Q_OBJECT
	Q_CLASSINFO("Version", "1.0")
	Q_CLASSINFO("Author", "Ophelia")
	Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
	Ffplay(QObject *parent=nullptr);
	~Ffplay();

	QString source() { return _source; };
	int setSource(const QString &s);
	Q_INVOKABLE  int play();
	Q_INVOKABLE void stop(void);
	int setRenderer(IFrameRenderer *renderer);
	void togglePause(void);
	void toggleMute(void);
	void updateVolume(int sign, double step);
	void step_to_next_frame(void);
	double getVideoFrameRate(void);
	QString vcodec(void);
	void setVCodec(QString c);

	void stopEventThread(void);
	QStringList findCameras(void);
	friend class FfplayReadThread;
	friend class FfplayAudioThread;
	friend class FfplayVideoThread;
	friend class FfplaySubtitleThread;
	friend class AudioCbThread;
	friend class FfplayEventThread;
signals:
	void sigStreamOpened();
	void sigStreamClosed();
	void sourceChanged();
private:
	void deInit();
	void init_opts(void);
	void uninit_opts(void);
	int playInEventThread();
	int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec);
	AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
		AVFormatContext *s, AVStream *st, AVCodec *codec);
	int64_t parse_time_or_die(const char *context, const char *timestr,
		int is_duration);
	double parse_number_or_die(const char *context, const char *numstr, int type,
		double min, double max);
	AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
		AVDictionary *codec_opts);

#if CONFIG_AVFILTER
	//int opt_add_vfilter(void *optctx, const char *opt, const char *arg);
#endif
	int cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1,
		enum AVSampleFormat fmt2, int64_t channel_count2);
	int64_t get_valid_channel_layout(int64_t channel_layout, int channels);
	int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);
	int packet_queue_put(PacketQueue *q, AVPacket *pkt);
	int packet_queue_put_nullpacket(PacketQueue *q, int stream_index);
	int packet_queue_init(PacketQueue *q);
	void packet_queue_flush(PacketQueue *q);
	void packet_queue_destroy(PacketQueue *q);
	void packet_queue_abort(PacketQueue *q);
	void packet_queue_start(PacketQueue *q);
	int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial);
	void decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond);
	int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub);
	void decoder_destroy(Decoder *d);
	void frame_queue_unref_item(Frame *vp);
	int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last);
	void frame_queue_destory(FrameQueue *f);
	void frame_queue_signal(FrameQueue *f);
	Frame *frame_queue_peek(FrameQueue *f);
	Frame *frame_queue_peek_next(FrameQueue *f);
	Frame *frame_queue_peek_last(FrameQueue *f);
	Frame *frame_queue_peek_writable(FrameQueue *f);
	Frame *frame_queue_peek_readable(FrameQueue *f);
	void frame_queue_push(FrameQueue *f);
	void frame_queue_next(FrameQueue *f);
	int frame_queue_nb_remaining(FrameQueue *f);
	int64_t frame_queue_last_pos(FrameQueue *f);
	void decoder_abort(Decoder *d, FrameQueue *fq);
	inline void fill_rectangle(int x, int y, int w, int h);
	int realloc_texture(SDL_Texture **texture, Uint32 new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture);
	void calculate_display_rect(SDL_Rect *rect,
		int scr_xleft, int scr_ytop, int scr_width, int scr_height,
		int pic_width, int pic_height, AVRational pic_sar);
	int upload_texture(SDL_Texture *tex, AVFrame *frame, struct SwsContext **img_convert_ctx);
	void video_image_display(VideoState *is);
	inline int compute_mod(int a, int b);
	void video_audio_display(VideoState *s);
	void stream_component_close(VideoState *is, int stream_index);
	void stream_close(void);
	void do_exit(void);
	void sigterm_handler(int sig);
	void set_default_window_size(int width, int height, AVRational sar);
	int video_open(VideoState *is);
	void video_display(VideoState *is);
	double get_clock(Clock *c);
	void set_clock_at(Clock *c, double pts, int serial, double time);
	void set_clock(Clock *c, double pts, int serial);
	void set_clock_speed(Clock *c, double speed);
	void init_clock(Clock *c, int *queue_serial);
	void sync_clock_to_slave(Clock *c, Clock *slave);
	int get_master_sync_type(VideoState *is);
	double get_master_clock(VideoState *is);
	void check_external_clock_speed(VideoState *is);
	void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes);
	void stream_toggle_pause(VideoState *is);
	double compute_target_delay(double delay, VideoState *is);
	double vp_duration(VideoState *is, Frame *vp, Frame *nextvp);
	void update_video_pts(VideoState *is, double pts, int64_t pos, int serial);
	void video_refresh(void *opaque, double *remaining_time);
	int queue_picture(VideoState *is, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial);
	int get_video_frame(VideoState *is, AVFrame *frame);
#if CONFIG_AVFILTER
	int configure_filtergraph(AVFilterGraph *graph, const char *filtergraph,
		AVFilterContext *source_ctx, AVFilterContext *sink_ctx);
	int configure_video_filters(AVFilterGraph *graph, VideoState *is, const char *vfilters, AVFrame *frame);
	int configure_audio_filters(VideoState *is, const char *afilters, int force_output_format);
#endif  /* CONFIG_AVFILTER */

	int decoder_start(Decoder *d, AVMediaType type, void *arg);
	void update_sample_display(VideoState *is, short *samples, int samples_size);
	int synchronize_audio(VideoState *is, int nb_samples);
	int audio_decode_frame(VideoState *is);
	static void sdl_audio_callback(void *opaque, Uint8 *stream, int len);
	int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params);
	int stream_component_open(VideoState *is, int stream_index);
	static int decode_interrupt_cb(void *ctx);
	int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue);
	int is_realtime(AVFormatContext *s);

	VideoState *stream_open(QString filename, AVInputFormat *iformat);
	void stream_cycle_channel(VideoState *is, int codec_type);
	void toggle_full_screen(VideoState *is);
	void toggle_audio_display(VideoState *is);
	void refresh_loop_wait_event(VideoState *is, SDL_Event *event);
	void seek_chapter(VideoState *is, int incr);
	void event_loop(VideoState *cur_stream);
	int opt_frame_size(void *optctx, const char *opt, const char *arg);
	int opt_width(void *optctx, const char *opt, const char *arg);
	int opt_height(void *optctx, const char *opt, const char *arg);
	int opt_format(void *optctx, const char *opt, const char *arg);
	int opt_frame_pix_fmt(void *optctx, const char *opt, const char *arg);
	int opt_sync(void *optctx, const char *opt, const char *arg);
	int opt_seek(void *optctx, const char *opt, const char *arg);
	int opt_duration(void *optctx, const char *opt, const char *arg);
	int opt_show_mode(void *optctx, const char *opt, const char *arg);
	static int lockmgr(void **mtx, enum AVLockOp op);
	void setVideo(bool enable);
	void setAudio(bool enable);
	void setSubtitle(bool enable);
	void setStartupVolume(int vol);
	void setVolume(int vol);
	void setShowStatus(int show);
	void setFast(bool f);
	void setGenpts(bool g);
	void setDecoderReorderPts(int val);
	void setSync(int val);
	void setFrameDrop(bool e);
	void setInfiniteBuffer(bool e);
	double get_rotation(AVStream *st);
#ifdef FFMPEG_HW_ACC
	enum AVPixelFormat find_fmt_by_hw_type(const enum AVHWDeviceType type);
	int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type);
	static int dxva2_retrieve_data(AVCodecContext *s, AVFrame *frame);
#endif
private:
	FfplayEventThread *_pEventThread;
	QString _source;
	typedef struct FfplayInstances {
		FfplayInstances() 
		{
			audioClientCnt = 0;
		}
		QList<Ffplay *> ffplayList;
		int audioClientCnt;
	}FfplayInstances;
	static FfplayInstances *_instance;
	VideoState *_is;
	IFrameRenderer* _renderer;
	 AVInputFormat *file_iformat;
	 const char *window_title;
	 int default_width;
	 int default_height;
	 int screen_width;
	 int screen_height;
	 int audio_disable;
	 int video_disable;
	 int subtitle_disable;
	 const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {"v","a",0,"s",0};
	 int seek_by_bytes;
	 int display_disable;
	 int borderless;
	 int startup_volume;
	 int show_status ;
	 int av_sync_type;
	 int64_t start_time;
	 int64_t duration;
	 int fast;
	 int genpts;
	 int lowres;
	 int decoder_reorder_pts;
	 int autoexit;
	 int exit_on_keydown;
	 int exit_on_mousedown;
	 int loop;
	 int framedrop;
	 int infinite_buffer;
	 enum VideoState::ShowMode show_mode;
	 QString audio_codec_name;
	 QString subtitle_codec_name;
	 QString video_codec_name;
	 double rdftspeed;
	 int64_t cursor_last_shown;
	 int cursor_hidden;
#if CONFIG_AVFILTER
	 const char **vfilters_list = NULL;
	 int nb_vfilters = 0;
	 char *afilters = NULL;
#endif
	 int autorotate;
	/* current context */
	 int is_full_screen;
	 int64_t audio_callback_time;
	 AVPacket flush_pkt;
#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)
	 SDL_Window *window;
	 SDL_Renderer *renderer;
	 AVDictionary *format_opts, *codec_opts, *resample_opts;
	 AVDictionary *sws_dict;
	 AVDictionary *swr_opts;
#ifdef FFMPEG_HW_ACC
	 bool _hwAcc;
	 enum AVHWDeviceType _accType;
	 AVBufferRef *_hwDeviceCtx;
	 static enum AVPixelFormat hw_pix_fmt;
	 static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

#endif
};


