#include "ffdemuxingdecoding.h"

FFDemuxer::FFDemuxer(QObject *parent)
:QObject(parent)
,m_url(QString(""))
, m_bIsRunning(false)
,m_pFC(NULL)
,m_pFormat(NULL)
,m_vStreamIdx(-1)
,m_width(0)
,m_height(0)
,m_frameRate(0)
,m_vCodecID(AV_CODEC_ID_NONE)
,m_pixelfromat(AV_PIX_FMT_NONE)
,spspps("")
,spspps_size(0)
,m_aStreamIdx(-1)
,m_aChannelCount(0)
,m_aBitsPerSample(0)
,m_aFrequency(0)
,m_aCodecID(AV_CODEC_ID_NONE)
,m_aFrameSize(0)
{
	av_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);

}


FFDemuxer::~FFDemuxer()
{
	if(isRunning())close();
	if (m_pFormat)
	{

	}
}

QString FFDemuxer::url()
{
	return m_url;
}

int FFDemuxer::setUrl(const QString url)
{
	m_url = url;
	return true;
}

bool FFDemuxer::isRunning(void)
{
	return m_bIsRunning;
}

Q_INVOKABLE int FFDemuxer::openUrl(const QString url)
{
	int ret;
	AVDictionary *format_opts=NULL;
	int scan_all_pmts_set = 0;
	qDebug() << "Open url : " << url;

	if (url.isEmpty())
	{
		hhtdebug("url is empty");
		return -1;
	}

	m_url = url;
	QByteArray baUrl = url.toLatin1();
	char *cUrl = baUrl.data();

	if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
		av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
		scan_all_pmts_set = 1;
	}
	ret = avformat_open_input(&m_pFC, cUrl, m_pFormat, &format_opts);
	if (ret != 0)
	{
		avstrerr("Call avformat_open_input function failed", ret);
		return -1;
	}
	if (scan_all_pmts_set)
		av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);
	av_dict_free(&format_opts);

	av_format_inject_global_side_data(m_pFC);

	if (ret=avformat_find_stream_info(m_pFC,NULL) < 0)
	{
		avformat_close_input(&m_pFC);
		avstrerr("Call av_find_stream_info function failed",ret);
		return -1;
	}

	av_dump_format(m_pFC, -1, cUrl, 0);
	for (unsigned int i = 0; i < m_pFC->nb_streams; i++)
	{
		if (m_pFC->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_vStreamIdx = i;
			m_width = m_pFC->streams[i]->codec->width;
			m_height = m_pFC->streams[i]->codec->height;
			m_frameRate = av_q2d(m_pFC->streams[i]->r_frame_rate);
			m_vCodecID = m_pFC->streams[i]->codec->codec_id;
			m_pixelfromat = m_pFC->streams[i]->codec->pix_fmt;
			spspps_size = m_pFC->streams[i]->codec->extradata_size;
			memcpy(spspps, m_pFC->streams[i]->codec->extradata, spspps_size);
		}
		else if (m_pFC->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_aStreamIdx = i;
			m_aChannelCount = m_pFC->streams[i]->codec->channels;
			switch (m_pFC->streams[i]->codec->sample_fmt)
			{
			case AV_SAMPLE_FMT_U8:
				m_aBitsPerSample = 8;
				break;
			case AV_SAMPLE_FMT_S16:
				m_aBitsPerSample = 16;
				break;
			case AV_SAMPLE_FMT_S32:
				m_aBitsPerSample = 32;
				break;
			default:
				break;
			}
			m_aFrequency = m_pFC->streams[i]->codec->sample_rate;
			m_aCodecID = m_pFC->streams[i]->codec->codec_id;
			m_aFrameSize = m_pFC->streams[i]->codec->frame_size;
		}
	}
	if (m_vStreamIdx < 0 && m_aStreamIdx < 0)
	{
		avformat_close_input(&m_pFC);
		avstrerr("Call av_find_stream_info function failed", ret);
		return -1;
	}
	m_bIsRunning = true;
	return 0;


}

