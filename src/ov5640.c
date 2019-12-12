#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32746g_discovery_sdram.h"
#include "DriversHW/inc/i2c.h"
#include "DriversHW/inc/lcdt.h"
#include "DriversHW/inc/dma2d.h"
#include "ov5640.h"
#include "ov5640cfg.h"
#include "ov5640af.h"		
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "DriversHW/inc/dcmi.h"




extern DMA_HandleTypeDef hdmadcmi;
extern DCMI_HandleTypeDef hdcmi;

//functions declaration"
static cameraStatePer_t OV5640_Init(void);
static void OV5640_JPEG_Mode(void);
static void OV5640_RGB565_Mode(void);
static cameraStatePer_t OV5640_OutSize_Set(uint16_t offx,uint16_t offy,uint16_t width,uint16_t height);
static cameraStatePer_t OV5640_Focus_Init(void);
static uint8_t OV5640_Auto_Focus(void);
static void OV5640_jpegMode(uint8_t jpg_size);
static void OV5640_autoFocusWindowInit(void);

//variable:
ov5640_data_t ov5640;
afWindow_t af;


uint32_t jpeg_buf_size = 0;
uint16_t curline = 0;
uint32_t camLine[XSIZE]={0};
bool CapturingFrame=false;
bool synchroFrame=true;

/////////Testowe///////
uint32_t LineEvent=0;
uint32_t FrameEvent=0;
uint32_t VsyncEvent=0;
uint32_t JPEG_wrongFrame = 0;
uint32_t JPEG_correctFrame = 0;


const uint16_t jpeg_size_tbl[][2]=
{     
	{160, 120},	//QQVGA 19200
	{176, 144},	//QCIF  25344
	{320, 240},	//QVGA  76800
	{400, 240},	//WQVGA 96000
	{352, 288},	//CIF   101376
	{640, 480},	//VGA   307200
	{800, 600},	//SVGA  480000
	{1280, 720},//720
};

const uint32_t jpeg_buf_max_size[]=
{     
	20*1024,	//QQVGA
	30*1024,	//QCIF
	100*1024,	//QVGA
	100*1024,	//WQVGA
	100*1024,	//CIF
	200*1024,	//VGA
	400*1024,	//SVGA 
	700*1024,
};


static cameraStatePer_t OV5640_Init(void)
{ 
	uint16_t i=0;
	uint16_t reg;        

	OV5640_POWER_ON;

  HAL_Delay(30); 

	reg=OV5640_RD_Reg(OV5640_CHIPIDH);
	reg<<=8;
	reg|=OV5640_RD_Reg(OV5640_CHIPIDL);
	if(reg!=OV5640_ID)
	{
		printf("ID: %d \r\n",reg);
		return camError;
	}
	OV5640_WR_Reg(0x3103,0X11);	//system clock from pad, bit[1]
	OV5640_WR_Reg(0X3008,0X82);	
	HAL_Delay(10);

	for(i=0;i<sizeof(ov5640_init_reg_tbl)/4;i++)
	{
		OV5640_WR_Reg(ov5640_init_reg_tbl[i][0],ov5640_init_reg_tbl[i][1]);
	} 

  HAL_Delay(50); 
  // Test for flash light
  OV5640_Flash_Lamp(1);
  HAL_Delay(50); 
  OV5640_Flash_Lamp(0);  

  ov5640.jpeg.xSize = XSIZE_JPEG;
  ov5640.jpeg.ySize = YSIZE_JPEG;
  ov5640.rgb.xSize = XSIZE_RGB;
  ov5640.rgb.ySize = YSIZE_RGB;
               
	return camReady; 	//ok
}

static void OV5640_JPEG_Mode(void) 
{
	uint16_t i=0; 

	for(i=0;i<(sizeof(OV5640_jpeg_reg_tbl)/4);i++)
	{
		OV5640_WR_Reg(OV5640_jpeg_reg_tbl[i][0],OV5640_jpeg_reg_tbl[i][1]);  
	}  
}

static void OV5640_RGB565_Mode(void) 
{
	uint16_t i=0;

	for(i=0;i<(sizeof(ov5640_rgb565_reg_tbl)/4);i++)
	{
		OV5640_WR_Reg(ov5640_rgb565_reg_tbl[i][0],ov5640_rgb565_reg_tbl[i][1]); 
	} 
} 

