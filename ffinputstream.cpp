#include "ffinputstream.h"

FFInputStream::FFInputStream(QObject* parent)
	:FFObject(parent)
	, _fileIndex(0)
	, _st(NULL)
	, _discard(0)
	, _userSetDiscard(0)
	, _decodingNeeded(0)
	, _decCtx(0)
	, _dec(0)
	, _decodedFrame(0)
	, _filterFrame(0)
	, _start(0)
	, _nextDts(0)
	, _dts(0)
	, _nextPts(0)
	, _pts(0)
	, _wrapCorrectionDone(0)
	, _filterInRescaleDeltaLast(0)
	, _minPts(0)
	, _maxPts(0)
	, _cfrNextPts(0)
	, _nbSamples(0)
	, _tsScale(0)
	, _sawFirstTs(0)
	, _decoderOpts(0)
	, _topFieldFirst(0)
	, _guessLayoutMax(0)
	, _autoRotate(0)
	, _fixSubDuration(0)
	, _dr1(0)
	, _filters(0)
	, _nbFilters(0)
	, _reinitFilters(0)
	, _hwaccelId(HWACCEL_NONE)
	, _hwaccelDevice(0)
	, _hwaccelOutputFormat(AV_PIX_FMT_NONE)
	, _activeHwaccelId(HWACCEL_NONE)
	, _hwaccelCtx(0)
	, _hwDeviceCtx(NULL)
	, _hwaccelUninit(0)
	, _hwaccelGetBuffer(0)
	, _hwaccelRetrieveData(0)
	, _hwaccelPixFmt(AV_PIX_FMT_NONE)
	, _hwaccelRetrievedPixFmt(AV_PIX_FMT_NONE)
	, _hwFramesCtx(0)
	, _dataSize(0)
	, _nbPackets(0)
	, _framesDecoded(0)
	, _samplesDecoded(0)
	, _dtsBuffer(0)
	, _nbDtsBuffer(0)
	, _gotOutput(0)
{
	_qsvDevice.clear();
	memset(&_framerate, 0, sizeof(_framerate));
	memset(&_prevSub, 0, sizeof(_prevSub));
	memset(&_sub2video, 0, sizeof(_sub2video));
}

FFInputStream::~FFInputStream()
{
	if (_hwaccelDevice)
		av_freep(&_hwaccelDevice);
}

