#include <QQuickWindow>
#include <QSGRendererInterface>
#include "videoitem.hpp"
#if VIDEO_RENDER_QSG_NODE
#ifdef FFPLAY
//FfplayThread::FfplayThread(Ffplay *ffplay, QObject *parent/*=nullptr*/)
//	:_f(ffplay)
//	, QThread(parent)
//{
//
//}
//
//FfplayThread::~FfplayThread()
//{
//
//}
//void FfplayThread::run()
//{
//	_f->play();
//	qDebug() << "FfplayThread exit" ;
//}
//
//void FfplayThread::stop(void)
//{
//	_f->stopRefreshThread();
//}

#else
FFmpegThread::FFmpegThread()
	:m_bStop(NULL)
	, m_pDemuxer(NULL)
	, m_pPacket(NULL)
	, m_pViDecoder(NULL)
	, m_pAuDecoder(NULL)
	, m_viStreamIdx(-1)
	, m_auStreamIdx(-1)
	, m_pViFrame(NULL)
	, m_pAuFrame(NULL)
	, m_pDispItem(NULL)
	, m_pVideoFrame(new VideoFrame)
	, m_audioOutput(NULL)
{
	m_pVideoFrame->lock = new QMutex;
}

FFmpegThread::~FFmpegThread()
{
	if (isRunning())
	{
		close();
	}
	if (m_pVideoFrame)
	{
		QMutex *lock = m_pVideoFrame->lock;
		lock->lock();
		delete m_pVideoFrame;
		m_pVideoFrame = NULL;
		lock->unlock();
		delete lock;
	}
	if(m_pViFrame)av_frame_free(&m_pViFrame);
	if (m_pAuFrame)av_frame_free(&m_pAuFrame);
	if (m_pPacket)av_packet_free(&m_pPacket);
}

int FFmpegThread::openUrl(const QString url)
{
#ifdef FFPLAY

#else
	if (!m_pDemuxer)
	{
		m_pDemuxer = new FFDemuxer();
	}
	int ret = m_pDemuxer->openUrl(url);
	if (ret < 0)
	{
		hhtdebug("Open url failed");
		//qDebug << "open " << url.data() << " failed";
		return -1;
	}
	m_formatCtx = m_pDemuxer->formatContext();
	m_viStreamIdx = m_pDemuxer->videoStreamIdx();
	m_auStreamIdx = m_pDemuxer->audioStreamIdx();
	m_pPacket = av_packet_alloc();
	if (m_viStreamIdx >= 0)m_pViFrame = av_frame_alloc();
	if (m_auStreamIdx >= 0)
	{
		m_pAuFrame = av_frame_alloc();
	}
#endif
	start();
	return 0;
}

int FFmpegThread::close()
{
	if (isRunning())
	{
		m_bStop = true;
		if (wait()) qDebug() << "FFmpeg thread quit!";
	}
#ifdef FFPLAY
#else
	if (m_pViDecoder)
	{
		delete m_pViDecoder;
		m_pViDecoder = NULL;
	}
	if (m_pAuDecoder)
	{
		delete m_pAuDecoder;
		m_pAuDecoder = NULL;
	}
	if (m_pDemuxer)
	{
		delete m_pDemuxer;
		m_pDemuxer = NULL;
	}
#endif
	return 0;
}

