#include "sts3215.hpp"

STS3215::STS3215(UART_HandleTypeDef* huart, uint8_t id)
 : _huart(huart), _id(id), _startPos(-1) {}

uint8_t STS3215::calcChecksum(uint8_t *msg, uint8_t len){
    uint8_t checksum = 0;
    for(int i=2; i < len - 1; i++){
        checksum += msg[i];
    }
    return ~checksum //ビット反転
}
