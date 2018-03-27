#include "muxer.h"

muxer::muxer(QObject * parent) : QObject(parent) {

}

muxer::~muxer() {

}

bool muxer::startMux(const char *filename)
{
	int have_video = 0, have_audio = 0;
	int encode_video = 0, encode_audio = 0;

	av_register_all();
	avformat_alloc_output_context2(&m_pOCtx, NULL, NULL, filename);
	if (!m_pOCtx) {
		printf("Could not deduce output format from file extension: using MPEG.\n");
		avformat_alloc_output_context2(&m_pOCtx, NULL, "mpeg", filename);
		return false;
	}
	m_pFormat = m_pOCtx->oformat;

	/* Add the audio and video streams using the default format codecs
	* and initialize the codecs. */
	if (m_pFormat->video_codec != AV_CODEC_ID_NONE) {
		add_stream(&video_st, m_pOCtx, &video_codec, m_pFormat->video_codec);
		have_video = 1;
		encode_video = 1;
	}
	if (m_pFormat->audio_codec != AV_CODEC_ID_NONE) {
		add_stream(&audio_st, m_pOCtx, &audio_codec, m_pFormat->audio_codec);
		have_audio = 1;
		encode_audio = 1;
	}
	return true;
}

void muxer::add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id)
{
	AVCodecContext *c;
	int i;

	/* find the encoder */
	*codec = avcodec_find_encoder(codec_id);
	if (!(*codec)) {
		fprintf(stderr, "Could not find encoder for '%s'\n",
			avcodec_get_name(codec_id));
		exit(1);
	}

	ost->st = avformat_new_stream(oc, NULL);
	if (!ost->st) {
		fprintf(stderr, "Could not allocate stream\n");
		exit(1);
	}
	ost->st->id = oc->nb_streams - 1;
	c = avcodec_alloc_context3(*codec);
	if (!c) {
		fprintf(stderr, "Could not alloc an encoding context\n");
		exit(1);
	}
	ost->enc = c;
	switch ((*codec)->type) {
	case AVMEDIA_TYPE_AUDIO:
		c->sample_fmt = (*codec)->sample_fmts ?
			(*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		c->bit_rate = 64000;
		c->sample_rate = 44100;
		if ((*codec)->supported_samplerates) {
			c->sample_rate = (*codec)->supported_samplerates[0];
			for (i = 0; (*codec)->supported_samplerates[i]; i++) {
				if ((*codec)->supported_samplerates[i] == 44100)
					c->sample_rate = 44100;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		c->channel_layout = AV_CH_LAYOUT_STEREO;
		if ((*codec)->channel_layouts) {
			c->channel_layout = (*codec)->channel_layouts[0];
			for (i = 0; (*codec)->channel_layouts[i]; i++) {
				if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
					c->channel_layout = AV_CH_LAYOUT_STEREO;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		ost->st->time_base.num = 1;
		ost->st->time_base.den = c->sample_rate;
		//ost->st->time_base = (AVRational) { 1, c->sample_rate };
		break;

	case AVMEDIA_TYPE_VIDEO:
		c->codec_id = codec_id;
		c->bit_rate = 400000;
		/* Resolution must be a multiple of two. */
		c->width = 352;
		c->height = 288;
		/* timebase: This is the fundamental unit of time (in seconds) in terms
												* of which frame timestamps are represented. For fixed-fps content,
												* timebase should be 1/framerate and timestamp increments should be
												* identical to 1. */
		//ost->st->time_base = (AVRational) { 1, STREAM_FRAME_RATE };
		ost->st->time_base.num = 1;
		ost->st->time_base.den = STREAM_FRAME_RATE;
		c->time_base = ost->st->time_base;
		c->gop_size = 12; /* emit one intra frame every twelve frames at most */
		c->pix_fmt = STREAM_PIX_FMT;
		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
			/* just for testing, we also add B-frames */
			c->max_b_frames = 2;
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			/* Needed to avoid using macroblocks in which some coeffs overflow.
																* This does not happen with normal video, it just happens here as
																* the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = 2;
		}
		break;
	default:
		break;
	}
	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}