void FFmpegThread::run()
{
#ifdef FFPLAY
	//char *
	//const char *argv[2] = {"ffplay",""};
	//ffplay(2,argv);
#else
	int ret;
	int cnt=0;
	while (!m_bStop)
	{
		ret = m_pDemuxer->readFrame(m_pPacket);
		if (ret < 0)
		{
			avstrerr("readFrame failed", ret);
			cnt++;
			if (cnt > 10)
			{
				cnt = 0;
			}
			msleep(33);
			continue;
		}


		if (m_pPacket->stream_index == m_viStreamIdx)
		{
			/*static int k;
			hhtdebug("video frame:%d",k++);*/
			if (!m_pViDecoder)
			{
				m_pViDecoder = new FFDShowDecoder();
				m_pViDecoder->init(m_formatCtx, m_viStreamIdx);
			}
			m_pVideoFrame->lock->lock();
			ret = m_pViDecoder->decode(m_pPacket, m_pViFrame);
			m_pVideoFrame->lock->unlock();
			if (ret < 0)
			{
				av_packet_unref(m_pPacket);
				avstrerr("Video decode failed", ret);
				continue;
			}
			

			av_packet_unref(m_pPacket);
			viPlay(m_pViFrame);

		}
		else if (0)//m_pPacket->stream_index == m_auStreamIdx)
		{
			//av_packet_unref(m_pPacket);
			//continue;
			if (!m_pAuDecoder)
			{
				m_pAuDecoder = new FFDShowDecoder();
				m_pAuDecoder->init(m_formatCtx, m_auStreamIdx);
				//AVCodec *decoder = m_pAuDecoder->decoder();


			}

			ret = m_pAuDecoder->decode(m_pPacket, m_pAuFrame);
			if (ret < 0)
			{
				avstrerr("Audio decode failed", ret);
				av_packet_unref(m_pPacket);
				printf("1");

				continue;
			}
			if (!m_audioOutput)
			{
				hhtdebug("m_audioOutput init");
				m_audioOutput = new AudioOutput(NULL);
				m_audioOutput->setSampleRate(m_pAuFrame->sample_rate);
				m_audioOutput->setChannelCount(m_pAuFrame->channels);
				switch (m_pAuFrame->format)
				{
				case AV_SAMPLE_FMT_U8:         ///< unsigned 8 bits
					m_audioOutput->setSampleSize(8);
					m_audioOutput->setSampleType(QAudioFormat::UnSignedInt);
					break;
				case AV_SAMPLE_FMT_S16:         ///< signed 16 bits
					m_audioOutput->setSampleSize(16);
					m_audioOutput->setSampleType(QAudioFormat::SignedInt);

					break;
				case AV_SAMPLE_FMT_S32:         ///< signed 32 bits
					m_audioOutput->setSampleSize(32);
					m_audioOutput->setSampleType(QAudioFormat::SignedInt);

					break;
				case AV_SAMPLE_FMT_FLT:         ///< float
					m_audioOutput->setSampleSize(32);
					m_audioOutput->setSampleType(QAudioFormat::Float);

					break;
				case AV_SAMPLE_FMT_DBL:         ///< double
					m_audioOutput->setSampleSize(64);
					m_audioOutput->setSampleType(QAudioFormat::Float);

					break;
				case AV_SAMPLE_FMT_U8P:         ///< unsigned 8 bits, planar
					m_audioOutput->setSampleSize(8);
					m_audioOutput->setSampleType(QAudioFormat::UnSignedInt);

					break;
				case AV_SAMPLE_FMT_S16P:        ///< signed 16 bits, planar
					m_audioOutput->setSampleSize(8);
					m_audioOutput->setSampleType(QAudioFormat::UnSignedInt);

					break;
				case AV_SAMPLE_FMT_S32P:        ///< signed 32 bits, planar
					m_audioOutput->setSampleSize(8);
					m_audioOutput->setSampleType(QAudioFormat::UnSignedInt);

					break;
				case AV_SAMPLE_FMT_FLTP:        ///< float, planar
					m_audioOutput->setSampleSize(32);
					m_audioOutput->setSampleType(QAudioFormat::Float);

					break;
				case AV_SAMPLE_FMT_DBLP:        ///< double, planar
					m_audioOutput->setSampleSize(8);
					m_audioOutput->setSampleType(QAudioFormat::UnSignedInt);

					break;
				case AV_SAMPLE_FMT_S64:         ///< signed 64 bits
					m_audioOutput->setSampleSize(8);
					m_audioOutput->setSampleType(QAudioFormat::UnSignedInt);

					break;
				case AV_SAMPLE_FMT_S64P:        ///< signed 64 bits, planar
					m_audioOutput->setSampleSize(8);
					m_audioOutput->setSampleType(QAudioFormat::UnSignedInt);
					break;
				default:
					break;
				}
				m_audioOutput->setCodec("audio/pcm");
				m_audioOutput->setByteOrder(QAudioFormat::LittleEndian);
				m_audioOutput->initializeAudio();
			}
			//viPlay(m_pAuFrame);
			auPlay(m_pAuFrame);
			av_packet_unref(m_pPacket);

		}
		else
		{
			//hhtdebug("Unknow stream index,something must wrong!");
		}
	}
#endif
}

