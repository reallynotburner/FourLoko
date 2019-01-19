#include "FourLoko_Defines.h"
#include "FourLoko.h"
#include "states.h"

void setup() {
  Serial.begin(115200);
  initPins();
  startIrPwm();
  wait();
}


void loop() {
}
