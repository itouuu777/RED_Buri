int16_t STS3215::getPosition() {
    uint8_t tx_msg[8] = {0xFF, 0xFF, _id, 4, 0x02, 56, 2, 0x00};
    tx_msg[7] = calcChecksum(tx_msg, 8);

    uint8_t rx_msg[8] = {0};

    // 送信
    HAL_HalfDuplex_EnableTransmitter(_huart);
    HAL_UART_Transmit(_huart, tx_msg, 8, 10);
    while(__HAL_UART_GET_FLAG(_huart, UART_FLAG_TC) == RESET);

    // 受信に切替
    HAL_HalfDuplex_EnableReceiver(_huart);

    // (A) 切替直後の小待ち（数百us〜1ms）
    for (volatile int i=0; i<3000; ++i) __NOP__();

    // (B) 受信直前にフラグをクリア
    __HAL_UART_CLEAR_OREFLAG(_huart);
    #ifdef UART_FLAG_RXNE
    __HAL_UART_CLEAR_FLAG(_huart, UART_FLAG_RXNE);
    #endif

    // (C) タイムアウトを余裕ある値に
    if (HAL_UART_Receive(_huart, rx_msg, 8, 100) == HAL_OK) {
        if (rx_msg[0]==0xFF && rx_msg[1]==0xFF && rx_msg[2]==_id) {
            return (int16_t)((rx_msg[6] << 8) | rx_msg[5]);
        }
    }
    return -1;
}
