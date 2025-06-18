#include "button.h"

/**
 * @brief Initialize the button configuration
 * @param p_button: Pointer to the Button structure
 * @param GPIOx: GPIO port (e.g., GPIOA, GPIOB)
 * @param GPIO_Pin: GPIO pin number
 * @param active_level: Logic level when button is active (BUTTON_ACTIVE_LOW or BUTTON_ACTIVE_HIGH)
 */
void Button_Init(Button_t* p_button, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, ButtonActiveLevel active_level) {
    if (p_button == NULL) return;

    // Basic GPIO configuration
    p_button->GPIOx = GPIOx;
    p_button->GPIO_Pin = GPIO_Pin;
    p_button->active_level = active_level;

    // Initialize default button state
    uint8_t default_state = (active_level == BUTTON_ACTIVE_LOW) ? 1 : 0;
    p_button->btn_current = default_state;
    p_button->btn_last = default_state;
    p_button->btn_filter = default_state;
    p_button->is_deboucing = 0;

    // Reset timing variables
    p_button->time_debounce = 0;
    p_button->last_tick = 0;
    p_button->press_time = 0;
    p_button->release_time = 0;

    // Reset advanced state flags
    p_button->is_pressed = 0;
    p_button->is_long_pressed = 0;
    p_button->is_very_long_pressed = 0;
    p_button->is_holding = 0;
    p_button->press_count = 0;
    p_button->last_press_time = 0;

    // Reset all callbacks to NULL
    p_button->pressed_callback = NULL;
    p_button->released_callback = NULL;
    p_button->short_press_callback = NULL;
    p_button->long_press_callback = NULL;
    p_button->very_long_press_callback = NULL;
    p_button->double_press_callback = NULL;
    p_button->hold_callback = NULL;
}

/**
 * @brief Assign a callback to a specific button event
 * @param p_button: Pointer to the Button structure
 * @param event: Event type to assign
 * @param callback: Function pointer to the callback
 */
void Button_SetCallback(Button_t* p_button, ButtonEvent event, void (*callback)(void)) {
    if (p_button == NULL) return;

    switch (event) {
        case BUTTON_EVENT_PRESSED:
            p_button->pressed_callback = callback;
            break;
        case BUTTON_EVENT_RELEASED:
            p_button->released_callback = callback;
            break;
        case BUTTON_EVENT_SHORT_PRESS:
            p_button->short_press_callback = callback;
            break;
        case BUTTON_EVENT_LONG_PRESS:
            p_button->long_press_callback = callback;
            break;
        case BUTTON_EVENT_VERY_LONG_PRESS:
            p_button->very_long_press_callback = callback;
            break;
        case BUTTON_EVENT_DOUBLE_PRESS:
            p_button->double_press_callback = callback;
            break;
        case BUTTON_EVENT_HOLD:
            p_button->hold_callback = callback;
            break;
        default:
            break;
    }
}

/**
 * @brief Perform debouncing logic for the button
 * @param p_button: Pointer to the Button structure
 * @param current_tick: Current system tick (e.g., HAL_GetTick())
 * @return uint8_t: 1 if state changed, 0 otherwise
 */
uint8_t Button_Debounce(Button_t* p_button, uint32_t current_tick) {
    if (p_button == NULL) return 0;

    uint8_t state_changed = 0;

    // Read current raw pin state
    uint8_t gpio_state = HAL_GPIO_ReadPin(p_button->GPIOx, p_button->GPIO_Pin);

    // If state differs from filtered value, start or continue debounce
    if (gpio_state != p_button->btn_filter) {
        if (p_button->is_deboucing == 0) {
            p_button->is_deboucing = 1;
            p_button->time_debounce = current_tick;
        }
        else if ((current_tick - p_button->time_debounce) >= BTN_DEBOUNCE_TIME) {
            // State confirmed stable after debounce delay
            p_button->btn_last = p_button->btn_current;
            p_button->btn_current = gpio_state;
            p_button->btn_filter = gpio_state;
            p_button->is_deboucing = 0;
            state_changed = 1;
        }
    } else {
        // Reset debounce if no change detected
        p_button->is_deboucing = 0;
    }

    return state_changed;
}

