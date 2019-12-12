#ifndef _OV5640_H
#define _OV5640_H

#include "stm32f7xx.h"
#include "DriversHW/inc/dcmi.h"
#include <stdbool.h>



#define OV5640_ID               0X5640  
#define OV5640_ADDR        		0X78		
#define OV5640_CHIPIDH          0X300A  	
#define OV5640_CHIPIDL          0X300B  


#define   OV5640_POWER_ON        cameraPwrUp();


#define   JPEG_BUF_SIZE_MAX     400*1024    

#define   QQVGA_160_120    0
#define   QCIF_176_144     1
#define   QVGA_320_240     2
#define   WQVGA_400_240    3
#define   CIF_352_288      4
#define   VGA_640_480      5
#define   SVGA_800_600     6
#define	  CAM_1280x720	   7


#define   XSIZE      480       
#define   YSIZE      272

#define		XSIZE_RGB 	XSIZE
#define		YSIZE_RGB	YSIZE
#define		XSIZE_JPEG	1280
#define		YSIZE_JPEG	720

#define   LCD_GRAM_ADDRESS    SDRAM_DEVICE_ADDR    

#define FOCUS_ZONE_X	16
#define FOCUS_ZONE_Y	12
#define FOCUS_VIRTUAL_VIEW_FINDER_X	80
#define FOCUS_VIRTUAL_VIEW_FINDER_Y	60

#define WINDOW_X_SIZE	100
#define WINDOW_Y_SIZE	70

extern const uint16_t jpeg_size_tbl[][2];
extern const uint32_t jpeg_buf_max_size[];
	
typedef enum OV5640_mode_e
{
	RGB = 0,
	JPEG,
}OV5640_mode_t;

typedef struct ov5640_data_s
{
	struct 
	{
		uint16_t xSize;
		uint16_t ySize;
	}jpeg;
	struct 
	{
		uint16_t xSize;
		uint16_t ySize;
	}rgb;
	struct 
	{
		bool state;
		uint8_t widthLeftH;
		uint8_t widthLeftL;
		uint8_t widthRightH;
		uint8_t widthRightL;
		uint8_t lengthTopH;
		uint8_t lengthTopL;
		uint8_t lengthBottomH;
		uint8_t lengthBottomL;
		uint16_t widthCenter;
		uint16_t lengthCenter;
		uint16_t xSize;
		uint16_t ySize;
	}windowFocus;
	struct
	{
		uint8_t x0Pos;
		uint8_t y0Pos;
		uint8_t xSize;
		uint8_t ySize;
	}focusZone;
	struct
	{
		int8_t brightness;

	}feature;

	
	OV5640_mode_t mode;

}ov5640_data_t;

typedef enum cameraStatePer_e
{
	camReset=0x00,
	camReady=0x01,
	camBusy=0x02,
	camTimeout=0x03,
	camError=0x04,
	camSuspended=0x05,
}cameraStatePer_t;


typedef struct afWindow_s
{
	bool afState;
	uint8_t afWindowWidthLeftH;
	uint8_t afWindowWidthLeftL;
	uint8_t afWindowWidthRightH;
	uint8_t afWindowWidthRightL;
	uint8_t afWindowLengthTopH;
	uint8_t afWindowLengthTopL;
	uint8_t afWindowLengthBottomH;
	uint8_t afWindowLengthBottomL;
}afWindow_t;

uint8_t OV5640_WR_Reg(uint16_t reg,uint8_t data);
uint8_t OV5640_RD_Reg(uint16_t reg);
void OV5640_Exposure(uint8_t exposure);
void OV5640_Light_Mode(uint8_t mode);
void OV5640_Color_Saturation(uint8_t sat);
void OV5640_Brightness(uint8_t bright);
void OV5640_Contrast(uint8_t contrast);
void OV5640_Sharpness(uint8_t sharp);
void OV5640_Special_Effects(uint8_t eft);
void OV5640_Test_Pattern(uint8_t mode);
void OV5640_Flash_Lamp(uint8_t sw);
// void OV5640_jpegMode(uint8_t jpg_size);
void OV5640_rgb565Mode(void);
void cameraRGB(void);

void readDataFromCamera(void);
void stopReadDataFromCamera(void);
//Autofocus
uint8_t OV5640_getAutoFocusWindowState(void);
void OV5640_autoFocusWindowOnOff(bool state);
void OV5640_setAutoFocusWindowPosition(uint16_t x, uint16_t y);
void OV5640_getAutoFocusWindowPosition(uint16_t *x, uint16_t *y);
void OV5640_Focus_Zone(void);
cameraStatePer_t OV5640_Get_Focus_Zone(void);
uint8_t OV5640_OutSize_Set2(uint16_t offx,uint16_t offy,uint16_t width,uint16_t height);
void OV5640_changeCameraMode(OV5640_mode_t mode);


bool getStateCapturingFrame(void);
bool setStateCapturingFrame(bool state);


#endif





















