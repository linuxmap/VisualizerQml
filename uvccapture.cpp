#include "uvccapture.h"
#include <QThread>

UvcCapture::UvcCapture(QObject *parent)
	:QObject(parent)
	, _source(-1)
	, _displayItem(NULL)
	, _currentColorSpace(-1)
	, m_pVideoGrabberFilter(NULL)
	, _pNullFilter(NULL)
	, _imageUrl("temp.png")
	, _pUvcControler(new UvcControler(this, this))
	, m_videoGrabberCb(new UvcGrabber(this))
	, _photoDirectory(QUrl())
	, m_bMediaRun(false)
{
	m_cameraNum = 0;
	m_capGraphBuilder2 = NULL;
	m_graphBuilder = NULL;
	m_mediaCtrl = NULL;
	m_srcFilter = NULL;
	_pCamMonik = NULL;

	m_mapColorRes.clear();
	m_mapResolution.clear();
	_resolutions.clear();
	emit resolutionsChanged();
	_colorSpaces.clear();
	emit colorSpacesChanged();
	m_mediaType = new AM_MEDIA_TYPE;
	emit imageCaptureChanged();
	CoInitialize(NULL);
	//_imageCatcher = new ImageCaptureThread();
}

UvcCapture::~UvcCapture()
{
	releaseComInstance();
	CoUninitialize();
	//if(_imageCatcher->isRunning())
	//	_imageCatcher->wait();
	//_imageCatcher->deleteLater();
}

int UvcCapture::setSource(const int index)
{
	return changeCamera(index);
}

int UvcCapture::setDisplayItem(IFrameRenderer* displayItem)
{
	_displayItem = reinterpret_cast<IFrameRenderer*>(displayItem);
	if (m_videoGrabberCb)m_videoGrabberCb->setRenderer(_displayItem);
	return 0;
}

void UvcCapture::setCurrentColorSpace(int cs)
{
	changeCorlorSpace(cs);
}

void UvcCapture::setCurrentResolution(int index)
{
	changeResolution(index);
}


void UvcCapture::setPhotoDirectory(QUrl url)
{
	_photoDirectory = url;
}

bool UvcCapture::play()
{
	return InitGraphFilters();
}

bool UvcCapture::stop()
{
	if (NULL != m_mediaCtrl)
	{
		OAFilterState state = State_Stopped;//State_Running;
		m_mediaCtrl->GetState(100, &state);

		if (state != State_Stopped)
		{
			HRESULT hr = S_OK;
			hr = m_mediaCtrl->Stop();
			m_bMediaRun = false;
			stateChanged();
		}

		return true;
	}
	else
		return false;
}

int UvcCapture::changeCamera(int index)
{
	UvcUacEnumerator *pEnumrator = UvcUacEnumerator::getInstance();
	if (index == _source || index >= pEnumrator->getCamNum())return -1;
	stop();
	SAFE_RELEASE(m_videoGrabberCb);
	emit imageCaptureChanged();
	//releaseComInstance();
	m_mapResolution.clear();
	_resolutions.clear();
	m_mapColorRes.clear();
	_colorSpaces.clear();
	_currentColorSpace = -1;
	SAFE_RELEASE(m_capGraphBuilder2);
	SAFE_RELEASE(m_graphBuilder);
	SAFE_RELEASE(m_mediaCtrl);
	SAFE_RELEASE(m_srcFilter);
	SAFE_RELEASE(_pNullFilter);
	SAFE_RELEASE(m_pVideoGrabberFilter);

	_pCamMonik = camMonik(index);
	buildGraphFilters(_pCamMonik);
	_source = index;
	emit sourceChanged();
	emit colorSpacesChanged();
	emit currentColorSpaceChanged();
	emit resolutionsChanged();
	emit currentResolutionChanged();
	return 0;

}

