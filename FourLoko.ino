#include "FourLoko_Defines.h"
#include "FourLoko.h"
#include "states.h"

const int arrayHistoryLength = 20;
int arrayHistory[arrayHistoryLength];
int arrayHistory2[arrayHistoryLength];
int loopCount = 0;
int detectTotal = 0;
int detectTotal2 = 0;

int currentRightSpeed = 0;
int currentLeftSpeed = 0;
int targetRightSpeed = 0;
int targetLeftSpeed = 0;
int maximumSlew = 1;

void setup() {
  Serial.begin(115200);
  initPins();
  for (int i = 0; i < arrayHistoryLength; i++) { // initialize the array
    arrayHistory[i] = 0;
  }
  startIrPwm();
  wait();
}


void loop() { // not using the loop for anything
}
