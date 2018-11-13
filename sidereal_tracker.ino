/* Copyright 2017, 2018 by James Lancaster
 *  
 *  Permission is given to use and modify under the terms of the terms of the 
 *  Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0) 
 *  https://creativecommons.org/licenses/by-nc-sa/4.0/
 *  
 *  So you can share it, modify it, but if you do you can't use it commercially 
 *  and have to share the modified code. (If you don't want to do that, given how simple
 *  it is, contact me.) 
 *  
 * This was developed to power my telescope when calculations weren't working right, so I  
 * developed it so I could adjust and find the right timing in the field. Then went back and
 * figured out what I'd made a mistake in. It was a simple mistake, a 6 for a 5, but baffled me
 * 
 * 
 * So I used a Mega, and It's dual pin row to deal with it, by experimentation. (Got some of my 
 * first GREAT shots with it.) I've ported that to pins on the Uno as well. (Less intuitive layout, 
 * IMO, as the designer, so take that for what it's worth. ;) ) 
 * 
 * On a Mega, it also means that using a CNC shield should be easy, as every normal Uno pin is left open.
 * 
 * 
 *  
 *  
 */


#include <EEPROM.h>


struct telescope_speed {
  unsigned long microseconds_per_step;
};

// STEP 1: Setup pins
// Set these if needed to other pins. This was developed for the mega originally. 
// To activate and change the value, simply take a wire from ground to the appropriate pin.

//If using an Arduino Mega, this allows for a fairly intuitive layout on the double row.
//#define MEGA_LAYOUT

//MUST BE SET FOR IT TO WORK 
#define Stepper_dir 2
#define Stepper_step 3

//None of the rest are strictly needed
#define LED_indicator 13 //Constant on UNO/Mega layouts

//If needed, for example the CNC Shield (https://forum.protoneer.co.nz/viewtopic.php?f=17&t=7092)
//#define Enable_pin 8

#ifndef MEGA_LAYOUT //On Non-Megas

#define Fast_Reverse A4
#define Faster_1 8
#define Faster_10 9
#define Faster_100 10
#define Faster_1000 11
//#define Faster_10000 30

#define Slower_1 7
#define Slower_10 6
#define Slower_100 5
#define Slower_1000 4
//#define Slower_10000 31


// This saves the value, allowing you to use the serial monitor to see it back at your computer
// 
#define Update_EEPROM A5
// This resets the saved value to that of the sketch, IF DONE ON STARTUP. Otherwise no effect.
#define Clear_EEPROM A3

#end //not MEGA_LAYOUT

#ifdef MEGA_LAYOUT

/* 
 *  Using a wire or resistor hooked to the bottom of the double header (Ground) USB is in upper left.
 *  1st Row: Don't touch, it's power
 *  2nd Row: + (left)/- (right) 1 microsecond
 *  3rd Row: +/- 10 microseconds
 *  4th Row: +/- 100 microseconds
 *  5th Row: +/- 1000 microseconds
 *  (6th row could be enabled for larger values) 
 *  
 *  Note to prevent runaway conditions, inputs are only checked every 3 seconds. 
 *  
 *  The top single row has the update and clear EEPROM (Clear only works when powering on) 
 *  
 */

#define Faster_1 22
#define Faster_10 24
#define Faster_100 26
#define Faster_1000 28
//#define Faster_10000 30

#define Slower_1 23
#define Slower_10 25
#define Slower_100 27
#define Slower_1000 29
//#define Slower_10000 31

#define Update_EEPROM 21
#define Clear_EEPROM 20

#endif



//Uncomment DEBUG_STEPS if you want it connected to the serial port. 
//#define DEBUG_STEPS
//#define TIME_PER_STEP


// TIME PER STEP = 1 revolution each sidereal day
// 1 sidereal day =  23 hours, 56 minutes, 4.0916 seconds
// = 86164.0916 seconds

// STEP 2: Input default value. 
/* **** TO CALCULATE MICROSECONDS PER STEP **** 
Find any gear ratios, the biggest is the number of worm Teeth, a few specified below.
Also if using belts or gears, that ratio, it needs to be exact. 

Then Do this: 
Find total steps per day = Worm gear teeth * Gear ratio * Steps per rotation of stepper * Microstep setting
Microseconds per step = 86164.0916 * 1000000 / Total Steps 
Then remove the // in front of the appropriate value. 
*/
// #define YOUR_CALCULATED VALUE

/* Left in for the genesis of the project, and how I messed up, seeing something as 6 turns per hour, and it was 5
 *  */

