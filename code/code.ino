/******************************************************************************************
 * ESP32 Motor + Servo Test (NO Encoders) + SBUS (BolderFlight) — CUSTOM RANGE
 * - SBUS ranges calibrated to:
 *      Min = 240
 *      Mid = 1024
 *      Max = 1807
 ******************************************************************************************/

#include <Arduino.h>
#include <math.h>
#include <ESP32Servo.h>
#include "sbus.h"

/* ===================== ENUMS ===================== */

enum Mode {
  MODE_STOP,
  MODE_FORWARD,
  MODE_BACKWARD,
  MODE_SIDE_LEFT,
  MODE_SIDE_RIGHT,
  MODE_DIGGING,
  MODE_ROTATE_CW,
  MODE_ROTATE_CCW,
  MODE_CALIBRATE
};
Mode currentMode = MODE_STOP;

enum DigState { DIG_PHASE1_PRESS, DIG_PHASE2_TRANSITION, DIG_DONE };
DigState digState = DIG_PHASE1_PRESS;
uint32_t digStateStartMs = 0;

/* ===================== CONSTANTS ===================== */

const int M_PWMA[4] = {25, 26, 27, 14};
const int M_PWMB[4] = {12, 13, 33, 32};

// Servo pins
const int SERVO_PIN[3] = {23, 15, 2};
Servo servo[3];

const float SERVO1_INIT = 45;
const float SERVO2_INIT = 135;
const float SERVO3_INIT = 90;
const float SERVO3_SIDE_LEFT  = 180;
const float SERVO3_SIDE_RIGHT = 0;

#define ROT_BASE_SPEED 180
#define BASE_SPEED     200
#define SIDE_M4_FACTOR 1.5
#define SIDE_OTH_RATIO 0.5
#define DIG_M1_SPEED   180
#define DIG_REAR_SPEED 120

#define DIG_PHASE1_MS  3000
#define DIG_PHASE2_MS  3000

/* ===================== SBUS SETTINGS ===================== */

bfs::SbusRx sbus_rx(&Serial2, 16, -1, true, false);
bfs::SbusData sbusData;

uint16_t sbusUs[16]; // mapped to 1000–2000 µs

// Your calibrated values
const int SBUS_MIN = 240;
const int SBUS_MID = 1024;
const int SBUS_MAX = 1807;

int sbusToUs(uint16_t v) {
  v = constrain(v, SBUS_MIN, SBUS_MAX);
  return map(v, SBUS_MIN, SBUS_MAX, 1000, 2000);
}

float normStick_us(int us, int center = 1500, int span = 400) {
  float x = (float)(us - center) / (float)span;
  if (fabs(x) < 0.08) x = 0;
  return constrain(x, -1.0f, 1.0f);
}

/* ===================== SERVO & MOTOR HELPERS ===================== */

void setServoDeg(int idx, float deg) {
  deg = constrain(deg, 0, 180);
  servo[idx].write(deg);
}

void stopAllMotors() {
  for (int i = 0; i < 4; i++) {
    analogWrite(M_PWMA[i], 0);
    analogWrite(M_PWMB[i], 0);
  }
}

void setMotorForward(int i, int speed) {
  analogWrite(M_PWMA[i], constrain(speed, 0, 255));
  analogWrite(M_PWMB[i], 0);
}

void setMotorReverse(int i, int speed) {
  analogWrite(M_PWMA[i], 0);
  analogWrite(M_PWMB[i], constrain(speed, 0, 255));
}

/* ===================== MOTION PROFILES ===================== */

void zeroAll() { stopAllMotors(); }

void applyForwardProfile() {
  setMotorForward(0, BASE_SPEED);
  setMotorReverse(1, BASE_SPEED);
  setMotorForward(2, BASE_SPEED);
  setMotorReverse(3, BASE_SPEED);

  setServoDeg(0, 90);
  setServoDeg(1, 90);
  setServoDeg(2, 90);
}

void applyBackwardProfile() {
  setMotorReverse(0, BASE_SPEED);
  setMotorForward(1, BASE_SPEED);
  setMotorReverse(2, BASE_SPEED);
  setMotorForward(3, BASE_SPEED);

  setServoDeg(0, 90);
  setServoDeg(1, 90);
  setServoDeg(2, 90);
}

void applySideProfile(bool left) {
  int m4Speed = BASE_SPEED * SIDE_M4_FACTOR;
  int otherSpeed = m4Speed * SIDE_OTH_RATIO;

  if (left) {
    setServoDeg(2, SERVO3_SIDE_LEFT);
    setMotorReverse(3, m4Speed);
    setMotorForward(0, otherSpeed);
    setMotorForward(1, otherSpeed);
    setMotorForward(2, otherSpeed);
  } else {
    setServoDeg(2, SERVO3_SIDE_RIGHT);
    setMotorForward(3, m4Speed);
    setMotorReverse(0, otherSpeed);
    setMotorReverse(1, otherSpeed);
    setMotorReverse(2, otherSpeed);
  }
}

/* ===================== DIGGING ===================== */

void startDiggingSequence() {
  digState = DIG_PHASE1_PRESS;
  digStateStartMs = millis();

  setServoDeg(0, SERVO1_INIT);
  setServoDeg(1, 135);
  setServoDeg(2, 0);

  setMotorForward(0, DIG_M1_SPEED);
  setMotorReverse(2, DIG_REAR_SPEED);
  setMotorReverse(3, DIG_REAR_SPEED);
}