void FFmpegThread::setDispItem(VideoItem * item)
{
	m_pDispItem = item;
}

int FFmpegThread::viPlay(AVFrame * frame)
{
	m_pVideoFrame->lock->lock();
	switch (frame->format)
	{
	case AV_PIX_FMT_YUV420P:
		m_pVideoFrame->pixFmt = PixelFormat(frame->format);
		memcpy(m_pVideoFrame->data, frame->data, DATA_POINTERS_NUM * sizeof(frame->data[0]));
		memcpy(m_pVideoFrame->lineSize, frame->linesize, DATA_POINTERS_NUM * sizeof(int));
		m_pVideoFrame->width = frame->width;
		m_pVideoFrame->height = frame->height;
		m_pVideoFrame->bIsDirty = true;

		if (m_pDispItem)
			m_pDispItem->dispRawData(m_pVideoFrame);

		break;
	case AV_PIX_FMT_YUVJ422P:
		m_pVideoFrame->pixFmt = PixelFormat(frame->format);
		memcpy(m_pVideoFrame->data, frame->data, DATA_POINTERS_NUM * sizeof(frame->data[0]));
		memcpy(m_pVideoFrame->lineSize, frame->linesize, DATA_POINTERS_NUM * sizeof(int));
		m_pVideoFrame->width = frame->width;
		m_pVideoFrame->height = frame->height;
		m_pVideoFrame->bIsDirty = true;

		if (m_pDispItem)
			m_pDispItem->dispRawData(m_pVideoFrame);
	default:
		m_pVideoFrame->lock->unlock();
		return -1;
	}
	m_pVideoFrame->lock->unlock();

	return 0;
	}

int FFmpegThread::auPlay(AVFrame *frame)
{
	quint64 ret;
	int size = av_get_bytes_per_sample(AVSampleFormat(frame->format));

	//for (int i = 0; i < frame->nb_samples;i++ )
	{
		for (int j = 0; j < frame->channels; j++)
		{
			ret = m_audioOutput->writeData(j, (char *)(frame->data[j]), size*frame->nb_samples);
			//if (ret != size*frame->nb_samples)
			//{
			//	hhtdebug("writing audio data failed,c:%d,b:%d", j, ret);
			//}
		}
	}
	return 0;
}
#endif
QList<VideoItem*> *VideoItem::_videoItems = new QList<VideoItem*>;
VideoItem::VideoItem(QQuickItem * parent)
	: QQuickItem(parent)
	, m_pRenderNode(NULL)
	, m_source(QString(""))
	, m_bIsPlaying(false)
	, _isVideoDirty(false)
#ifdef FFPLAY
	, _avFrame(NULL)
//	, _ffplayThread(NULL)
	//, _ffplay(new Ffplay(this))
	, _videoFrameRate(0.0)
#else
	, m_pFFmpegThread(NULL)
#endif
{
	setFlag(ItemHasContents);
	//m_pCamCap = new CCameraCapture(this);
	_videoItems->append(this);
	//connect(_ffplay, SIGNAL(sigStreamOpened()), this, SLOT(sltStreamOpened()));
	//connect(_ffplay, SIGNAL(sigStreamClosed()), this, SLOT(sltStreamClosed()));
}