//For Meade 6600, it looks like there is a 24 hours, and 6 turns per hour, or 1 turn per 10 minutes, so we need 0.1 rpm.
//For now let's just try it as that.
//At 32x microstepping, and 200 steps/rev = 6400 steps per 10 minutes or 600 seconds.
// = 0.09375 seconds = 93.75 milliseconds = 93750 microseconds/step
// 10.66666... steps per second

//Should BE:
//#define default_microseconds_per_step 15625
//Observed:
//Timer from EEPROM:18735
//#define default_microseconds_per_step 18625

//Interestingly, with 120 tooth gears, accounting for the sidreal day, and with 5 turns an hour (both values from the internet, it looks like it should be 18750
//#define default_microseconds_per_step 18750

//Some Common ones. 

//Old EQ1 with 144 worm, 2.05 gear, 200 steps 32 microsteps. 
//#define default_microseconds_per_step 45607

//EQ1, Small Astromasters, 127EQ with 96 worm, 41/20 (2.05 gear), 200 steps, 32 microsteps
//Gear from here: https://www.thingiverse.com/thing:1033336 or here: https://www.thingiverse.com/thing:1923236
#define default_microsteps_per_step 68410

/* Notable mounts (gleaned from internet resources, some of which are wrong, but that's why it's adjustable!)
 *  EQ1: 96 teeth (Apparently some old ones had 144 teeth, or 100 teeth for Orion version?)
 *  EQ2: 100 teeth
 *  EQ3, EQ3-2: 130 teeth
 *  EQ4: 144 teeth
 *  EQ5, Orion Sky View Pro, Vixen Polaris, Meade LXD55/75: 144 teeth (SVP Verified)
 *  Small Astromaster (aka EQ1), Powerseeker 127EQ: 96 teeth (Verified)
 *  Large Astromaster EQ-2: 130 or 136 teeth.
 *  Mizar AR-1: 120 teeth (Meade 6600 uses Meade labeled, but aparently Mizar produced version, Verified)
 *  
 *  
 */

//How long (microseconds) it is held high, generally there's a minimum value. 
#define step_delay 250
//How quickly to reverse.
#define Fast_reverse 250

unsigned long microseconds_per_step = default_microseconds_per_step;
unsigned long next_step = 0;
unsigned long input_check = 0;
void setup() {
  int i;

  pinMode(Stepper_dir, OUTPUT);
  pinMode(Stepper_step, OUTPUT);
  #ifdef Enable_pin
    pinMode(Enable_pin, OUTPUT);
    digitalWrite(Enable_pin, HIGH);
  #endif

  pinMode(LED_indicator, OUTPUT);
  Serial.begin(115200);
  digitalWrite(Stepper_dir, HIGH);
  digitalWrite(LED_indicator, HIGH);
  delay(3);
  digitalWrite(LED_indicator, LOW);
  next_step = micros() + microseconds_per_step;
  pinMode(Fast_reverse, INPUT_PULLUP);
  digitalWrite(Stepper_dir, HIGH);
  for (i = 0; i < 5000; i++) {
    digitalWrite(Stepper_dir, LOW);
    digitalWrite(Stepper_step, HIGH);
    digitalWrite(LED_indicator, HIGH);
    delayMicroseconds(step_delay);
    digitalWrite(Stepper_step, LOW);
    digitalWrite(LED_indicator, LOW);
    delayMicroseconds(step_delay);

  }
    for (i = 0; i < 5000; i++) {
    digitalWrite(Stepper_dir, HIGH);
    digitalWrite(Stepper_step, HIGH);
    digitalWrite(LED_indicator, HIGH);
    delayMicroseconds(step_delay);
    digitalWrite(Stepper_step, LOW);
    digitalWrite(LED_indicator, LOW);
    delayMicroseconds(step_delay);

  }
  pinMode(Faster_1, INPUT_PULLUP);
  pinMode(Faster_10, INPUT_PULLUP);
  pinMode(Faster_100, INPUT_PULLUP);
  pinMode(Faster_1000, INPUT_PULLUP);
  //  pinMode(Faster_10000, INPUT_PULLUP);
  pinMode(Slower_1, INPUT_PULLUP);
  pinMode(Slower_10, INPUT_PULLUP);
  pinMode(Slower_100, INPUT_PULLUP);
  pinMode(Slower_1000, INPUT_PULLUP);
  //  pinMode(Slower_10000, INPUT_PULLUP);
  pinMode(Update_EEPROM, INPUT_PULLUP);
  pinMode(Clear_EEPROM, INPUT_PULLUP);
  delay(1000);
  if (digitalRead(Clear_EEPROM) == LOW) {
    microseconds_per_step = default_microseconds_per_step;
    Serial.print("Timer from Default:");
    Serial.print(microseconds_per_step);
    Serial.print('\n');
    EEPROM.put(0, microseconds_per_step);
    Serial.print("Timer Written:");
    Serial.print(microseconds_per_step);
    Serial.print('\n');
  } else {
    EEPROM.get(0, microseconds_per_step);
    Serial.print("Timer from EEPROM:");
    Serial.print(microseconds_per_step);
    Serial.print('\n');
  }
}

