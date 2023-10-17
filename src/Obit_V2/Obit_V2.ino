#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/wdt.h> //Needed to enable/disable watch dog timer

#include <ThreeWire.h>  
#include <RtcDS1302.h>

#include <SPI.h>
//EPD
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "Ap_29demo.h"  

// ATtiny84 pins
int const buzzer = 6;  //ATtiny84 pin 7 -> buzzer
int const btn = 8; //ATtiny84 pin 5 -> button
bool btn_pressed = false;

uint8_t last_hour = 0;
uint8_t last_minute = 0;
bool half_passed = false;

// Pin 11 (2) - > IO PIN 6
// Pin 6 (7) - > SCLK PIN 7
// Pin 10 (3)  - > CE PIN 5
ThreeWire myWire(2,7,3); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {  
  pinMode(buzzer, INPUT);
  pinMode(btn, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(btn),
                            button_interrupt_handler,
                            FALLING);
  
  ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
  //Power down various bits of hardware to lower power usage  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();
  
  Rtc.Begin();

  if (Rtc.GetIsWriteProtected())
  {
      //Serial.println("RTC was write protected, enabling writing now");
      Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
      //Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  if (Rtc.IsDateTimeValid()){
    RtcDateTime now = Rtc.GetDateTime();
    last_hour = now.Hour();
    last_minute = now.Minute();
    half_passed = now.Minute() > 30;
  }

   // DIsplay configuration
   pinMode(PIN_BUSY, INPUT);  //BUSY
   pinMode(PIN_RES, OUTPUT); //RES 
   pinMode(PIN_DC, OUTPUT); //DC   
   pinMode(PIN_CS, OUTPUT); //CS   
   //SPI
   SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0)); 
   SPI.begin ();
   EPD_HW_Init_Fast();
   RtcDateTime now = Rtc.GetDateTime();
   Display(now, true);
   EPD_DeepSleep();
}

void loop () {
  if (btn_pressed) reset();
  
  RtcDateTime now = Rtc.GetDateTime();
  int hour = now.Hour();
  int minute = now.Minute();
  if (hour != last_hour){
    Song();
    Display(now, true);
    
    half_passed = false;
  }else if(minute == 30 and not half_passed){
    // Half hour
    Buzz();
    Display(now, true);
    half_passed = true;
  }else if (minute != last_minute){
    Display(now, false);
  }
  last_hour = hour;
  last_minute = minute;
  sleep(8);
}

// Low power **********************************************

void sleep(int seconds){
  if (seconds == 0)
  {
    return;
  }
  else if (seconds % 2 != 0)
  {
    seconds--;
    setup_watchdog(6);
  }
  else if (seconds >= 8)
  {
    seconds = seconds - 8;
    setup_watchdog(9);
  }
  else if (seconds >= 4)
  {
    seconds = seconds - 4;
    setup_watchdog(8);
  }
  else if (seconds >= 2)
  {
    seconds = seconds - 2;
    setup_watchdog(7);
  }

  sleep_mode();
  sleep(seconds);
}

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler){

  if (timerPrescaler > 9) timerPrescaler = 9; //Limit incoming amount to legal settings

  byte bb = timerPrescaler & 7; 
  if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCSR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCSR = bb; //Set new watchdog timeout value
  WDTCSR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}

//This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect){
  //watchdog_counter++;
}

// Function ****************************************************

void Display(RtcDateTime now, boolean full_screen_refresh){
  if (full_screen_refresh){
    EPD_HW_Init_Fast();
    //EPD_WhiteScreen_White(); //Clear screen function.
    EPD_SetRAMValue_White(); // to much ?
  }
  int hour = now.Hour();
  int minute = now.Minute();
  EPD_Dis_Part_Time(32,60+32*0,Num[minute%10],         //x-A,y-A,DATA-A
                  32,60+32*1,Num[minute/10],         //x-B,y-B,DATA-B
                  32,60+32*2,gImage_numdot, //x-C,y-C,DATA-C
                  32,60+32*3,Num[hour%10],        //x-D,y-D,DATA-D
                  32,60+32*4,Num[hour/10],32,64); //x-E,y-E,DATA-E,Resolution 32*64
  EPD_DeepSleep();
}

void reset(){
  Buzz();
  EPD_HW_Init();
  EPD_SetRAMValue_White();
  last_hour = 18;
  last_minute = 0;
  half_passed = false;
  if (Rtc.GetIsWriteProtected()) Rtc.SetIsWriteProtected(false);
  if (!Rtc.GetIsRunning()) Rtc.SetIsRunning(true);  

  RtcDateTime reset_date = RtcDateTime("Jan 01 2023", "18:00:00");
  Rtc.SetDateTime(reset_date);

  RtcDateTime now = Rtc.GetDateTime();
  Display(now, true);
  btn_pressed = false;
}
void reset_detailed(){
  Buzz();
  EPD_HW_Init();
  EPD_SetRAMValue_White();
  last_hour = 0;
  last_minute = 0;
  half_passed = false;
  int last_state = HIGH;
  Rtc.Begin();
  if (Rtc.GetIsWriteProtected()) Rtc.SetIsWriteProtected(false);
  if (!Rtc.GetIsRunning()) Rtc.SetIsRunning(true);

  RtcDateTime reset_date = RtcDateTime(2023, 1, 1, last_hour, 0, 0);
  Display(reset_date, true);
  unsigned long start_time = millis();
  int setup_time = 2000;
  while (millis() - start_time < setup_time){
    int state = digitalRead(btn);
    if (state != last_state){
      if (state == LOW){
        start_time = millis();
        last_hour += 1;
        if (last_hour ==24) last_hour = 0;
        reset_date = RtcDateTime(2023, 1, 1, last_hour, 0, 0);
        Display(reset_date, false);
      }
      last_state = state;
    }
  }
  Buzz();
  start_time = millis();
  while (millis() - start_time < setup_time){
    int state = digitalRead(btn);
    if (state != last_state){
      if (state == LOW){
        start_time = millis();
        last_minute += 1;
        if (last_minute == 60) last_minute = 0;
        reset_date = RtcDateTime(2023, 1, 1, last_hour, last_minute, 0);
        Display(reset_date, false);
      }
      last_state = state;
    }
  }
  half_passed = last_minute > 30;
  Buzz();
  Rtc.SetDateTime(reset_date);
  Rtc.SetIsWriteProtected(true);
  btn_pressed = false;
}

void button_interrupt_handler(){
  btn_pressed = true;
}
void Song(){
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  pinMode(buzzer, INPUT);
}

void Buzz (){
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  pinMode(buzzer, INPUT);
}
