#ifndef BUTTON_H
#define BUTTON_H

#include "stm32f1xx_hal.h"

// Timing thresholds for different button events (in milliseconds)
#define BTN_SHORT_THRESHOLD         300   // Max duration for a short press
#define BTN_LONG_THRESHOLD          1000  // Minimum duration for a long press
#define BTN_VERY_LONG_THRESHOLD     3000  // Minimum duration for a very long press
#define BTN_DOUBLE_PRESS_INTERVAL   300   // Max interval between two presses for a double press
#define BTN_HOLD_INTERVAL           200   // Interval between repeated hold callbacks
#define BTN_DEBOUNCE_TIME			15	  // Button debounce time

// Button active level type
typedef enum {
    BUTTON_ACTIVE_LOW,   // Pull-up configuration (pressed = LOW)
    BUTTON_ACTIVE_HIGH   // Pull-down configuration (pressed = HIGH)
} ButtonActiveLevel;

// Button event types
typedef enum {
    BUTTON_EVENT_NONE,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS,
    BUTTON_EVENT_VERY_LONG_PRESS,
    BUTTON_EVENT_DOUBLE_PRESS,
    BUTTON_EVENT_HOLD
} ButtonEvent;

// Main button structure
typedef struct {
    GPIO_TypeDef* GPIOx;         // GPIO port (e.g., GPIOA, GPIOB)
    uint16_t GPIO_Pin;           // GPIO pin number (e.g., GPIO_PIN_0)
    ButtonActiveLevel active_level; // Active level configuration (pull-up or pull-down)

    // Raw button state variables
    uint8_t btn_current;         // Current raw state
    uint8_t btn_last;            // Previous raw state
    uint8_t btn_filter;          // Debounced state
    uint8_t is_deboucing;        // Debouncing in progress

    // Timing variables
    uint32_t time_debounce;      // Timestamp when debounce started
    uint32_t last_tick;          // General-purpose last tick timestamp
    uint32_t press_time;         // Timestamp when button was pressed
    uint32_t release_time;       // Timestamp when button was released

    // Extended button state flags
    uint8_t is_pressed;              // Button is currently pressed
    uint8_t is_long_pressed;         // Long press was detected
    uint8_t is_very_long_pressed;    // Very long press was detected
    uint8_t is_holding;              // Button is being held
    uint8_t press_count;             // Number of consecutive presses (for double press)
    uint32_t last_press_time;        // Timestamp of the previous press (for double press)

    // Callback function pointers for events
    void (*pressed_callback)(void);          // Called when button is pressed
    void (*released_callback)(void);         // Called when button is released
    void (*short_press_callback)(void);      // Called on short press
    void (*long_press_callback)(void);       // Called on long press
    void (*very_long_press_callback)(void);  // Called on very long press
    void (*double_press_callback)(void);     // Called on double press
    void (*hold_callback)(void);             // Called continuously while button is held
} Button_t;

// Function prototypes
void Button_Init(Button_t* p_button, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, ButtonActiveLevel active_level);
void Button_SetCallback(Button_t* p_button, ButtonEvent event, void (*callback)(void));
uint8_t Button_Debounce(Button_t* p_button, uint32_t current_tick);
void Button_Handle(Button_t* p_button, uint32_t current_tick);
uint8_t Button_IsPressed(Button_t* p_button);
uint8_t Button_GetEvent(Button_t* p_button, ButtonEvent event);

#endif /* BUTTON_H */