void loop() {
  
  int changed = 0;
  #ifdef DEBUG_STEPS
  Serial.print("Loop Start:\n");
  #endif
  // put your main code here, to run repeatedly:
  if (digitalRead(Fast_reverse) == LOW) { //Internal pullup
    while (digitalRead(Fast_reverse) == LOW) {
      digitalWrite(Stepper_dir, LOW);
      digitalWrite(Stepper_step, HIGH);
      digitalWrite(LED_indicator, HIGH);
      delayMicroseconds(step_delay);
      digitalWrite(Stepper_step, LOW);
      digitalWrite(LED_indicator, LOW);
      delayMicroseconds(step_delay);
      Serial.print("Loop backwards:\n");
    }
  }
  #ifdef DEBUG_STEPS
  Serial.print("Reading inputs Start:\n");
  #endif
  if (input_check < micros()) {
    #ifdef DEBUG_STEPS
    Serial.print("Reading inputs Start:\n");
    #endif
    if (digitalRead(Faster_1) == LOW) {
      microseconds_per_step = microseconds_per_step - 1;
      changed = 1;
    }

    if (digitalRead(Faster_10) == LOW) {
      microseconds_per_step = microseconds_per_step - 10;
      changed = 1;
    }
    if (digitalRead(Faster_100) == LOW) {
      microseconds_per_step = microseconds_per_step - 100;
      changed = 1;
    }
    if (digitalRead(Faster_1000) == LOW) {
      microseconds_per_step = microseconds_per_step - 1000;
      changed = 1;
    }
    if (digitalRead(Slower_1) == LOW) {
      microseconds_per_step = microseconds_per_step + 1;
      changed = 1;
    }
    if (digitalRead(Slower_10) == LOW) {
      microseconds_per_step = microseconds_per_step + 10;
      changed = 1;
    }
    if (digitalRead(Slower_100) == LOW) {
      microseconds_per_step = microseconds_per_step + 100;
      changed = 1;
    }
    if (digitalRead(Slower_1000) == LOW) {
      microseconds_per_step = microseconds_per_step + 1000;
      changed = 1;
    }
    Serial.print("Reading inputs Done:\n");
    input_check = micros() + 3000000;
    if (changed == 1) {
      Serial.print("Timer:");
      Serial.print(microseconds_per_step);
      Serial.print('\n');
    }
    if (digitalRead(Update_EEPROM) == LOW) {
      EEPROM.put(0, microseconds_per_step);
      Serial.print("Timer Written:");
      Serial.print(microseconds_per_step);
      Serial.print('\n');
    }

  }
#ifdef DEBUG_STEPS
  else 

  {
    Serial.print("Reading inputs SKIPPED\n");
  }
#endif
  digitalWrite(Stepper_dir, HIGH);
  while ( micros() < next_step) { // could do something more accurate, but at this rate, but we aren't going that fast...
    delayMicroseconds(4);
#ifdef DEBUG_STEPS
    Serial.print(micros());
    Serial.print('\n');
    Serial.print("Next:");
    Serial.print(next_step); Serial.print('\n'); //* /
#endif
  }

#ifdef DEBUG_STEPS
  Serial.print('\n'); Serial.print('\n');
  Serial.print("STEP");
  Serial.print(microseconds_per_step);
  Serial.print('\n'); Serial.print('\n');
#endif
  //delayMicroseconds(microseconds_per_step - step_delay);
  digitalWrite(Stepper_step, HIGH);
  digitalWrite(LED_indicator, HIGH);
  delayMicroseconds(step_delay);
  digitalWrite(Stepper_step, LOW);
  digitalWrite(LED_indicator, LOW);
  //  Serial.write("S\n");
  next_step = next_step + microseconds_per_step; //At about 70 minutes it overflows... as does this, so there shouldn't even be a hickup
#ifdef DEBUG_STEPS
  Serial.print(next_step);
  Serial.print('\n');
#endif
  delayMicroseconds(step_delay);
}
