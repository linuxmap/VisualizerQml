#include "ffhwaccelerator.h"
#include <string.h>
#include "libavutil/avstring.h"
FFHWAccelerator::HWAccel FFHWAccelerator::hwaccels[] = {
#if HAVE_VDPAU_X11
	{ "vdpau", hwaccel_decode_init, HWACCEL_VDPAU, AV_PIX_FMT_VDPAU,	AV_HWDEVICE_TYPE_VDPAU },
#endif
#if CONFIG_D3D11VA
{ "d3d11va", hwaccel_decode_init, HWACCEL_D3D11VA, AV_PIX_FMT_D3D11,AV_HWDEVICE_TYPE_D3D11VA },
#endif
#if CONFIG_DXVA2
{ "dxva2", hwaccel_decode_init, HWACCEL_DXVA2, AV_PIX_FMT_DXVA2_VLD,AV_HWDEVICE_TYPE_DXVA2 },
#endif
#if CONFIG_VDA
{ "vda",   videotoolbox_init,  HWACCEL_VDA,   AV_PIX_FMT_VDA,AV_HWDEVICE_TYPE_NONE },
#endif
#if CONFIG_VIDEOTOOLBOX
{ "videotoolbox",   /*videotoolbox_init,*/   HWACCEL_VIDEOTOOLBOX,   AV_PIX_FMT_VIDEOTOOLBOX,AV_HWDEVICE_TYPE_NONE },
#endif
#if CONFIG_LIBMFX
{ "qsv",   QsvAccelerator::init,  HWACCEL_QSV,   AV_PIX_FMT_QSV,AV_HWDEVICE_TYPE_NONE },
#endif
#if CONFIG_VAAPI
{ "vaapi", hwaccel_decode_init, HWACCEL_VAAPI, AV_PIX_FMT_VAAPI,AV_HWDEVICE_TYPE_VAAPI },
#endif
#if CONFIG_CUVID
{ "cuvid", cuvid_init, HWACCEL_CUVID, AV_PIX_FMT_CUDA,AV_HWDEVICE_TYPE_NONE },
#endif
{ 0 },
};
FFHWAccelerator::FFHWAccelerator(FFInputStream *s, QObject*parent /*= nullptr*/)
	:FFObject(parent)
	, _is(s)
{
}

FFHWAccelerator::~FFHWAccelerator()
{
}
const FFHWAccelerator::HWAccel *FFHWAccelerator::get_hwaccel(enum AVPixelFormat pix_fmt)
{
	int i;
	for (i = 0; hwaccels[i].name; i++)
		if (hwaccels[i].pix_fmt == pix_fmt)
			return &hwaccels[i];
	return NULL;
}
int FFHWAccelerator::hwaccel_decode_init(AVCodecContext *avctx)
{
	FFInputStream *ist = (FFInputStream * )avctx->opaque;
	ist->_hwaccelRetrieveData = &hwaccel_retrieve_data;
	return 0;
}
int FFHWAccelerator::hwDeviceSetupForDecode(void)
{
	enum AVHWDeviceType type;
	HWDevice *dev;
	int err;

	if (_is->_hwaccelDevice) {
		dev = hw_device_get_by_name(_is->_hwaccelDevice);
		if (!dev) {
			//char *tmp;
			QString tmp;
			type = hw_device_match_type_by_hwaccel(_is->_hwaccelId);
			if (type == AV_HWDEVICE_TYPE_NONE) {
				// No match - this isn't necessarily invalid, though,
				// because an explicit device might not be needed or
				// the hwaccel setup could be handled elsewhere.
				return 0;
			}
			tmp.clear();
			tmp.append(av_hwdevice_get_type_name(type));
			tmp.append(":");
			tmp.append(_is->_hwaccelDevice);
			//tmp = asprintf("%s:%s", av_hwdevice_get_type_name(type),
			//	_is->_hwaccelDevice);
			if (tmp.isEmpty())
				return AVERROR(ENOMEM);
			err = hw_device_init_from_string(tmp.toStdString().data(), &dev);
			//av_free(tmp);
			if (err < 0)
				return err;
		}
	}
	else {
		if (_is->_hwaccelId != HWACCEL_NONE)
			type = hw_device_match_type_by_hwaccel(_is->_hwaccelId);
		else
			type = hw_device_match_type_in_name(_is->_dec->name);
		if (type != AV_HWDEVICE_TYPE_NONE) {
			dev = hw_device_get_by_type(type);
			if (!dev) {
				hw_device_init_from_string(av_hwdevice_get_type_name(type),
					&dev);
			}
		}
		else {
			// No device required.
			return 0;
		}
	}

	if (!dev) {
		av_log(_is->_decCtx, AV_LOG_WARNING, "No device available "
			"for decoder (device type %s for codec %s).\n",
			av_hwdevice_get_type_name(type), _is->_dec->name);
		return 0;
	}
	_is->_decCtx->hw_device_ctx = av_buffer_ref(dev->device_ref);
	if (!_is->_decCtx->hw_device_ctx)
		return AVERROR(ENOMEM);
	return 0;

}

