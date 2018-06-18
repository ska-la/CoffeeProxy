//-----
#include <Arduino.h>

#define CYCLENGTH 60000        //--- repeat time (cycle) ---------------
/*#define CYCLECOUNT 14          //--- repeat count ----------------------*/
#define CYCLECOUNT 13          //--- repeat count ----------------------

#define RESET_BUTTON 2         //--- Reset button assignment -----------
#define RELAY_OUT    8         //--- connect to Relay ------------------
#define HALF_PIN     10        //--- 1/2 total capacity when LOW -------
#define CALIBRATE_ME 12        //--- Calibrate button ------------------
#define LED_PIN      13        //--- LED -------------------------------
#define BUZZER_PIN   6         //--- pin assigned to buzzer ------------
#define PROXY_LED    11        //--- LED to READY status ---------------
#define MY_NOTES     14        //--- notes in melody amount ------------
#define DEGREE_RANGE 70        //--- 25 - 95 C ----

/*--------------------------- temperature delta -----------------------*/
//unsigned short m1[CYCLECOUNT] = {10,10,10,5,5,5,5,5,4,4,4,3,3,3};
unsigned short m1[CYCLECOUNT] = {10,10,10,10,10,6,5,4,3,2,2,2,2};

unsigned short curCycle = 0;
unsigned int dutyTime = 0;

unsigned int timeToDegree = 0;  //--- time to degree (msec/C) value ----

unsigned int setOne = 0;        //--- value for full volume ------------
unsigned int setTwo = 0;        //--- value for half volume ------------

//-------------------------- EEPROM addr offset ------------------------
unsigned int addrOne = 0;
unsigned int addrTwo = sizeof(int);

volatile unsigned long startTime = 0;
volatile unsigned long ledTime = 0;

volatile boolean needCalibrate = false;
volatile boolean turnedOn = false;

int melody[MY_NOTES] = {
  NOTE_D4, NOTE_D4, NOTE_F4, NOTE_F4, NOTE_D4, NOTE_D4, NOTE_AS3,
  NOTE_D4, NOTE_D4, NOTE_F4, NOTE_F4, NOTE_D4, NOTE_D4, NOTE_F4
};
int noteDurations[MY_NOTES] = {
  4, 4, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 2
};

void ledBlink( unsigned short leD, unsigned long a ) {
  if ( millis() - ledTime > a ) {
    if ( digitalRead( leD ) == LOW ) {
      digitalWrite( leD, HIGH );
    } else {
      digitalWrite( leD, LOW );
    }
    ledTime = millis();
  }
}

void eepInitialValues() {
  EEPROM.get( addrOne, setOne );
  EEPROM.get( addrTwo, setTwo );
  Serial.println( setOne );
  Serial.println( "---" );
  Serial.println( setTwo );
}

void doReset() {
  while ( millis() - startTime < 3000 ) {
    if ( digitalRead( RESET_BUTTON ) != LOW ) {
      break;
    }
    delay(500);
  }
  if ( digitalRead( RESET_BUTTON ) == LOW ) {
    EEPROM.put( addrOne, int(0) );
    EEPROM.put( addrTwo, int(0) );
  }
}

void doCalibrate() {
static boolean firstPress = false;
static boolean buttonPressed = false;
static boolean calibrDone = false;

  if ( digitalRead( CALIBRATE_ME ) == LOW ) {
    buttonPressed = true;
    delay( 100 );
    if ( firstPress ) {
      timeToDegree = (millis() - startTime) / DEGREE_RANGE;
      digitalWrite( RELAY_OUT, HIGH );
      if ( digitalRead( HALF_PIN ) == LOW ) {
        EEPROM.put(addrTwo, timeToDegree);
      } else {
        EEPROM.put(addrOne, timeToDegree);
      }
      calibrDone = true;
      firstPress = false;
      digitalWrite( LED_PIN, LOW );
      Serial.println( "Stop calibration .. " );
      needCalibrate = false;
    }
  } else {
    if ( buttonPressed && ! calibrDone ) {
      firstPress = true;
      digitalWrite( RELAY_OUT, LOW );
      startTime = millis();
      Serial.println( "Start calibration .. " );
      buttonPressed = false;
    }
  }
  if ( ! calibrDone ) {
    if ( firstPress ) {
      ledBlink( LED_PIN, 1000 );
    } else {
      ledBlink( LED_PIN, 300 );
    }
  }
}

void finalCut() {
int noteDuration = 0;
int pauseBetweenNotes = 0;
  for (int thisNote = 0; thisNote < MY_NOTES; thisNote++) {
    noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);
    pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}

void doCoffee() {
  if ( curCycle < CYCLECOUNT ) {
    if ( dutyTime == 0 ) {
      dutyTime = timeToDegree * m1[curCycle] ;
      digitalWrite( LED_PIN, HIGH );
      digitalWrite( RELAY_OUT, LOW );
      startTime = millis();
      turnedOn = true;
    } else {
      if ( turnedOn ) {
        if ( ( millis() - startTime ) > dutyTime ) {
          digitalWrite( LED_PIN, LOW );
          digitalWrite( RELAY_OUT, HIGH );
          turnedOn = false;
          Serial.println( curCycle + 1 );
        }
      } else {
        if ( ( millis() - startTime ) > CYCLENGTH ) {
          curCycle++;
          dutyTime = 0;
        }
      }
    }
  } else {
    ledBlink(PROXY_LED,1000);
  }
//  finalCut();
}