// set the output size
static cameraStatePer_t OV5640_OutSize_Set(uint16_t offx,uint16_t offy,uint16_t width,uint16_t height)
{ 
    OV5640_WR_Reg(0X3212,0X03);  	

    OV5640_WR_Reg(0x3808,width>>8);	
    OV5640_WR_Reg(0x3809,width&0xff);
    OV5640_WR_Reg(0x380a,height>>8);
    OV5640_WR_Reg(0x380b,height&0xff);

    OV5640_WR_Reg(0x3810,offx>>8);	
    OV5640_WR_Reg(0x3811,offx&0xff);

    OV5640_WR_Reg(0x3812,offy>>8);	
    OV5640_WR_Reg(0x3813,offy&0xff);

    OV5640_WR_Reg(0X3212,0X13);		
    OV5640_WR_Reg(0X3212,0Xa3);	
                                         
    if(width==jpeg_size_tbl[QQVGA_160_120][0] && height==jpeg_size_tbl[QQVGA_160_120][1])
    {
            jpeg_buf_size = jpeg_buf_max_size[QQVGA_160_120];
    }
    else if(width==jpeg_size_tbl[QCIF_176_144][0] && height==jpeg_size_tbl[QCIF_176_144][1])
    {
            jpeg_buf_size = jpeg_buf_max_size[QCIF_176_144];
    }
    else if(width==jpeg_size_tbl[QVGA_320_240][0] && height==jpeg_size_tbl[QVGA_320_240][1])
    {
            jpeg_buf_size = jpeg_buf_max_size[QVGA_320_240];
    }
    else if(width==jpeg_size_tbl[WQVGA_400_240][0] && height==jpeg_size_tbl[WQVGA_400_240][1])
    {
            jpeg_buf_size = jpeg_buf_max_size[WQVGA_400_240];
    }
    else if(width==jpeg_size_tbl[CIF_352_288][0] && height==jpeg_size_tbl[CIF_352_288][1])
    {
            jpeg_buf_size = jpeg_buf_max_size[CIF_352_288];
    }
    else if(width==jpeg_size_tbl[VGA_640_480][0] && height==jpeg_size_tbl[VGA_640_480][1])
    {
            jpeg_buf_size = jpeg_buf_max_size[VGA_640_480];
    }
    else if(width==jpeg_size_tbl[SVGA_800_600][0] && height==jpeg_size_tbl[SVGA_800_600][1])
    {
            jpeg_buf_size = jpeg_buf_max_size[SVGA_800_600];
    }
    else if(width==jpeg_size_tbl[CAM_1280x720][0] && height==jpeg_size_tbl[CAM_1280x720][1])
    {
            jpeg_buf_size = jpeg_buf_max_size[CAM_1280x720];
    }
   
    return camReady; 
}

static cameraStatePer_t OV5640_Focus_Init(void)
{ 
	uint16_t i; 
	uint16_t addr=0x8000;
	uint8_t state=0x8F;
	OV5640_WR_Reg(0x3000, 0x20);	//reset 	 
	for(i=0;i<sizeof(OV5640_AF_Config);i++) 
	{
		OV5640_WR_Reg(addr,OV5640_AF_Config[i]);
		addr++;
	}  
	OV5640_WR_Reg(0x3022,0x00);
	OV5640_WR_Reg(0x3023,0x00);
	OV5640_WR_Reg(0x3024,0x00);
	OV5640_WR_Reg(0x3025,0x00);
	OV5640_WR_Reg(0x3026,0x00);
	OV5640_WR_Reg(0x3027,0x00);
	OV5640_WR_Reg(0x3028,0x00);
	OV5640_WR_Reg(0x3029,0x7f);
	OV5640_WR_Reg(0x3000,0x00); 
	i=0;
	do
	{
		state=OV5640_RD_Reg(0x3029);	
		HAL_Delay(5);
		i++;
		if(i>1000)return camError;
	}while(state!=0x70); 
	return camReady;    
}  

static uint8_t OV5640_Auto_Focus(void)
{
	uint8_t temp=0;   
	uint16_t retry=0; 
	OV5640_WR_Reg(0x3023,0x01);
	OV5640_WR_Reg(0x3022,0x08);
	do 
	{
		temp=OV5640_RD_Reg(0x3023); 
		retry++;
		if(retry>1000)return 2;
		HAL_Delay(5);
	} while(temp!=0x00);   
	OV5640_WR_Reg(0x3023,0x01);
	OV5640_WR_Reg(0x3022,0x04);
	retry=0;
	do 
	{
		temp=OV5640_RD_Reg(0x3023); 
		retry++;
		if(retry>1000)return 2;
		HAL_Delay(5);
	}while(temp!=0x00);
	return 0;
} 

