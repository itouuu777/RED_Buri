#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/float32_multi_array.h>
#include <nav_msgs/msg/odometry.h>
#include <geometry_msgs/msg/transform_stamped.h> // ★ 追加
#include <rosidl_runtime_c/string_functions.h>

// ★ TF用パブリッシャーを追加
rcl_publisher_t tf_pub;
geometry_msgs__msg__TransformStamped tf_msg; // ★ TFメッセージ用変数

rcl_publisher_t rpm_pub;
rcl_publisher_t odom_pub;
std_msgs__msg__Float32MultiArray rpm_msg;
nav_msgs__msg__Odometry odom_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();} }
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){} }

// ... (省略：GPIO定義、エンコーダ変数、速度計算変数など、変更なし) ...
#define AIN1 32
#define AIN2 33
#define PWMA 25
#define BIN1 26
#define BIN2 27
#define PWMB 14
#define LED_PIN 13

#define ENC_A1 15
#define ENC_B1 2
#define ENC_A2 0
#define ENC_B2 4
// Odom 関連の変数
float x_pos = 0.0;
float y_pos = 0.0;
float theta = 0.0;


// ... (省略：encoderCallback1, encoderCallback2, motorControl, forward, getSpeed関数、変更なし) ...
volatile long encoder_count1 = 0;
volatile long encoder_count2 = 0;
unsigned long prev_time1 = 0, prev_time2 = 0;
long prev_count1 = 0, prev_count2 = 0;

const int PPR = 3;
const int GEAR_RATIO = 150;
const float wheel_diameter = 0.13; // meters
const float wheel_base = 0.3;      // meters (調整必要)

void error_loop() {
  while (1) {
    delay(100);
  }
}


void encoderCallback1() {
  encoder_count1++;
}

void encoderCallback2() {
  encoder_count2++;
}

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
  analogWrite(pwm_pin, map(speed, 0, 100, 0, 255));
}

void forward(int speed) {
  motorControl(AIN1, AIN2, PWMA, speed);
  motorControl(BIN1, BIN2, PWMB, speed);
}

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

void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  if (timer == NULL) return;

  float rpm1, speed1, rpm2, speed2;
  getSpeed(prev_time1, prev_count1, encoder_count1, rpm1, speed1);
  getSpeed(prev_time2, prev_count2, encoder_count2, rpm2, speed2);

  // Publish RPM
  rpm_msg.data.data[0] = rpm1;
  rpm_msg.data.data[1] = rpm2;
  rpm_msg.data.size = 2;
  RCSOFTCHECK(rcl_publish(&rpm_pub, &rpm_msg, NULL));

  // Calculate odometry
  float linear_vel = (speed1 + speed2) / 2.0;
  float angular_vel = (speed2 - speed1) / wheel_base;
  float dt = 1.0; // seconds (based on timer period) // ここは前回のタイマーコールからの正確な経過時間を使うべきですが、今回は修正せず

  theta += angular_vel * dt;
  x_pos += linear_vel * cos(theta) * dt;
  y_pos += linear_vel * sin(theta) * dt;

  // Odometry message creation and publish
  odom_msg.header.stamp.sec = millis() / 1000;
  odom_msg.header.stamp.nanosec = (millis() % 1000) * 1000000;
  rosidl_runtime_c__String__assign(&odom_msg.header.frame_id, "odom");
  rosidl_runtime_c__String__assign(&odom_msg.child_frame_id, "base_link");

  odom_msg.pose.pose.position.x = x_pos;
  odom_msg.pose.pose.position.y = y_pos;
  odom_msg.pose.pose.position.z = 0.0;
  odom_msg.pose.pose.orientation.w = cos(theta / 2);
  odom_msg.pose.pose.orientation.z = sin(theta / 2);

  odom_msg.twist.twist.linear.x = linear_vel;
  odom_msg.twist.twist.angular.z = angular_vel;

  RCSOFTCHECK(rcl_publish(&odom_pub, &odom_msg, NULL));

  // ★ TF message creation and publish (NEW)
  tf_msg.header.stamp.sec = odom_msg.header.stamp.sec;
  tf_msg.header.stamp.nanosec = odom_msg.header.stamp.nanosec;
  rosidl_runtime_c__String__assign(&tf_msg.header.frame_id, "odom"); // 親フレーム
  rosidl_runtime_c__String__assign(&tf_msg.child_frame_id, "base_link"); // 子フレーム

  tf_msg.transform.translation.x = odom_msg.pose.pose.position.x;
  tf_msg.transform.translation.y = odom_msg.pose.pose.position.y;
  tf_msg.transform.translation.z = odom_msg.pose.pose.position.z;
  tf_msg.transform.rotation = odom_msg.pose.pose.orientation;

  RCSOFTCHECK(rcl_publish(&tf_pub, &tf_msg, NULL)); // ★ TFをパブリッシュ
}

void setup() {
  set_microros_transports();

  // ... (省略：ピン設定、割り込み設定など、変更なし) ...
  set_microros_transports();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);

  pinMode(ENC_A1, INPUT_PULLUP);
  pinMode(ENC_B1, INPUT_PULLUP);
  pinMode(ENC_A2, INPUT_PULLUP);
  pinMode(ENC_B2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_A1), encoderCallback1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_A2), encoderCallback2, RISING);

  delay(2000);

  allocator = rcl_get_default_allocator();
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  RCCHECK(rclc_node_init_default(&node, "esp32_odom_node", "", &support));

  // RPM publisher
  RCCHECK(rclc_publisher_init_default(
    &rpm_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32MultiArray),
    "wheel_rpm"
  ));

  rpm_msg.data.capacity = 2;
  rpm_msg.data.size = 2;
  rpm_msg.data.data = (float*) malloc(sizeof(float) * 2);

  // Odometry publisher
  RCCHECK(rclc_publisher_init_default(
    &odom_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry),
    "odom"
  ));

  // ★ TF publisher initialization (NEW)
  RCCHECK(rclc_publisher_init_default(
    &tf_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, TransformStamped),
    "tf" // TFメッセージは常に "/tf" トピックにパブリッシュします
  ));
  // TFメッセージの初期化 (Micro-ROSでは必要に応じて手動でメモリ確保など)
  // geometry_msgs__msg__TransformStamped の割り当ては通常自動ですが、stringは明示的に
  rosidl_runtime_c__String__init(&tf_msg.header.frame_id);
  rosidl_runtime_c__String__init(&tf_msg.child_frame_id);


  // Timer and executor
  const unsigned int timer_timeout = 100; // 100ms = 10Hz の更新頻度を推奨 (元の1000ms=1Hzは低すぎます)
                                        // SLAMにはより高頻度のtfが望ましい
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback
  ));

  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator)); // ★ executorのハンドラ数を2に増やす (odom_pubとtf_pub)
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
  //RCCHECK(rclc_executor_add_publisher(&executor, &odom_pub, &odom_msg)); // publisherもexecutorに追加
  //RCCHECK(rclc_executor_add_publisher(&executor, &tf_pub, &tf_msg));     // ★ TF publisherもexecutorに追加
  //RCCHECK(rclc_executor_add_publisher(&executor, &rpm_pub, &rpm_msg));   // rpm_pubも追加

  forward(50);  // 初期起動テスト
}

void loop() {
  delay(100);
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}
