#pragma once
#include <Qedit.h>
#include "uvccapture.h"
#include "mtype.h"
#include "ffdshowdecoder.h"
class UvcCapture;
class ImageCaptureThread :public QThread
{
	Q_OBJECT
public:
	ImageCaptureThread(QObject *parent = NULL);
	~ImageCaptureThread();
	int saveAvframe(QString url, AVFrame *frame);
	bool isSavingImage();
	void run()override;
private:
	QString _photoPath;
	AVFrame *_frame;
	SwsContext *_sws_ctx;
};

class UvcGrabber : public ISampleGrabberCB
{
	Q_GADGET
public:
	explicit UvcGrabber(UvcCapture *parent);
	~UvcGrabber();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
	virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample);
	virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);
	BOOL SaveBitmap(BYTE * pBuffer, long lBufferSize);
	bool setMediaType(AM_MEDIA_TYPE &mt);
	int setRenderer(IFrameRenderer* renderer);
	int catchImage();
	int catchImage(QString photoPath);

private:
	int m_ref;
	UvcCapture *_pCamera;
	QMutex   m_sampleCbMutex;
	unsigned int m_fBufferLen;
	AM_MEDIA_TYPE			*m_mediaType;
	long m_videoWidth;
	long m_videoHeight;
	FFDShowDecoder *_pVideoDecoder;
	AVFrame *_pAvFrame;
	IFrameRenderer* _pRenderer;
	bool _isYuy2;
	FILE * _tempfile;
	ImageCaptureThread *_imageCatcher;
	QMutex _locker;
	bool _toCatchImage;
	QString _photoPath;
};
QML_DECLARE_TYPE(UvcGrabber)