int UvcCapture::changeCorlorSpace(int index)
{
	if (index < 0 || index >= m_mapColorRes.size() || index == _currentColorSpace)return -1;
	stop();
	SAFE_RELEASE(m_videoGrabberCb);
	emit imageCaptureChanged();

	//releaseComInstance();
	m_mapResolution.clear();
	QMap< QString, QMap<int, int> >::const_iterator i = m_mapColorRes.constBegin();
	m_mapResolution = (i + index).value();
	m_currentResWidth = m_mapResolution.lastKey ();
	m_currentResHeight = m_mapResolution.value(m_currentResWidth);
	m_currentColorSpace = (i + index).key();
	_resolutions.clear();
	QMap<int, int>::const_iterator j = m_mapResolution.constBegin();
	while (j != m_mapResolution.constEnd()) {
		QString s;
		s.clear();
		s.append(QString::number(j.key()));
		s.append('x');
		s.append(QString::number(j.value()));
		_resolutions.append(s);
		j++;
	}
	emit resolutionsChanged();
	//m_mapColorRes.clear();
	//_colorSpaces.clear();
	//emit colorSpacesChanged();
	SAFE_RELEASE(m_capGraphBuilder2);
	SAFE_RELEASE(m_graphBuilder);
	SAFE_RELEASE(m_mediaCtrl);
	SAFE_RELEASE(m_srcFilter);
	SAFE_RELEASE(_pNullFilter);
	SAFE_RELEASE(m_pVideoGrabberFilter);
	buildGraphFilters(_pCamMonik);
	_currentColorSpace = index;
	emit currentColorSpaceChanged();
	emit currentResolutionChanged();
	return 0;
}

int UvcCapture::changeResolution(int index)
{
	if (index < 0 || index >= m_mapResolution.size() || index == _currentResolution)return -1;
	stop();
	QMap< int, int >::const_iterator i = m_mapResolution.constBegin();
	m_currentResWidth = (i + index).key();
	m_currentResHeight = (i + index).value();
	SAFE_RELEASE(m_videoGrabberCb);
	emit imageCaptureChanged();
	SAFE_RELEASE(m_capGraphBuilder2);
	SAFE_RELEASE(m_graphBuilder);
	SAFE_RELEASE(m_mediaCtrl);
	SAFE_RELEASE(m_srcFilter);
	SAFE_RELEASE(_pNullFilter);
	SAFE_RELEASE(m_pVideoGrabberFilter);
	buildGraphFilters(_pCamMonik);
	_currentResolution = index;
	emit resolutionsChanged();
	return 0;
}

int UvcCapture::nextCamera()
{
	UvcUacEnumerator *pEnumrator = UvcUacEnumerator::getInstance();
	int next = _source + 1;
	if (next >= pEnumrator->getCamNum())
		next = 0;
	return changeCamera(next);
}

int UvcCapture::camWidth()
{
	return m_currentResWidth;
}

int UvcCapture::camHeight()
{
	return m_currentResHeight;
}

int UvcCapture::catchImage()
{
	if (_photoDirectory.isEmpty())return -1;
	static int photoCnt;
	//m_videoGrabberCb->catchImage();
	QString photoPath;
	QString photoDirectory;
	photoPath.clear();
	photoPath.clear();
	photoDirectory = _photoDirectory.path();
	photoPath.append(photoDirectory);
	photoPath.append("/IMG_");
	photoPath.append(QString::number(photoCnt));
	photoPath.append(".jpg");
	if (photoPath.at(0) == QChar('/'))
		photoPath.remove(0, 1);
	m_videoGrabberCb->catchImage(photoPath);
	photoCnt++;
	return 0;
}

int UvcCapture::initUvcUacDevices()
{
	int ret;
	UvcUacEnumerator *pEnumrator = UvcUacEnumerator::getInstance();
	m_cameraNum = pEnumrator->enumDevices();
	if (m_cameraNum <= 0)
		return -1;
	UsbDeviceInfo *pInfo;
	int i = 0;
	for (; i < m_cameraNum; )
	{
		ret = pEnumrator->getCamInfo(i, &pInfo);
		if (ret < 0 || NULL == pInfo)
			continue;
		if (pInfo->displayname.contains("vid_1d6b") || pInfo->displayname.contains("vid_2757"))
		{
			break;
		}
		i++;
	}
	if (i == m_cameraNum)
	{
		i = 0;
		ret = pEnumrator->getCamInfo(i, &pInfo);
		if (ret < 0 || NULL == pInfo)
			return -1;
	}
	_pCamMonik = pInfo->pMonik;
	_source = i;
	emit sourceChanged();
	emit sourceChanged();
	return 0;
}

