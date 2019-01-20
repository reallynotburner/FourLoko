/**
   states.h
   Joshua Brown 2018

   The actual jobs that the sumo robot has to do.
*/
unsigned long now = 0;

void dragRace() {
  now = millis();
  int currentSpeed = 0;
  int slewRate = 1;

  unsigned long then = now + 200; // when to stop experiment

  motor(left, currentSpeed, coasting);
  motor(right, currentSpeed, coasting);
  while (currentSpeed < 255) { //
    delayMicroseconds(250);
    currentSpeed += slewRate;
    motor(left, currentSpeed, coasting);
    motor(right, currentSpeed, coasting);
  }
  delay(100);

  brake(right);
  brake(left);
}

void edgeRecover(int mode) {
  unsigned long edgeRecoverTime = millis(); // get the current time
  while (millis() < edgeRecoverTime + recoveryBrakeTime) { // how long to slam the brakes
    brake(right);
    brake(left);
  }
  edgeRecoverTime = millis();
  while (millis() < edgeRecoverTime + recoveryReverseTime) { // how long to back it up
    motor(left, -searchSpeed, braking);
    motor(right, -searchSpeed, braking);
  }
  edgeRecoverTime = millis();
  while (millis() < edgeRecoverTime + recoveryTurnTime) { // how long to turn to re-enter battle
    if (mode == left) {
      motor(left, searchSpeed, braking);
      motor(right, -searchSpeed, braking);
    } else {
      motor(left, -searchSpeed, braking);
      motor(right, searchSpeed, braking);
    }
  }
}

void fight() {
  unsigned long fightBegan = millis();
  setBlueLed(false); // turn off all lights
  setGreenLed(false);
  int opponentPosition = nothingDetected;
  int priorTracking = left; // todo, consider making priorTracking "none"
  bool rightEdgeDetected = false;
  bool lefttEdgeDetected = false;

  // opening move, N.A. for now!
  /**
     todo: jab 19 January 2019
     This should be interruptible by opponent detection
     Like this is the opening move UNTIL opponent detection occurs.
     ALSO should have exit based on time "now" instead.  Perhaps priorTracking could have more than two states....
  */

  while ((millis() < fightBegan + openingMoveTime) && whereIsOpponent() == nothingDetected) {
    motor(left, searchSpeed, braking);
    motor(right, searchSpeed, braking);
  }

  searchRight();

  while (1) { // loop forever, only reset can stop this.
    now = millis();
    /**
       now is an important state marker.  It can time-bound what we are doing here.
       that plus "priorTracking" would be very powerful, no?
       TODO: use now!!!!
    */


    opponentPosition = whereIsOpponent();

    if (getUsrBtn2() && getUsrBtn1()) {
      break;
    }
    if (opponentPosition == nothingDetected) { // we can allow checking of line sensors
      // TODO: recover from ring edge right sensor verfied working 19 January 2019
      // TODO: recover from ring edge left sensor verfied working 19 January 2019


      lefttEdgeDetected = leftEdge();
      rightEdgeDetected = rightEdge();

      if (lefttEdgeDetected && rightEdgeDetected) {
        edgeRecover(center);
      } else if (lefttEdgeDetected) {
        edgeRecover(left);
      } else if (rightEdgeDetected) {
        edgeRecover(right);
      } else {
        Serial.print("no edge and no detection, will track ");
        // search the priorTracking
        if (priorTracking == right) { // todo revert me
          Serial.println("right");
          searchRight();
        } else {
          Serial.println("left");
          searchLeft(); // TODO: low priority, make this something variable, seems silly to always have it search left
        }
      }



      /**
         TODO, is there something more sublime we can do with opponentPosition == "nothing detected"?
         perhaps using time "now" we could see how old the last known opponentPosition is.
         perhaps toggle the search direction?
      */

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
    delay(10); // TODO: what if we updated more often?  Like every 250uS?
  }


  brake(right); // before returning to the calling function, stop both motors!
  brake(left);
  return;
  /**
     TODO: consider, wouldn't it be better to go to wait() status instead of simply returning?
     TEST: where does this go when it returns?  I think it would go to countdown, then immediately return to wait's while loop.
     Verify it is so! jab 19 January 2019
  */
}

void countDown() {
  /**
     consider a cancel mode here.  if any user button is pressed, branch back out to wait mode...
     TODO. jab 19 Jan 2019
  */
  brake(left);
  brake(right);
  for (int i = 0; i < 5; i++ ) {
    setBlueLed(true);
    setGreenLed(true);
    delay(500);
    setBlueLed(false);
    setGreenLed(false);
    delay(500);
  }

  fight();
  return;
}

/**
   wait

   simply stay put, receive user button inputs, check on battery status, launch into countdown mode
*/
void wait() {
  int currentCount = 0;
  int voltage = 0;

  // todo: consider adding motor disable here...
  voltage = getVsense();
  setGreenLed(false);
  setBlueLed(false);

  while (1) {

    if (getUsrBtn2()) {
      while (getUsrBtn2()) {} // wait until button is released
      countDown();
      delay(1000); // not sure what this is doing.  Debouncing? That can't be it...
    }
    // todo: consider putting in different statuses, like which opening move to do.


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
    Serial.print("edgeLeft: ");
    Serial.print(analogRead(edgeLeft));
    Serial.print(" edgeRight: ");
    Serial.println(analogRead(edgeRight));
    currentCount++;
    delay(10);
  }
}

