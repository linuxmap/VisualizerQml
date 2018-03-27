#include "DShow.h"
#include "ffdshowdecoder.h"
#include <dvdmedia.h>
#include <strmif.h>
#include <uuids.h>
#include "libavutil/error.h"

#ifdef FFMPEG_HW_ACC
enum AVPixelFormat FFDShowDecoder::hw_pix_fmt = AV_PIX_FMT_DXVA2_VLD;
#endif
const PixelFormatTag ff_raw_pix_fmt_tags[] = {
	{ AV_PIX_FMT_YUV420P, MKTAG('I', '4', '2', '0') }, /* Planar formats */
{ AV_PIX_FMT_YUV420P, MKTAG('I', 'Y', 'U', 'V') },
{ AV_PIX_FMT_YUV420P, MKTAG('y', 'v', '1', '2') },
{ AV_PIX_FMT_YUV420P, MKTAG('Y', 'V', '1', '2') },
{ AV_PIX_FMT_YUV410P, MKTAG('Y', 'U', 'V', '9') },
{ AV_PIX_FMT_YUV410P, MKTAG('Y', 'V', 'U', '9') },
{ AV_PIX_FMT_YUV411P, MKTAG('Y', '4', '1', 'B') },
{ AV_PIX_FMT_YUV422P, MKTAG('Y', '4', '2', 'B') },
{ AV_PIX_FMT_YUV422P, MKTAG('P', '4', '2', '2') },
{ AV_PIX_FMT_YUV422P, MKTAG('Y', 'V', '1', '6') },
/* yuvjXXX formats are deprecated hacks specific to libav*,
they are identical to yuvXXX  */
{ AV_PIX_FMT_YUVJ420P, MKTAG('I', '4', '2', '0') }, /* Planar formats */
{ AV_PIX_FMT_YUVJ420P, MKTAG('I', 'Y', 'U', 'V') },
{ AV_PIX_FMT_YUVJ420P, MKTAG('Y', 'V', '1', '2') },
{ AV_PIX_FMT_YUVJ422P, MKTAG('Y', '4', '2', 'B') },
{ AV_PIX_FMT_YUVJ422P, MKTAG('P', '4', '2', '2') },
{ AV_PIX_FMT_GRAY8,    MKTAG('Y', '8', '0', '0') },
{ AV_PIX_FMT_GRAY8,    MKTAG('Y', '8', ' ', ' ') },

{ AV_PIX_FMT_YUYV422, MKTAG('Y', 'U', 'Y', '2') }, /* Packed formats */
{ AV_PIX_FMT_YUYV422, MKTAG('Y', '4', '2', '2') },
{ AV_PIX_FMT_YUYV422, MKTAG('V', '4', '2', '2') },
{ AV_PIX_FMT_YUYV422, MKTAG('V', 'Y', 'U', 'Y') },
{ AV_PIX_FMT_YUYV422, MKTAG('Y', 'U', 'N', 'V') },
{ AV_PIX_FMT_YUYV422, MKTAG('Y', 'U', 'Y', 'V') },
{ AV_PIX_FMT_YVYU422, MKTAG('Y', 'V', 'Y', 'U') }, /* Philips */
{ AV_PIX_FMT_UYVY422, MKTAG('U', 'Y', 'V', 'Y') },
{ AV_PIX_FMT_UYVY422, MKTAG('H', 'D', 'Y', 'C') },
{ AV_PIX_FMT_UYVY422, MKTAG('U', 'Y', 'N', 'V') },
{ AV_PIX_FMT_UYVY422, MKTAG('U', 'Y', 'N', 'Y') },
{ AV_PIX_FMT_UYVY422, MKTAG('u', 'y', 'v', '1') },
{ AV_PIX_FMT_UYVY422, MKTAG('2', 'V', 'u', '1') },
{ AV_PIX_FMT_UYVY422, MKTAG('A', 'V', 'R', 'n') }, /* Avid AVI Codec 1:1 */
{ AV_PIX_FMT_UYVY422, MKTAG('A', 'V', '1', 'x') }, /* Avid 1:1x */
{ AV_PIX_FMT_UYVY422, MKTAG('A', 'V', 'u', 'p') },
{ AV_PIX_FMT_UYVY422, MKTAG('V', 'D', 'T', 'Z') }, /* SoftLab-NSK VideoTizer */
{ AV_PIX_FMT_UYVY422, MKTAG('a', 'u', 'v', '2') },
{ AV_PIX_FMT_UYVY422, MKTAG('c', 'y', 'u', 'v') }, /* CYUV is also Creative YUV */
{ AV_PIX_FMT_UYYVYY411, MKTAG('Y', '4', '1', '1') },
{ AV_PIX_FMT_GRAY8,   MKTAG('G', 'R', 'E', 'Y') },
{ AV_PIX_FMT_NV12,    MKTAG('N', 'V', '1', '2') },
{ AV_PIX_FMT_NV21,    MKTAG('N', 'V', '2', '1') },

/* nut */
{ AV_PIX_FMT_RGB555LE, MKTAG('R', 'G', 'B', 15) },
{ AV_PIX_FMT_BGR555LE, MKTAG('B', 'G', 'R', 15) },
{ AV_PIX_FMT_RGB565LE, MKTAG('R', 'G', 'B', 16) },
{ AV_PIX_FMT_BGR565LE, MKTAG('B', 'G', 'R', 16) },
{ AV_PIX_FMT_RGB555BE, MKTAG(15 , 'B', 'G', 'R') },
{ AV_PIX_FMT_BGR555BE, MKTAG(15 , 'R', 'G', 'B') },
{ AV_PIX_FMT_RGB565BE, MKTAG(16 , 'B', 'G', 'R') },
{ AV_PIX_FMT_BGR565BE, MKTAG(16 , 'R', 'G', 'B') },
{ AV_PIX_FMT_RGB444LE, MKTAG('R', 'G', 'B', 12) },
{ AV_PIX_FMT_BGR444LE, MKTAG('B', 'G', 'R', 12) },
{ AV_PIX_FMT_RGB444BE, MKTAG(12 , 'B', 'G', 'R') },
{ AV_PIX_FMT_BGR444BE, MKTAG(12 , 'R', 'G', 'B') },
{ AV_PIX_FMT_RGBA64LE, MKTAG('R', 'B', 'A', 64) },
{ AV_PIX_FMT_BGRA64LE, MKTAG('B', 'R', 'A', 64) },
{ AV_PIX_FMT_RGBA64BE, MKTAG(64 , 'R', 'B', 'A') },
{ AV_PIX_FMT_BGRA64BE, MKTAG(64 , 'B', 'R', 'A') },
{ AV_PIX_FMT_RGBA,     MKTAG('R', 'G', 'B', 'A') },
{ AV_PIX_FMT_RGB0,     MKTAG('R', 'G', 'B',  0) },
{ AV_PIX_FMT_BGRA,     MKTAG('B', 'G', 'R', 'A') },
{ AV_PIX_FMT_BGR0,     MKTAG('B', 'G', 'R',  0) },
{ AV_PIX_FMT_ABGR,     MKTAG('A', 'B', 'G', 'R') },
{ AV_PIX_FMT_0BGR,     MKTAG(0 , 'B', 'G', 'R') },
{ AV_PIX_FMT_ARGB,     MKTAG('A', 'R', 'G', 'B') },
{ AV_PIX_FMT_0RGB,     MKTAG(0 , 'R', 'G', 'B') },
{ AV_PIX_FMT_RGB24,    MKTAG('R', 'G', 'B', 24) },
{ AV_PIX_FMT_BGR24,    MKTAG('B', 'G', 'R', 24) },
{ AV_PIX_FMT_YUV411P,  MKTAG('4', '1', '1', 'P') },
{ AV_PIX_FMT_YUV422P,  MKTAG('4', '2', '2', 'P') },
{ AV_PIX_FMT_YUVJ422P, MKTAG('4', '2', '2', 'P') },
{ AV_PIX_FMT_YUV440P,  MKTAG('4', '4', '0', 'P') },
{ AV_PIX_FMT_YUVJ440P, MKTAG('4', '4', '0', 'P') },
{ AV_PIX_FMT_YUV444P,  MKTAG('4', '4', '4', 'P') },
{ AV_PIX_FMT_YUVJ444P, MKTAG('4', '4', '4', 'P') },
{ AV_PIX_FMT_MONOWHITE,MKTAG('B', '1', 'W', '0') },
{ AV_PIX_FMT_MONOBLACK,MKTAG('B', '0', 'W', '1') },
{ AV_PIX_FMT_BGR8,     MKTAG('B', 'G', 'R',  8) },
{ AV_PIX_FMT_RGB8,     MKTAG('R', 'G', 'B',  8) },
{ AV_PIX_FMT_BGR4,     MKTAG('B', 'G', 'R',  4) },
{ AV_PIX_FMT_RGB4,     MKTAG('R', 'G', 'B',  4) },
{ AV_PIX_FMT_RGB4_BYTE,MKTAG('B', '4', 'B', 'Y') },
{ AV_PIX_FMT_BGR4_BYTE,MKTAG('R', '4', 'B', 'Y') },
{ AV_PIX_FMT_RGB48LE,  MKTAG('R', 'G', 'B', 48) },
{ AV_PIX_FMT_RGB48BE,  MKTAG(48, 'R', 'G', 'B') },
{ AV_PIX_FMT_BGR48LE,  MKTAG('B', 'G', 'R', 48) },
{ AV_PIX_FMT_BGR48BE,  MKTAG(48, 'B', 'G', 'R') },
{ AV_PIX_FMT_GRAY10LE,    MKTAG('Y', '1',  0 , 10) },
{ AV_PIX_FMT_GRAY10BE,    MKTAG(10 ,  0 , '1', 'Y') },
{ AV_PIX_FMT_GRAY12LE,    MKTAG('Y', '1',  0 , 12) },
{ AV_PIX_FMT_GRAY12BE,    MKTAG(12 ,  0 , '1', 'Y') },
{ AV_PIX_FMT_GRAY16LE,    MKTAG('Y', '1',  0 , 16) },
{ AV_PIX_FMT_GRAY16BE,    MKTAG(16 ,  0 , '1', 'Y') },
{ AV_PIX_FMT_YUV420P9LE,  MKTAG('Y', '3', 11 ,  9) },
{ AV_PIX_FMT_YUV420P9BE,  MKTAG(9 , 11 , '3', 'Y') },
{ AV_PIX_FMT_YUV422P9LE,  MKTAG('Y', '3', 10 ,  9) },
{ AV_PIX_FMT_YUV422P9BE,  MKTAG(9 , 10 , '3', 'Y') },
{ AV_PIX_FMT_YUV444P9LE,  MKTAG('Y', '3',  0 ,  9) },
{ AV_PIX_FMT_YUV444P9BE,  MKTAG(9 ,  0 , '3', 'Y') },
{ AV_PIX_FMT_YUV420P10LE, MKTAG('Y', '3', 11 , 10) },
{ AV_PIX_FMT_YUV420P10BE, MKTAG(10 , 11 , '3', 'Y') },
{ AV_PIX_FMT_YUV422P10LE, MKTAG('Y', '3', 10 , 10) },
{ AV_PIX_FMT_YUV422P10BE, MKTAG(10 , 10 , '3', 'Y') },
{ AV_PIX_FMT_YUV444P10LE, MKTAG('Y', '3',  0 , 10) },
{ AV_PIX_FMT_YUV444P10BE, MKTAG(10 ,  0 , '3', 'Y') },
{ AV_PIX_FMT_YUV420P12LE, MKTAG('Y', '3', 11 , 12) },
{ AV_PIX_FMT_YUV420P12BE, MKTAG(12 , 11 , '3', 'Y') },
{ AV_PIX_FMT_YUV422P12LE, MKTAG('Y', '3', 10 , 12) },
{ AV_PIX_FMT_YUV422P12BE, MKTAG(12 , 10 , '3', 'Y') },
{ AV_PIX_FMT_YUV444P12LE, MKTAG('Y', '3',  0 , 12) },
{ AV_PIX_FMT_YUV444P12BE, MKTAG(12 ,  0 , '3', 'Y') },
{ AV_PIX_FMT_YUV420P14LE, MKTAG('Y', '3', 11 , 14) },
{ AV_PIX_FMT_YUV420P14BE, MKTAG(14 , 11 , '3', 'Y') },
{ AV_PIX_FMT_YUV422P14LE, MKTAG('Y', '3', 10 , 14) },
{ AV_PIX_FMT_YUV422P14BE, MKTAG(14 , 10 , '3', 'Y') },
{ AV_PIX_FMT_YUV444P14LE, MKTAG('Y', '3',  0 , 14) },
{ AV_PIX_FMT_YUV444P14BE, MKTAG(14 ,  0 , '3', 'Y') },
{ AV_PIX_FMT_YUV420P16LE, MKTAG('Y', '3', 11 , 16) },
{ AV_PIX_FMT_YUV420P16BE, MKTAG(16 , 11 , '3', 'Y') },
{ AV_PIX_FMT_YUV422P16LE, MKTAG('Y', '3', 10 , 16) },
{ AV_PIX_FMT_YUV422P16BE, MKTAG(16 , 10 , '3', 'Y') },
{ AV_PIX_FMT_YUV444P16LE, MKTAG('Y', '3',  0 , 16) },
{ AV_PIX_FMT_YUV444P16BE, MKTAG(16 ,  0 , '3', 'Y') },
{ AV_PIX_FMT_YUVA420P,    MKTAG('Y', '4', 11 ,  8) },
{ AV_PIX_FMT_YUVA422P,    MKTAG('Y', '4', 10 ,  8) },
{ AV_PIX_FMT_YUVA444P,    MKTAG('Y', '4',  0 ,  8) },
{ AV_PIX_FMT_YA8,         MKTAG('Y', '2',  0 ,  8) },
{ AV_PIX_FMT_PAL8,        MKTAG('P', 'A', 'L',  8) },

{ AV_PIX_FMT_YUVA420P9LE,  MKTAG('Y', '4', 11 ,  9) },
{ AV_PIX_FMT_YUVA420P9BE,  MKTAG(9 , 11 , '4', 'Y') },
{ AV_PIX_FMT_YUVA422P9LE,  MKTAG('Y', '4', 10 ,  9) },
{ AV_PIX_FMT_YUVA422P9BE,  MKTAG(9 , 10 , '4', 'Y') },
{ AV_PIX_FMT_YUVA444P9LE,  MKTAG('Y', '4',  0 ,  9) },
{ AV_PIX_FMT_YUVA444P9BE,  MKTAG(9 ,  0 , '4', 'Y') },
{ AV_PIX_FMT_YUVA420P10LE, MKTAG('Y', '4', 11 , 10) },
{ AV_PIX_FMT_YUVA420P10BE, MKTAG(10 , 11 , '4', 'Y') },
{ AV_PIX_FMT_YUVA422P10LE, MKTAG('Y', '4', 10 , 10) },
{ AV_PIX_FMT_YUVA422P10BE, MKTAG(10 , 10 , '4', 'Y') },
{ AV_PIX_FMT_YUVA444P10LE, MKTAG('Y', '4',  0 , 10) },
{ AV_PIX_FMT_YUVA444P10BE, MKTAG(10 ,  0 , '4', 'Y') },
{ AV_PIX_FMT_YUVA420P16LE, MKTAG('Y', '4', 11 , 16) },
{ AV_PIX_FMT_YUVA420P16BE, MKTAG(16 , 11 , '4', 'Y') },
{ AV_PIX_FMT_YUVA422P16LE, MKTAG('Y', '4', 10 , 16) },
{ AV_PIX_FMT_YUVA422P16BE, MKTAG(16 , 10 , '4', 'Y') },
{ AV_PIX_FMT_YUVA444P16LE, MKTAG('Y', '4',  0 , 16) },
{ AV_PIX_FMT_YUVA444P16BE, MKTAG(16 ,  0 , '4', 'Y') },

{ AV_PIX_FMT_GBRP,         MKTAG('G', '3', 00 ,  8) },
{ AV_PIX_FMT_GBRP9LE,      MKTAG('G', '3', 00 ,  9) },
{ AV_PIX_FMT_GBRP9BE,      MKTAG(9 , 00 , '3', 'G') },
{ AV_PIX_FMT_GBRP10LE,     MKTAG('G', '3', 00 , 10) },
{ AV_PIX_FMT_GBRP10BE,     MKTAG(10 , 00 , '3', 'G') },
{ AV_PIX_FMT_GBRP12LE,     MKTAG('G', '3', 00 , 12) },
{ AV_PIX_FMT_GBRP12BE,     MKTAG(12 , 00 , '3', 'G') },
{ AV_PIX_FMT_GBRP14LE,     MKTAG('G', '3', 00 , 14) },
{ AV_PIX_FMT_GBRP14BE,     MKTAG(14 , 00 , '3', 'G') },
{ AV_PIX_FMT_GBRP16LE,     MKTAG('G', '3', 00 , 16) },
{ AV_PIX_FMT_GBRP16BE,     MKTAG(16 , 00 , '3', 'G') },

{ AV_PIX_FMT_GBRAP,        MKTAG('G', '4', 00 ,  8) },
{ AV_PIX_FMT_GBRAP10LE,    MKTAG('G', '4', 00 , 10) },
{ AV_PIX_FMT_GBRAP10BE,    MKTAG(10 , 00 , '4', 'G') },
{ AV_PIX_FMT_GBRAP12LE,    MKTAG('G', '4', 00 , 12) },
{ AV_PIX_FMT_GBRAP12BE,    MKTAG(12 , 00 , '4', 'G') },
{ AV_PIX_FMT_GBRAP16LE,    MKTAG('G', '4', 00 , 16) },
{ AV_PIX_FMT_GBRAP16BE,    MKTAG(16 , 00 , '4', 'G') },

{ AV_PIX_FMT_XYZ12LE,      MKTAG('X', 'Y', 'Z' , 36) },
{ AV_PIX_FMT_XYZ12BE,      MKTAG(36 , 'Z' , 'Y', 'X') },

{ AV_PIX_FMT_BAYER_BGGR8,    MKTAG(0xBA, 'B', 'G', 8) },
{ AV_PIX_FMT_BAYER_BGGR16LE, MKTAG(0xBA, 'B', 'G', 16) },
{ AV_PIX_FMT_BAYER_BGGR16BE, MKTAG(16  , 'G', 'B', 0xBA) },
{ AV_PIX_FMT_BAYER_RGGB8,    MKTAG(0xBA, 'R', 'G', 8) },
{ AV_PIX_FMT_BAYER_RGGB16LE, MKTAG(0xBA, 'R', 'G', 16) },
{ AV_PIX_FMT_BAYER_RGGB16BE, MKTAG(16  , 'G', 'R', 0xBA) },
{ AV_PIX_FMT_BAYER_GBRG8,    MKTAG(0xBA, 'G', 'B', 8) },
{ AV_PIX_FMT_BAYER_GBRG16LE, MKTAG(0xBA, 'G', 'B', 16) },
{ AV_PIX_FMT_BAYER_GBRG16BE, MKTAG(16,   'B', 'G', 0xBA) },
{ AV_PIX_FMT_BAYER_GRBG8,    MKTAG(0xBA, 'G', 'R', 8) },
{ AV_PIX_FMT_BAYER_GRBG16LE, MKTAG(0xBA, 'G', 'R', 16) },
{ AV_PIX_FMT_BAYER_GRBG16BE, MKTAG(16,   'R', 'G', 0xBA) },

/* quicktime */
{ AV_PIX_FMT_YUV420P, MKTAG('R', '4', '2', '0') }, /* Radius DV YUV PAL */
{ AV_PIX_FMT_YUV411P, MKTAG('R', '4', '1', '1') }, /* Radius DV YUV NTSC */
{ AV_PIX_FMT_UYVY422, MKTAG('2', 'v', 'u', 'y') },
{ AV_PIX_FMT_UYVY422, MKTAG('2', 'V', 'u', 'y') },
{ AV_PIX_FMT_UYVY422, MKTAG('A', 'V', 'U', 'I') }, /* FIXME merge both fields */
{ AV_PIX_FMT_UYVY422, MKTAG('b', 'x', 'y', 'v') },
{ AV_PIX_FMT_YUYV422, MKTAG('y', 'u', 'v', '2') },
{ AV_PIX_FMT_YUYV422, MKTAG('y', 'u', 'v', 's') },
{ AV_PIX_FMT_YUYV422, MKTAG('D', 'V', 'O', 'O') }, /* Digital Voodoo SD 8 Bit */
{ AV_PIX_FMT_RGB555LE,MKTAG('L', '5', '5', '5') },
{ AV_PIX_FMT_RGB565LE,MKTAG('L', '5', '6', '5') },
{ AV_PIX_FMT_RGB565BE,MKTAG('B', '5', '6', '5') },
{ AV_PIX_FMT_BGR24,   MKTAG('2', '4', 'B', 'G') },
{ AV_PIX_FMT_BGR24,   MKTAG('b', 'x', 'b', 'g') },
{ AV_PIX_FMT_BGRA,    MKTAG('B', 'G', 'R', 'A') },
{ AV_PIX_FMT_RGBA,    MKTAG('R', 'G', 'B', 'A') },
{ AV_PIX_FMT_RGB24,   MKTAG('b', 'x', 'r', 'g') },
{ AV_PIX_FMT_ABGR,    MKTAG('A', 'B', 'G', 'R') },
{ AV_PIX_FMT_GRAY16BE,MKTAG('b', '1', '6', 'g') },
{ AV_PIX_FMT_RGB48BE, MKTAG('b', '4', '8', 'r') },
{ AV_PIX_FMT_RGBA64BE,MKTAG('b', '6', '4', 'a') },

/* vlc */
{ AV_PIX_FMT_YUV410P,     MKTAG('I', '4', '1', '0') },
{ AV_PIX_FMT_YUV411P,     MKTAG('I', '4', '1', '1') },
{ AV_PIX_FMT_YUV422P,     MKTAG('I', '4', '2', '2') },
{ AV_PIX_FMT_YUV440P,     MKTAG('I', '4', '4', '0') },
{ AV_PIX_FMT_YUV444P,     MKTAG('I', '4', '4', '4') },
{ AV_PIX_FMT_YUVJ420P,    MKTAG('J', '4', '2', '0') },
{ AV_PIX_FMT_YUVJ422P,    MKTAG('J', '4', '2', '2') },
{ AV_PIX_FMT_YUVJ440P,    MKTAG('J', '4', '4', '0') },
{ AV_PIX_FMT_YUVJ444P,    MKTAG('J', '4', '4', '4') },
{ AV_PIX_FMT_YUVA444P,    MKTAG('Y', 'U', 'V', 'A') },
{ AV_PIX_FMT_YUVA420P,    MKTAG('I', '4', '0', 'A') },
{ AV_PIX_FMT_YUVA422P,    MKTAG('I', '4', '2', 'A') },
{ AV_PIX_FMT_RGB8,        MKTAG('R', 'G', 'B', '2') },
{ AV_PIX_FMT_RGB555LE,    MKTAG('R', 'V', '1', '5') },
{ AV_PIX_FMT_RGB565LE,    MKTAG('R', 'V', '1', '6') },
{ AV_PIX_FMT_BGR24,       MKTAG('R', 'V', '2', '4') },
{ AV_PIX_FMT_BGR0,        MKTAG('R', 'V', '3', '2') },
{ AV_PIX_FMT_RGBA,        MKTAG('A', 'V', '3', '2') },
{ AV_PIX_FMT_YUV420P9LE,  MKTAG('I', '0', '9', 'L') },
{ AV_PIX_FMT_YUV420P9BE,  MKTAG('I', '0', '9', 'B') },
{ AV_PIX_FMT_YUV422P9LE,  MKTAG('I', '2', '9', 'L') },
{ AV_PIX_FMT_YUV422P9BE,  MKTAG('I', '2', '9', 'B') },
{ AV_PIX_FMT_YUV444P9LE,  MKTAG('I', '4', '9', 'L') },
{ AV_PIX_FMT_YUV444P9BE,  MKTAG('I', '4', '9', 'B') },
{ AV_PIX_FMT_YUV420P10LE, MKTAG('I', '0', 'A', 'L') },
{ AV_PIX_FMT_YUV420P10BE, MKTAG('I', '0', 'A', 'B') },
{ AV_PIX_FMT_YUV422P10LE, MKTAG('I', '2', 'A', 'L') },
{ AV_PIX_FMT_YUV422P10BE, MKTAG('I', '2', 'A', 'B') },
{ AV_PIX_FMT_YUV444P10LE, MKTAG('I', '4', 'A', 'L') },
{ AV_PIX_FMT_YUV444P10BE, MKTAG('I', '4', 'A', 'B') },
{ AV_PIX_FMT_YUV420P12LE, MKTAG('I', '0', 'C', 'L') },
{ AV_PIX_FMT_YUV420P12BE, MKTAG('I', '0', 'C', 'B') },
{ AV_PIX_FMT_YUV422P12LE, MKTAG('I', '2', 'C', 'L') },
{ AV_PIX_FMT_YUV422P12BE, MKTAG('I', '2', 'C', 'B') },
{ AV_PIX_FMT_YUV444P12LE, MKTAG('I', '4', 'C', 'L') },
{ AV_PIX_FMT_YUV444P12BE, MKTAG('I', '4', 'C', 'B') },
{ AV_PIX_FMT_YUV420P16LE, MKTAG('I', '0', 'F', 'L') },
{ AV_PIX_FMT_YUV420P16BE, MKTAG('I', '0', 'F', 'B') },
{ AV_PIX_FMT_YUV444P16LE, MKTAG('I', '4', 'F', 'L') },
{ AV_PIX_FMT_YUV444P16BE, MKTAG('I', '4', 'F', 'B') },

/* special */
{ AV_PIX_FMT_RGB565LE,MKTAG(3 ,  0 ,  0 ,  0) }, /* flipped RGB565LE */
{ AV_PIX_FMT_YUV444P, MKTAG('Y', 'V', '2', '4') }, /* YUV444P, swapped UV */

{ AV_PIX_FMT_NONE, 0 },
};
FFDShowDecoder::FFDShowDecoder(QObject *parent)
	: FFObject(parent)
	, _ist(NULL)
	, _hwAccelerator(NULL)
	, _pDecoder(NULL)
	, _pDecoderCtx(NULL)
	, m_pFrame(NULL)
	, m_pPacket(NULL)
	, m_vWidth(0)
	, m_vHeight(0)
	, m_bHwAcc(true)
	, m_accType(/*HWACCEL_QSV*/AV_HWDEVICE_TYPE_DXVA2)
	, m_pHwDeviceCtx(NULL)
	, _pFmtCtx(NULL)
{
	av_register_all();
	//_h264bsfc= av_bitstream_filter_init("h264_mp4toannexb");
}