bool UvcCapture::InitGraphFilters()
{
	if (initUvcUacDevices() < 0)
	{
		qDebug() << "Init devices  failed";
		goto failed_release;
	}
	if (!_pCamMonik)goto failed_release;
	//IID_ICaptureGraphBuilder2
#if 0
	HRESULT hr = S_OK;
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2, (void**)&m_capGraphBuilder2);
	if (FAILED(hr))
	{
		qDebug() << "Create  capture graph builder failed";
		goto failed_release;
	}

	//IID_IGraphBuilder
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void**)&m_graphBuilder);
	if (FAILED(hr))
	{
		qDebug() << "Create  filter graph  failed";
		goto failed_release;
	}
	//IID_ICaptureGraphBuilder2设置IID_IGraphBuilder
	hr = m_capGraphBuilder2->SetFiltergraph(m_graphBuilder);
	if (FAILED(hr))
	{
		qDebug() << "Set  filter graph  failed";
		goto failed_release;
	}

	//IID_IBaseFilter:IMoniker绑定的方式获取视频源Filter
	hr = _pCamMonik->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&m_srcFilter);
	if (FAILED(hr))
	{
		qDebug() << "Bind Camera moniker failed";
		goto failed_release;
	}
	hr = m_graphBuilder->AddFilter(m_srcFilter, L"Video Capture");
	if (FAILED(hr))
	{
		qDebug() << "Add camera src  filter failed";
		goto failed_release;
	}
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("1.grf");
#endif
	if (m_mapResolution.isEmpty())
	{
		if (!getCameraResolutions())
		{
			qDebug() << "Get camera resolutions  failed";
			goto failed_release;
		}
	}
	setOutPinParams();

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&m_pVideoGrabberFilter);
	if (FAILED(hr) || NULL == m_pVideoGrabberFilter)
	{
		qDebug() << "Create video sample grabber failed";
		goto failed_release;
	}
	hr = m_graphBuilder->AddFilter(m_pVideoGrabberFilter, L"VideoGrabber");
	if (FAILED(hr))
	{
		qDebug() << "Add  video sample grabber filter failed";
		goto failed_release;
	}
	ISampleGrabber * pGrabber;
	m_pVideoGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
#if VIDEO_DECODER==VIDEO_DECODER_FFMPEG
	hr = pGrabber->SetMediaType(NULL);
#endif 
	if (FAILED(hr))
	{
		qDebug() << "Video sample grabber set media type failed";
		goto failed_release;
	}

	hr = pGrabber->SetOneShot(false); // Disable "one-shot" mode
	if (FAILED(hr))
	{
		goto failed_release;
	}
	hr = pGrabber->SetBufferSamples(false); // Disable sample buffering
	if (FAILED(hr))
	{
		goto failed_release;
	}
#if VIDEO_DECODER==VIDEO_DECODER_FFMPEG
	hr = pGrabber->SetCallback(m_videoGrabberCb, 0); // Set our callback
#endif
	if (FAILED(hr))
	{
		qDebug() << "Video sample grabber set callback failed";
		goto failed_release;
	}
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("2.grf");
#endif
	hr = CoCreateInstance(
		CLSID_NullRenderer,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IBaseFilter,
		(void**)&_pNullFilter
	);
	if (FAILED(hr))
	{
		qDebug() << "Create  null render failed";
		goto failed_release;
	}
	hr = m_graphBuilder->AddFilter(_pNullFilter, L"NullRender");
	if (FAILED(hr))
	{
		qDebug() << "Add  null render failed";
		goto failed_release;
	}
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("3.grf");
#endif
	hr = m_capGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
		m_srcFilter, m_pVideoGrabberFilter, _pNullFilter);
	if (FAILED(hr))
	{
		qDebug() << "Render stream failed while media type=MEDIATYPE_Video";
		goto failed_release;
	}
	//SaveGraph("1.grf");

	hr = pGrabber->GetConnectedMediaType(m_mediaType);
	if (FAILED(hr))
	{
		qDebug("Get connected media type failed:0x%x,%s", hr, strerror(hr));
		goto failed_release;
	}
	if (m_mediaType->formattype == FORMAT_VideoInfo)
	{
		if (m_mediaType->cbFormat >= sizeof(VIDEOINFOHEADER))
		{
			//VIDEOINFOHEADER *videoInfoHeader = reinterpret_cast<VIDEOINFOHEADER*>(m_mediaType->pbFormat);
			m_videoGrabberCb->setMediaType(*m_mediaType);
		}
	}
	FreeMediaType(*m_mediaType);
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("4.grf");
#endif


