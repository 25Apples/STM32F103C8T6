#include "Servo.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void SERVO_WRITE(Servo *sv, uint8_t ANGLE)
{
    // ANGLE từ 0 → 180
    // Tương ứng với độ rộng xung từ khoảng 544 → 2400 (micro giây)
    uint16_t CCR = map(ANGLE, 0, 180, 544, 2400);

    switch (sv->CHANNEL)
    {
        case TIM_CHANNEL_1:
            sv->htim->Instance->CCR1 = CCR;
            break;

        case TIM_CHANNEL_2:
            sv->htim->Instance->CCR2 = CCR;
            break;

        case TIM_CHANNEL_3:
            sv->htim->Instance->CCR3 = CCR;
            break;

        case TIM_CHANNEL_4:
            sv->htim->Instance->CCR4 = CCR;
            break;
    }
}

void SERVO_Init(Servo *sv, TIM_HandleTypeDef *htim, uint16_t CHANNEL)
{
    sv->htim = htim;
    sv->CHANNEL = CHANNEL;
    HAL_TIM_PWM_Start(htim, CHANNEL);  // Khởi động PWM ở kênh tương ứng
}
