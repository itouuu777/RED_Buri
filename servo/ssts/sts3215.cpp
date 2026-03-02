// Core/Src/STS3215.cpp
#include "STS3215.hpp"

STS3215::STS3215(UART_HandleTypeDef* huart, uint8_t id,
                 GPIO_TypeDef* ledPort, uint16_t ledPin)
: huart_(huart), id_(id), startPos_(-1),
  ledPort_(ledPort), ledPin_(ledPin)
{}

uint8_t STS3215::calcChecksum(const uint8_t* msg, size_t len) {
    // 仕様：msg[2] ～ msg[len-2] の加算を反転 → msg[len-1]
    uint8_t sum = 0;
    for (size_t i = 2; i + 1 < len; ++i) sum += msg[i];
    return static_cast<uint8_t>(~sum);
}

HAL_StatusTypeDef STS3215::setMode(uint8_t mode) {
    uint8_t msg[8] = {0xFF, 0xFF, id_, 4, 3, 33, mode, 0};
    msg[7] = calcChecksum(msg, sizeof(msg));

    toTx();
    auto st = HAL_UART_Transmit(huart_, msg, sizeof(msg), 30);
    if (st != HAL_OK) return st;
    while (__HAL_UART_GET_FLAG(huart_, UART_FLAG_TC) == RESET) { /*wait*/ }
    toRx();
    return HAL_OK;
}

HAL_StatusTypeDef STS3215::setPosition(uint16_t position, uint16_t time_ms, uint16_t speed) {
    uint8_t msg[13] = {0xFF, 0xFF, id_, 9, 3, 42};
    msg[6]  = static_cast<uint8_t>(position & 0xFF);
    msg[7]  = static_cast<uint8_t>((position >> 8) & 0xFF);
    msg[8]  = static_cast<uint8_t>(time_ms & 0xFF);
    msg[9]  = static_cast<uint8_t>((time_ms >> 8) & 0xFF);
    msg[10] = static_cast<uint8_t>(speed & 0xFF);
    msg[11] = static_cast<uint8_t>((speed >> 8) & 0xFF);
    msg[12] = calcChecksum(msg, sizeof(msg));

    toTx();
    auto st = HAL_UART_Transmit(huart_, msg, sizeof(msg), 30);
    if (st != HAL_OK) return st;
    while (__HAL_UART_GET_FLAG(huart_, UART_FLAG_TC) == RESET) { /*wait*/ }
    toRx();
    return HAL_OK;
}

int16_t STS3215::getPosition(uint32_t timeout_ms) {
    // Read 命令: FF FF ID 04 02 38 02 CHK
    uint8_t tx_msg[8] = {0xFF, 0xFF, id_, 4, 2, 56, 2, 0};
    tx_msg[7] = calcChecksum(tx_msg, sizeof(tx_msg));

    // 送信前に受信側の残留を掃除
    __HAL_UART_CLEAR_OREFLAG(huart_);
#if defined(__HAL_UART_CLEAR_IDLEFLAG)
    __HAL_UART_CLEAR_IDLEFLAG(huart_);
#endif
#if defined(__HAL_UART_FLUSH_DRREGISTER)
    __HAL_UART_FLUSH_DRREGISTER(huart_);
#endif

    toTx();
    if (HAL_UART_Transmit(huart_, tx_msg, sizeof(tx_msg), timeout_ms) != HAL_OK) return -1;
    while (__HAL_UART_GET_FLAG(huart_, UART_FLAG_TC) == RESET) { /*wait*/ }

    toRx();

    // ★ 半二重の安定化：送→受の切替直後に 1ms ガード
    HAL_Delay(1);

    // 念のため再クリア
    __HAL_UART_CLEAR_OREFLAG(huart_);
#if defined(__HAL_UART_CLEAR_IDLEFLAG)
    __HAL_UART_CLEAR_IDLEFLAG(huart_);
#endif

    // --- ヘッダ受信 ---
    // [0]=0xFF, [1]=0xFF, [2]=ID, [3]=LEN(=ERR+PARAM+CHK), [4]=ERR
    uint8_t hdr[5] = {0};
    if (HAL_UART_Receive(huart_, hdr, 5, timeout_ms) != HAL_OK) return -1;
    if (hdr[0] != 0xFF || hdr[1] != 0xFF || hdr[2] != id_) return -1;

    uint8_t len = hdr[3]; // ERR + PARAM + CHK
    if (len < 3 || len > 20) return -1;

    // --- ボディ受信（PARAM + CHK）---
    uint8_t body[32] = {0};
    if (HAL_UART_Receive(huart_, body, len, timeout_ms) != HAL_OK) return -1;

    // --- チェックサム検証 ---
    uint8_t frame[5 + 32];
    std::memcpy(frame,     hdr,  5);
    std::memcpy(frame + 5, body, len);
    uint8_t chk = calcChecksum(frame, 5 + len);
    if (chk != frame[5 + len - 1]) return -1;

    // 位置抽出（PARAM=2byte想定: Low=body[1], High=body[2]）
    // body[0] は ERR、body[1..] が PARAM…
    if (len < 3) return -1;
    int16_t pos = static_cast<int16_t>((body[2] << 8) | body[1]);
    return pos;
}

int16_t STS3215::syncCenter(uint32_t retries, uint32_t interval_ms) {
    HAL_Delay(50); // 安定待ち
    int16_t pos = -1;
    for (uint32_t i = 0; i < retries; ++i) {
        pos = getPosition();
        if (pos != -1) {
            startPos_ = pos;
            ledOff(); // 成功＝LED消灯
            return pos;
        }
        HAL_Delay(interval_ms);
    }
    // 失敗時のフォールバック：2048 & LED点灯
    startPos_ = 2048;
    ledOn();
    return startPos_;
}

void STS3215::moveRelativeTicks(int16_t offset_ticks) {
    if (startPos_ == -1) {
        // まだ基準なし → 一度だけ取得を試みる
        if (syncCenter(50, 10) == -1) return;
    }
    int32_t target = static_cast<int32_t>(startPos_) + static_cast<int32_t>(offset_ticks);
    if (target > 4095) target = 4095;
    if (target < 0)    target = 0;
    (void)setPosition(static_cast<uint16_t>(target), /*time*/0, /*speed*/0);
}

void STS3215::moveRelativeDeg(float offset_deg) {
    moveRelativeTicks(degToTicks(offset_deg));
}

int16_t STS3215::degToTicks(float deg) {
    // 4096/360 ≒ 11.377... 1tick ≈ 0.08789°
    float ticks = deg * (4096.0f / 360.0f);
#if defined(__GNUC__)
    long it = lroundf(ticks);
#else
    long it = static_cast<long>(ticks >= 0 ? ticks + 0.5f : ticks - 0.5f);
#endif
    if (it < -4096) it = -4096;
    if (it >  4096) it =  4096;
    return static_cast<int16_t>(it);
}