FFDShowDecoder::~FFDShowDecoder()
{
	deInit();
	//av_bitstream_filter_close(_h264bsfc);
}

enum AVPixelFormat FFDShowDecoder::get_format(AVCodecContext *s, const enum AVPixelFormat *pix_fmts)
{
	FFInputStream *ist = (FFInputStream *)s->opaque;
	const enum AVPixelFormat *p;
	int ret;

	for (p = pix_fmts; *p != -1; p++) {
		const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(*p);
		const FFHWAccelerator::HWAccel *hwaccel;

		if (!(desc->flags & AV_PIX_FMT_FLAG_HWACCEL))
			break;

		hwaccel = FFHWAccelerator::get_hwaccel(*p);
		if (!hwaccel ||
			(ist->_activeHwaccelId && ist->_activeHwaccelId != hwaccel->id) ||
			(ist->_hwaccelId != HWACCEL_AUTO && ist->_hwaccelId != hwaccel->id))
			continue;

		ret = hwaccel->init(s);
		if (ret < 0) {
			if (ist->_hwaccelId == hwaccel->id) {
				av_log(NULL, AV_LOG_FATAL,
					"%s hwaccel requested for input stream #%d:%d, "
					"but cannot be initialized.\n", hwaccel->name,
					ist->_fileIndex, ist->_st->index);
				return AV_PIX_FMT_NONE;
			}
			continue;
		}

		if (ist->_hwFramesCtx) {
			s->hw_frames_ctx = av_buffer_ref(ist->_hwFramesCtx);
			if (!s->hw_frames_ctx)
				return AV_PIX_FMT_NONE;
		}

		ist->_activeHwaccelId = hwaccel->id;
		ist->_hwaccelPixFmt = *p;
		break;
	}

