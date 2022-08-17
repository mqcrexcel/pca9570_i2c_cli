/*
    i2c - Scanner
*/



#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "pca9570.h"

#define I2C_MASTER_SCL_IO 4
#define I2C_MASTER_SDA_IO 12
#define I2C_MASTER_NUM 0            /* I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /* I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /* I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /* I2C master doesn't need buffer */

#define EX_UART_NUM UART_NUM_0
#define BUF_SIZE (1024)

char *array_cmd[3];
int delay_for_blink = 0;

/******************************************************************************
 *
 * Description:
 *    Initialization i2C master
 *
 *****************************************************************************/
static esp_err_t i2c_master_init(void)
{
        int i2c_master_port = I2C_MASTER_NUM;
        i2c_config_t conf = {
                .mode = I2C_MODE_MASTER,
                .sda_io_num = I2C_MASTER_SDA_IO,
                .scl_io_num = I2C_MASTER_SCL_IO,
                .sda_pullup_en = GPIO_PULLUP_ENABLE,
                .scl_pullup_en = GPIO_PULLUP_ENABLE,
                .master.clk_speed = I2C_MASTER_FREQ_HZ,
        };
        i2c_param_config(i2c_master_port, &conf);
        return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/******************************************************************************
 *
 * Description:
 *    Initialization UART driver
 *
 *****************************************************************************/        
void uart_init(void)
{
        uart_config_t uart_config = {
                .baud_rate = 115200,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .source_clk = UART_SCLK_APB,
        };
        uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
        uart_param_config(EX_UART_NUM, &uart_config);
        uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void demo_led_mode(void)
{
        pca9570_set_led(LED_RED, LED_ON);
        vTaskDelay(500 / portTICK_RATE_MS);
        pca9570_set_led(LED_RED, LED_OFF);
        vTaskDelay(500 / portTICK_RATE_MS);
        pca9570_set_led(LED_BLUE, LED_ON);
        vTaskDelay(500 / portTICK_RATE_MS);
        pca9570_set_led(LED_BLUE, LED_OFF);
        vTaskDelay(500 / portTICK_RATE_MS);
        for (int i = 0; i < 20; i++)
        {
                pca9570_toggle_led(LED_GREEN);
                vTaskDelay(100 / portTICK_RATE_MS);
        }
}

/******************************************************************************
 *
 * Description:
 *    Blink LED in 10 cycles.
 *
 * Params:
 *    [in]  color         - The color LEDs need to blink.
 *                           LED_GREEN  :   LED Green
 *                           LED_BLUE   :   LED Blue
 *                           LED_RED    :   LED Red
 *
 *    [in]  interval      - The interval in ms
 *
 *****************************************************************************/
void blink_led(uint8_t color, uint interval)
{   
        for (int count = 0; count <= 10; count ++)
        {
                pca9570_toggle_led(color);
                vTaskDelay(interval / portTICK_RATE_MS);
        }
        pca9570_set_led(color, LED_OFF);
}

void app_main(void)
{       
        /* Initial i2C */
        i2c_master_init();
        /* Initial uart */
        uart_init();
        /* Initial pca9570 */
        pca9570_init();
        demo_led_mode();
        
        /* Buffer data for uart read */
        uint8_t *data = (uint8_t*)malloc(BUF_SIZE);
        printf("\n"
               "Type 'help' to get the list of commands.\n");

        /* Main loop */
        while (true)
        {   
                char *array_split;
                uint8_t array_number = 0;
                const int len = uart_read_bytes(EX_UART_NUM, data, BUF_SIZE, 100/portTICK_RATE_MS);    
                
                if (len > 0) {                   
                        data[len] = 0;
                        array_split = strtok((const char*) data," ");
                        
                        while (array_split != NULL) {
                            
                                if (array_number <= 3) {
                                        array_cmd[array_number ++] = array_split;
                                }

                                if (array_number > 3) {
                                        printf("\n"
                                               " Too many arguments, please try agian.\n"
                                               "Type 'help' to get the list of commands.\n");
                                        memset(array_cmd, 0, sizeof array_cmd);
                                        break;       
                                }
                                array_split = strtok(NULL, " ");
                        }
                }

                else {}/* Do nothing ! */

                /* CMD Interface processing for pattern 'help' */        
                if (array_cmd[0]!= NULL && strcmp(array_cmd[0], "help") == 0) {
                        printf("\n"
                               "The command follow struture flow as below.\n"
                               "[pattern] [color] [parameter(optional)].\n"
                               "Where:.\n"
                               "[pattern] is either 'blink', 'shine' or 'off'.\n" 
                               "[color] is either 'green', 'red' or 'blue'.\n"
                               ".\n"
                               "[parameter] is:.\n"
                               "blinking interval in ms for 'blink' pattern.\n"
                               "'cancel' which will set all LEDs off.\n"
                               "'demo' which will cycle through different modes in whichever way/order.\n");
                }

                /* CMD Interface processing for pattern 'cancel' */     
                else if (array_cmd[0]!= NULL && strcmp(array_cmd[0], "cancel") == 0) {
                        /* Set all LED off */
                        pca9570_set_led(LED_RED, LED_OFF);
                        pca9570_set_led(LED_BLUE, LED_OFF);
                        pca9570_set_led(LED_GREEN, LED_OFF);     
                        printf("\n"
                               "Sell all LEDs off\n");         
                }

                /* CMD Interface processing for pattern 'demo' */ 
                else if (array_cmd[0]!= NULL && strcmp(array_cmd[0], "demo") == 0) { 
                        /* Run demo program */   
                        printf("\n"
                               "Go to demo led mode\n");   
                        demo_led_mode();             
                }
                
                /* CMD Interface processing for pattern 'shine' */ 
                else if (array_cmd[0]!= NULL && strcmp(array_cmd[0], "shine") == 0) {

                        /* Shine processing for LED RED */    
                        if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "red") == 0) {
                                pca9570_set_led(LED_RED, LED_ON);
                                printf("\n"
                                       "LED RED on\n");
                        }

                        /* Shine processing for LED BLUE */           
                        else if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "blue") == 0) {
                                pca9570_set_led(LED_BLUE, LED_ON);
                                printf("\n"
                                       "LED BLUE on\n");
                        }

                        /* Shine processing for LED GREEN */  
                        else if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "green") == 0) {
                                pca9570_set_led(LED_GREEN, LED_ON);
                                printf("\n"
                                       "LED GREEN on\n");
                        }

                        /* Wrong color processing */ 
                        else 
                        {
                                printf("\n"
                                       "[color] incorrect, please try agian.\n"
                                       "Type 'help' to get the list of commands.\n");
                        }
                } 

                /* CMD Interface processing for pattern 'off' */ 
                else if (array_cmd[0]!= NULL && strcmp(array_cmd[0], "off") == 0) {

                        /* Off processing for LED RED */      
                        if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "red") == 0) {
                                pca9570_set_led(LED_RED, LED_OFF);
                                printf("\n"
                                       "LED RED off\n");
                        }

                        /* Off processing for LED BLUE */           
                        else if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "blue") == 0) {
                                pca9570_set_led(LED_BLUE, LED_OFF);
                                printf("\n"
                                       "LED BLUE off\n");
                        }

                        /* Off processing for LED GREEN */   
                        else if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "green") == 0) {
                                pca9570_set_led(LED_GREEN, LED_OFF);
                                printf("\n"
                                       "LED GREEN off\n");
                        }
                        
                        /* Wrong color processing */ 
                        else 
                        {
                            printf("\n"
                                   "[color] incorrect, please try agian.\n"
                                   "Type 'help' to get the list of commands\n");
                        }       
                } 

                /* CMD Interface processing for pattern 'blink' */ 
                else if (array_cmd[0]!= NULL && strcmp(array_cmd[0], "blink") == 0) {
                        
                        /* Blink processing for LED RED */ 
                        if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "red") == 0) {    

                                if(array_cmd[2] != NULL) {
                                        delay_for_blink = atoi(array_cmd[2]);
                                        
                                        /* Wrong interval value processing */
                                        if(delay_for_blink == 0) {
                                                printf("\n"
                                                       "Delay value is Zero or invalid, please try agian\n");               
                                        }

                                        else if(delay_for_blink > 0) {
                                                blink_led(LED_RED, delay_for_blink);
                                                printf("\n"
                                                       "LED RED blinking\n");            
                                        }
                                        
                                        else {}/* Do nothing ! */
                                }

                                /* Missing interval value processing */
                                else if(array_cmd[2] == NULL) {
                                        printf("\n"
                                               "Please input interval value\n");    
                                }

                                else {}/* Do nothing ! */  
                        }

                        /* Blink processing for LED BLUE */     
                        else if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "blue") == 0) {                               
                                
                                if(array_cmd[2] != NULL) {
                                        delay_for_blink = atoi(array_cmd[2]);
                                        
                                        /* Wrong interval value processing */
                                        if(delay_for_blink == 0) {
                                            printf("\n"
                                                   "Delay value is Zero or invalid, please try agian\n");               
                                        }

                                            else if(delay_for_blink > 0) {
                                            blink_led(LED_BLUE, delay_for_blink);
                                            printf("\n"
                                                   "LED BLUE blinking\n");            
                                        }

                                        else {}/* Do nothing ! */
                                }

                                /* Missing interval value processing */
                                else if(array_cmd[2] == NULL) {
                                        printf("\n"
                                               "Please input interval value\n");    
                                        }

                                else {}/* Do nothing ! */  
                        }

                        /* Blink processing for LED GREEN */ 
                        else if (array_cmd[1]!= NULL && strcmp(array_cmd[1], "green") == 0) {    

                                if(array_cmd[2] != NULL) {
                                        delay_for_blink = atoi(array_cmd[2]);

                                        /* Wrong interval value processing */
                                        if(delay_for_blink == 0) {
                                                printf("\n"
                                                       "Delay value is Zero or invalid, please try agian\n");               
                                        }

                                        else if(delay_for_blink > 0) {
                                                blink_led(LED_GREEN, delay_for_blink);
                                                printf("\n"
                                                       "LED GREEN blinking\n");            
                                        }

                                        else {}/* Do nothing ! */
                                }

                                /* Missing interval value processing */
                                else if(array_cmd[2] == NULL) {
                                        printf("\n"
                                               "Please input interval value\n");    
                                }

                                else {}/* Do nothing ! */  
                        }

                        else 
                        {
                            printf("\n"
                                "[color] incorrect, please try agian.\n"
                                "Type 'help' to get the list of commands.\n");
                        }
                }

                /* Wrong  pattern processing */ 
                else if (array_cmd[0]!= NULL)
                {
                        printf("\n"
                               "[pattern] incorrect, please try agian.\n"
                               "Type 'help' to get the list of commands.\n");
                }

                else {} /* Do nothing ! */

                /* Clear all array_cmd */
                memset(array_cmd, 0, sizeof array_cmd);
        }   
}