#if VIDEO_STILL_CAPTURE
	m_pUvcStillCap = new CUvcStillCapture(m_visualizerWidget, m_srcFilter, m_capGraphBuilder2, m_graphBuilder);
#endif
	//IID_ICaptureGraphBuilder2获取IID_IMediaControl
	hr = m_graphBuilder->QueryInterface(IID_IMediaControl, (void**)&m_mediaCtrl);
	if (FAILED(hr))
	{
		qDebug() << "Query IID_IMediaControl interface failed";
		goto failed_release;
	}
	//pGrabber->Release();
	m_mediaCtrl->Run();
	m_bMediaRun = true;
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("5.grf");
#endif
	return true;
#endif
	buildGraphFilters(_pCamMonik);
	emit sourceChanged();
	emit colorSpacesChanged();
	emit currentColorSpaceChanged();
	emit resolutionsChanged();
	emit currentResolutionChanged();
	return 0;
failed_release:
	//releaseComInstance();
	nextCamera();
	return false;
}

int UvcCapture::buildGraphFilters(IMoniker *pMoniker)
{
	if (!pMoniker)goto failed_release;
	//IID_ICaptureGraphBuilder2
	HRESULT hr = S_OK;
	if (!m_capGraphBuilder2)
	{
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
			IID_ICaptureGraphBuilder2, (void**)&m_capGraphBuilder2);
		if (FAILED(hr))
		{
			qDebug() << "Create  capture graph builder failed";
			goto failed_release;
		}
	}

	//IID_IGraphBuilder
	if (!m_graphBuilder)
	{
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void**)&m_graphBuilder);
		if (FAILED(hr))
		{
			qDebug() << "Create  filter graph  failed";
			goto failed_release;
		}
		//IID_ICaptureGraphBuilder2设置IID_IGraphBuilder
		hr = m_capGraphBuilder2->SetFiltergraph(m_graphBuilder);
		if (FAILED(hr))
		{
			qDebug() << "Set  filter graph  failed";
			goto failed_release;
		}
	}


	//IID_IBaseFilter:IMoniker绑定的方式获取视频源Filter
	if (!m_srcFilter)
	{
		hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&m_srcFilter);
		if (FAILED(hr))
		{
			qDebug() << "Bind Camera moniker failed";
			goto failed_release;
		}
		hr = m_graphBuilder->AddFilter(m_srcFilter, L"Video Capture");
		if (FAILED(hr))
		{
			qDebug() << "Add camera src  filter failed";
			goto failed_release;
		}
	}
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("1.grf");
#endif
	if (m_mapResolution.isEmpty())
	{
		if (!getCameraResolutions())
		{
			qDebug() << "Get camera resolutions  failed";
			goto failed_release;
		}
	}
	setOutPinParams();
	if (!m_pVideoGrabberFilter)
	{
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&m_pVideoGrabberFilter);
		if (FAILED(hr) || NULL == m_pVideoGrabberFilter)
		{
			qDebug() << "Create video sample grabber failed";
			goto failed_release;
		}
		hr = m_graphBuilder->AddFilter(m_pVideoGrabberFilter, L"VideoGrabber");
		if (FAILED(hr))
		{
			qDebug() << "Add  video sample grabber filter failed";
			goto failed_release;
		}
	}
	ISampleGrabber * pGrabber;
	m_pVideoGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
	hr = pGrabber->SetMediaType(NULL);
	if (FAILED(hr))
	{
		qDebug() << "Video sample grabber set media type failed";
		goto failed_release;
	}

	hr = pGrabber->SetOneShot(false); // Disable "one-shot" mode
	if (FAILED(hr))
	{
		goto failed_release;
	}
	hr = pGrabber->SetBufferSamples(false); // Disable sample buffering
	if (FAILED(hr))
	{
		goto failed_release;
	}
	if (!m_videoGrabberCb) {
		m_videoGrabberCb = new UvcGrabber(this);
		emit imageCaptureChanged();
	}
	hr = pGrabber->SetCallback(m_videoGrabberCb, 0); // Set our callback
	m_videoGrabberCb->setRenderer(_displayItem);
	if (FAILED(hr))
	{
		qDebug() << "Video sample grabber set callback failed";
		goto failed_release;
	}
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("2.grf");
#endif
	if (!_pNullFilter)
	{
		hr = CoCreateInstance(
			CLSID_NullRenderer,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IBaseFilter,
			(void**)&_pNullFilter
		);
		if (FAILED(hr))
		{
			qDebug() << "Create  null render failed";
			goto failed_release;
		}
		hr = m_graphBuilder->AddFilter(_pNullFilter, L"NullRender");
		if (FAILED(hr))
		{
			qDebug() << "Add  null render failed";
			goto failed_release;
		}
	}
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("3.grf");
#endif
	hr = m_capGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
		m_srcFilter, m_pVideoGrabberFilter, _pNullFilter);
	if (FAILED(hr))
	{
		qDebug() << "Render stream failed while media type=MEDIATYPE_Video";
		goto failed_release;
	}

	hr = pGrabber->GetConnectedMediaType(m_mediaType);
	if (FAILED(hr))
	{
		qDebug("Get connected media type failed:0x%x,%s", hr, strerror(hr));
		goto failed_release;
	}

	if (m_mediaType->formattype == FORMAT_VideoInfo)
	{
		if (m_mediaType->cbFormat >= sizeof(VIDEOINFOHEADER))
		{
			//VIDEOINFOHEADER *videoInfoHeader = reinterpret_cast<VIDEOINFOHEADER*>(m_mediaType->pbFormat);
			m_videoGrabberCb->setMediaType(*m_mediaType);
		}
	}
	FreeMediaType(*m_mediaType);