	return *p;
}
int FFDShowDecoder::get_buffer(AVCodecContext *s, AVFrame *frame, int flags)
{
	FFInputStream * ist = (FFInputStream *)s->opaque;
	if (ist->_hwaccelGetBuffer && frame->format == ist->_hwaccelPixFmt)
		return ist->_hwaccelGetBuffer(s, frame, flags);

	return avcodec_default_get_buffer2(s, frame, flags);
}

int FFDShowDecoder::init(AM_MEDIA_TYPE type)
{
	deInit();
	//av_log_set_level(AV_LOG_TRACE);
	_pFmtCtx = avformat_alloc_context();
	if (_pFmtCtx == NULL)
	{
		printf("alloc _pFmtCtx failed\n");
		goto error;
	}
	AVCodecParameters *par;
	AVStream *st;
	int ret = AVERROR(EIO);

	st = avformat_new_stream(_pFmtCtx, NULL);
	if (!st) {
		ret = AVERROR(ENOMEM);
		goto error;
	}
	enum dshowDeviceType devtype = VideoDevice;
	st->id = VideoDevice;
	par = st->codecpar;
	if (devtype == VideoDevice)
	{
		BITMAPINFOHEADER *bih = NULL;
		AVRational time_base;
		if (type.formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER *v = (VIDEOINFOHEADER *)type.pbFormat;
			time_base.num = v->AvgTimePerFrame;
			time_base.den = 10000000;
			bih = &v->bmiHeader;
		}
		else if (type.formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2 *v = (VIDEOINFOHEADER2 *)type.pbFormat;
			time_base.num = v->AvgTimePerFrame;
			time_base.den = 10000000;
			bih = &v->bmiHeader;
		}
		if (!bih) {
			av_log(_pFmtCtx, AV_LOG_ERROR, "Could not get media type.\n");
			goto error;
		}

		st->avg_frame_rate = av_inv_q(time_base);
		st->r_frame_rate = av_inv_q(time_base);

		par->codec_type = AVMEDIA_TYPE_VIDEO;
		par->width = bih->biWidth;
		par->height = bih->biHeight;
		par->codec_tag = bih->biCompression;
		par->format = dshow_pixfmt(bih->biCompression, bih->biBitCount);
		if (bih->biCompression == MKTAG('H', 'D', 'Y', 'C')) {
			av_log(_pFmtCtx, AV_LOG_DEBUG, "attempt to use full range for HDYC...\n");
			par->color_range = AVCOL_RANGE_MPEG; // just in case it needs this...
		}
		if (par->format == AV_PIX_FMT_NONE) {
			if (bih->biCompression == 0x35363248)
			{
				printf("HEVC codec \n");
				par->codec_id = AV_CODEC_ID_HEVC;
			}
			else
			{
				const AVCodecTag *const tags[] = { avformat_get_riff_video_tags(), NULL };
				par->codec_id = av_codec_get_id(tags, bih->biCompression);
			}

			if (par->codec_id == AV_CODEC_ID_NONE) {
				av_log(_pFmtCtx, AV_LOG_ERROR, "Unknown compression type. "
					"Please report type 0x%X.\n", (int)bih->biCompression);
				return AVERROR_PATCHWELCOME;
			}
			par->bits_per_coded_sample = bih->biBitCount;
		}
		else {
			par->codec_id = AV_CODEC_ID_RAWVIDEO;
			if (bih->biCompression == BI_RGB || bih->biCompression == BI_BITFIELDS) {
				par->bits_per_coded_sample = bih->biBitCount;
				par->extradata = (uint8_t*)av_malloc(9 + AV_INPUT_BUFFER_PADDING_SIZE);
				if (par->extradata) {
					par->extradata_size = 9;
					memcpy(par->extradata, "BottomUp", 9);
				}
			}
		}
	}
	else {
		WAVEFORMATEX *fx = NULL;

		if (type.formattype == FORMAT_WaveFormatEx)
		{
			fx = (WAVEFORMATEX *)type.pbFormat;
		}
		if (!fx) {
			av_log(_pFmtCtx, AV_LOG_ERROR, "Could not get media type.\n");
			goto error;
		}

		par->codec_type = AVMEDIA_TYPE_AUDIO;
		//par->format = sample_fmt_bits_per_sample(fx->wBitsPerSample);
		//par->codec_id = waveform_codec_id(par->format);
		par->sample_rate = fx->nSamplesPerSec;
		par->channels = fx->nChannels;
	}
	_ist = new FFInputStream();
	_ist->_decodingNeeded = true;
	_pDecoder = NULL;
	if (m_bHwAcc&&m_accType != AV_HWDEVICE_TYPE_NONE)//m_bHwAcc&&m_accType != HWACCEL_NONE)
	{
		//_ist->_hwaccelId = m_accType;
		switch (/*_ist->_hwaccelId*/m_accType)
		{
		case AV_HWDEVICE_TYPE_QSV://HWACCEL_QSV:
			switch (par->codec_id)
			{
			case AV_CODEC_ID_H264:
				/* initialize the decoder */
				_pDecoder = avcodec_find_decoder_by_name("h264_qsv");
				{
					uint8_t ppssps[] = {
					//0x00,0x00, 0x00, 0x01,0x67, 0x4D, 0x00, 0x32,
					//0x3F, 0x35, 0x40, 0x48, 0x01 ,0x47 ,0x4D ,0xC0,
					//0x40 ,0x40 ,0x40, 0x80,
					//0x00 ,0x00 ,0x00 ,0x01, 0x68, 0x3F ,0x3C, 0x80
						0x01,0x64,0x00,0x1f,0xff,0xe1,0x00,0x1a,0x67,
						0x64,0x00,0x1f,0xac,0xd1,0x80,0x50,0x05,0xbb,
						0x01,0x10,0x00,0x00,0x03,0x00,0x10,0x00,0x00,
						0x03,0x03,0x00,0xf1,0x83,0x11,0xa0,0x01,0x00,
						0x05,0x68,0xe9,0x6b,0x2c,0x8b
				};
					par->extradata = (uint8_t*)av_malloc(sizeof(ppssps) /*+ AV_INPUT_BUFFER_PADDING_SIZE*/);
					if (par->extradata) {
						par->extradata_size = sizeof(ppssps) /*+ AV_INPUT_BUFFER_PADDING_SIZE*/;
						memcpy(par->extradata, ppssps, sizeof(ppssps));
					}
				}

				break;
			case AV_CODEC_ID_HEVC:
				_pDecoder = avcodec_find_decoder_by_name("hevc_qsv");
				break;
			default:
				break;
			}
			_ist->_hwaccelDevice = av_strdup("0");
			break;
		default:break;
		}
	}
	if (!_pDecoder) {
		/* find the video stream information */
		ret = av_find_best_stream(_pFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &_pDecoder, 0);
		if (ret < 0) {
			fprintf(stderr, "Cannot find a video stream in the input file\n");
			return -1;
		}
	}
	_pDecoderCtx = avcodec_alloc_context3(_pDecoder);
	if (NULL == _pDecoderCtx)
	{
		hhtdebug("avcodec_alloc_context3 failed");
		return -1;
	}
	if (avcodec_parameters_to_context(_pDecoderCtx, st->codecpar) < 0)
		return -1;
#if 0
	_ist->_decCtx = _pDecoderCtx;
	_ist->_st = st;
	_ist->_decCtx->opaque = _ist;
	_ist->_decCtx->get_format = get_format;
	_ist->_decCtx->get_buffer2 = get_buffer;
	_ist->_decCtx->thread_safe_callbacks = 1;

	av_opt_set_int(_ist->_decCtx, "refcounted_frames", 1, 0);
	if (_ist->_decCtx->codec_id == AV_CODEC_ID_DVB_SUBTITLE &&
		(_ist->_decodingNeeded & DECODING_FOR_OST)) {
		av_dict_set(&_ist->_decoderOpts, "compute_edt", "1", AV_DICT_DONT_OVERWRITE);
		if (_ist->_decodingNeeded & DECODING_FOR_FILTER)
			av_log(NULL, AV_LOG_WARNING, "Warning using DVB subtitles for filtering and output at the same time is not fully supported, also see -compute_edt [0|1]\n");
	}

	av_dict_set(&_ist->_decoderOpts, "sub_text_format", "ass", AV_DICT_DONT_OVERWRITE);

	/* Useful for subtitles retiming by lavf (FIXME), skipping samples in
	* audio, and video decoders such as cuvid or mediacodec */
	av_codec_set_pkt_timebase(_ist->_decCtx, _ist->_st->time_base);

	if (!av_dict_get(_ist->_decoderOpts, "threads", NULL, 0))
		av_dict_set(&_ist->_decoderOpts, "threads", "auto", 0);
	//_pDecoderCtx->hwaccel_flags |= AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH;
	_hwAccelerator = new FFHWAccelerator(_ist);
	ret = _hwAccelerator->hwDeviceSetupForDecode();
	if (ret < 0) {
		printf("Device setup failed for "
			"decoder on input stream #%d:%d ",
			_ist->_fileIndex, _ist->_st->index);
		return ret;
	}
#endif
#if 1
	if (m_bHwAcc)
	{
		_ist->_decCtx = _pDecoderCtx;
		_pDecoderCtx->pix_fmt = find_fmt_by_hw_type(m_accType);
		//_pDecoderCtx->hwaccel_flags |= AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH;
		hw_pix_fmt = find_fmt_by_hw_type(m_accType);
		if (hw_pix_fmt == -1) {
			printf("Find hw pixel format failed");
		}
		_pDecoderCtx->get_format = FFDShowDecoder::get_hw_format;
		_pDecoderCtx->opaque = _ist;
		if (hw_decoder_init(_pDecoderCtx, m_accType) < 0)
		{
			printf("hw_decoder_init failed");
		}
	}
#endif
	
	//_pDecoderCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	if (avcodec_open2(_pDecoderCtx, _pDecoder, NULL) < 0)
	{
		printf("Could not open codec.\n");
		return false;
	}
	if (!m_pFrame)
	{
		m_pFrame = av_frame_alloc();
		if (NULL == m_pFrame)
		{
			hhtdebug("av_frame_alloc failed");
			return -1;
		}

	}
	if (!m_pPacket)
	{
		m_pPacket = av_packet_alloc();
		if (NULL == m_pPacket)
		{
			hhtdebug("av_packet_alloc failed");
			return -1;
		}
		av_init_packet(m_pPacket);

	}
	return 0;
error:
	return -1;
}

