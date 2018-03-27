#include "uvcdshowgrabber.h"
ImageCaptureThread::ImageCaptureThread(QObject *parent /*= NULL*/)
	:QThread(parent)
	, _photoPath(QString())
	, _sws_ctx(NULL)
{
	_frame = av_frame_alloc();
	/* create scaling context */
}

ImageCaptureThread::~ImageCaptureThread()
{
	av_frame_free(&_frame);
	sws_freeContext(_sws_ctx);
}

int ImageCaptureThread::saveAvframe(QString photoPath, AVFrame *frame)
{
	if (isRunning() || photoPath.isEmpty())return -1;
	av_frame_ref(_frame, frame);
	_photoPath = photoPath;
	start();
	return 0;
}

bool ImageCaptureThread::isSavingImage()
{
	return isRunning();
}

void ImageCaptureThread::run()
{
	int ret;
	uint8_t  *dst_data[4];
	SwsContext *sws_ctx;
	sws_ctx = sws_getCachedContext(_sws_ctx, _frame->width, _frame->height, AVPixelFormat(_frame->format),
		_frame->width, _frame->height, AV_PIX_FMT_RGB24,
		SWS_BILINEAR, NULL, NULL, NULL);
	if (!sws_ctx) {
		fprintf(stderr,
			"Impossible to create scale context for the conversion "
			"fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
			av_get_pix_fmt_name(AVPixelFormat(_frame->format)), _frame->width, _frame->height,
			av_get_pix_fmt_name(AV_PIX_FMT_RGB24), _frame->width, _frame->height);
		av_frame_unref(_frame);
		return;
	}
	if (_sws_ctx != sws_ctx)
	{
		sws_freeContext(_sws_ctx);
		_sws_ctx = sws_ctx;
	}
	/* buffer is going to be written to rawvideo file, no alignment */
	int linesize[4];
	if ((ret = av_image_alloc(dst_data, linesize,
		_frame->width, _frame->height, AV_PIX_FMT_RGB24, 1)) < 0) {
		fprintf(stderr, "Could not allocate destination image\n");
		av_frame_unref(_frame);
		return;

	}
	//int dst_bufsize = ret;

	sws_scale(_sws_ctx, (const unsigned char* const*)_frame->data, _frame->linesize, 0, _frame->height,
		dst_data,/* _frame->linesize*/linesize);
	QImage image(dst_data[0], _frame->width, _frame->height, QImage::Format_RGB888);
	image.save(_photoPath, "JPEG", 100);

	av_frame_unref(_frame);
	av_freep(&dst_data[0]);
	return;
}

UvcGrabber::UvcGrabber(UvcCapture *parent)
	:_pCamera(parent)
	, m_fBufferLen(0)
	, m_mediaType(NULL)
	, m_videoWidth(0)
	, m_videoHeight(0)
	//, m_pVideoFrame(new VideoFrame)
	, _pVideoDecoder(NULL)
	, _tempfile(NULL)
	, _isYuy2(false)
	, _toCatchImage(false)
	, _photoPath(QString())
	, _pRenderer(NULL)
{
	_pAvFrame = av_frame_alloc();
	AddRef();
	//char * tempfilename[] = "temp.h264";
	_tempfile = fopen("uvc.h264", "w+");
	_imageCatcher = new ImageCaptureThread();
}
UvcGrabber::~UvcGrabber() {
	if (_imageCatcher->isRunning())
		_imageCatcher->wait();
	_imageCatcher->deleteLater();
	_locker.lock();
	if (_pRenderer)
		//QMetaObject::invokeMethod(_pRenderer, "renderFrame", Q_ARG(AVFrame*, NULL));
		_pRenderer->renderFrame((AVFrame*)NULL);
	if (_pAvFrame)av_frame_free(&_pAvFrame);
	if (m_mediaType)
	{
		DeleteMediaType(m_mediaType);
		m_mediaType = NULL;
	}
	if (_pVideoDecoder)
	{
		delete _pVideoDecoder;
		_pVideoDecoder = NULL;
	}
	_locker.unlock();
}


