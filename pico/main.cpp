#include "pico/stdlib.h"
#include <stdio.h>
#include <math.h>
#include "hardware/gpio.h"
#include "hardware/pwm.h"
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

absolute_time_t prev_time1;
absolute_time_t prev_time2;
long prev_count1 = 0;
long prev_count2 = 0;

const int PPR = 3;
const int GEAR_RATIO = 150;
const float wheel_diameter = 0.13;

void encoder_callback1(uint gpio, uint32_t events) {
    encoder_count1++;
}

void encoder_callback2(uint gpio, uint32_t events) {
    encoder_count2++;
}

void motor_control(uint in1, uint in2, uint pwm_pin, int speed) {
    gpio_put(in1, speed > 0);
    gpio_put(in2, speed < 0);
    speed = abs(speed);
    pwm_set_gpio_level(pwm_pin, speed * 655 / 100);  // scale 0-100 to 0-65535
}

void forward(int speed) {
    motor_control(AIN1, AIN2, PWMA, speed);
    motor_control(BIN1, BIN2, PWMB, speed);
}

void stop_motors() {
    motor_control(AIN1, AIN2, PWMA, 0);
    motor_control(BIN1, BIN2, PWMB, 0);
}

void get_speed(absolute_time_t &prev_time, long &prev_count, long encoder_count,
               float &wheel_rpm, float &speed_mps) {
    absolute_time_t now = get_absolute_time();
    float dt = absolute_time_diff_us(prev_time, now) / 1e6f;

    if (dt <= 0) {
        wheel_rpm = speed_mps = 0;
        return;
    }

    long delta = encoder_count - prev_count;
    float pps = delta / dt;
    float rps = pps / PPR;
    float rpm = rps * 60.0;
    wheel_rpm = rpm / GEAR_RATIO;

    float wheel_circumference = M_PI * wheel_diameter;
    speed_mps = (wheel_rpm / 60.0f) * wheel_circumference;

    prev_time = now;
    prev_count = encoder_count;
}

int main() {
    stdio_init_all();  // USB シリアル初期化

    while (!stdio_usb_connected()) {
        sleep_ms(100);  // ホストPCが接続されるまで待つ（任意）
    }
    printf("ok");
    stdio_init_all();
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(AIN1); gpio_set_dir(AIN1, GPIO_OUT);
    gpio_init(AIN2); gpio_set_dir(AIN2, GPIO_OUT);
    gpio_init(BIN1); gpio_set_dir(BIN1, GPIO_OUT);
    gpio_init(BIN2); gpio_set_dir(BIN2, GPIO_OUT);

    gpio_set_function(PWMA, GPIO_FUNC_PWM);
    gpio_set_function(PWMB, GPIO_FUNC_PWM);

    uint sliceA = pwm_gpio_to_slice_num(PWMA);
    uint sliceB = pwm_gpio_to_slice_num(PWMB);
    pwm_set_wrap(sliceA, 65535); pwm_set_enabled(sliceA, true);
    pwm_set_wrap(sliceB, 65535); pwm_set_enabled(sliceB, true);

    gpio_init(ENC_A1); gpio_set_dir(ENC_A1, GPIO_IN); gpio_pull_up(ENC_A1);
    gpio_init(ENC_A2); gpio_set_dir(ENC_A2, GPIO_IN); gpio_pull_up(ENC_A2);
    gpio_set_irq_enabled_with_callback(ENC_A1, GPIO_IRQ_EDGE_RISE, true, &encoder_callback1);
    gpio_set_irq_enabled(ENC_A2, GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled_with_callback(ENC_A2, GPIO_IRQ_EDGE_RISE, true, &encoder_callback2);

    prev_time1 = get_absolute_time();
    prev_time2 = get_absolute_time();

    sleep_ms(5000);
    gpio_put(LED_PIN, 1);

    while (true) {
        forward(50);
        sleep_ms(1000);

        float rpm1, speed1, rpm2, speed2;
        get_speed(prev_time1, prev_count1, encoder_count1, rpm1, speed1);
        get_speed(prev_time2, prev_count2, encoder_count2, rpm2, speed2);

        printf("wheel_rpm1: %.2f\n", rpm1);
        printf("wheel_rpm2: %.2f\n", rpm2);
    }

    return 0;
}
