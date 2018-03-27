#include "qsvaccelerator.h"

int QsvAccelerator::init(AVCodecContext *s)
{
	FFInputStream *ist = (FFInputStream * )s->opaque;
	AVHWFramesContext *frames_ctx;
	AVQSVFramesContext *frames_hwctx;
	int ret;

	if (!ist->_hwDeviceCtx) {
		ret = qsv_device_init(ist);
		if (ret < 0)
			return ret;
	}
	av_buffer_unref(&ist->_hwFramesCtx);
	ist->_hwFramesCtx = av_hwframe_ctx_alloc(ist->_hwDeviceCtx);
	if (!ist->_hwFramesCtx)
		return AVERROR(ENOMEM);

	frames_ctx = (AVHWFramesContext*)ist->_hwFramesCtx->data;
	frames_hwctx = (AVQSVFramesContext *)frames_ctx->hwctx;

	frames_ctx->width = FFALIGN(s->coded_width, 32);
	frames_ctx->height = FFALIGN(s->coded_height, 32);
	frames_ctx->format = AV_PIX_FMT_QSV;
	frames_ctx->sw_format = s->sw_pix_fmt;
	frames_ctx->initial_pool_size = 64;
	frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

	ret = av_hwframe_ctx_init(ist->_hwFramesCtx);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error initializing a QSV frame pool\n");
		return ret;
	}

	ist->_hwaccelGetBuffer = qsv_get_buffer;
	ist->_hwaccelUninit = qsv_uninit;

	return 0;
}
int QsvAccelerator::qsv_get_buffer(AVCodecContext *s, AVFrame *frame, int flags)
{
	Q_UNUSED(flags);
	FFInputStream *ist = (FFInputStream * )s->opaque;
	return av_hwframe_get_buffer(ist->_hwFramesCtx, frame, 0);
}

void QsvAccelerator::qsv_uninit(AVCodecContext *s)
{
	FFInputStream *ist = (FFInputStream *)s->opaque;
	av_buffer_unref(&ist->_hwFramesCtx);
}

int QsvAccelerator::qsv_device_init(FFInputStream *ist)
{
	int err;
	AVDictionary *dict = NULL;

	if (!ist->_qsvDevice.isEmpty()) {
		err = av_dict_set(&dict, "child_device", ist->_qsvDevice.toStdString().data(), 0);
		if (err < 0)
			return err;
	}

	err = av_hwdevice_ctx_create(&ist->_hwDeviceCtx, AV_HWDEVICE_TYPE_QSV,
		ist->_hwaccelDevice, dict, 0);
	if (err < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error creating a QSV device\n");
		goto err_out;
	}

err_out:
	if (dict)
		av_dict_free(&dict);
	return err;
}

