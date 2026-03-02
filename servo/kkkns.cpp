/* USER CODE BEGIN Includes */
#include "math.h"
#include <stdio.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define PI 3.14159
/* USER CODE END PD */

/* USER CODE BEGIN PV */
const uint8_t servoID = 1; 
volatile int16_t startPos = -1;
/* USER CODE END PV */

/* USER CODE BEGIN 0 */
// ==========================================
// ここにサーボ制御用の関数を全部ぶち込む
// ==========================================

// LED制御用
static inline void led_on(void)  { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); }
static inline void led_off(void) { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); }

// チェックサム計算
uint8_t STS_CalcChecksum(uint8_t *msg, uint8_t len) {
    uint8_t checksum = 0;
    for (int i = 2; i < len - 1; i++) checksum += msg[i];
    return ~checksum;
}

// 角度からTickへの変換
static inline int16_t deg_to_ticks(float deg) {
    float ticks = deg * (4096.0f / 360.0f);
    int32_t it = (int32_t)lroundf(ticks);
    if (it < -4096) it = -4096;
    if (it >  4096) it =  4096;
    return (int16_t)it;
}

// 位置指定コマンド送信
void Pos_command(uint8_t id, uint16_t position) {
    uint8_t message[13] = {0xFF, 0xFF, id, 9, 3, 42};
    message[6] = position & 0xFF;
    message[7] = (position >> 8) & 0xFF;
    message[8] = 0x00; message[9] = 0x00;
    message[10] = 0x00; message[11] = 0x00;
    message[12] = STS_CalcChecksum(message, 13);

    HAL_HalfDuplex_EnableTransmitter(&huart1);
    HAL_UART_Transmit(&huart1, message, 13, 10);
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);
    HAL_HalfDuplex_EnableReceiver(&huart1);
}

// 現在位置の取得
int16_t STS3215_GetPosition(uint8_t id) {
    uint8_t tx_msg[8] = {0xFF, 0xFF, id, 4, 2, 56, 2, 8};
    uint8_t rx_msg[8] = {0};
    tx_msg[7] = STS_CalcChecksum(tx_msg, 8);

    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_FLUSH_DRREGISTER(&huart1);

    HAL_HalfDuplex_EnableTransmitter(&huart1);
    HAL_UART_Transmit(&huart1, tx_msg, 8, 10);
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);

    HAL_HalfDuplex_EnableReceiver(&huart1);
    
    if (HAL_UART_Receive(&huart1, rx_msg, 8, 20) == HAL_OK) {
        if (rx_msg[0] == 0xFF && rx_msg[1] == 0xFF) {
            return (int16_t)((rx_msg[6] << 8) | rx_msg[5]);
        }
    }
    return -1;
}

// 起動時の初期位置キャリブレーション
int16_t SyncServoCenter(uint8_t id) {
    int16_t pos = -1;
    HAL_Delay(50); // サーボの安定待ち
    for (int i = 0; i < 100; i++) {
        pos = STS3215_GetPosition(id);
        if (pos != -1) break;
        HAL_Delay(50);
    }
    if (pos != -1) {
        led_off();
        return pos;
    } else {
        led_on();
        return 2048; 
    }
}

// 相対移動
void MoveServoRelative(uint8_t id, int16_t current_startPos, int16_t offset) {
    if (current_startPos == -1) return; 
    int targetPosition = current_startPos + offset;
    if (targetPosition > 4095) targetPosition = 4095;
    if (targetPosition < 0)    targetPosition = 0;
    Pos_command(id, targetPosition);
}
// ==========================================
/* USER CODE END 0 */


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  // MX_ADC1_Init(); // 必要に応じて
  // MX_TIM8_Init(); // 必要に応じて

  /* USER CODE BEGIN 2 */
  // ==========================================
  // ここが元々の setup() に相当する部分
  // ==========================================
  HAL_Delay(100); // 起動直後の長めの安定待ち
  
  // 1. 初期位置を取得
  startPos = SyncServoCenter(servoID);
  
  // 2. 例：初期位置から180度の位置へ移動する量を計算
  int16_t offset = deg_to_ticks(180.0f);
  
  HAL_Delay(1000);
  // ==========================================
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // ==========================================
      // ここが元々の loop() に相当する部分
      // ==========================================
      
      MoveServoRelative(servoID, startPos, offset);
      
      HAL_Delay(5000); // 5秒待つなど
      
      /* USER CODE END WHILE */

      /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