void FFDShowDecoder::changeResol(long width, long height)
{

}

bool FFDShowDecoder::decode(unsigned char *buff, int buffLen)
{
	return decode(0, buff, buffLen);
}

bool FFDShowDecoder::decode(double SampleTime, unsigned char *buff, int buffLen)
{
	int ret;
	m_pPacket->dts = SampleTime;
	m_pPacket->pts = SampleTime;

	m_pPacket->data = buff;
	m_pPacket->size = buffLen;

//	ret = av_bitstream_filter_filter(_h264bsfc, _pDecoderCtx, NULL, &m_pPacket->data, &m_pPacket->size, m_pPacket->data, m_pPacket->size, 0);

	ret = avcodec_send_packet(_pDecoderCtx, m_pPacket);
	if (ret < 0)
	{
		printf("Error during decoding\n");
		return false;
	}
	ret = avcodec_receive_frame(_pDecoderCtx, m_pFrame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
		printf("Error during receive frame\n");
		return false;
	}
	else if (ret < 0) {
		printf("Error while decoding\n");
		return false;
	}
	static int i;
	if (!i)
	{
		i++;
		printf("Format after decode is:%d\n", m_pFrame->format);
	}
	return true;
}
bool FFDShowDecoder::decode(double SampleTime, unsigned char *buff, int buffLen, bool isIFrame)
{
	int ret;
	m_pPacket->dts = SampleTime;
	m_pPacket->pts = SampleTime;

	m_pPacket->data = buff;
	m_pPacket->size = buffLen;

	//ret = av_bitstream_filter_filter(_h264bsfc, _pDecoderCtx, NULL, &m_pPacket->data, &m_pPacket->size, m_pPacket->data, m_pPacket->size, isIFrame);
	_pDecoderCtx->profile = FF_PROFILE_HEVC_MAIN;
	ret = avcodec_send_packet(_pDecoderCtx, m_pPacket);
	if (ret < 0)
	{
		printf("Error during decoding\n");
		return false;
	}

	ret = avcodec_receive_frame(_pDecoderCtx, m_pFrame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
		printf("Error during receive frame\n");
		return false;
	}
	else if (ret < 0) {
		printf("Error while decoding\n");
		return false;
	}

	static int i = printf("Format after decode is:%d\n", m_pFrame->format);
	Q_UNUSED(i);
	return true;
}