VideoItem::~VideoItem() 
{
	//stop();
	//if (_ffplayThread) 
	//{
	//	if (_ffplayThread->isRunning())_ffplayThread->wait();
	//	delete _ffplayThread;
	//	_ffplayThread = NULL;
	//}
	//if (_ffplay)
	//{
	//	delete _ffplay;
	//	_ffplay = NULL;
	//}
}

#if 0
Q_INVOKABLE int VideoItem::play(void)
{
#ifdef FFPLAY
	if (!_ffplayThread)
	{
		_ffplayThread = new FfplayThread(_ffplay);
		connect(_ffplayThread, SIGNAL(finished()), this, SIGNAL(playingStateChanged()));
	}
	else
	{
		if (_ffplayThread->isRunning())
		{
			hhtdebug("Playing,stop play first!");
			return -1;
		}
	}
	_ffplayThread->start();

	return 0;
#else
	if (!m_pFFmpegThread)m_pFFmpegThread = new FFmpegThread();
	m_pFFmpegThread->setDispItem(this);
	m_bRenderReady = true;
	int ret= m_pFFmpegThread->openUrl(m_source);
	if (ret >= 0)
	{
		m_bIsPlaying = true;
		return 0;
	}
	else return -1;
#endif

}


Q_INVOKABLE int VideoItem::stop(void)
{
	m_bRenderReady = false;
#ifdef FFPLAY
	if (_ffplayThread)
	{
		if(_ffplayThread->isRunning())
		_ffplayThread->stop();
	}
#else
	if (m_pFFmpegThread)
	{
		delete m_pFFmpegThread;
		m_pFFmpegThread = NULL;
	}

	m_bIsPlaying = false;
	m_pFrame = NULL;
	if (m_pRenderNode) {
		m_pRenderNode->displayRawData(m_pFrame);
	}
#endif
	return 0;
}

Q_INVOKABLE int VideoItem::togglePause(void)
{
	if (_ffplay&&_ffplayThread&&_ffplayThread->isRunning())
	{
		_ffplay->togglePause();
		return 0;
	}
	else
	{
		return -1;
	}
}

Q_INVOKABLE int VideoItem::toggleMute(void)
{
	if (_ffplay&&_ffplayThread&&_ffplayThread->isRunning())
	{
		_ffplay->toggleMute();
		return 0;
	}
	else
	{
		return -1;
	}
}

Q_INVOKABLE int VideoItem::updateVolume(int sign, double step)
{
	if (_ffplay&&_ffplayThread&&_ffplayThread->isRunning())
	{
		_ffplay->updateVolume(sign,step);
		return 0;
	}
	else
	{
		return -1;
	}
}

Q_INVOKABLE int VideoItem::stepPlay(void)
{
	if (_ffplay&&_ffplayThread&&_ffplayThread->isRunning())
	{
		_ffplay->step_to_next_frame();
		return 0;
	}
	else
	{
		return -1;
	}
}
#endif


void VideoItem::updateVideo()
{
	_isVideoDirty = true;
	emit textureChanged();
}

bool VideoItem::selfUpdate(void)
{
	if (m_pRenderNode)
	{
		return m_pRenderNode->getSelfUpdate();
	}
	else
		return true;
}

void VideoItem::setSelfUpdate(bool e)
{
	if (m_pRenderNode)
	{
		return m_pRenderNode->setSelfUpdate(e);
	}
}

