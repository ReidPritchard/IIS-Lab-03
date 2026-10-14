// Virtual lock with haptic feedback
//
// Three capacitive touch pads are the lock pins. An FSR on the tension wrench sets
// the tension (low / valid / high). Two vibration motors (pick and wrench) tell the
// user what is happening inside the lock. Set the pins in SEQUENCE order while
// holding valid tension to open the lock.

// Pick motor (driver channel A) and wrench motor (driver channel B)
#define ENA 13
#define IN1 12
#define IN2 14
#define ENB 23
#define IN3 26
#define IN4 25

// FSR on the tension wrench
#define FSR_PIN 4

// Touch-capable GPIOs used as lock pins
const int TOUCH_PINS[] = {27, 33, 32};
const int SEQUENCE[] = {1, 0, 2}; // Pin 2, then pin 1, then pin 3

// FSR thresholds (raw analogRead counts, higher = more pressure)
#define TENSION_VALID 350
#define TENSION_HIGH 900

#define OVER_TENSION_REPEAT_MS 500  // Repeat the warning while tension stays high

// Tension states
#define T_LOW 0
#define T_VALID 1
#define T_HIGH 2

// Haptic patterns: each step is { pick PWM, wrench PWM, duration ms }
struct Step { uint8_t pick, wrench; uint16_t ms; };

const Step PIN_CONTACT[]     = { {180, 0, 100} };
const Step PIN_ACK[]         = { {110, 0, 100} };  // Touching a pin that is already set
const Step TENSION_ENGAGED[] = { {0, 190, 120} };
const Step PIN_SET[]         = { {255, 0, 120}, {0, 0, 80}, {0, 230, 160} };
const Step FALSE_SET[]       = { {220, 0, 110}, {0, 0, 70}, {170, 0, 100}, {0, 0, 60}, {220, 0, 110} };
const Step OVER_TENSION[]    = { {0, 230, 200}, {0, 0, 90}, {0, 230, 160} };
const Step RELEASE_RESET[]   = { {0, 190, 140}, {0, 0, 110}, {0, 190, 140} };
const Step UNLOCKED[] = {  // "Shave and a haircut... two bits" on both motors
  {210, 210, 230}, {0, 0, 90},  // Shave
  {210, 210, 115}, {0, 0, 45},  // and
  {210, 210, 115}, {0, 0, 45},  // a
  {210, 210, 230}, {0, 0, 90},  // hair-
  {210, 210, 230}, {0, 0, 90},  // -cut
  {0, 0, 320},                  // (rest)
  {255, 255, 230}, {0, 0, 90},  // two
  {255, 255, 600},              // bits
};

#define PLAY(p) play(p, sizeof(p) / sizeof(p[0]))

int touchThreshold[3];
bool wasTouched[3];
bool pinSet[3];
int pinsSet = 0;
int tension = T_LOW;
float fsr = 0;
unsigned long lastOverTension = 0;

// Blocking: plays the whole pattern before returning
void play(const Step *steps, int n) {
  for (int i = 0; i < n; i++) {
    analogWrite(ENA, steps[i].pick);
    analogWrite(ENB, steps[i].wrench);
    delay(steps[i].ms);
  }
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void resetLock() {
  for (int i = 0; i < 3; i++) pinSet[i] = false;
  pinsSet = 0;
}

void onTouch(int i) {
  Serial.printf("Pin %d touched, tension %d\n", i + 1, tension);

  if (pinSet[i]) {
    PLAY(PIN_ACK);
  } else if (tension != T_VALID) {
    PLAY(PIN_CONTACT);  // Too little tension, or binding: pins can't set
  } else if (i == SEQUENCE[pinsSet]) {
    pinSet[i] = true;
    pinsSet++;
    PLAY(PIN_SET);
    if (pinsSet == 3) {
      Serial.println("Unlocked!");
      PLAY(UNLOCKED);
      resetLock();
    }
  } else {
    PLAY(FALSE_SET);
  }
}

void onTensionChange() {
  Serial.printf("Tension %d (fsr %.0f)\n", tension, fsr);

  if (tension == T_VALID) {
    PLAY(TENSION_ENGAGED);
  } else if (tension == T_HIGH) {
    lastOverTension = millis();
    PLAY(OVER_TENSION);
  } else if (pinsSet > 0) {
    resetLock();
    PLAY(RELEASE_RESET);
  }
}

void setup() {
  Serial.begin(115200);

  // Vibration motors only spin one direction
  pinMode(IN1, OUTPUT); digitalWrite(IN1, HIGH);
  pinMode(IN2, OUTPUT); digitalWrite(IN2, LOW);
  pinMode(IN3, OUTPUT); digitalWrite(IN3, HIGH);
  pinMode(IN4, OUTPUT); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  // Calibrate touch pads (don't touch them during startup). Reading drops when touched.
  for (int i = 0; i < 3; i++) {
    touchThreshold[i] = touchRead(TOUCH_PINS[i]) * 0.7;
  }

  fsr = analogRead(FSR_PIN);
}

void loop() {
  // Smoothed FSR reading
  fsr += 0.3 * (analogRead(FSR_PIN) - fsr);
  int newTension = fsr >= TENSION_HIGH ? T_HIGH : fsr >= TENSION_VALID ? T_VALID : T_LOW;
  if (newTension != tension) {
    tension = newTension;
    onTensionChange();
  }

  if (tension == T_HIGH && millis() - lastOverTension >= OVER_TENSION_REPEAT_MS) {
    lastOverTension = millis();
    PLAY(OVER_TENSION);
  }

  // React to each new touch
  for (int i = 0; i < 3; i++) {
    bool touched = touchRead(TOUCH_PINS[i]) < touchThreshold[i];
    if (touched && !wasTouched[i]) onTouch(i);
    wasTouched[i] = touched;
  }

  delay(5);
}
