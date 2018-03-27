#pragma once
#include "uvccapture.h"
#include <Vidcap.h>
#include "hiuvcextensionunit.h"
#include "hi_uvc_ext_ctrl.h"

class UvcControlerThread :public QThread
{
	Q_OBJECT
	Q_ENUMS(UvcControlCmd)
public:
	enum UvcControlCmd {
		UvcControlCmdNone=0,
		UvcControlZoomStop,
		UvcControlZoomIn,
		UvcControlZoomOut
	};
	~UvcControlerThread();
	void run() Q_DECL_OVERRIDE;
	friend class UvcControler;
private:
	UvcControlerThread(UvcControler *controler,QObject *parent = nullptr);
	void addCmd(UvcControlCmd cmd);
	UvcControlCmd takeCmd(void);
	QPointer<UvcControler> _controler;
	QList<UvcControlCmd> _cmdList;
	QList<UvcControlCmd> _repeatCmdList;
	QMutex _cmdListLocker;
	bool   _run;
};
class UvcControler :public QObject
{
	Q_OBJECT
	Q_ENUMS(CamProp)
	Q_ENUMS(CamAutoFlags)
public:
	enum CamProp {
		CamPropertyBrightness=0,
		CamPropertyContrast,
		CamPropertyHue,
		CamPropertySaturation,
		CamPropertySharpness,
		CamPropertyGamma,
		CamPropertyColor,
		CamPropertyWb,//white balance
		CamPropertyBlc,//backlight compensation
		CamPropertyGain,
		CamPropertyPan,
		CamPropertyTilt,
		CamPropertyRoll,
		CamPropertyZoom,
		CamPropertyExposure,
		CamPropertyIris,
		CamPropertyFocus,
		CamPropertyCount



	};
	enum CamAutoFlags {
		CamAutoFlagsDisable = 0,
		CamAutoFlagsEnable = 1,
		CamAutoFlagsNotSupport = 2,
	};
	struct CamPropInfo
	{
		bool bSupport;
		long min;
		long max;
		long steppingDelta;
		long default;
		bool bSuppAuto;
		long curValue;
		bool bAuto;
	};
	~UvcControler();
	Q_INVOKABLE int zoomIn();
	Q_INVOKABLE int zoomOut();
	Q_INVOKABLE int zoomStop();
	void cameraChanged();
	friend class UvcCapture;
	friend class UvcControlerThread;
private:
	UvcControler(UvcCapture *camera,QObject *parent=nullptr);
	int getCamPropInfo(CamProp prop, struct CamPropInfo &info);
	int setCamProp(CamProp prop, long value, bool bAuto);
	void dshowType(CamProp prop, int &dshowType, bool &isVideoProcInf);
	HRESULT FindExtensionNode(IKsTopologyInfo *pKsTopologyInfo, GUID guid, DWORD *node);
	int  getAllPropInfo(void);
	int  propertyBeReady(void);
	int doZoomIn();
	int doZoomOut();
	IBaseFilter *camFilter();
	UvcCapture *_camera;
	CamPropInfo _propInfo[CamPropertyCount];
	bool		_propReady;
	QMutex m_uvcCtrlMutex;
	UvcControlerThread *_controlThread;

};
QML_DECLARE_TYPE(UvcControler)