int FFHWAccelerator::hwaccel_retrieve_data(AVCodecContext *avctx, AVFrame *input)
{
	FFInputStream *ist = (FFInputStream * )avctx->opaque;
	AVFrame *output = NULL;
	enum AVPixelFormat output_format = ist->_hwaccelOutputFormat;
	int err;

	if (input->format == output_format) {
		// Nothing to do.
		return 0;
	}

	output = av_frame_alloc();
	if (!output)
		return AVERROR(ENOMEM);

	output->format = output_format;

	err = av_hwframe_transfer_data(output, input, 0);
	if (err < 0) {
		av_log(avctx, AV_LOG_ERROR, "Failed to transfer data to "
			"output frame: %d.\n", err);
		goto fail;
	}

	err = av_frame_copy_props(output, input);
	if (err < 0) {
		av_frame_unref(output);
		goto fail;
	}

	av_frame_unref(input);
	av_frame_move_ref(input, output);
	av_frame_free(&output);

	return 0;

fail:
	av_frame_free(&output);
	return err;
}

void FFHWAccelerator::hwaccel_get_buffer()
{

}

FFHWAccelerator::HWDevice *FFHWAccelerator::hw_device_get_by_type(enum AVHWDeviceType type)
{
	HWDevice *found = NULL;
	int i;
	for (i = 0; i < nb_hw_devices; i++) {
		if (hw_devices[i]->type == type) {
			if (found)
				return NULL;
			found = hw_devices[i];
		}
	}
	return found;
}

FFHWAccelerator::HWDevice *FFHWAccelerator::hw_device_get_by_name(const char *name)
{
	int i;
	for (i = 0; i < nb_hw_devices; i++) {
		if (!strcmp(hw_devices[i]->name, name))
			return hw_devices[i];
	}
	return NULL;
}


FFHWAccelerator::HWDevice * FFHWAccelerator::hw_device_get_by_name(const QString name)
{
	return hw_device_get_by_name(name.toStdString().data());
}

FFHWAccelerator::HWDevice *FFHWAccelerator::hw_device_add(void)
{
	int err;
	err = av_reallocp_array(&hw_devices, nb_hw_devices + 1,
		sizeof(*hw_devices));
	if (err) {
		nb_hw_devices = 0;
		return NULL;
	}
	hw_devices[nb_hw_devices] = (HWDevice *)av_mallocz(sizeof(HWDevice));
	if (!hw_devices[nb_hw_devices])
		return NULL;
	return hw_devices[nb_hw_devices++];
}