void cameraRGB(void)
{
	OV5640_Init();
	OV5640_RGB565_Mode();	
	OV5640_Focus_Init(); 	
	OV5640_Light_Mode(0);	   //set auto
	OV5640_Color_Saturation(3); //default
	OV5640_Brightness(4);	//default
	OV5640_Contrast(3);     //default
	OV5640_Sharpness(33);	//set auto
	OV5640_Auto_Focus();

	OV5640_rgb565Mode();
	OV5640_autoFocusWindowInit();
}

void cameraJPEG(void)
{
	OV5640_Init();
	OV5640_RGB565_Mode();	
	OV5640_Focus_Init(); 	
	OV5640_Light_Mode(0);	   //set auto
	OV5640_Color_Saturation(3); //default
	OV5640_Brightness(4);	//default
	OV5640_Contrast(3);     //default
	OV5640_Sharpness(33);	//set auto
	OV5640_Auto_Focus();

	OV5640_jpegMode(CAM_1280x720);
}



// Feature:

//Brightness
//     bright:  0 - 8
void OV5640_Brightness(uint8_t bright)
{
	uint8_t brtval;
	if(bright<4)brtval=4-bright;
	else brtval=bright-4;
	OV5640_WR_Reg(0x3212,0x03);	//start group 3
	OV5640_WR_Reg(0x5587,brtval<<4);
	if(bright<4)OV5640_WR_Reg(0x5588,0x09);
	else OV5640_WR_Reg(0x5588,0x01);
	OV5640_WR_Reg(0x3212,0x13); //end group 3
	OV5640_WR_Reg(0x3212,0xa3); //launch group 3
}

const static uint8_t OV5640_SATURATION_TBL[7][6]=
{ 
	{0X0C,0x30,0X3D,0X3E,0X3D,0X01},//-3 
	{0X10,0x3D,0X4D,0X4E,0X4D,0X01},//-2	
	{0X15,0x52,0X66,0X68,0X66,0X02},//-1	
	{0X1A,0x66,0X80,0X82,0X80,0X02},//+0	
	{0X1F,0x7A,0X9A,0X9C,0X9A,0X02},//+1	
	{0X24,0x8F,0XB3,0XB6,0XB3,0X03},//+2
	{0X2B,0xAB,0XD6,0XDA,0XD6,0X04},//+3
}; 

// Color Saturation: 
//   sat:  0 - 6 
void OV5640_Color_Saturation(uint8_t sat)
{ 
	uint8_t i;
	OV5640_WR_Reg(0x3212,0x03);	//start group 3
	OV5640_WR_Reg(0x5381,0x1c);
	OV5640_WR_Reg(0x5382,0x5a);
	OV5640_WR_Reg(0x5383,0x06);
	for(i=0;i<6;i++)  OV5640_WR_Reg(0x5384+i,OV5640_SATURATION_TBL[sat][i]);    
	OV5640_WR_Reg(0x538b, 0x98);
	OV5640_WR_Reg(0x538a, 0x01);
	OV5640_WR_Reg(0x3212, 0x13); //end group 3
	OV5640_WR_Reg(0x3212, 0xa3); //launch group 3	
}

const static uint8_t OV5640_EXPOSURE_TBL[7][6]=
{
    {0x10,0x08,0x10,0x08,0x20,0x10},//-3  
    {0x20,0x18,0x41,0x20,0x18,0x10},//-2
    {0x30,0x28,0x61,0x30,0x28,0x10},//-1 
    {0x38,0x30,0x61,0x38,0x30,0x10},//0  
    {0x40,0x38,0x71,0x40,0x38,0x10},//+1 
    {0x50,0x48,0x90,0x50,0x48,0x20},//+2   
    {0x60,0x58,0xa0,0x60,0x58,0x20},//+3    
};

//exposure: 0 - 6,
void OV5640_Exposure(uint8_t exposure)
{
        OV5640_WR_Reg(0x3212,0x03);	//start group 3
        OV5640_WR_Reg(0x3a0f,OV5640_EXPOSURE_TBL[exposure][0]); 
        OV5640_WR_Reg(0x3a10,OV5640_EXPOSURE_TBL[exposure][1]); 
        OV5640_WR_Reg(0x3a1b,OV5640_EXPOSURE_TBL[exposure][2]); 
        OV5640_WR_Reg(0x3a1e,OV5640_EXPOSURE_TBL[exposure][3]); 
        OV5640_WR_Reg(0x3a11,OV5640_EXPOSURE_TBL[exposure][4]); 
        OV5640_WR_Reg(0x3a1f,OV5640_EXPOSURE_TBL[exposure][5]); 
        OV5640_WR_Reg(0x3212,0x13); //end group 3
        OV5640_WR_Reg(0x3212,0xa3); //launch group 3
}


