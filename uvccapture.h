#ifndef CCAMERACAPTURE_H
#define CCAMERACAPTURE_H

#include <QMap>
#include <QQmlEngine>
#include <QPixmap> 
#include <QMutex>
//#define DEBUG_DRECTSHOW_GRAPHICS
#ifdef DEBUG_DRECTSHOW_GRAPHICS
#include <atlstr.h>
#endif
#include <initguid.h>
#include "common.h"
#include "uvcuacinfo.h"
#include "uvccontroler.h"
#include "uvcdshowgrabber.h"

DEFINE_GUID(MEDIASUBTYPE_HEVC, 0x35363248, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

class UvcGrabber;
class UvcControler;

class UvcCapture :public QObject
{
	Q_OBJECT
		Q_CLASSINFO("Version", "1.0")
		Q_CLASSINFO("Author", "Ophelia")
		Q_PROPERTY(int source READ source WRITE setSource NOTIFY sourceChanged)
		Q_PROPERTY(IFrameRenderer* displayItem READ displayItem WRITE setDisplayItem)
		Q_PROPERTY(QStringList colorSpaces READ colorSpaces  NOTIFY colorSpacesChanged)
		Q_PROPERTY(int currentColorSpace READ currentColorSpace WRITE setCurrentColorSpace NOTIFY currentColorSpaceChanged)
		Q_PROPERTY(QStringList resolutions READ resolutions  NOTIFY resolutionsChanged)
		Q_PROPERTY(int currentResolution READ currentResolution WRITE setCurrentResolution NOTIFY currentResolutionChanged)
		Q_PROPERTY(UvcControler *uvcControler READ uvcControler CONSTANT)
		//Q_PROPERTY(UvcGrabber *imageCapture READ imageCapture NOTIFY imageCaptureChanged)
		Q_PROPERTY(QUrl photoDirectory READ photoDirectory WRITE setPhotoDirectory)
		Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
public:
	UvcCapture(QObject *parent = NULL);
	~UvcCapture();
	int source(void) { return _source; }
	int setSource(const int index);
	IFrameRenderer* displayItem() { return _displayItem; }
	int setDisplayItem(IFrameRenderer* displayItem);
	QStringList colorSpaces() {	return _colorSpaces; }
	int currentColorSpace() { return _currentColorSpace; };
	void setCurrentColorSpace(int cs);
	QStringList resolutions() { return _resolutions; }
	int currentResolution() { return _currentResolution; }
	void setCurrentResolution(int index);
	UvcControler * uvcControler() { return _pUvcControler; }
	//UvcGrabber *imageCapture() {

	//	return m_videoGrabberCb;
	//};
	QUrl photoDirectory() {return _photoDirectory;};
	void setPhotoDirectory(QUrl url);
	bool isRunning() { return m_bMediaRun; }
	Q_INVOKABLE bool play();
	Q_INVOKABLE bool stop();
	Q_INVOKABLE int changeCamera(int index);
	Q_INVOKABLE int changeCorlorSpace(int index);
	Q_INVOKABLE int changeResolution(int index);
	Q_INVOKABLE int nextCamera();
	Q_INVOKABLE int  camWidth();
	Q_INVOKABLE int	 camHeight();
	Q_INVOKABLE int catchImage();
	
	friend class UvcGrabber;
	friend class UvcControler;
signals:
	void uvcChanged();
	void sourceChanged();
	void colorSpacesChanged();
	void currentColorSpaceChanged();
	void resolutionsChanged();
	void currentResolutionChanged();
	void imageCaptureChanged();
	void stateChanged();
private:
	int initUvcUacDevices(void);
	bool InitGraphFilters();
	int  buildGraphFilters(IMoniker *pMoniker);

	bool getCameraResolutions();
	bool setOutPinParams();
	GUID getColorSpaceGUID();
	bool releaseComInstance();
	IMoniker * nextCamMonik(void);
	IMoniker *camMonik(int index);
#ifdef DEBUG_DRECTSHOW_GRAPHICS
	void SaveGraph(CString wFileName);
#endif
	Q_DISABLE_COPY(UvcCapture)

	IMoniker                *_pCamMonik;
	ICaptureGraphBuilder2   *m_capGraphBuilder2;
	IGraphBuilder           *m_graphBuilder;
	IMediaControl           *m_mediaCtrl;
	bool                     m_bMediaRun;
	IBaseFilter             *m_srcFilter;
	IBaseFilter				*m_pVideoGrabberFilter;
	IBaseFilter				*_pNullFilter;
	int                     m_cameraNum;
	QMap<int, int>          m_mapResolution;
	QStringList             _resolutions;
	int                     m_currentResWidth;
	int                     m_currentResHeight;
	QMap< QString, QMap<int, int> >     m_mapColorRes;
	QString                 m_currentColorSpace;
	QStringList				_colorSpaces;
	int						_currentColorSpace;
	int						_currentResolution;
	AM_MEDIA_TYPE			*m_mediaType;
	UvcGrabber		*m_videoGrabberCb;
	int _source;
	QString _imageUrl;
	IFrameRenderer *_displayItem;
	UvcControler *_pUvcControler;
	QUrl _photoDirectory;
};
QML_DECLARE_TYPE(UvcCapture)

#endif // CCAMERACAPTURE_H
