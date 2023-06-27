#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/wdt.h> //Needed to enable/disable watch dog timer

/*
 Pin  5  - 0          WHITE LED 1 (PWM)
 Pin  6  - 1          RED LED 2 (PWM)
*/
#define WHITE_LED 0
#define RED_LED 1

int counter = 1;
int fade_size = 18;
int fade_table[]={0,20,23,27,32,37,44,51,60,71,83,98,115,134,158,185,217,255};

void setup(){
  pinMode(RED_LED, OUTPUT);// LED connected to pin 5 which is recognised as pin 0 by arduino
  pinMode(WHITE_LED, OUTPUT);
  ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
  //Power down various bits of hardware to lower power usage  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();
}

void loop(){
  sleep(10);
  flashLed (counter++);
  if(counter>6) counter=1;
}

void dot(){
  digitalWrite(RED_LED, HIGH);
  delay(100);
  digitalWrite(RED_LED, LOW);
  delay(100);
}

void dash(){
  digitalWrite(RED_LED, HIGH);
  delay(200);
  digitalWrite(RED_LED, LOW);
  delay(100);
}

void flashLed (int pattern){
	switch (pattern){
	case 1: //C
	 pinMode(RED_LED, OUTPUT);
   dash();
   dot();
   dash();
   dot();
   pinMode(RED_LED,INPUT);
   break;
   
  case 2: //A
   pinMode(RED_LED, OUTPUT);
   dot();
   dash();
   pinMode(RED_LED,INPUT);
   break;

  case 3: //Y
   pinMode(RED_LED, OUTPUT);
   dash();
   dot();
   dash();
   dash();
   pinMode(RED_LED,INPUT);
   break;
   
  case 4: //O
   pinMode(RED_LED, OUTPUT);
   dash();
   dash();
   dash();
   pinMode(RED_LED,INPUT);
   break;

  case 5: //N
   pinMode(RED_LED, OUTPUT);
   dash();
   dot();
   pinMode(RED_LED,INPUT);
   break;
   
  case 6:
		pinMode(WHITE_LED,OUTPUT);
		// Prefade
	    for(int i=0;i<fade_size;i++){
		  analogWrite(WHITE_LED, fade_table[i]);
			delay(60);
		}
		
		// Blink
		delay(30);
		digitalWrite(WHITE_LED, HIGH);
		delay(100);
		digitalWrite(WHITE_LED, LOW);
		delay(100);
		digitalWrite(WHITE_LED, HIGH);
    delay(100);
    digitalWrite(WHITE_LED, LOW);
    delay(100);
    digitalWrite(WHITE_LED, HIGH);
		delay(30);
		
		// Fade
		for(int i=fade_size-1;i>=0;i--){
			analogWrite(WHITE_LED, fade_table[i]);
			delay(20);
		}
		pinMode(WHITE_LED, INPUT);
    break;
	}
}

void sleep(int seconds){
  if (seconds == 0)
  {
    return;
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
  else if (seconds % 2 != 0)
  {
    seconds--;
    setup_watchdog(6);
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
  WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}

//This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect){
  //watchdog_counter++;
}
