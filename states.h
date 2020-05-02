/**
   states.h
   Joshua Brown 2018

   The actual jobs that the sumo robot has to do.
*/
unsigned long now = 0;

/**
 * dragRace
 * Fun name for testing slew rate code.
 * Hypothesis: if you change the commands to the motors at a certain rate, you won't
 * have wheelies, and you'll have maximum traction.  Looks like about 250 uS is the
 * minimum tiem we can wait and modify what the motors are doing.
 * 
 * Not currently used in the code.
 */
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

/**
 * edgeRecover
 * When the edge is detected, this is how I want to get away from the edge.
 * During backing up and turning, we will return if an oppponent detection occurs.
 */
void edgeRecover(int mode) {
  unsigned long edgeRecoverTime = millis(); // get the current time
  int opponentPosition = nothingDetected;
  
  // how long to slam the brakes
  while (millis() < edgeRecoverTime + recoveryBrakeTime) {
    brake(right);
    brake(left);
  }
  
  edgeRecoverTime = millis(); // reset the clock
  
  // how go backwards to get away from the edge
  while (millis() < edgeRecoverTime + recoveryReverseTime) {
    opponentPosition = whereIsOpponent();
    if (opponentPosition == nothingDetected) {
      motor(left, -searchSpeed, braking);
      motor(right, -searchSpeed, braking);
    } else {
      return;
    }
  }


  brake(right);
  brake(left);


  edgeRecoverTime = millis(); // reset the clock
  
  // how long to turn to re-enter Dohyo center area
  while (millis() < edgeRecoverTime + recoveryTurnTime) {
    // BIG TODO: put in opponent detection here so we can break out of this mess!!!!!!!
    opponentPosition = whereIsOpponent();
    if (opponentPosition == nothingDetected) {
      if (mode == left) {
        motor(left, searchSpeed, braking);
        motor(right, -searchSpeed, braking);
      } else {
        motor(left, -searchSpeed, braking);
        motor(right, searchSpeed, braking);
      }
    } else {
      return;
    }

  }


  brake(right);
  brake(left);

  edgeRecoverTime = millis(); // reset the clock
  
  // drive forward a bit to get away from the edge
  while (millis() < edgeRecoverTime + recoveryStraightTime) {
    opponentPosition = whereIsOpponent();
    if (leftEdge() || rightEdge()) {
      break;
    } else if (opponentPosition == nothingDetected ) {
      motor(left, searchSpeed, braking);
      motor(right, searchSpeed, braking);
    } else {
      return;
    }
  }
}

/**
 * fight
 * This is the main loop we are in when when we are engaged in the sumo fight
 */
void fight() {
  unsigned long fightBegan = millis();
  setBlueLed(false); // turn off all lights
  setGreenLed(false);
  int opponentPosition = nothingDetected;
  int priorTracking = left; // todo, consider making priorTracking "none"
  bool rightEdgeDetected = false;
  bool lefttEdgeDetected = false;

  /**
   * Opening Move
     
     todo: jab 19 January 2019 SHOULD HAVE SEVERAL OPTIONS BASED ON startMode
     todo: jab 19 January 2019 ALSO: should move this to it's own function, because this is NOT fighting
  */
  while ((millis() < fightBegan + openingMoveTime) && whereIsOpponent() == nothingDetected) {
    motor(left, searchSpeed, braking);
    motor(right, searchSpeed, braking);
  }
  searchRight(); // todo, should also modify which direction based on startMode

  
  /**
   * main loop
   * we detect if we are seeing opponent, 
   * we call for edge recovery modes if we see the edge without an opponent detection
   * we return if the user presses both buttons at the same time.
   */
  while (1) {
    now = millis();
    /**
       now is an important state marker.  It can time-bound what we are doing here.
       TODO: use now!!!!
    */

    opponentPosition = whereIsOpponent();

    if (getUsrBtn2() && getUsrBtn1()) {
      // TODO: during Sacrament RoboRumble grudge matches, the bot would go back to the wait loop
      // Consider: debounce and noise reject here.
      break;
    }
    // only if opponent sensors are dark, can we deal with Dohyo edge recovery
    if (opponentPosition == nothingDetected) {

      lefttEdgeDetected = leftEdge();
      rightEdgeDetected = rightEdge();

      if (lefttEdgeDetected && rightEdgeDetected) {
        edgeRecover(center);
      } else if (lefttEdgeDetected) {
        edgeRecover(left);
        priorTracking = right;
      } else if (rightEdgeDetected) {
        edgeRecover(right);
        priorTracking = left;

      } else {
        // search the priorTracking
        if (priorTracking == right) {
          searchRight();
        } else {
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
      delay(1000); // not sure what this is doing.  It looks like when we go if we ever have a cancel mode out of countdown.
    }

    if (getUsrBtn1()) {
      while (getUsrBtn1()) {} // wait until button is released
      startModeIncrement();
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
    Serial.print("edgeLeft: ");
    Serial.print(analogRead(edgeLeft));
    Serial.print(" edgeRight: ");
    Serial.println(analogRead(edgeRight));
    currentCount++;
    delay(10);
  }
}