QSGNode * VideoItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *nodeData)
{
	QSGRenderNode *n = static_cast<QSGRenderNode *>(node);
	if (!n) {
		QSGRendererInterface *ri = window()->rendererInterface();
		if (!ri)
			return nullptr;
		switch (ri->graphicsApi()) {
		case QSGRendererInterface::OpenGL:
#if QT_CONFIG(opengl)
			if (!m_pRenderNode&&m_bRenderReady)
			{
				m_pRenderNode = new OpenGLRenderNode(this);
				emit renderNodeChanged();
				//_ffplay->setRenderer(m_pRenderNode);
				if (_source.data())dynamic_cast<IVideoProducer*>(_source.data())->setRenderer(m_pRenderNode);
				connect(this, SIGNAL(textureChanged()), this, SLOT(update()), Qt::QueuedConnection);

			}
			else hhtdebug("Error:m_pRenderNode should not allocate again");
			n = m_pRenderNode;
			break;
#endif
		case QSGRendererInterface::Direct3D12:
#if QT_CONFIG(d3d12)
			n = new D3D12RenderNode(this);
			break;
#endif
		//case QSGRendererInterface::Software:
		//	n = new SoftwareRenderNode(this);
		//	break;
		default:
			return nullptr;
		}
	}

	if(m_pRenderNode&&_isVideoDirty)
	{
		nodeData->transformNode->markDirty(QSGNode::DirtyMaterial);

		_isVideoDirty = false;
	}
	return n;

}


double VideoItem::videoFrameRate(void)
{
	return _videoFrameRate;
}

int VideoItem::setSource(QObject*  source)
{
	if (source == _source.data())return -1;
	_source=source;
	if (source)dynamic_cast<IVideoProducer*>(source)->setRenderer(m_pRenderNode);
	return 0;
}

void VideoItem::setScale(qreal scale)
{
	QQuickItem::setScale(scale);
	hhtdebug("emit scaleChanged");
	emit scaleChanged(scale);
}

#if 0
bool VideoItem::isPlaying(void)
{
	if (_ffplayThread)
	{
		return _ffplayThread->isRunning();
	}
	else
		return false;
}

QString VideoItem::vcodec(void)
{
	return _ffplay->vcodec();
}

void VideoItem::setVCodec(const QString c)
{
	_ffplay->setVCodec(c);
}
#endif

OpenGLRenderNode * VideoItem::renderNode()
{
	return m_pRenderNode;
}


void VideoItem::sltStreamOpened(void)
{
	//_videoFrameRate =_ffplay->getVideoFrameRate();
	emit videoFrameRateChanged();
	if (_videoFrameRate == 0)return;
	double frameRateMax=0;
	int  maxFrameRateIndex;
	for (int i = 0; i < _videoItems->count(); i++)
	{
		double frameRate = (*_videoItems)[i]->videoFrameRate();
		if (frameRate > frameRateMax)
		{
			frameRateMax = frameRate;
			maxFrameRateIndex = i;
		}
	}
	for (int i = 0; i < _videoItems->count(); i++)
	{
		if (i == maxFrameRateIndex)(*_videoItems)[i]->setSelfUpdate(true);
		else (*_videoItems)[i]->setSelfUpdate(false);
	}

}

void VideoItem::sltStreamClosed(void)
{
	//_videoFrameRate = _ffplay->getVideoFrameRate();
	emit videoFrameRateChanged();
	//if (_videoFrameRate == 0)return;
	double frameRateMax=0.0;
	int  maxFrameRateIndex;
	qDebug() << "frameRate: ";
	for (int i = 0; i < _videoItems->count(); i++)
	{
		double frameRate = (*_videoItems)[i]->videoFrameRate();
		if (frameRate > frameRateMax)
		{
			frameRateMax = frameRate;
			maxFrameRateIndex = i;
		}
		qDebug() << frameRate<<"; ";
	}
	for (int i = 0; i < _videoItems->count(); i++)
	{
		if (i == maxFrameRateIndex)(*_videoItems)[i]->setSelfUpdate(true);
		else (*_videoItems)[i]->setSelfUpdate(false);
	}

}


bool VideoItem::event(QEvent *e)
{
	//qDebug() << e->type();
	//switch (e->type())
	//{

	//default:break;
	//}
	return QQuickItem::event(e);
}
#endif



