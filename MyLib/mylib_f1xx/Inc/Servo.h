#ifndef __SERVO_H__
#define __SERVO_H__

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"

typedef struct {
    TIM_HandleTypeDef *htim;
    uint16_t CHANNEL;
} Servo;

void SERVO_Init(Servo *sv, TIM_HandleTypeDef *htim, uint16_t CHANNEL);
void SERVO_WRITE(Servo *sv, uint8_t ANGLE);

#endif /* __SERVO_H__ */