const static uint8_t OV5640_LIGHTMODE_TBL[5][7]=
{ 
	{0x04,0X00,0X04,0X00,0X04,0X00,0X00},//Auto 
	{0x06,0X1C,0X04,0X00,0X04,0XF3,0X01},//Sunny
	{0x05,0X48,0X04,0X00,0X07,0XCF,0X01},//Office
	{0x06,0X48,0X04,0X00,0X04,0XD3,0X01},//Cloudy
	{0x04,0X10,0X04,0X00,0X08,0X40,0X01},//Home
}; 

// light mode:
//      0: auto
//      1: sunny
//      2: office
//      3: cloudy
//      4: home
void OV5640_Light_Mode(uint8_t mode)
{
	uint8_t i;
	OV5640_WR_Reg(0x3212,0x03);	//start group 3
	for(i=0;i<7;i++)OV5640_WR_Reg(0x3400+i,OV5640_LIGHTMODE_TBL[mode][i]); 
	OV5640_WR_Reg(0x3212,0x13); //end group 3
	OV5640_WR_Reg(0x3212,0xa3); //launch group 3	
}


//Contrast:
//     contrast:  0 - 6
void OV5640_Contrast(uint8_t contrast)
{
	uint8_t reg0val=0X00;
	uint8_t reg1val=0X20;
	switch(contrast)
	{
		case 0://-3
			reg1val=reg0val=0X14;	 	 
			break;	
		case 1://-2
			reg1val=reg0val=0X18; 	 
			break;	
		case 2://-1
			reg1val=reg0val=0X1C;	 
			break;	
		case 4://1
			reg0val=0X10;	 	 
			reg1val=0X24;	 	 
			break;	
		case 5://2
			reg0val=0X18;	 	 
			reg1val=0X28;	 	 
			break;	
		case 6://3
			reg0val=0X1C;	 	 
			reg1val=0X2C;	 	 
			break;	
	} 
	OV5640_WR_Reg(0x3212,0x03); //start group 3
	OV5640_WR_Reg(0x5585,reg0val);
	OV5640_WR_Reg(0x5586,reg1val); 
	OV5640_WR_Reg(0x3212,0x13); //end group 3
	OV5640_WR_Reg(0x3212,0xa3); //launch group 3
}

// Sharpness:
//    sharp: 0 - 33   (0: close , 33: auto , other: Sharpness)

void OV5640_Sharpness(uint8_t sharp)
{
	if(sharp<33)
	{
		OV5640_WR_Reg(0x5308,0x65);
		OV5640_WR_Reg(0x5302,sharp);
	}else	// auto
	{
		OV5640_WR_Reg(0x5308,0x25);
		OV5640_WR_Reg(0x5300,0x08);
		OV5640_WR_Reg(0x5301,0x30);
		OV5640_WR_Reg(0x5302,0x10);
		OV5640_WR_Reg(0x5303,0x00);
		OV5640_WR_Reg(0x5309,0x08);
		OV5640_WR_Reg(0x530a,0x30);
		OV5640_WR_Reg(0x530b,0x04);
		OV5640_WR_Reg(0x530c,0x06);
	}
}

const static uint8_t OV5640_EFFECTS_TBL[7][3]=
{ 
		{0X06,0x40,0X10}, // normal
		{0X1E,0xA0,0X40},
		{0X1E,0x80,0XC0},
		{0X1E,0x80,0X80},
		{0X1E,0x40,0XA0},
		{0X40,0x40,0X10},
		{0X1E,0x60,0X60},
}; 
	    
void OV5640_Special_Effects(uint8_t eft)
{ 
	OV5640_WR_Reg(0x3212,0x03); //start group 3
	OV5640_WR_Reg(0x5580,OV5640_EFFECTS_TBL[eft][0]);
	OV5640_WR_Reg(0x5583,OV5640_EFFECTS_TBL[eft][1]);// sat U
	OV5640_WR_Reg(0x5584,OV5640_EFFECTS_TBL[eft][2]);// sat V
	OV5640_WR_Reg(0x5003,0x08);
	OV5640_WR_Reg(0x3212,0x13); //end group 3
	OV5640_WR_Reg(0x3212,0xa3); //launch group 3
}


