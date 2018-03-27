#pragma once
#include <stdlib.h>
#include <QtCore>
#include <QQmlEngine>
extern "C"{
#include <mfx/mfxvideo.h>
#include "libavutil/dict.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_qsv.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavcodec/qsv.h"
#include "libavcodec/avcodec.h"
}
#include "ffinputstream.h"

class QsvAccelerator :public QObject
{
	Q_OBJECT
public:
	static int init(AVCodecContext *s);
	static int qsv_get_buffer(AVCodecContext *s, AVFrame *frame, int flags);
	static void qsv_uninit(AVCodecContext *s);
private:
	QsvAccelerator() {};
	static int qsv_device_init(FFInputStream *ist);
};
QML_DECLARE_TYPE(QsvAccelerator)
