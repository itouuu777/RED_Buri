// Core/Src/main.cpp
#include <cstdint>

extern "C" {
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
void SystemClock_Config(void);
}

#include "STS3215.hpp"

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();   // USART1 を Half-Duplex (Single Wire) に設定しておく
  MX_ADC1_Init();
  MX_TIM8_Init();

  // PA5(LED) を渡すのは任意（渡さなければLED制御しない）
  STS3215 servo(&huart1, /*ID=*/1, GPIOA, GPIO_PIN_5);

  HAL_Delay(50); // サーボ安定待ち

  // 旧: startPos = SyncServoCenter(1);
  int16_t startPos = servo.syncCenter(/*retries=*/1000, /*interval_ms=*/50);

  // 旧: int16_t offset = deg_to_ticks(180.0f);
  //     MoveServoRelative(1, startPos, offset);
  servo.moveRelativeDeg(180.0f);

  while (1) {
    HAL_Delay(5000);
    // 例：戻すなら
    // servo.moveRelativeDeg(-180.0f);
  }
}
