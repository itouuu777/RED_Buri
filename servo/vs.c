/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#include "math.h"
#define PI 3.14159
const uint8_t servoID = 1; // サーボのID
int centerPosition = 2048;//サーボ位置のセンター値
int goalPosition = 40;//サーボの目標位置
float radiansval = 0.0; // サインカーブ算出用のラジアン値
float radiansIncrement = 0.06; // ループ毎のラジアン値の増加量
float maxPosition = 1600;// センター値を2048としたとき、±どこまで振るか(0-2048)
volatile int16_t startPos = -1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
static inline void led_on(void)  { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); }
static inline void led_off(void) { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); }
static inline void led_blink_ok(void)   { led_on(); HAL_Delay(20);  led_off(); }  // 成功は短く
static inline void led_blink_fail(void) { led_on(); HAL_Delay(200); led_off(); }  // 失敗は長く
uint8_t STS_CalcChecksum(uint8_t *msg, uint8_t len) {
    uint8_t checksum = 0;
    for (int i = 2; i < len - 1; i++) checksum += msg[i];
    return ~checksum;
}



void SystemClock_Config(void);
void STS_SetMode(uint8_t id, uint8_t mode) {
    uint8_t message[8];
    message[0] = 0xFF;   // ヘッダ
    message[1] = 0xFF;   // ヘッダ
    message[2] = id;     // サーボID
    message[3] = 4;      // パケット長 (IDからChecksumまでのデータ数)
    message[4] = 3;      // 命令: WRITE (書き込み)
    message[5] = 33;     // 書き込む住所: 33番 (Running Mode)
    message[6] = mode;   // 0 または 1

    // チェックサムの計算 (IDからデータ末尾までを足して反転)
    uint8_t checksum = 0;
    for (int i = 2; i < 7; i++) {
        checksum += message[i];
    }
    message[7] = ~checksum;

    // 送信処理
    HAL_HalfDuplex_EnableTransmitter(&huart1);
    HAL_UART_Transmit(&huart1, message, 8, 10);

    // 送信完了を待つ (TCフラグ)
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);

    HAL_HalfDuplex_EnableReceiver(&huart1);
}

void Pos_command(uint8_t id, uint16_t position) {
    uint8_t message[13] = {0xFF, 0xFF, id, 9, 3, 42};
    message[6] = position & 0xFF;
    message[7] = (position >> 8) & 0xFF;
    message[8] = 0x00; message[9] = 0x00; // Time
    message[10] = 0x00; message[11] = 0x00; // Speed
    message[12] = STS_CalcChecksum(message, 13);

    HAL_HalfDuplex_EnableTransmitter(&huart1);
    HAL_UART_Transmit(&huart1, message, 13, 10);
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);
    HAL_HalfDuplex_EnableReceiver(&huart1);
}

int16_t STS3215_GetPosition(uint8_t id) {
    uint8_t tx_msg[8] = {0xFF, 0xFF, id, 4, 2, 56, 2, 8};
    uint8_t rx_msg[8] = {0};
    tx_msg[7] = STS_CalcChecksum(tx_msg, 8);

    // 1. 受信バッファの掃除（OREエラー等をリセット）
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_FLUSH_DRREGISTER(&huart1);

    // 2. 送信モードに切り替えて送信
    HAL_HalfDuplex_EnableTransmitter(&huart1);
    HAL_UART_Transmit(&huart1, tx_msg, 8, 10);

    // 3. 完全に送り終わるのを待つ
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);

    // 4. 受信モードに切り替え
    HAL_HalfDuplex_EnableReceiver(&huart1);
    // 5. 受信 (タイムアウト20ms)
    // 1-wireの場合、自分の送信データがバッファに入る場合があるため
    // 実際のデータが来るまで少し待機が必要な場合があります
    if (HAL_UART_Receive(&huart1, rx_msg, 8, 20) == HAL_OK) {
        if (rx_msg[0] == 0xFF && rx_msg[1] == 0xFF) {
            return (int16_t)((rx_msg[6] << 8) | rx_msg[5]);
        }
    }
    return -1;
}


static inline int16_t deg_to_ticks(float deg) {
    // 4096/360 = 11.377... 1ステップ≈0.08789°
    float ticks = deg * (4096.0f / 360.0f);
    int32_t it = (int32_t)lroundf(ticks); // <math.h>
    if (it < -4096) it = -4096; // 念のため過大防止
    if (it >  4096) it =  4096;
    return (int16_t)it;
}



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(50); // サーボの安定待ち

    // 起動時の現在位置を取得して基準にする
    //int16_t startPos = -1;
    
    for(int i=0; i<100; i++) { // 10回リトライ
        startPos = STS3215_GetPosition(servoID);
        if(startPos != -1) break;
        HAL_Delay(50);
    }

    if (startPos != -1) {
        centerPosition = startPos;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // 成功:LED消灯
    } else {
        centerPosition = 2048; // 失敗:デフォルト
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);   // 失敗:LED点灯
    }

    HAL_Delay(100); // 10秒待機は長いので短縮


     if (startPos != -1) {
        // 2. 目標位置を計算 (startPos から +30度)
        int16_t designate = deg_to_ticks(110.0f);
        int targetPosition = startPos - designate; //マイナスが閉まる方向

        // 0-4095の範囲を超えないようにガード
        if (targetPosition > 4095) targetPosition = 4095;
        if (targetPosition < 0)    targetPosition = 0;

        // 3. 移動命令を送る
        // (もしゆっくり動かしたい場合は、Pos_command内のspeed引数を調整してください)
        Pos_command(servoID, targetPosition);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // 成功
    } else {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);   // 失敗
    }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {




    /* USER CODE END WHILE */



    /* USER CODE BEGIN 3 */
    HAL_Delay(1000);


	  // ID 1のサーボを「連続回転モード」にする
	  //STS_SetMode(1, 1);
	  //HAL_Delay(10); // 反映待ち

	  // 連続回転モードの時は、Pos_commandの「位置」が「回転速度」に変わります
	  // (速度を指定して回し続けることができます)
	  //Pos_command(1, 500);
}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
