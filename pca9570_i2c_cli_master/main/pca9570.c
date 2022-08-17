#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "pca9570.h"

/******************************************************************************
 *
 * Description:
 *    Initialization PCA9570 : Do nothing
 *
 *****************************************************************************/
void pca9570_init(void)
{
    /* Do Nothing */
}

/* Default buffer send to Salve. 0x03: P0 to P2 are set to high (LED Off) */
uint8_t write_buf[2] = {0, 0x03}; 

/******************************************************************************
 *
 * Description:
 *    Set LED states (on or off).
 *
 * Params:
 *    [in]  pca_led_color  - The color LEDs need to turn on or off.
 *                           LED_GREEN  :   LED Green
 *                           LED_BLUE   :   LED Blue
 *                           LED_RED    :   LED Red
 *
 *    [in]  led_state      - The state of led (LED_ON/LED_OFF)
 *                           LED_ON :   Turn LED on
 *                           LED_OFF:   Turn the LED off
 *
 *****************************************************************************/
void pca9570_set_led(uint8_t led_color, bool led_state)
{
    if (!led_state)
    { /* 0 mean turn LED on */
        write_buf[1] &= ~(1 << led_color);
    }
    if (led_state)
    { /* 1 mean turn LED off */
        write_buf[1] |= 1 << led_color;
    }
    i2c_master_write_to_device(I2C_NUM_0, PCA9570_ADR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
}

/******************************************************************************
 *
 * Description:
 *    Toggle LED states
 *
 * Params:
 *    [in]  pca_led_color  - The color LEDs need to turn on or off.
 *                           LED_GREEN  :   LED Green
 *                           LED_BLUE   :   LED Blue
 *                           LED_RED    :   LED Red
 *
 *****************************************************************************/
void pca9570_toggle_led(uint8_t led_color)
{
    write_buf[1] ^= 1 << led_color;
    i2c_master_write_to_device(I2C_NUM_0, PCA9570_ADR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
}