void updateDiggingSequence() {
  uint32_t t = millis() - digStateStartMs;

  if (digState == DIG_PHASE1_PRESS) {
    float pct = min(1.0f, t / (float)DIG_PHASE1_MS);
    float s2 = 135 - pct * (135 - 120);
    setServoDeg(1, s2);

    if (t >= DIG_PHASE1_MS) {
      digState = DIG_PHASE2_TRANSITION;
      digStateStartMs = millis();
      setMotorForward(0, DIG_M1_SPEED + 15);
    }
  }
  else if (digState == DIG_PHASE2_TRANSITION) {
    float pct = min(1.0f, t / (float)DIG_PHASE2_MS);
    float s1 = 45 - pct * 45;
    float s2 = 120 - pct * 30;
    setServoDeg(0, s1);
    setServoDeg(1, s2);
    setServoDeg(2, 90);

    if (t >= DIG_PHASE2_MS) {
      digState = DIG_DONE;
      applyForwardProfile();
      currentMode = MODE_FORWARD;
    }
  }
}

/* ===================== ROTATION ===================== */

void applyRotateCWProfile() {
  setMotorReverse(0, ROT_BASE_SPEED);
  setMotorForward(1, ROT_BASE_SPEED);
  setMotorReverse(2, ROT_BASE_SPEED);
  setMotorForward(3, ROT_BASE_SPEED);
}

void applyRotateCCWProfile() {
  setMotorForward(0, ROT_BASE_SPEED);
  setMotorReverse(1, ROT_BASE_SPEED);
  setMotorForward(2, ROT_BASE_SPEED);
  setMotorReverse(3, ROT_BASE_SPEED);
}

/* ===================== MODE MANAGEMENT ===================== */

void setMode(Mode m) {
  currentMode = m;
  zeroAll();

  switch (m) {
    case MODE_STOP: break;
    case MODE_FORWARD:     applyForwardProfile(); break;
    case MODE_BACKWARD:    applyBackwardProfile(); break;
    case MODE_SIDE_LEFT:   applySideProfile(true); break;
    case MODE_SIDE_RIGHT:  applySideProfile(false); break;
    case MODE_DIGGING:     startDiggingSequence(); break;
    case MODE_ROTATE_CW:   applyRotateCWProfile(); break;
    case MODE_ROTATE_CCW:  applyRotateCCWProfile(); break;
  }
}

Mode readDesiredModeSerial() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c=='s'||c=='S') return MODE_STOP;
    if (c=='f'||c=='F') return MODE_FORWARD;
    if (c=='b'||c=='B') return MODE_BACKWARD;
    if (c=='l'||c=='L') return MODE_SIDE_LEFT;
    if (c=='r'||c=='R') return MODE_SIDE_RIGHT;
    if (c=='d'||c=='D') return MODE_DIGGING;
    if (c=='x'||c=='X') return MODE_ROTATE_CW;
    if (c=='y'||c=='Y') return MODE_ROTATE_CCW;
  }
  return currentMode;
}

/* ===================== SBUS → ROBOT CONTROL ===================== */

bool lastDigOn = false;

void handleRCMode() {
  float lr   = normStick_us(sbusUs[0]);
  float fb   = normStick_us(sbusUs[1]);
  float rot  = normStick_us(sbusUs[3]);
  bool digOn = (sbusUs[7] > 1600);

  bool wantMove   = (fabs(fb) > 0.15f) || (fabs(lr) > 0.15f);
  bool wantRotate = (fabs(rot) > 0.20f);
  bool wantDig    = digOn;

  int activeCount = (int)wantMove + (int)wantRotate + (int)wantDig;

  if (activeCount >= 2) {
    setMode(MODE_STOP);
    lastDigOn = digOn;
    return;
  }

  if (wantDig) {
    if (!lastDigOn) setMode(MODE_DIGGING);
    lastDigOn = true;
    return;
  } else lastDigOn = false;

  if (wantRotate) {
    setMode(rot > 0 ? MODE_ROTATE_CW : MODE_ROTATE_CCW);
    return;
  }

  if (wantMove) {
    if (fabs(fb) >= fabs(lr)) setMode(fb > 0 ? MODE_FORWARD : MODE_BACKWARD);
    else setMode(lr > 0 ? MODE_SIDE_RIGHT : MODE_SIDE_LEFT);
    return;
  }

  setMode(MODE_STOP);
}

/* ===================== SERVO SWITCH/ANGLE ===================== */

void handleRCServos() {
  if (currentMode == MODE_DIGGING) return;

  int selRaw = sbusUs[6];
  int angleRaw = sbusUs[5];

  if (selRaw < 900 || selRaw > 2100) return;
  if (angleRaw < 900 || angleRaw > 2100) return;

  int sel = 1;
  if (selRaw < 1300) sel = 0;
  else if (selRaw > 1700) sel = 2;

  float angle = (float)(angleRaw - 1000) * 180.0f / 1000.0f;
  setServoDeg(sel, angle);
}

/* ===================== SETUP ===================== */

void setup() {
  Serial.begin(115200);
  delay(300);

  for (int i = 0; i < 4; i++) {
    pinMode(M_PWMA[i], OUTPUT);
    pinMode(M_PWMB[i], OUTPUT);
  }

  for (int i = 0; i < 3; i++) {
    servo[i].attach(SERVO_PIN[i]);
  }

  setServoDeg(0, 90);
  setServoDeg(1, 90);
  setServoDeg(2, 90);

  sbus_rx.Begin();
  setMode(MODE_STOP);
}

/* ===================== LOOP ===================== */

void loop() {
  if (sbus_rx.Read()) {
    sbusData = sbus_rx.data();

    // Convert all channels to microseconds
    for (int i = 0; i < 16; i++) {
      sbusUs[i] = sbusToUs(sbusData.ch[i]);
    }

    handleRCServos();
    handleRCMode();
  }

  Mode fromSerial = readDesiredModeSerial();
  if (fromSerial != currentMode) setMode(fromSerial);

  if (currentMode == MODE_DIGGING) updateDiggingSequence();
}
