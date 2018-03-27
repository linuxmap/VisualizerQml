#pragma once
#include <QtCore>
#include <QQmlEngine>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/hwcontext.h"
}
#define HAVE_VDPAU_X11      1
#define CONFIG_D3D11VA      1
#define CONFIG_DXVA2         1
#define CONFIG_LIBMFX       1
#define CONFIG_VAAPI         1
#include "ffobject.h"
#include "ffinputstream.h"
#if  CONFIG_LIBMFX
#include "qsvaccelerator.h"
#endif
class FFHWAccelerator :public FFObject
{
	Q_OBJECT
public:
	typedef struct HWAccel {
		const char *name;
		int(*init)(AVCodecContext *s);
		enum HWAccelID id;
		enum AVPixelFormat pix_fmt;
		enum AVHWDeviceType device_type;
	} HWAccel;

	typedef struct HWDevice {
		char *name;
		enum AVHWDeviceType type;
		AVBufferRef *device_ref;
	} HWDevice;

	explicit FFHWAccelerator(FFInputStream *s, QObject*parent = nullptr);
	~FFHWAccelerator();
	static const HWAccel *get_hwaccel(enum AVPixelFormat pix_fmt);
	static int hwaccel_decode_init(AVCodecContext *avctx);
	int hwDeviceSetupForDecode(void);
	static int hwaccel_retrieve_data(AVCodecContext *avctx, AVFrame *input);
	void hwaccel_get_buffer();
private:
	HWDevice *hw_device_get_by_type(enum AVHWDeviceType type);
	HWDevice *hw_device_get_by_name(const char * name);
	HWDevice *hw_device_get_by_name(const QString name);
	HWDevice *hw_device_add(void);
	int hw_device_init_from_string(const char *arg, HWDevice **dev_out);
	void hw_device_free_all(void);
	enum AVHWDeviceType hw_device_match_type_by_hwaccel(enum HWAccelID hwaccel_id);
	enum AVHWDeviceType hw_device_match_type_in_name(const char *codec_name);
	static HWAccel hwaccels[];
	FFInputStream *_is;
	AVCodecContext * _pCodecCtx;
	int nb_hw_devices;
	HWDevice **hw_devices;
	QsvAccelerator *_qsvAccelerator;
	
};
QML_DECLARE_TYPE(FFHWAccelerator)