///////////////////////

// Flash Lamp
//  sw:  0: off
//       1:  on
void OV5640_Flash_Lamp(uint8_t sw)
{
	OV5640_WR_Reg(0x3016,0X02);
	OV5640_WR_Reg(0x301C,0X02); 
	if(sw)OV5640_WR_Reg(0X3019,0X02); 
	else OV5640_WR_Reg(0X3019,0X00);
} 




uint8_t OV5640_getAutoFocusWindowState(void)
{
	return ((OV5640_RD_Reg(0x5003)>>0x01) & 0x01);
}

void OV5640_autoFocusWindowOnOff(bool state)
{
	if(state == true)
	{
		af.afState = true; OV5640_WR_Reg(0x5003, 0x0A);
	}
	if(state == false)
	{
		af.afState = false; OV5640_WR_Reg(0x5003, 0x08);
	}
}

static void OV5640_autoFocusWindowInit(void)
{
	ov5640.windowFocus.xSize = (FOCUS_ZONE_X * ov5640.rgb.xSize)/FOCUS_VIRTUAL_VIEW_FINDER_X;
	ov5640.windowFocus.ySize = (FOCUS_ZONE_Y * ov5640.rgb.ySize)/FOCUS_VIRTUAL_VIEW_FINDER_Y;
	ov5640.windowFocus.widthLeftH = ((ov5640.rgb.xSize / 2) - (ov5640.windowFocus.xSize / 2)) >> 8;
	ov5640.windowFocus.widthLeftL = ((ov5640.rgb.xSize / 2) - (ov5640.windowFocus.xSize / 2)) & 0xFF;
	ov5640.windowFocus.widthRightH = ((ov5640.rgb.xSize / 2) + (ov5640.windowFocus.xSize / 2)) >> 8;
	ov5640.windowFocus.widthRightL = ((ov5640.rgb.xSize / 2) + (ov5640.windowFocus.xSize / 2)) & 0xFF;

	ov5640.windowFocus.lengthTopH = ((ov5640.rgb.ySize / 2) - (ov5640.windowFocus.ySize / 2)) >> 8;
	ov5640.windowFocus.lengthTopL = ((ov5640.rgb.ySize / 2) - (ov5640.windowFocus.ySize / 2)) & 0xFF;
	ov5640.windowFocus.lengthBottomH = ((ov5640.rgb.ySize / 2) + (ov5640.windowFocus.ySize / 2)) >> 8;
	ov5640.windowFocus.lengthBottomL = ((ov5640.rgb.ySize / 2) + (ov5640.windowFocus.ySize / 2)) & 0xFF;

	ov5640.focusZone.xSize = FOCUS_VIRTUAL_VIEW_FINDER_X;
	ov5640.focusZone.ySize = FOCUS_VIRTUAL_VIEW_FINDER_Y;


	OV5640_WR_Reg(0x5003, 0x0A);
	OV5640_WR_Reg(0x501f, 0x02);
	OV5640_WR_Reg(0x5027, 0x03);
	OV5640_WR_Reg(0x5028, ov5640.windowFocus.widthLeftH);
	OV5640_WR_Reg(0x5029, ov5640.windowFocus.widthLeftL);
	OV5640_WR_Reg(0x502A, ov5640.windowFocus.widthRightH);
	OV5640_WR_Reg(0x502B, ov5640.windowFocus.widthRightL);
	OV5640_WR_Reg(0x502C, ov5640.windowFocus.lengthTopH);
	OV5640_WR_Reg(0x502D, ov5640.windowFocus.lengthTopL);
	OV5640_WR_Reg(0x502E, ov5640.windowFocus.lengthBottomH);
	OV5640_WR_Reg(0x502F, ov5640.windowFocus.lengthBottomL);
	OV5640_WR_Reg(0x5030, 0);
	OV5640_WR_Reg(0x5031, 5);
	OV5640_WR_Reg(0x5032, 0);
	OV5640_WR_Reg(0x5033, 5);
	OV5640_WR_Reg(0x5034, 0XFF);
	OV5640_WR_Reg(0x5035, 0XFF);
	OV5640_WR_Reg(0x5036, 0X0);
}

