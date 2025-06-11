// ----- ピン定義 -----
#define AIN1 10
#define AIN2 11
#define PWMA 3
#define BIN1 7
#define BIN2 6
#define PWMB 2
#define LED_PIN 25

#define ENC_A1 12
#define ENC_B1 13
#define ENC_A2 21
#define ENC_B2 22

volatile long encoder_count1 = 0;
volatile long encoder_count2 = 0;

unsigned long prev_time1 = 0;
unsigned long prev_time2 = 0;
long prev_count1 = 0;
long prev_count2 = 0;

const int PPR = 3;
const int GEAR_RATIO = 150;
const float wheel_diameter = 0.13; // m

// ----- 割り込み関数 -----
void encoderCallback1() {
  encoder_count1++;
}

void encoderCallback2() {
  encoder_count2++;
}

// ----- モーター制御 -----
void motorControl(int in1, int in2, int pwm_pin, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    speed = -speed;
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
  analogWrite(pwm_pin, map(speed, 0, 100, 0, 255)); // 0–100 → 0–255
}

void forward(int speed) {
  motorControl(AIN1, AIN2, PWMA, speed);
  motorControl(BIN1, BIN2, PWMB, speed);
}

void stopMotors() {
  motorControl(AIN1, AIN2, PWMA, 0);
  motorControl(BIN1, BIN2, PWMB, 0);
}

// ----- 速度計算 -----
void getSpeed(unsigned long &prev_time, long &prev_count, long encoder_count,
              float &wheel_rpm, float &speed_mps) {
  unsigned long now = millis();
  float dt = (now - prev_time) / 1000.0;
  if (dt <= 0) {
    wheel_rpm = 0;
    speed_mps = 0;
    return;
  }
  long delta = encoder_count - prev_count;
  float pps = delta / dt;
  float rps = pps / PPR;
  float rpm = rps * 60;
  wheel_rpm = rpm / GEAR_RATIO;
  float wheel_circumference = PI * wheel_diameter;
  speed_mps = (wheel_rpm / 60.0) * wheel_circumference;

  prev_time = now;
  prev_count = encoder_count;
}

// ----- セットアップ -----
void setup() {
  Serial.begin(115200);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(ENC_A1, INPUT_PULLUP);
  pinMode(ENC_B1, INPUT_PULLUP);
  pinMode(ENC_A2, INPUT_PULLUP);
  pinMode(ENC_B2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_A1), encoderCallback1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_A2), encoderCallback2, RISING);

  digitalWrite(LED_PIN, LOW);
  delay(5000); // 初期待機
  digitalWrite(LED_PIN, HIGH);
}

// ----- メインループ -----
void loop() {
  forward(50);
  delay(1000);

  float rpm1, speed1, rpm2, speed2;
  getSpeed(prev_time1, prev_count1, encoder_count1, rpm1, speed1);
  getSpeed(prev_time2, prev_count2, encoder_count2, rpm2, speed2);

  Serial.print("wheel_rpm1: "); Serial.println(rpm1);
  Serial.print("wheel_rpm2: "); Serial.println(rpm2);
}
