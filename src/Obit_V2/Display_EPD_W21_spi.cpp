#include "Display_EPD_W21_spi.h"
#include <SPI.h>

//SPI write byte
void SPI_Write(unsigned char value)
{				   			 
   SPI.transfer(value);
}

//SPI write 2 bytes
void SPI_Write16(unsigned int value)
{                 
   SPI.transfer16(value);
}

//SPI write command
void EPD_W21_WriteCMD(unsigned char command)
{
	EPD_W21_CS_0;
	EPD_W21_DC_0;  // D/C#   0:command  1:data  
	SPI_Write(command);
	EPD_W21_CS_1;
}
//SPI write data
void EPD_W21_WriteDATA(unsigned char datas)
{
	EPD_W21_CS_0;
	EPD_W21_DC_1;  // D/C#   0:command  1:data
	SPI_Write(datas);
	EPD_W21_CS_1;
}

//SPI write 2 bytes data
void EPD_W21_WriteDATA16(unsigned int datas)
{
  EPD_W21_CS_0;
  EPD_W21_DC_1;  // D/C#   0:command  1:data
  SPI_Write16(datas);
  EPD_W21_CS_1;
}
