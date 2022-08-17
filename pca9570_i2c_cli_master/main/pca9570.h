#ifndef PCA9570_H_  
#define PCA9570_H_

#define I2C_MASTER_TIMEOUT_MS       10
#define PCA9570_ADR                 0x24

#define LED_GREEN                   0       /*!< LED Green on Pin P0 */
#define LED_BLUE                    1       /*!< LED Blue on Pin P1 */
#define LED_RED                     2       /*!< LED Red on Pin P2 */
#define LED_ON                      0
#define LED_OFF                     1


#endif

void pca9570_init(void);
void pca9570_set_led(uint8_t led_color, bool led_state);
void pca9570_toggle_led(uint8_t led_color);