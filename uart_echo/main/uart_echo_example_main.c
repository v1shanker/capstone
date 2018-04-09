/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

/**
 * This is an example which echos any data it receives on UART1 back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define ECHO_TEST_TXD  (GPIO_NUM_4)
#define ECHO_TEST_RXD  (GPIO_NUM_5)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (4096)

static void echo_task()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);

    // Configure a temporary buffer for the incoming data
    //uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
	
	const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
	
	size_t length;
	int len;
	char buffer[3];
	char response[3] = {'A', 'T', '!'};
    while (1) {
        length = 0;
		len = 0;
		while (length < 3){
			uart_get_buffered_data_len(UART_NUM_1, &length);
		}
		len = uart_read_bytes(UART_NUM_1, (uint8_t*)buffer, 3, 20/ portTICK_RATE_MS);
		
		printf("Input of size %d: %c%c%c\n", len, buffer[0], buffer[1], buffer[2]);
		
		
		if ((char)(buffer[0] == 'A') || (char)(buffer[1] == 'T') || (char)(buffer[2] == '?')){
			uart_write_bytes(UART_NUM_1, response, 3);
		}
    }
}

void app_main()
{
    xTaskCreate(echo_task, "uart_echo_task", 4096, NULL, 10, NULL);
}
