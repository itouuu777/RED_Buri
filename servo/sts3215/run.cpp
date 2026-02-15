// stm32/nucleo_f446re/lib/run/run.hpp
#pragma once

#include "main.h"
#include "sts3215.hpp" // サーボの部品（さっき作ったやつ）
#include <cmath>

// main.cで定義されているhuart1を外部から参照
extern UART_HandleTypeDef huart1;

namespace run
{
    // --- 変数はすべてここに移動 (inlineをつける) ---
    inline STS3215 servo(&huart1, 1);
    inline float rad = 0.0f;

    /**
     * 初期化：mainの「USER CODE BEGIN 2」で呼ぶ
     */
    inline void setup()
    {
        // 1. サーボを位置制御モードにする
        servo.setMode(0);
        
        // 2. 起動時の位置をセンターとして記憶
        servo.syncCenter();
        
        // 成功したらLEDを短く光らせる（お好みで）
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }

    /**
     * メインループ：mainの「while(1)」の中で呼ぶ
     */
    inline void loop()
    {
        // 1. サイン波でオフセット計算
        // maxPosition = 1600 相当の動き
        float offset = sinf(rad) * 1600.0f;
        
        // 2. 相対移動命令
        servo.moveRelative((int16_t)offset);

        // 3. パラメータ更新
        rad += 0.06f;
        if (rad > 2.0f * 3.14159f) rad -= 2.0f * 3.14159f;

        // 4. 周期調整 (50Hz)
        HAL_Delay(20);
    }
}