int FFDShowDecoder::getAVFrame(AVFrame *frame)
{
	av_frame_move_ref(frame, m_pFrame);
	return 0;
}

#ifdef FFMPEG_HW_ACC
enum AVPixelFormat FFDShowDecoder::get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
	const enum AVPixelFormat *p;

	for (p = pix_fmts; *p != -1; p++) {
		if (*p == hw_pix_fmt)
		{
			if (*pix_fmts == AV_PIX_FMT_QSV) {
				FFInputStream *ist = (FFInputStream *)ctx->opaque;
				AVHWFramesContext  *frames_ctx;
				AVQSVFramesContext *frames_hwctx;
				int ret;

				/* create a pool of surfaces to be used by the decoder */
				ctx->hw_frames_ctx = av_hwframe_ctx_alloc(ist->_decCtx->hw_device_ctx);
				if (!ctx->hw_frames_ctx)
					return AV_PIX_FMT_NONE;
				frames_ctx = (AVHWFramesContext*)ctx->hw_frames_ctx->data;
				frames_hwctx = (AVQSVFramesContext *)frames_ctx->hwctx;

				frames_ctx->format = AV_PIX_FMT_QSV;
				frames_ctx->sw_format = ctx->sw_pix_fmt;
				frames_ctx->width = FFALIGN(ctx->coded_width, 32);
				frames_ctx->height = FFALIGN(ctx->coded_height, 32);
				frames_ctx->initial_pool_size = 32;

				frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

				ret = av_hwframe_ctx_init(ctx->hw_frames_ctx);
				if (ret < 0)
					return AV_PIX_FMT_NONE;
			}
			return *p;
		}
	}
	fprintf(stderr, "Failed to get HW surface format.\n");
	return AV_PIX_FMT_NONE;
}
#endif
void FFDShowDecoder::deInit()
{
	if (_ist) {
		if (_ist->_hwaccelUninit)
			_ist->_hwaccelUninit(_ist->_decCtx);
		delete _ist;
		_ist = NULL;
	}
	if (m_pPacket)
		av_packet_free(&m_pPacket);
	if (m_pFrame)
		av_frame_free(&m_pFrame);
	if (_pDecoderCtx)
		avcodec_free_context(&_pDecoderCtx);
	if (_pFmtCtx)
		avformat_free_context(_pFmtCtx);
}