/**
 * @brief Check if the button is currently pressed
 * @param p_button: Pointer to the Button structure
 * @return uint8_t: 1 if pressed, 0 if not
 */
uint8_t Button_IsPressed(Button_t* p_button) {
    if (p_button == NULL) return 0;

    uint8_t pressed_state = (p_button->active_level == BUTTON_ACTIVE_LOW) ? 0 : 1;
    return (p_button->btn_current == pressed_state);
}

/**
 * @brief Query a specific button event status
 * @param p_button: Pointer to the Button structure
 * @param event: Button event type
 * @return uint8_t: 1 if the event has occurred, 0 otherwise
 */
uint8_t Button_GetEvent(Button_t* p_button, ButtonEvent event) {
    if (p_button == NULL) return 0;

    switch (event) {
        case BUTTON_EVENT_PRESSED:
            return p_button->is_pressed;
        case BUTTON_EVENT_LONG_PRESS:
            return p_button->is_long_pressed;
        case BUTTON_EVENT_VERY_LONG_PRESS:
            return p_button->is_very_long_pressed;
        case BUTTON_EVENT_HOLD:
            return p_button->is_holding;
        default:
            return 0;
    }
}

/**
 * @brief Handle and evaluate all button events
 * @param p_button: Pointer to the Button structure
 * @param current_tick: Current system tick
 */
void Button_Handle(Button_t* p_button, uint32_t current_tick) {
    if (p_button == NULL) return;

    // Check for debounced state change
    uint8_t state_changed = Button_Debounce(p_button, current_tick);

    if (state_changed) {
        uint8_t is_pressed = Button_IsPressed(p_button);

        if (is_pressed && !p_button->is_pressed) {
            // Button just pressed
            p_button->is_pressed = 1;
            p_button->press_time = current_tick;
            p_button->is_long_pressed = 0;
            p_button->is_very_long_pressed = 0;

            if (p_button->pressed_callback != NULL) {
                p_button->pressed_callback();
            }

            // Check for double press
            if (current_tick - p_button->last_press_time < BTN_DOUBLE_PRESS_INTERVAL) {
                p_button->press_count++;

                if (p_button->press_count >= 2) {
                    if (p_button->double_press_callback != NULL) {
                        p_button->double_press_callback();
                    }
                    p_button->press_count = 0;
                }
            } else {
                p_button->press_count = 1;
            }

            p_button->last_press_time = current_tick;
        }
        else if (!is_pressed && p_button->is_pressed) {
            // Button just released
            p_button->is_pressed = 0;
            p_button->is_holding = 0;
            p_button->release_time = current_tick;
            uint32_t press_duration = current_tick - p_button->press_time;

            if (p_button->released_callback != NULL) {
                p_button->released_callback();
            }

            // Handle short press
            if (!p_button->is_long_pressed && !p_button->is_very_long_pressed &&
                press_duration < BTN_SHORT_THRESHOLD) {
                if (p_button->short_press_callback != NULL) {
                    p_button->short_press_callback();
                }
            }
        }
    }

    // Check for long, very long press, and hold (checked continuously)
    if (p_button->is_pressed) {
        uint32_t press_duration = current_tick - p_button->press_time;

        // Very long press
        if (!p_button->is_very_long_pressed && press_duration >= BTN_VERY_LONG_THRESHOLD) {
            p_button->is_very_long_pressed = 1;

            if (p_button->very_long_press_callback != NULL) {
                p_button->very_long_press_callback();
            }
        }
        // Long press
        else if (!p_button->is_long_pressed && press_duration >= BTN_LONG_THRESHOLD) {
            p_button->is_long_pressed = 1;

            if (p_button->long_press_callback != NULL) {
                p_button->long_press_callback();
            }
        }

        // Hold callback (repeated)
        if (p_button->hold_callback != NULL) {
            if (!p_button->is_holding) {
                if (press_duration >= BTN_HOLD_INTERVAL) {
                    p_button->is_holding = 1;
                    p_button->last_tick = current_tick;
                    p_button->hold_callback();
                }
            } else {
                if (current_tick - p_button->last_tick >= BTN_HOLD_INTERVAL) {
                    p_button->last_tick = current_tick;
                    p_button->hold_callback();
                }
            }
        }
    }
}