void OV5640_focusZone(void)
{
	ov5640.focusZone.x0Pos = (ov5640.windowFocus.widthCenter * FOCUS_VIRTUAL_VIEW_FINDER_X) / ov5640.rgb.xSize;
	ov5640.focusZone.x0Pos-=(FOCUS_ZONE_X/2);
	ov5640.focusZone.y0Pos = (ov5640.windowFocus.lengthCenter * FOCUS_VIRTUAL_VIEW_FINDER_Y) / ov5640.rgb.ySize;
	ov5640.focusZone.y0Pos-=(FOCUS_ZONE_Y/2);


	OV5640_WR_Reg(0x3023, 0x01);
	OV5640_WR_Reg(0x3022, 0x08);
	while(OV5640_RD_Reg(0x3023) != 0 );



	OV5640_WR_Reg(0x3024, ov5640.focusZone.x0Pos);
	OV5640_WR_Reg(0x3025, ov5640.focusZone.y0Pos);
	OV5640_WR_Reg(0x3023, 0x01);
	OV5640_WR_Reg(0x3022, 0x81);
	while(OV5640_RD_Reg(0x3023) != 0 );

	OV5640_WR_Reg(0x3023, 0x01);
	OV5640_WR_Reg(0x3022, 0x12);
	while(OV5640_RD_Reg(0x3023) != 0 );


	OV5640_WR_Reg(0x3023, 0x01);
	OV5640_WR_Reg(0x3022, 0x03);
	while(OV5640_RD_Reg(0x3023) != 0 );








//--------------------------------------------------------//
	// OV5640_WR_Reg(0x3023, 0x01);
	// OV5640_WR_Reg(0x3022, 0x12);
	// while(OV5640_RD_Reg(0x3023) != 0 );
	// OV5640_WR_Reg(0x3024, ov5640.focusZone.x0Pos);
	// OV5640_WR_Reg(0x3025, ov5640.focusZone.y0Pos);
	// OV5640_WR_Reg(0x3023, 0x01);
	// OV5640_WR_Reg(0x3022, 0x81);
	// while(OV5640_RD_Reg(0x3023) != 0 );
	// OV5640_WR_Reg(0x3023, 0x01);
	// OV5640_WR_Reg(0x3022, 0x81);
	// while(OV5640_RD_Reg(0x3023) != 0 );
	// OV5640_WR_Reg(0x3023, 0x01);
	// OV5640_WR_Reg(0x3022, 0x12);
	// while(OV5640_RD_Reg(0x3023) != 0 );
}

uint8_t OV5640_getFocusStatus(void)
{
	static uint8_t status[100] = {0};
	uint8_t i=0;

	status[i] = OV5640_RD_Reg(0x3029);
	i++;
	if(status[i-1] == status[i]) { i--; }
	return status;
}

// Get focus result
cameraStatePer_t OV5640_Get_Focus_Zone(void)
{
	uint8_t fzone[5]={0};
	uint8_t i=0;
	uint8_t status;
	OV5640_WR_Reg(0x3023, 0x01);
	OV5640_WR_Reg(0x3022, 0x07);
	while(OV5640_RD_Reg(0x3023) != 0 );
	fzone[0]=OV5640_RD_Reg(0x3024);
	fzone[1]=OV5640_RD_Reg(0x3025);
	fzone[2]=OV5640_RD_Reg(0x3026);
	fzone[3]=OV5640_RD_Reg(0x3027);
	fzone[4]=OV5640_RD_Reg(0x3028);
	
	for(i=0; i<5; i++)
	{
		if(fzone[i] == 0) { status &= ~(1<<i); }
		else { status |= (1<<i); }
	}
	if(status == 0) return camReady;
	else return camError;
}

//Release Auto focus
void OV5640_Release_Auto_Focus(void)
{
	OV5640_WR_Reg(0x3023, 0x01);
	OV5640_WR_Reg(0x3022, 0x08);
	while(OV5640_RD_Reg(0x3023) != 0 );
}

void OV5640_Re_Lunch_Zone(void)
{
	OV5640_WR_Reg(0x3023, 0x01);
	OV5640_WR_Reg(0x3022, 0x12);
	while(OV5640_RD_Reg(0x3023) != 0 );
}

