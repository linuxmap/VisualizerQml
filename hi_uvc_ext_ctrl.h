#ifndef __HI_UVC_EXT_UNIT_CTRL_H__
#define __HI_UVC_EXT_UNIT_CTRL_H__

#include <initguid.h>

/******************* H264 extension unit start  *****************/

#define UNIT_XU_H264 (10)             /*ext unit id*/
#define GUID_UVCX_H264_XU {0x41, 0x76, 0x9E, 0xA2, 0x04, 0xDE, 0xE3, 0x47, \
				0x8B, 0x2B, 0xF4, 0x34, 0x1A, 0xFF, 0x00, 0x3B}
DEFINE_GUID(UVCEXTENUNIT_HICAMERA_H264,
	0xA29E7641, 0xDE04, 0x47E3, 0x2B, 0x8B, 0x3B, 0x00, 0xFF, 0x1A, 0x34, 0xF4);

#define UVCX_PICTURE_TYPE_CONTROL 0x09

/******************* H264 extension unit end    *****************/


/************ HICAMERA extension unit start *********************/

#define UNIT_XU_HICAMERA (0x11)         /*ext unit id*/
//{9A1E7291-6843-4683-6D92-39BC7906EE49}
#define UVC_GUID_HI_CAMERA {0x91, 0x72, 0x1e, 0x9a, 0x43, 0x68, 0x83, 0x46, \
				0x6d, 0x92, 0x39, 0xbc, 0x79, 0x06, 0xee, 0x49} /*guid*/

//DEFINE_GUID(UVCEXTENUNIT_HICAMERA_CONTROL, \
//	0x9A1E7291, 0x6843, 0x4683, 0x6D, 0x92, 0x39, 0xBC, 0x79, 0x06, 0xEE, 0x49);

/* CTRL id follow*/
#define HI_XUID_MOVE_WINDOW 			0x01/*cmd 0-stop 1-mvUp 2-mvDown 3-mvLeft 4-mvRight*/
#define HI_XUID_SET_VOLUME_ABSOLUTE 	0x02/*min:0 max:60*/
#define HI_XUID_SET_VOLUME_RELATIVE 	0x03/* 0:inc !0:dec */
#define HI_XUID_SET_IMAGE_ROTATE		0x04/* 0:Clockwise90 1:Anticlockwise90*/
#define HI_XUID_SET_LIGHT_STATUS		0x05/* 0:light off !0:light on*/
#define HI_XUID_SET_IMAGE_FLIP			0x06/* reverse both  any value*/
#define HI_XUID_SET_IMAGE_MIRROR		0x07/* reverse both  any value*/

/************ HICAMERA extension unit end   *********************/


#endif //endif __HI_UVC_EXT_UNIT_CTRL_H__