HRESULT STDMETHODCALLTYPE UvcGrabber::QueryInterface(
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown)
	{
		*ppvObject = (void *) static_cast<ISampleGrabberCB*> (this);
		return NOERROR;
	}
	return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE UvcGrabber::AddRef(void)
{
	m_ref++;
	return ULONG(m_ref);
}
ULONG STDMETHODCALLTYPE UvcGrabber::Release(void)
{
	m_ref--;
	if (m_ref <= 0)
	{
		m_ref = 0;
		delete this;
	}
	return ULONG(m_ref);
}

HRESULT STDMETHODCALLTYPE UvcGrabber::SampleCB(double SampleTime, IMediaSample *pSample)
{
	_locker.lock();
#if 0
	static int frameCnt;
	static int beginTime;
	static int endTime;
	if (0 == frameCnt)
	{
		beginTime = g_programTimer.elapsed();
		frameCnt++;
	}
	else if (frameCnt < 100)
		frameCnt++;
	else if (100 == frameCnt)
	{
		frameCnt = 0;
		endTime = g_programTimer.elapsed();
		if (endTime > beginTime)
		{
			float fps = 100.0 / ((endTime - beginTime) / 1000);
			hhtdebug("FPS in:%f", fps);
		}
	}
#endif	
	AM_MEDIA_TYPE *pMediaType = NULL;
	HRESULT hr = pSample->GetMediaType(&pMediaType);
	if (FAILED(hr))
	{
		qDebug("Func:%s(%d):Get media type failed", __FUNCTION__, __LINE__);
	}
	else if (pMediaType)
	{
		qDebug() << "Media type change";
		setMediaType(*pMediaType);
	}
	bool isIFrame;
	hr = pSample->IsSyncPoint();
	if (hr == S_OK)
	{
		isIFrame = true;
	}
	else
	{
		isIFrame = false;
	}
	long buffLength = pSample->GetActualDataLength();
	BYTE *buff = NULL;
	hr = pSample->GetPointer(&buff);
	if (FAILED(hr))
	{
		qDebug("GetPointer failed :0x%x", hr);
	}

	//static bool bstart;
	//uint8_t typeCode = buff[4];
	//printf("type code:0x%02x\n", typeCode);
#if 0
	static int pktCnt;
	pktCnt++;
	if (_tempfile)
	{

		if ((fwrite(buff, 1, buffLength, _tempfile)) < 0)
		{
			fprintf(stderr, "Failed to dump raw data.\n");
		}
	}
	else if (pktCnt == 100 && _tempfile)
	{
		fclose(_tempfile);
	}
#endif
	if (MEDIATYPE_Video == m_mediaType->majortype)
	{
		if (MEDIASUBTYPE_MJPG == m_mediaType->subtype
			|| MEDIASUBTYPE_H264 == m_mediaType->subtype
			|| MEDIASUBTYPE_HEVC == m_mediaType->subtype
			)
		{
			if (!_pVideoDecoder)return S_OK;

			int ret = _pVideoDecoder->decode(SampleTime, buff, buffLength, isIFrame);
			if (ret)
			{
				ret = _pVideoDecoder->getAVFrame(_pAvFrame);
				if (ret < 0)
				{
					hhtdebug("Video decode failed");
				}
				if (_pRenderer)
					//QMetaObject::invokeMethod(_pRenderer, "renderFrame", Q_ARG(AVFrame*, _pAvFrame));
					_pRenderer->renderFrame(_pAvFrame);
				if (_toCatchImage)
				{
					_imageCatcher->saveAvframe(_photoPath, _pAvFrame);
					_toCatchImage = false;
				}
				av_frame_unref(_pAvFrame);
			}
		}
		else if (MEDIASUBTYPE_YUY2 == m_mediaType->subtype)
		{
			_pAvFrame->format = AV_PIX_FMT_YUYV422;
			_pAvFrame->linesize[0] = m_videoWidth;
			_pAvFrame->width = m_videoWidth;
			_pAvFrame->height = m_videoHeight;
			_pAvFrame->data[0] = buff;
			if (_pRenderer)
				//QMetaObject::invokeMethod(_pRenderer, "renderFrame", Q_ARG(AVFrame*, NULL));
				_pRenderer->renderFrame(_pAvFrame);
		}
	}
	else
	{

	}
	_locker.unlock();

	return S_OK;
}
HRESULT STDMETHODCALLTYPE UvcGrabber::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	return S_OK;
}
BOOL UvcGrabber::SaveBitmap(BYTE * pBuffer, long lBufferSize)
{
#if 0
	HANDLE hf = CreateFile(
		L"1.bmp", GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, NULL, NULL);
	if (hf == INVALID_HANDLE_VALUE)return 0;
	// 写文件头 
	BITMAPFILEHEADER bfh;
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfType = 'MB';
	bfh.bfSize = sizeof(bfh) + lBufferSize + sizeof(BITMAPINFOHEADER);
	bfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
	DWORD dwWritten = 0;
	WriteFile(hf, &bfh, sizeof(bfh), &dwWritten, NULL);
	// 写位图格式
	BITMAPINFOHEADER bih;
	memset(&bih, 0, sizeof(bih));
	bih.biSize = sizeof(bih);
	bih.biWidth = 2592;
	bih.biHeight = 1944;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	WriteFile(hf, &bih, sizeof(bih), &dwWritten, NULL);
	// 写位图数据
	WriteFile(hf, pBuffer, lBufferSize, &dwWritten, NULL);
	CloseHandle(hf);
#endif
	return 0;
}
bool UvcGrabber::setMediaType(AM_MEDIA_TYPE &mt)
{
	if (MEDIATYPE_Video != mt.majortype)
		return true;
	if (FORMAT_VideoInfo != mt.formattype)
		return true;
	if (_pRenderer)
		//QMetaObject::invokeMethod(_pRenderer, "renderFrame", Q_ARG(AVFrame*, NULL));
		_pRenderer->renderFrame((AVFrame*)NULL);

	if (m_mediaType)
	{
		DeleteMediaType(m_mediaType);
		m_mediaType = NULL;
	}
	m_mediaType = CreateMediaType(&mt);
	if (m_mediaType->cbFormat >= sizeof(VIDEOINFOHEADER))
	{
		VIDEOINFOHEADER *videoInfoHeader = reinterpret_cast<VIDEOINFOHEADER*>(m_mediaType->pbFormat);
		m_videoWidth = videoInfoHeader->bmiHeader.biWidth;
		m_videoHeight = videoInfoHeader->bmiHeader.biHeight;
	}

	if (MEDIASUBTYPE_H264 == mt.subtype
		|| MEDIASUBTYPE_MJPG == mt.subtype
		|| MEDIASUBTYPE_HEVC == mt.subtype
		)
	{
		if (!_pVideoDecoder)
		{
			_pVideoDecoder = new FFDShowDecoder();
		}
		int ret = _pVideoDecoder->init(mt);
		if (ret < 0)
		{
			hhtdebug("Init decoder failed");
		}
		if (_pRenderer)
			_pRenderer->renderModeChanged(false);
		_isYuy2 = false;
	}
	else if (MEDIASUBTYPE_YUY2 == mt.subtype)
	{
		if (_pRenderer)
			_pRenderer->renderModeChanged(true);
		_isYuy2 = true;
	}
	else {
		printf("Can't support media type\n");
		_isYuy2 = false;
	}

	return true;
}
int UvcGrabber::setRenderer(IFrameRenderer * renderer)
{
	_pRenderer = renderer;
	if (_pRenderer) {
		//QMetaObject::invokeMethod(_pRenderer, "renderModeChanged", Q_ARG(bool, false));
		//QMetaObject::invokeMethod(_pRenderer, "renderFrame", Q_ARG(AVFrame*, NULL));
		//_pRenderer->renderModeChanged(false);
		//_pRenderer->renderFrame((AVFrame*)NULL);
	}
	return 0;
}

int UvcGrabber::catchImage()
{
	_toCatchImage = true;
	return 0;
}

int UvcGrabber::catchImage(QString photoPath)
{
	_photoPath = photoPath;
	_toCatchImage = true;
	return 0;
}