Q_INVOKABLE int FFDemuxer::close()
{
	if (!m_bIsRunning)return 0;
	avformat_close_input(&m_pFC);
	m_vStreamIdx = -1;
	m_aStreamIdx = -1;
	m_url.clear();
	m_bIsRunning = false;
	return 0;
}

int FFDemuxer::videoStreamIdx()
{
	return m_vStreamIdx;
}

int FFDemuxer::audioStreamIdx()
{
	return m_aStreamIdx;
}

AVFormatContext * FFDemuxer::formatContext()
{
	return m_pFC;
}

int FFDemuxer::readFrame(AVPacket *pkt)
{
	if (!pkt)
	{
		pkt = av_packet_alloc();
		if (NULL == pkt)
		{
			hhtdebug("av_packet_alloc failed");
			return -1;
		}
	}
	if (!m_pFC)
	{
		hhtdebug("Open the url first,please!");
		return -1;
	}
	return av_read_frame(m_pFC, pkt);
}


FFDecoder::FFDecoder(QObject *parent /*= Q_NULLPTR*/, AVFormatContext * fc/*=NULL*/)
	:QObject(parent)
	, m_pFC(fc)
	, m_pCodecCtx(NULL)
	,m_pCodec(NULL)
{
	av_register_all();
}

FFDecoder::~FFDecoder()
{
	//if(m_pCodecCtx)avcodec_free_context(&m_pCodecCtx);
}

int FFDecoder::init(AVFormatContext * fc, int streamIdx)
{
	if (streamIdx < 0 || streamIdx >= (int)fc->nb_streams)
		return -1;
	m_pCodecCtx = fc->streams[streamIdx]->codec;

	/* prepare audio output */
	if (m_pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
	{
#if 0
		if (enc->channels > 0)
			enc->request_channels = FFMIN(2, enc->channels);
		else
			enc->request_channels = 2;
#endif

		/*Hardcoding the codec id to PCM_MULAW if the audio
		codec id returned by the lib is CODEC_ID_NONE */

#if 0
		if (enc->codec_id == AV_CODEC_ID_NONE)
		{
			enc->codec_id = CODEC_ID_PCM_MULAW;
			enc->channels = 1;
			enc->sample_rate = 16000;
			enc->bit_rate = 128;
		}
#endif
	}

	m_pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	m_pCodecCtx->idct_algo = FF_IDCT_AUTO;
	m_pCodecCtx->flags2 |= CODEC_FLAG2_FAST;
	m_pCodecCtx->skip_frame = AVDISCARD_DEFAULT;
	m_pCodecCtx->skip_idct = AVDISCARD_DEFAULT;
	m_pCodecCtx->skip_loop_filter = AVDISCARD_DEFAULT;
	m_pCodecCtx->error_concealment = 3;

	av_opt_set_int(m_pCodecCtx, "refcounted_frames", 1, 0);

	if (!m_pCodec || avcodec_open2(m_pCodecCtx, m_pCodec,NULL) < 0)
		return -1;
#if 0
	avcodec_thread_init(enc, 1);
	enc->thread_count = 1;
	fc->streams[streamIdx]->discard = AVDISCARD_DEFAULT;
#endif

	return 0;
}
int FFDecoder::decode(AVPacket *pkt, AVFrame *frame)
{
	int ret = avcodec_send_packet(m_pCodecCtx, pkt);
	if (ret < 0) {
		avstrerr("avcodec_send_packet", ret);
		return ret;
	}
	ret = avcodec_receive_frame(m_pCodecCtx, frame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
		avstrerr("avcodec_receive_frame",ret);
		return ret;
	}
	else if (ret < 0) {
		avstrerr("avcodec_receive_frame", ret);
		return ret;
	}
	static int i;
	if (!i)
	{
		i++;
		printf("Format after decode is:%d\n", frame->format);
	}
	return 0;
}

AVCodec * FFDecoder::decoder(void)
{
	return m_pCodec;
}