void OV5640_setAutoFocusWindowPosition(uint16_t x, uint16_t y) // x i y koordynaty środka przesunietego
{
	uint16_t xTmp = 0, yTmp = 0;

	// if((x > (ov5640.windowFocus.xSize/2)) || (x < (ov5640.rgb.xSize - (ov5640.windowFocus.xSize/2))) || (y > (ov5640.windowFocus.ySize/2)) || (y > (ov5640.rgb.ySize - (ov5640.windowFocus.ySize/2))) )
	if(((x > (ov5640.windowFocus.xSize/2)) && (x < (ov5640.rgb.xSize - (ov5640.windowFocus.xSize/2))) ) && ((y > (ov5640.windowFocus.ySize/2)) && (y < (ov5640.rgb.ySize - (ov5640.windowFocus.ySize/2))) ) )
	{
		ov5640.windowFocus.widthLeftH = (((x - (ov5640.windowFocus.xSize/2))>>8) & 0xF);
		ov5640.windowFocus.widthLeftL = ((x - (ov5640.windowFocus.xSize/2)) & 0xFF);

		ov5640.windowFocus.widthRightH = (((x + (ov5640.windowFocus.xSize/2))>>8) & 0xF);
		ov5640.windowFocus.widthRightL = ((x + (ov5640.windowFocus.xSize/2)) & 0xFF);

		ov5640.windowFocus.lengthTopH = (((y - ov5640.windowFocus.ySize/2)>>8) & 0x07);
		ov5640.windowFocus.lengthTopL = ((y - ov5640.windowFocus.ySize/2) & 0xFF);

		ov5640.windowFocus.lengthBottomH = (((y + ov5640.windowFocus.ySize/2)>>8) & 0x07);
		ov5640.windowFocus.lengthBottomL = ((y + ov5640.windowFocus.ySize/2) & 0xFF);
	}

	ov5640.windowFocus.widthCenter = x;
	ov5640.windowFocus.lengthCenter = y;

	OV5640_WR_Reg(0x5028, ov5640.windowFocus.widthLeftH);
	OV5640_WR_Reg(0x5029, ov5640.windowFocus.widthLeftL);

	OV5640_WR_Reg(0x502A, ov5640.windowFocus.widthRightH);
	OV5640_WR_Reg(0x502B, ov5640.windowFocus.widthRightL);

	OV5640_WR_Reg(0x502C, ov5640.windowFocus.lengthTopH);
	OV5640_WR_Reg(0x502D, ov5640.windowFocus.lengthTopL);

	OV5640_WR_Reg(0x502E, ov5640.windowFocus.lengthBottomH);
	OV5640_WR_Reg(0x502F, ov5640.windowFocus.lengthBottomL);
}

void OV5640_getAutoFocusWindowPosition(uint16_t *x, uint16_t *y)
{
	ov5640.windowFocus.widthCenter = ( OV5640_RD_Reg(0x5028) & 0x0F) << 8;
	ov5640.windowFocus.widthCenter |= OV5640_RD_Reg(0x5029);
	ov5640.windowFocus.widthCenter += (ov5640.windowFocus.xSize/2);
	*x = ov5640.windowFocus.widthCenter;

	ov5640.windowFocus.lengthCenter = ( OV5640_RD_Reg(0x502C) & 0x07) << 8;
	ov5640.windowFocus.lengthCenter |= OV5640_RD_Reg(0x502D);
	ov5640.windowFocus.lengthCenter += (ov5640.windowFocus.ySize/2);
	*y = ov5640.windowFocus.lengthCenter;
}



static void OV5640_jpegMode(uint8_t jpg_size)
{       
	ov5640.mode = JPEG;  
	stopReadDataFromCamera();    
 	OV5640_JPEG_Mode();	
 	OV5640_OutSize_Set(4, 0, jpeg_size_tbl[jpg_size][0],jpeg_size_tbl[jpg_size][1]);                     

  OV5640_WR_Reg(0x3035,0X41); // slow down OV5640 clocks 
  OV5640_WR_Reg(0x3036,0x80); 
  
 //  OV5640_WR_Reg(0x3035,0X31); // slow down OV5640 clocks ,adapt to the refresh rate of the LCD 
	// OV5640_WR_Reg(0x3036,0x98); 
          
  // __HAL_DCMI_ENABLE_IT(&hdcmi,DCMI_IT_FRAME);      
  
        
}

void OV5640_changeCameraMode(OV5640_mode_t mode)
{
	uint16_t i=0, j=0;
	uint16_t tmpTab[60][2] = {0,0};

	if((sizeof(ov5640_rgb565_reg_tbl)/4) == (sizeof(OV5640_jpeg_reg_tbl)/4))
	{
		if(mode == RGB) 
		{
			OV5640_rgb565Mode();
			OV5640_autoFocusWindowOnOff(true);
			ov5640.mode = RGB;
		}
	else 
		{
			OV5640_jpegMode(CAM_1280x720);
			OV5640_autoFocusWindowOnOff(false);
			ov5640.mode = JPEG;
		}
	}
}


