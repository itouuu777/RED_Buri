#include "gpio.h"
#include "tim.h"
#include "c620_can.hpp"
#include "c620_control.hpp"
#include "step_axis.hpp"
#include <cstddef>
#include "sts3215.hpp"
#include "usart.h"
#include <cstdio>
#include <cmath>

C620CAN c620_can;
C620Control c620_control(c620_can);
STS3215 servo(&huart1, 1, GPIOA, GPIO_PIN_5); // UART, ID, LED(port/pin)

namespace run
{
    // step
    constexpr uint32_t kDefaultStepHz = 300U; // パルスの周波数

    // タイマ本体, タイマch, IOポート, pin番号, 正方向のときにDIRをHIGHにするか, モーター1ステップ角(def 1.8), マイクロステップ倍率(pulse/revの1/200), 減速比） 
    StepAxis axis1{&htim2, TIM_CHANNEL_1, GPIOB, GPIO_PIN_7, true, 1.8f, 2U, 15.0f}; // PUL=PA15, DIR=PB7
    StepAxis axis2{&htim3, TIM_CHANNEL_1, GPIOA, GPIO_PIN_4, true, 1.8f, 2U, 15.0f}; // PUL=PA6,  DIR=PA4
    StepAxis axis3{&htim4, TIM_CHANNEL_1, GPIOB, GPIO_PIN_4, true, 1.8f, 2U, 15.0f}; // PUL=PB6,  DIR=PB4

    StepAxis* const axes[] = {&axis1, &axis2, &axis3};
    constexpr size_t kAxisCount = sizeof(axes) / sizeof(axes[0]);



    void setup()
    {
        c620_can.init();                 // CAN開始 + filter + RX通知
        HAL_TIM_Base_Start_IT(&htim6); // C620 control loop

        /*
        axis1.setStepFrequencyHz(kDefaultStepHz);
        axis2.setStepFrequencyHz(kDefaultStepHz);
        axis3.setStepFrequencyHz(kDefaultStepHz);
        int16_t start = servo.syncCenter(200, 20);
        */

        // servo setanglefromzerodeg用
        servo.captureZero(40);
    }

    void loop()
    {
        /*
        static bool go_home = false;

        if (!go_home) {
            move_axis_to_deg(0, 10.0f); // 絶対角度, (ID, 角度)
            move_axis_to_deg(1, 180.0f);
            move_axis_to_deg(2, 180.0f);
        } else {
            move_axis_to_deg(0, 0.0f);
            move_axis_to_deg(1, 0.0f);
            move_axis_to_deg(2, 0.0f);
        }
        static uint32_t t = 0;


        go_home = !go_home;
        HAL_Delay(500);
        
        // servo
        float now = servo.getAngleDeg();         // 失敗時 -1.0f
        servo.setAngleDeg(30.0f, 0, 200);       // 絶対180度 絶対角度, 移動時間, 速度
        servo.moveByDeg(+30.0f, 40, 0, 1000);     // 現在位置から-30度 角度, 接続タイムアウト, 移動時間, 速度

        servo.setAngleFromZeroDeg(30.0f, 0, 200);   // 起動時位置を0°として30°
        float a = servo.getAngleFromZeroDeg(40);    
        */


        //c620_control.setSpeedTarget(4, 50.0f);
        //const float currentDeg = servo.getAngleDeg(40);    // 失敗=-1.0f
        //printf("%.2f\r\n", currentDeg);
       // HAL_Delay(100);
        //servo.moveByDeg(-30.0f, 100, 0, 500);  // -30°
        HAL_Delay(2000);
        //servo.setAngleDeg(180.0f, 0, 300); 
        
    const float currentDeg = servo.getAngleDeg(40); // 失敗=-1.0f
    printf("%.2f\r\n", currentDeg);
    HAL_Delay(200); 
    servo.setAngleDeg(180.0f, 0, 300); 

        /*
        servo.moveClampedByDeg(+30.0f, 0, 200);  // 0..360で飽和
        printf("target=%.2f current=%.2f raw=%d\r\n",
       servo.getClampedTargetDeg(),
       servo.getCurrentDegFromZero(40),
       servo.getPosition(40));
        HAL_Delay(600);
        printf("time");
        */



    }
}

extern "C" void setup_c()
{
    run::setup();
}

extern "C" void loop_c()
{
    run::loop();
}
