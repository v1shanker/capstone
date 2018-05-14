/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>

#include "driver/uart.h"
#include "esp_intr.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/dport_reg.h"
#include "soc/uart_struct.h"


#include "esp_log.h"
#include "definitions.h"

static const char *N = "Serial";

static void android_rx_main()
{
	uart_event_t event;
	size_t bytes_read;
	int pos;

	uint8_t pattern_dump[5];
	uint8_t message[MESSAGE_LEN];

	for (;;){
		if(xQueueReceive(android_uart_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
			switch(event.type){
				case UART_PATTERN_DET:
					/* Get actual data */
					pos = uart_pattern_pop_pos(ANDROID_PORT);
					bytes_read = uart_read_bytes(ANDROID_PORT, message, pos , 20/portTICK_RATE_MS);
					message[pos] = '\0';

					/* Remove pattern */
					uart_read_bytes(ANDROID_PORT, pattern_dump, 1, 20/portTICK_RATE_MS);
					pattern_dump[1] = '\0';
					printf("%s\n", message);

					/* Send to motor */
					if (message[0] == 'M'){
						if (!xQueueSend(motor_in_queue, (void *)(message), 2000/portTICK_RATE_MS)){
							printf("Killing motor task\n");
							vTaskDelete(motor_handle);
							xQueueReset(motor_in_queue);
							xTaskCreate(motor_main, "motor_interface",4096, NULL,1, &motor_handle);
						}
					}

					/* Send to LIDAR */
					else if (message[0] == 'L'){
						xQueueSend(lidar_in_queue, (void *)(message), (portTickType)portMAX_DELAY);
					}

					else {
						printf("UnknownACommand\n");
					}
					break;
					
				case UART_DATA:
					break;
					
				default:
					break;
			}
		}


	}
}

static void android_tx_main(){
	output_info temp;
	
	char message[512];
	char *error = "FAIL\n";
	char *success = "ACK\n";
	int len;
	
	for (;;){
		if (xQueueReceive(android_out_queue, (void *)(&temp),(portTickType)portMAX_DELAY)){
			uart_write_bytes(ANDROID_PORT, &temp.type, 1);
			
			if (temp.outcome == 0){
				uart_write_bytes(ANDROID_PORT, error, 5);
			} else {
				if (temp.type == 'L'){
					sprintf(message, "%u %u %u %u %u %u\n",
								temp.data[0], temp.data[1], temp.data[2],
								temp.data[3], temp.data[4], temp.data[5]);
					len = strlen(message);
					//printf("Length is %d\n", len);
					uart_write_bytes(ANDROID_PORT, message, len);
				} else {
					uart_write_bytes(ANDROID_PORT, success, 4);
				}
			}
		}
		
		//printf("TXSent\n");
	}
} 

void app_main()
{
	esp_log_level_set(N, ESP_LOG_INFO);

	uart_config_t uart_config1 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

	uart_config_t uart_config2 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
	
	uart_intr_config_t lidar_intr = {
		.intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M,
		.rxfifo_full_thresh = LIDAR_THRESHOLD,
		.rx_timeout_thresh = UART_TOUT_THRESH_DEFAULT,
		.txfifo_empty_intr_thresh = UART_EMPTY_THRESH_DEFAULT,
	};

	/* Set up lidar serial connection */
    uart_param_config(LIDAR_PORT, &uart_config1);
    uart_set_pin(LIDAR_PORT, LIDAR_TXD, LIDAR_RXD, SERIAL_RTS, SERIAL_CTS);
    uart_driver_install(LIDAR_PORT, L_RX_SIZE, L_TX_SIZE, 1000, &lidar_uart_queue, 0);
    uart_enable_pattern_det_intr(LIDAR_PORT, (char)0x81, PATTERN_NUM, 10000, 10, 10);
	
    /* Reset the pattern queue length to record at most 20 pattern positions. */
    uart_pattern_queue_reset(LIDAR_PORT, 20);

	/* Set up android serial connection */
	uart_param_config(ANDROID_PORT, &uart_config2);
    uart_set_pin(ANDROID_PORT, PHONE_TXD, PHONE_RXD, SERIAL_RTS, SERIAL_CTS);
    uart_driver_install(ANDROID_PORT, A_RX_SIZE, A_TX_SIZE, 1000, &android_uart_queue, 0);
    uart_enable_pattern_det_intr(ANDROID_PORT, '\n', PATTERN_NUM, 10000, 10, 10);

	uart_pattern_queue_reset(ANDROID_PORT, 20);

	/* Create message queues */
	android_out_queue = xQueueCreate(20,sizeof(output_info));
	motor_in_queue = xQueueCreate(20,sizeof(char)*MESSAGE_LEN);
	lidar_in_queue = xQueueCreate(20,sizeof(char)*MESSAGE_LEN);

    xTaskCreate(android_rx_main, "android_rx, interface", 4096, NULL, 1, NULL);
	xTaskCreate(android_tx_main, "android_tx, interface", 4096, NULL, 1, NULL);
	//xTaskCreate(lidar_main, "lidar_interface", 16000, NULL, 2, &lidar_handle);
	xTaskCreate(motor_main, "motor_interface",4096, NULL,1, &motor_handle);
}