void OV5640_rgb565Mode(void)
{        
	stopReadDataFromCamera();   
  curline = 0; 	
  ov5640.mode = RGB;
  
  OV5640_RGB565_Mode();	
  OV5640_OutSize_Set(4,0, XSIZE, YSIZE);	

	// OV5640_WR_Reg(0x3035,0X31); // slow down OV5640 clocks ,adapt to the refresh rate of the LCD 
 //  OV5640_WR_Reg(0x3036,0x98); 
  // OV5640_WR_Reg(0x3035,0X31); // slow down OV5640 clocks ,adapt to the refresh rate of the LCD 
  // OV5640_WR_Reg(0x3036,0xE0); 
 // OV5640_WR_Reg(0x3035,0X21); // slow down OV5640 clocks ,adapt to the refresh rate of the LCD
 // OV5640_WR_Reg(0x3036,0x98);
  OV5640_WR_Reg(0x3035,0X41); // slow down OV5640 clocks ,adapt to the refresh rate of the LCD
 	OV5640_WR_Reg(0x3036,0x80);

}

void readDataFromCamera(void)
{
	/* Start the Camera capture */
	uint8_t *memAddress;
	memAddress=0xC003FC10;
	if(ov5640.mode == JPEG)
	{
		while(HAL_DCMI_GetState(&hdcmi) != HAL_DCMI_STATE_READY);
   	HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t*)memAddress, jpeg_buf_size );
	}
	else
	{
		while(HAL_DCMI_GetState(&hdcmi) != HAL_DCMI_STATE_READY);
		HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)camLine, XSIZE/2); 
	} 
  	__HAL_DCMI_ENABLE_IT(&hdcmi,DCMI_IT_FRAME);
}

void stopReadDataFromCamera(void)
{
	HAL_DCMI_Stop(&hdcmi);
	synchroFrame = false;
}

bool getStateCapturingFrame(void)
{
	return CapturingFrame;
}

bool setStateCapturingFrame(bool state)
{
	CapturingFrame=state;
	return CapturingFrame;
}

cameraStatePer_t getStateCamPer(void)
{
	return HAL_DCMI_GetState(&hdcmi);
}


// interupt from DCMI
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	FrameEvent++;
	synchroFrame = true;
	curline = 0;
}

void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	uint32_t i=0, j=0;
	uint32_t jpeglen=0, jpgstart=0, jpegstop=0;
    uint8_t  head=0, endJpegFrame=0;
	uint8_t *memAddress;
	static uint8_t test = 0;
	memAddress=0xC0000000;

	if (ov5640.mode == RGB)
	{
		if(synchroFrame == true)
		{
			if(dma2dStartTransmision((uint32_t)&camLine, 0xC0000000+XSIZE*2*curline, XSIZE/2, 1) == DMA2D_OK)
			{
				dma2dPollTransfer(10);
				if(curline < YSIZE-1)  curline++; 
			}
		}
	}
	else //jpeg
	{
		HAL_DCMI_Stop(hdcmi);
		LineEvent++;
		
		for(i=0;i<jpeg_buf_size; i++)//search for 0XFF 0XD8 and 0XFF 0XD9, get size of JPG 
        {
            if((*(memAddress + i)==0XFF)&&(*(memAddress + i+1)==0XD8))
            {
                    jpgstart=i;
                    head=1;	// Already found  FF D8
            }
            if((*(memAddress + i)==0XFF)&&(*(memAddress + i+1)==0XC0)&&head)
            {
            	j=0;
            }
            if((*(memAddress + i)==0XFF)&&(*(memAddress + i+1)==0XDA)&&head)
            {
            	j=0;
            }

            if((*(memAddress + i)==0XFF)&&(*(memAddress + i+1)==0XD9)&&head) //search for FF D9
            {
            	jpegstop=i;
                jpeglen=i-jpgstart+2;
                endJpegFrame = 1;
                break;
            }
        }
      if(	(head == 1) && (endJpegFrame == 1)) { JPEG_correctFrame++; }
    	else { JPEG_wrongFrame++; }
	}
}


void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	uint8_t i=0;
	VsyncEvent++;
	i++;
}

void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
	static uint8_t i=0;
	i++;
	asm("bkpt #1");
}