int FFHWAccelerator::hw_device_init_from_string(const char *arg, HWDevice **dev_out)
{
	// "type=name:device,key=value,key2=value2"
	// "type:device,key=value,key2=value2"
	// -> av_hwdevice_ctx_create()
	// "type=name@name"
	// "type@name"
	// -> av_hwdevice_ctx_create_derived()

	AVDictionary *options = NULL;
	char *type_name = NULL, *name = NULL, *device = NULL;
	enum AVHWDeviceType type;
	HWDevice *dev, *src;
	AVBufferRef *device_ref = NULL;
	int err;
	const char *errmsg, *p, *q;
	size_t k;

	k = strcspn(arg, ":=@");
	p = arg + k;

	type_name = av_strndup(arg, k);
	if (!type_name) {
		err = AVERROR(ENOMEM);
		goto fail;
	}
	type = av_hwdevice_find_type_by_name(type_name);
	if (type == AV_HWDEVICE_TYPE_NONE) {
		errmsg = "unknown device type";
		goto invalid;
	}

	if (*p == '=') {
		k = strcspn(p + 1, ":@");

		name = av_strndup(p + 1, k);
		if (!name) {
			err = AVERROR(ENOMEM);
			goto fail;
		}
		if (hw_device_get_by_name(name)) {
			errmsg = "named device already exists";
			goto invalid;
		}

		p += 1 + k;
	}
	else {
		// Give the device an automatic name of the form "type%d".
		// We arbitrarily limit at 1000 anonymous devices of the same
		// type - there is probably something else very wrong if you
		// get to this limit.
		size_t index_pos;
		int index, index_limit = 1000;
		index_pos = strlen(type_name);
		name = (char *)av_malloc(index_pos + 4);
		if (!name) {
			err = AVERROR(ENOMEM);
			goto fail;
		}
		for (index = 0; index < index_limit; index++) {
			snprintf(name, index_pos + 4, "%s%d", type_name, index);
			if (!hw_device_get_by_name(name))
				break;
		}
		if (index >= index_limit) {
			errmsg = "too many devices";
			goto invalid;
		}
	}

	if (!*p) {
		// New device with no parameters.
		err = av_hwdevice_ctx_create(&device_ref, type,
			NULL, NULL, 0);
		if (err < 0)
			goto fail;

	}
	else if (*p == ':') {
		// New device with some parameters.
		++p;
		q = strchr(p, ',');
		if (q) {
			device = av_strndup(p, q - p);
			if (!device) {
				err = AVERROR(ENOMEM);
				goto fail;
			}
			err = av_dict_parse_string(&options, q + 1, "=", ",", 0);
			if (err < 0) {
				errmsg = "failed to parse options";
				goto invalid;
			}
		}

		err = av_hwdevice_ctx_create(&device_ref, type,
			device ? device : p, options, 0);
		if (err < 0)
			goto fail;

	}
	else if (*p == '@') {
		// Derive from existing device.

		src = hw_device_get_by_name(p + 1);
		if (!src) {
			errmsg = "invalid source device name";
			goto invalid;
		}

		err = av_hwdevice_ctx_create_derived(&device_ref, type,
			src->device_ref, 0);
		if (err < 0)
			goto fail;
	}
	else {
		errmsg = "parse error";
		goto invalid;
	}
	dev = hw_device_add();
	if (!dev) {
		err = AVERROR(ENOMEM);
		goto fail;
	}

	dev->name = name;
	dev->type = type;
	dev->device_ref = device_ref;

	if (dev_out)
		*dev_out = dev;

	name = NULL;
	err = 0;
done:
	av_freep(&type_name);
	av_freep(&name);
	av_freep(&device);
	av_dict_free(&options);
	return err;
invalid:
	av_log(NULL, AV_LOG_ERROR,
		"Invalid device specification \"%s\": %s\n", arg, errmsg);
	err = AVERROR(EINVAL);
	goto done;
fail:
	av_log(NULL, AV_LOG_ERROR,
		"Device creation failed: %d.\n", err);
	av_buffer_unref(&device_ref);
	goto done;
}

void FFHWAccelerator::hw_device_free_all(void)
{
	int i;
	for (i = 0; i < nb_hw_devices; i++) {
		av_freep(&hw_devices[i]->name);
		av_buffer_unref(&hw_devices[i]->device_ref);
		av_freep(&hw_devices[i]);
	}
	av_freep(&hw_devices);
	nb_hw_devices = 0;
}

enum AVHWDeviceType FFHWAccelerator::hw_device_match_type_by_hwaccel(enum HWAccelID hwaccel_id)
{
	int i;
	if (hwaccel_id == HWACCEL_NONE)
		return AV_HWDEVICE_TYPE_NONE;
	for (i = 0; hwaccels[i].name; i++) {
		if (hwaccels[i].id == hwaccel_id)
			return hwaccels[i].device_type;
	}
	return AV_HWDEVICE_TYPE_NONE;
}

enum AVHWDeviceType FFHWAccelerator::hw_device_match_type_in_name(const char *codec_name)
{
	const char *type_name;
	enum AVHWDeviceType type;
	for (type = av_hwdevice_iterate_types(AV_HWDEVICE_TYPE_NONE);
		type != AV_HWDEVICE_TYPE_NONE;
		type = av_hwdevice_iterate_types(type)) {
		type_name = av_hwdevice_get_type_name(type);
		if (strstr(codec_name, type_name))
			return type;
	}
	return AV_HWDEVICE_TYPE_NONE;
}