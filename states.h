/**
   states.h
   Joshua Brown 2018

   The actual jobs that the sumo robot has to do.
*/
unsigned long now = 0;

void fight() {
  int fightBegan = millis();
  setBlueLed(false); // turn off all lights
  setGreenLed(false);
  int opponentPosition = nothingDetected;
  int priorTracking = left;

  // opening move, N.A. for now!
//  motor(left, searchSpeed, braking);
//  motor(right, searchSpeed, braking);
//  delay(150);

  searchRight();

  while (1) { // loop forever, only reset can stop this.
    now = millis();


    opponentPosition = whereIsOpponent();

    if (opponentPosition == nothingDetected) { // we can allow checking of line sensors
      // recover from ring edge right
      // recover from ring edge left

      // search the priorTracking
      if (priorTracking == right) { // todo revert me!QQQQQQ
        searchRight();
      } else {
        searchLeft();
      }
          
    } else { // we have a detection!!!!!!
      switch (opponentPosition) {
        case 2:
          priorTracking = left;
          motor(right, attackSpeed, braking);
          motor(left, -searchSpeed, braking);
          break;
        case 1:
          priorTracking = left;
          motor(right, attackSpeed, braking);
          motor(left, 0, braking);
          break;
        case 0:
          priorTracking = left;
          motor(right, attackSpeed, braking);
          motor(left, attackSpeed, braking);
          break;
        case -1:
          priorTracking = right;
          motor(left, attackSpeed, braking);
          motor(right, 0, braking);
          break;
        case -2:
          priorTracking = right;
          motor(left, attackSpeed, braking);
          motor(right, -searchSpeed, braking);
          break;
        default: // should never be reached, but OK,
          priorTracking = left;
          searchLeft();
          break;
      }
    }
    delay(10);
  }
}

void countDown() {
  for (int i = 0; i < 5; i++ ) {
    setBlueLed(true);
    setGreenLed(true);
    delay(500);
    setBlueLed(false);
    setGreenLed(false);
    delay(500);
  }
  fight();
  brake(right);
  brake(left);
}

void wait() {
  int currentCount = 0;
  int voltage = 0;
  voltage = getVsense();
  setGreenLed(false);
  setBlueLed(false);

  while (1) {

    if (getUsrBtn2()) {
      while (getUsrBtn2()) {} // wait until button is released
      countDown();
      break;
    }

    if (currentCount > totalCount) {
      currentCount = 0;
    }

    if (voltage > fiftyPercentBattery) { // longBlink
      if (currentCount < longBlink) {
        setBlueLed(true);
      } else {
        setBlueLed(false);
      }

    } else if (voltage > twentyPercentBattery) { // fastBlink

      if (currentCount < shortBlink) {
        setBlueLed(true);
      } else {
        setBlueLed(false);
      }

    } else { // always on
      setBlueLed(true);
    }
    currentCount++;
    delay(10);
  }
}