#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("4.grf");
#endif


#if VIDEO_STILL_CAPTURE
	m_pUvcStillCap = new CUvcStillCapture(m_visualizerWidget, m_srcFilter, m_capGraphBuilder2, m_graphBuilder);
#endif
	//IID_ICaptureGraphBuilder2获取IID_IMediaControl
	hr = m_graphBuilder->QueryInterface(IID_IMediaControl, (void**)&m_mediaCtrl);
	if (FAILED(hr))
	{
		qDebug() << "Query IID_IMediaControl interface failed";
		goto failed_release;
	}
	//pGrabber->Release();
	m_mediaCtrl->Run();
	m_bMediaRun = true;
	stateChanged();
	_pUvcControler->cameraChanged();
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	SaveGraph("5.grf");
#endif
	return true;

failed_release:
	//releaseComInstance();
	nextCamera();
	return false;
}


bool UvcCapture::getCameraResolutions()
{
	HRESULT hr = S_OK;
	IPin *srcOutPin = NULL;
	hr = m_capGraphBuilder2->FindPin(m_srcFilter, PINDIR_OUTPUT, NULL, NULL, FALSE, 0, &srcOutPin);
	if (FAILED(hr))
		return false;

	IAMStreamConfig *streamCfg;
	hr = srcOutPin->QueryInterface(IID_IAMStreamConfig, (void**)&streamCfg);
	if (FAILED(hr))
	{
		srcOutPin->Release();
		srcOutPin = NULL;
		return false;
	}

	int count, size;
	hr = streamCfg->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr))
	{
		streamCfg->Release();
		srcOutPin->Release();
		streamCfg = NULL;
		srcOutPin = NULL;
		return false;
	}

	AM_MEDIA_TYPE *mt = NULL;
	BYTE *pSCC = new BYTE[size];

	qDebug() << "--------------AM_MEDIA_TYPE-----------";
	QMap< int, int > resMap;
	resMap.clear();
	m_mapColorRes.clear();
	for (int i = 0; i < count; i++)
	{
		hr = streamCfg->GetStreamCaps(i, &mt, pSCC);
		tagSIZE resSize = ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->InputSize;
		int cx = resSize.cx, cy = resSize.cy;
		QString colorSpace("DEFAULT");
		if (MEDIASUBTYPE_MJPG == mt->subtype)
			colorSpace = "MJPG";
		else if (MEDIASUBTYPE_YVYU == mt->subtype)
			colorSpace = "YVYU";
		else if (MEDIASUBTYPE_YUY2 == mt->subtype)
			colorSpace = "YUY2";
		else if (MEDIASUBTYPE_YUYV == mt->subtype)
			colorSpace = "YUYV";
		else if (MEDIASUBTYPE_H264 == mt->subtype)
			colorSpace = "H264";
		else if (MEDIASUBTYPE_HEVC == mt->subtype)
			colorSpace = "HEVC";
		if (m_mapColorRes.contains(colorSpace))
		{
			resMap = m_mapColorRes.value(colorSpace);
			if (!resMap.contains(cx) && resMap.value(cx) != cy)
				resMap.insertMulti(cx, cy);

			m_mapColorRes.insert(colorSpace, resMap);
		}
		else
		{
			resMap.clear();
			resMap.insert(cx, cy);
			m_mapColorRes.insert(colorSpace, resMap);
		}
	}
	if (m_mapColorRes.contains("H264"))
		m_currentColorSpace = "H264";
	else if (m_mapColorRes.contains("HEVC"))
		m_currentColorSpace = "HEVC";
	else if (m_mapColorRes.contains("MJPG"))
		m_currentColorSpace = "MJPG";
	else
		m_currentColorSpace = m_mapColorRes.begin().key();

	QMap< QString, QMap<int, int> > ::const_iterator j = m_mapColorRes.begin();
	_currentColorSpace = 0;
	while (j != m_mapColorRes.constEnd()) {
		if (m_currentColorSpace != j.key())
		{
			_currentColorSpace++;
			j++;
		}
		else break;
	}
	emit currentColorSpaceChanged();
	m_mapResolution = m_mapColorRes.value(m_currentColorSpace);
	QMap<int, int>::const_iterator i = m_mapResolution.constBegin();
	while (i != m_mapResolution.constEnd()) {
		QString s;
		s.clear();
		s.append(QString::number(i.key()));
		s.append('x');
		s.append(QString::number(i.value()));
		_resolutions.append(s);
		i++;
	}
	emit resolutionsChanged();
	QMap<int, int>::iterator itEnd = m_mapResolution.end();
	itEnd--;

	_currentResolution = m_mapResolution.size() - 1;
	emit currentResolutionChanged();
	_colorSpaces = m_mapColorRes.keys();
	emit colorSpacesChanged();

	m_currentResWidth = itEnd.key();
	m_currentResHeight = itEnd.value();

	streamCfg->Release();
	srcOutPin->Release();
	streamCfg = NULL;
	srcOutPin = NULL;
	delete[]pSCC;
	if (NULL != mt)
	{
		//deleteMediaType(mt);
		DeleteMediaType(mt);
		mt = NULL;
	}

	return true;
}

