#include <EEPROM.h>
#include "pitches.h"
#include "coffee_extra.h"

void setup() {
  Serial.begin(115200);
  pinMode( LED_PIN, OUTPUT );
  pinMode( RELAY_OUT, OUTPUT );
  pinMode( PROXY_LED, OUTPUT );
  digitalWrite( RELAY_OUT, HIGH );      //--- because low signal drives a relay ---
  pinMode( CALIBRATE_ME, INPUT_PULLUP );
  pinMode( RESET_BUTTON, INPUT_PULLUP );
  pinMode( HALF_PIN, INPUT_PULLUP );
  analogReference( DEFAULT );           //--- define a reference voltage for ADC --

  if ( digitalRead(RESET_BUTTON) == LOW ) {
    startTime = millis();
    doReset();
  }

  eepInitialValues();
  if ( setOne == 0 && ( digitalRead( HALF_PIN ) == HIGH ) ) {
    curCycle = 255;
    needCalibrate = true;
  }
  if ( setTwo == 0 && ( digitalRead( HALF_PIN ) == LOW ) ) {
    curCycle = 255;
    needCalibrate = true;
  }

  if ( curCycle < CYCLECOUNT ) {
    if ( digitalRead( HALF_PIN ) == HIGH ) {
      timeToDegree = setOne;
    } else {
      timeToDegree = setTwo;
    }
  }
}

void loop() {

  if ( needCalibrate ) {
    doCalibrate();
  } else {
    doCoffee();
  }
  if ( showFinish ) {
    ledBlink(PROXY_LED,1000);
  }

}


