/**
   FourLoko.h
   Joshua Brown 2017

   helper functions for FourLoko, my minisumo bot
*/

int leftEdge() {
  return analogRead(edgeLeft);
}
int rightEdge() {
  return analogRead(edgeRight);
}

bool getUsrBtn1() {
  int result = false;
  if (digitalRead(usrBtn1) == 0) {
    result = true;
  }
  return result;
}

bool getUsrBtn2() {
  int result = false;
  if (digitalRead(usrBtn2) == 0) {
    result = true;
  }
  return result;
}


void initIrPwm() {
  TCCR1A = B01000000;
  TCCR1B = B00001001;
  OCR1A = 0;
}

void stopIrPwm() {
  OCR1A = 0;
}

void startIrPwm() {
  OCR1A = ocrConstant;
}

void printOpponentSensors(int *detectionArray) {
  for (int i = 0; i < 5; i++) { // todo: is there a length property I can extract to make this dynamic?
    Serial.print(detectionArray[i]);
    Serial.print(", ");
  }
  Serial.println("");
}

void getOpponentSensors(int *detectionArray) {
  detectionArray[0] = !digitalRead(fLefttPx);
  detectionArray[1] = !digitalRead(leftPx);
  detectionArray[2] = !digitalRead(ctrPx);
  detectionArray[3] = !digitalRead(rightPx);
  detectionArray[4] = !digitalRead(fRightPx);
}

int whereIsOpponent() { // 0 is straight ahead, + is to the left - is to the right
  int result = 0; // which direction did is the opponent, 0 is straight ahead, nothingDetected is no detection, -2 is far right, 2 is far left.
  int detectionArray[5]; // declare array of current opponennt sensor states
  int total = 0;
  int accum = 0;

  getOpponentSensors(&detectionArray[0]);
  //  printOpponentSensors(detectionArray);

  // find the centroid of the array returned by the getOpponentSensors
  for (int i = 0; i < 5; i++) {
    total += detectionArray[i]; // find total "mass" of the collection of bodies
  }

  accum += 2*detectionArray[0];
  accum += 1*detectionArray[1];
  // the center one doesn't deflect the direction to either side!  
  // accum += 0*detectionArray[2];
  accum += -1*detectionArray[3];
  accum += -2*detectionArray[4];

  result = total == 0 ? nothingDetected : accum / total;
  
  return result;
}

void updateGyroDisplacement() {
  int delta = analogRead(gyroZ) - zeroRateGyroZ;
  if (abs(delta) > zThetaDeadband) {
    zThetaDisplacement = zThetaDisplacement + delta;
  }
}


void motorsDisable() {
  digitalWrite(motorStby, LOW);
}

void motorsEnable() {
  digitalWrite(motorStby, HIGH);
}

void brake(int motor) {
  if (motor == left) {
    digitalWrite(in2L, HIGH);
    digitalWrite(in1L, HIGH);
  } else {
    digitalWrite(in2R, HIGH);
    digitalWrite(in1R, HIGH);
  }
  motorsEnable();
}

void coast(int motor) {
  if (motor == left) {
    digitalWrite(in2L, LOW);
    digitalWrite(in1L, LOW);
  } else {
    digitalWrite(in2R, LOW);
    digitalWrite(in1R, LOW);
  }
  motorsEnable();
}

void motor(int motor, int velocity, int brakeState) {
  if (motor == left) {

    if (velocity > 0) {
      if (brakeState == coasting) {
        digitalWrite(in1L, LOW);
        analogWrite(in2L, velocity);
      } else if (brakeState == braking) {
        digitalWrite(in1L, LOW);
        analogWrite(in2L, velocity);
      }
    } else {
      velocity = -velocity;
      if (brakeState == coasting) {
        digitalWrite(in1L, HIGH);
        analogWrite(in2L, (255 - velocity));
      } else if (brakeState == braking) {
        digitalWrite(in1L, HIGH);
        analogWrite(in2L, (255 - velocity));
      }

    }
  } else if (motor == right) {

    if (velocity > 0) {
      if (brakeState == coasting) {
        digitalWrite(in1R, HIGH);
        analogWrite(in2R, (255 - velocity));
      } else if (brakeState == braking) {
        digitalWrite(in1R, HIGH);
        analogWrite(in2R, (255 - velocity));
      }
    } else {
      velocity = -velocity;
      if (brakeState == coasting) {
        digitalWrite(in1R, LOW);
        analogWrite(in2R, velocity);
      } else if (brakeState == braking) {
        digitalWrite(in1R, LOW);
        analogWrite(in2R, velocity);
      }
    }
  }
  motorsEnable();
}

void initPins() {
  pinMode(rightPx, INPUT);
  pinMode(in1L, OUTPUT);
  pinMode(in2L, OUTPUT);
  pinMode(in1R, OUTPUT);
  pinMode(in2R, OUTPUT);
  pinMode(irPwm, OUTPUT);
  pinMode(usrBtn1, INPUT);
  pinMode(usrBtn2, INPUT);
  pinMode(fLefttPx, INPUT);
  pinMode(leftPx, INPUT);
  pinMode(vSense, INPUT);
  pinMode(edgeLeft, INPUT);
  pinMode(edgeRight, INPUT);
  pinMode(fRightPx, INPUT);
  pinMode(gyroZ, INPUT);
  pinMode(motorStby, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  motorsDisable();
  initIrPwm();

  // ensure motor driver pins are LOW
  coast(left);
  coast(right);
}

void motorModeDemo() {
  int testSpeed = 255;
  int driveDuration = 300;
  int restDuration = 2000;

  motor(left, -testSpeed, coasting);
  Serial.println("left coasting backward");
  delay(driveDuration);
  coast(left);
  delay(restDuration);
  motor(left, -testSpeed, braking);
  Serial.println("left braking backward");
  delay(driveDuration);
  brake(left);
  delay(restDuration);
  motor(left, testSpeed, coasting);
  Serial.println("left coasting forward");
  delay(driveDuration);
  coast(left);
  delay(restDuration);
  motor(left, testSpeed, braking);
  Serial.println("left braking forward");
  delay(driveDuration);
  brake(left);
  delay(restDuration);

  motor(right, -testSpeed, coasting);
  Serial.println("right coasting backward");
  delay(driveDuration);
  coast(right);
  delay(restDuration);
  motor(right, -testSpeed, braking);
  Serial.println("right braking backward");
  delay(driveDuration);
  brake(right);
  delay(restDuration);
  motor(right, testSpeed, coasting);
  Serial.println("right coasting forward");
  delay(driveDuration);
  coast(right);
  delay(restDuration);
  motor(right, testSpeed, braking);
  Serial.println("right braking forward");
  delay(driveDuration);
  brake(right);
  delay(restDuration);
}

int getVsense() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(vSense);
  }
  return total / 10;
}

void setGreenLed(bool value) {
  if (value == true) {
    digitalWrite(greenLed, LOW);
  } else {
    digitalWrite(greenLed, HIGH);
  }
}

void setBlueLed(bool value) {
  if (value == true) {
    digitalWrite(blueLed, LOW);
  } else {
    digitalWrite(blueLed, HIGH);
  }
}

void searchLeft(){
  motor(left, 0, braking);
  motor(right, searchSpeed, braking);
}

void searchRight(){
  motor(left, searchSpeed, braking);
  motor(right, 50, braking);
}






