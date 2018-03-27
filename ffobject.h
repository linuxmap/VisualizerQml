#pragma once
#include <QtCore>
class FFObject :public QObject
{
	Q_OBJECT
	Q_ENUMS(HWAccelID)
public:
	enum HWAccelID {
		HWACCEL_NONE = 0,
		HWACCEL_AUTO,
		HWACCEL_VDPAU,
		HWACCEL_DXVA2,
		HWACCEL_VDA,
		HWACCEL_VIDEOTOOLBOX,
		HWACCEL_QSV,
		HWACCEL_VAAPI,
		HWACCEL_CUVID,
		HWACCEL_D3D11VA,
	};
protected:
	FFObject(QObject*parent=nullptr);
};