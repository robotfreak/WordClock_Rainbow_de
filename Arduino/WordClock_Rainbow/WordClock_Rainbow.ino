/*
 
 WORD CLOCK - 8x8 Rainbowduino Desktop Edition 
 Hacked up by Russ Hughes
 
 Adapted and Remixed from:
 
 WORD CLOCK - 8x8 NeoPixel Desktop Edition
 by Andy Doro
 
 Hardware:
 
 - Rainbowduino
 - Tripple Color RGB common Anode Display
 - DS3231 RTC breakout
 - Two pcb mount push buttons & tiny PC board
  
Software:

This code requires the following libraries:
 - Rainbowduino http://www.seeedstudio.com/wiki/images/4/43/Rainbowduino_for_Arduino1.0.zip
 - RTClib https://github.com/mizraith/RTClib

#include <RTC_DS3231.h>
#include <Rainbowduino.h>

 Aword clock using a Rainbowduino and 8x8 RGB LEDs for a color shift effect.
 
 grid pattern

 A T W E N T Y D
 Q U A R T E R Y
 F I V E H A L F
 D P A S T O R O
 F I V E I G H T
 S I X T H R E E
 T W E L E V E N
 F O U R N I N E
  
 Acknowledgements:
  - Thanks Dano for faceplate / 3D models & project inspiration!
  - Thanks Andy Doro for his NeoPixel Word Clock

 */

// include the library code:
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include <Rainbowduino.h>

// define masks for each word. we add them with "bitwise or" to generate a mask for the entire "phrase".
// have to use two different 32-bits numbers, luckily none of the words are spread across top and bottom halves of the NeoMatrix.

#define MFIVE topMask |= 0xF000        // these are in hexadecimal
#define MTEN topMask |= 0x58000000
#define AQUARTER topMask |= 0x80FE0000
#define TWENTY topMask |= 0x7E000000
#define HALF topMask |= 0xF00
#define PAST topMask |= 0x78
#define TO topMask |= 0xC
#define ONE bottomMask |= 0x43
#define TWO bottomMask |= 0xC040
#define THREE bottomMask |= 0x1F0000
#define FOUR bottomMask |= 0xF0
#define FIVE bottomMask |= 0xF0000000
#define SIX bottomMask |= 0xE00000
#define SEVEN bottomMask |= 0x800F00
#define EIGHT bottomMask |= 0x1F000000
#define NINE bottomMask |= 0xF
#define TEN bottomMask |= 0x1010100
#define ELEVEN bottomMask |= 0x3F00
#define TWELVE bottomMask |= 0xF600
#define ANDYDORO topMask |= 0x89010087

// define delays

#define SHIFTDELAY 100	// controls color shifting speed

// define pins for time adjust buttons

#define HOURPIN A0
#define MINPIN A1

// Globals 

uint32_t buffer[64];	// "double buffer" to reduce flicking during updates

unsigned long long topMask;	// 64-bits is too big!
unsigned long bottomMask;	// use two 32-bit numbers to create masks.

RTC_DS3231 RTC; 			// Clock object
DateTime thetime; 			// Current clock time

unsigned long currentMillis;		// used for delay
unsigned long previousMillis = 0;	// used for delay

int j;   // integer for the color shifting effect

void setup() {
	//Serial for debugging
	Serial.begin(9600);
	    
	// time setting pins
	pinMode(HOURPIN, INPUT_PULLUP);
	pinMode(MINPIN, INPUT_PULLUP);
	    
	// start clock
	Wire.begin();
	RTC.begin();
	
	if (!RTC.isrunning()) {
		Serial.println("RTC is NOT running!");
	    RTC.adjust(DateTime(__DATE__, __TIME__)); //  sets the RTC to the date & time this sketch was compiled
	}
	
	Rb.init();
}

void loop() {

	thetime = RTC.now();

	if (digitalRead(HOURPIN) == LOW)  {
    	while(digitalRead(HOURPIN) == LOW) 
    		;
	   	thetime+= 3600;
    	RTC.adjust(thetime);
  	}

  	if (digitalRead(MINPIN) == LOW)  {
   		while(digitalRead(MINPIN) == LOW) 
     		;
		thetime+= 60;
    	RTC.adjust(thetime);
  }
  
  displayTime();
}