bool UvcCapture::setOutPinParams()
{
	HRESULT hr = S_OK;
	IPin *srcOutPin = NULL;
	hr = m_capGraphBuilder2->FindPin(m_srcFilter, PINDIR_OUTPUT, NULL, NULL, FALSE, 0, &srcOutPin);
	if (FAILED(hr))
		return false;
	IAMStreamConfig *streamCfg;
	hr = srcOutPin->QueryInterface(IID_IAMStreamConfig, (void**)&streamCfg);

	if (FAILED(hr))
	{
		srcOutPin->Release();
		srcOutPin = NULL;
		return false;
	}
	int count, size;
	hr = streamCfg->GetNumberOfCapabilities(&count, &size);

	if (FAILED(hr))
	{
		streamCfg->Release();
		srcOutPin->Release();
		streamCfg = NULL;
		srcOutPin = NULL;
		return false;
	}

	AM_MEDIA_TYPE *mt = NULL;
	BYTE *pSCC = new BYTE[size];
	GUID cs = getColorSpaceGUID();

	for (int i = 0; i < count; i++)
	{
		hr = streamCfg->GetStreamCaps(i, &mt, pSCC);
		int cx = ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->InputSize.cx;
		int cy = ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->InputSize.cy;
		if (cx != m_currentResWidth)
			continue;
		if (cy != m_currentResHeight)
			continue;
		if (cs != mt->subtype)
			continue;
		hr = streamCfg->SetFormat(mt);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	streamCfg->Release();
	srcOutPin->Release();
	streamCfg = NULL;
	srcOutPin = NULL;
	delete[]pSCC;
	if (NULL != mt)
	{
		//deleteMediaType(mt);
		DeleteMediaType(mt);
		mt = NULL;
	}

	return true;
}

GUID UvcCapture::getColorSpaceGUID()
{
	GUID cs = MEDIASUBTYPE_YUY2;
	if ("MJPG" == m_currentColorSpace)
		cs = MEDIASUBTYPE_MJPG;
	else if ("YUY2" == m_currentColorSpace)
		cs = MEDIASUBTYPE_YUY2;
	else if ("YVYU" == m_currentColorSpace)
		cs = MEDIASUBTYPE_YVYU;
	else if ("YUYV" == m_currentColorSpace)
		cs = MEDIASUBTYPE_YUYV;
	else if ("H264" == m_currentColorSpace)
		cs = MEDIASUBTYPE_H264;
	else if ("HEVC" == m_currentColorSpace)
		cs = MEDIASUBTYPE_HEVC;
	return cs;
}

bool UvcCapture::releaseComInstance()
{
	stop();

#if VIDEO_STILL_CAPTURE
	if (m_pUvcStillCap)
	{
		delete m_pUvcStillCap;
		m_pUvcStillCap = NULL;
	}
#endif
	_pCamMonik = NULL;
	SAFE_RELEASE(m_capGraphBuilder2);
	SAFE_RELEASE(m_graphBuilder);
	SAFE_RELEASE(m_mediaCtrl);
	SAFE_RELEASE(m_srcFilter);
	SAFE_RELEASE(_pNullFilter);
	SAFE_RELEASE(m_videoGrabberCb);
	if (m_mediaType)
	{
		//deleteMediaType(m_mediaType);
		//m_mediaType = NULL;
		//DeleteMediaType(m_mediaType);
	}
	return true;
}

IMoniker * UvcCapture::nextCamMonik(void)
{
	UvcUacEnumerator *pEnumrator = UvcUacEnumerator::getInstance();
	int next = _source + 1;
	if (next >= pEnumrator->getCamNum())
		next = 0;
	return camMonik(next);
}

IMoniker * UvcCapture::camMonik(int index)
{
	UvcUacEnumerator *pEnumrator = UvcUacEnumerator::getInstance();
	return pEnumrator->getCamMonik(index);
}

#ifdef DEBUG_DRECTSHOW_GRAPHICS
void UvcCapture::SaveGraph(CString wFileName)
{
	HRESULT hr;

	IStorage* pStorage = NULL;

	// First, create a document file that will hold the GRF file
	hr = ::StgCreateDocfile(wFileName.AllocSysString(), STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pStorage);
	if (FAILED(hr))
	{
		qDebug() << "Can not create a document";
		return;
	}

	// Next, create a stream to store.
	WCHAR wszStreamName[] = L"ActiveMovieGraph";
	IStream *pStream;

	hr = pStorage->CreateStream(wszStreamName, STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, 0, &pStream);
	if (FAILED(hr))
	{
		qDebug() << "Can not create a stream";
		pStorage->Release();
		return;
	}

	// The IpersistStream::Save method converts a stream
	// into a persistent object.
	IPersistStream *pPersist = NULL;

	hr = m_graphBuilder->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
	if (FAILED(hr))
	{
		qDebug() << "QueryInterface failed";

	}
	hr = pPersist->Save(pStream, TRUE);
	if (FAILED(hr))
	{
		qDebug() << "Save failed";

	}
	if (SUCCEEDED(hr))
	{
		hr = pStorage->Commit(STGC_DEFAULT);
		if (FAILED(hr))
		{
			qDebug() << "can not store it";
		}
	}
	pStream->Release();
	pPersist->Release();
	pStorage->Release();
}
#endif