int FFDShowDecoder::printCtxOption()
{
	const AVOption *pOption = NULL;
	uint8_t   *val;
	while (pOption = av_opt_next(_pDecoderCtx, pOption))
	{
		if (av_opt_get(_pDecoderCtx, pOption->name, 0, &val))
		{
			if (val)
			{
				printf("%s = %s\n", pOption->name, val);
			}
			av_freep(&val);
		}
	}
	return true;
}

int FFDShowDecoder::hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
	int err = 0;

	if ((err = av_hwdevice_ctx_create(&m_pHwDeviceCtx, type, "auto", NULL, 0)) < 0)
	{
		fprintf(stderr, "Failed to create specified HW device.\n");
		return err;

	}
	ctx->hw_device_ctx = av_buffer_ref(m_pHwDeviceCtx);

	return err;
}

enum AVPixelFormat FFDShowDecoder::find_fmt_by_hw_type(const enum AVHWDeviceType type)
{
	enum AVPixelFormat fmt;

	switch (type) {
	case AV_HWDEVICE_TYPE_VAAPI:
		fmt = AV_PIX_FMT_VAAPI;
		break;
	case AV_HWDEVICE_TYPE_DXVA2:
		fmt = AV_PIX_FMT_DXVA2_VLD;
		break;
	case AV_HWDEVICE_TYPE_D3D11VA:
		fmt = AV_PIX_FMT_D3D11;
		break;
	case AV_HWDEVICE_TYPE_VDPAU:
		fmt = AV_PIX_FMT_VDPAU;
		break;
	case AV_HWDEVICE_TYPE_QSV:
		fmt = AV_PIX_FMT_QSV;
		break;
	case AV_HWDEVICE_TYPE_CUDA:
		fmt = AV_PIX_FMT_CUDA;
	default:
		fmt = AV_PIX_FMT_NONE;
		break;

	}

