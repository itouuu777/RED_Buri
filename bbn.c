#include "gpio.h"
#include "c620_can.hpp"
#include "c620_control.hpp"
#include "sts3215.hpp"
#include "usart.h"
#include "cmath"
#include <cstdio>

C620CAN c620_can;
C620Control c620_control(c620_can);

namespace run 
{
    STS3215 servo_1(&huart1, 1);
    int16_t targetAngle = 60;

    void setup()
    {
        servo_1.setPosition(200);
        int16_t currentPos = servo_1.AdjustPosition();
        printf("%d\n", currentPos);
        if (currentPos == 2048){
            printf("%d\n", currentPos);
            printf("not");
        }
        
    }
    
    void loop()
    {
        servo_1.setPosition(10);
        servo_1.moveRelative(20);
        
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
