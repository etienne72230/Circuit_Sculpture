#ifndef _DISPLAY_EPD_W21_SPI_
#define _DISPLAY_EPD_W21_SPI_
#include "Arduino.h"

//Attiny84 IO settings
//HSCLK---4
//HMOSI--5
#define PIN_BUSY 0 //PA0 pin 13
#define PIN_RES 1 //PA1 pin 12 //9 PB1 pin 3
#define PIN_DC 2 //PA2 pin 11
#define PIN_CS 3 //PB0 pin 10

//Arduino IO settings
//HSCLK---13
//HMOSI--11
/*#define PIN_BUSY 4
#define PIN_RES 5
#define PIN_DC 6
#define PIN_CS 7*/

#define isEPD_W21_BUSY digitalRead(PIN_BUSY) 
#define EPD_W21_RST_0 digitalWrite(PIN_RES,LOW)
#define EPD_W21_RST_1 digitalWrite(PIN_RES,HIGH)
#define EPD_W21_DC_0  digitalWrite(PIN_DC,LOW)
#define EPD_W21_DC_1  digitalWrite(PIN_DC,HIGH)
#define EPD_W21_CS_0 digitalWrite(PIN_CS,LOW)
#define EPD_W21_CS_1 digitalWrite(PIN_CS,HIGH)

void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);

#endif 