	return fmt;
}
enum AVPixelFormat FFDShowDecoder::dshow_pixfmt(DWORD biCompression, WORD biBitCount)
{
	switch (biCompression) {
	case BI_BITFIELDS:
	case BI_RGB:
		switch (biBitCount) { /* 1-8 are untested */
		case 1:
			return AV_PIX_FMT_MONOWHITE;
		case 4:
			return AV_PIX_FMT_RGB4;
		case 8:
			return AV_PIX_FMT_RGB8;
		case 16:
			return AV_PIX_FMT_RGB555;
		case 24:
			return AV_PIX_FMT_BGR24;
		case 32:
			return AV_PIX_FMT_0RGB32;
		}
	}
	return FFDShowDecoder::avpriv_find_pix_fmt(ff_raw_pix_fmt_tags, biCompression); // all others
}
enum AVPixelFormat FFDShowDecoder::avpriv_find_pix_fmt(const PixelFormatTag *tags, unsigned int fourcc)
{
	while (tags->pix_fmt >= 0) {
		if (tags->fourcc == fourcc)
			return tags->pix_fmt;
		tags++;
	}
	return AV_PIX_FMT_NONE;
}
#if 0
void FFDShowDecoder::avpriv_set_pts_info(AVStream *s, int pts_wrap_bits,
	unsigned int pts_num, unsigned int pts_den)
{
	AVRational new_tb;
	if (av_reduce(&new_tb.num, &new_tb.den, pts_num, pts_den, INT_MAX)) {
		if (new_tb.num != pts_num)
			av_log(NULL, AV_LOG_DEBUG,
				"st:%d removing common factor %d from timebase\n",
				s->index, pts_num / new_tb.num);
	}
	else
		av_log(NULL, AV_LOG_WARNING,
			"st:%d has too large timebase, reducing\n", s->index);

	if (new_tb.num <= 0 || new_tb.den <= 0) {
		av_log(NULL, AV_LOG_ERROR,
			"Ignoring attempt to set invalid timebase %d/%d for st:%d\n",
			new_tb.num, new_tb.den,
			s->index);
		return;
	}
	s->time_base = new_tb;
#if FF_API_LAVF_AVCTX
	FF_DISABLE_DEPRECATION_WARNINGS
		av_codec_set_pkt_timebase(s->codec, new_tb);
	FF_ENABLE_DEPRECATION_WARNINGS
#endif
		av_codec_set_pkt_timebase(s->internal->avctx, new_tb);
	s->pts_wrap_bits = pts_wrap_bits;
}
#endif


