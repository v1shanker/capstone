/*
 * @file esp_uart.c
 * @brief Implements uart code for LIDAR driver
 *
 * @author Naveen
 * @bug None
 */


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "definitions.h"


void getData(uint8_t *buffer, size_t bytes_required){
	size_t buffered_len;
	uart_event_t event;
	
	if(xQueueReceive(lidar_uart_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
		switch(event.type){
			case UART_DATA:
				uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);
				if (buffered_len < bytes_required){
					break;
				}
				
				uart_read_bytes(LIDAR_PORT, buffer, bytes_required,20/portTICK_RATE_MS);
				buffer[bytes_required] = '\0';
				return;
			default:
				break;
		}
	}
}
	
void getHeader(uint8_t *data)
{
	uart_event_t event;
	size_t bytes_read;
	size_t pos;
	
	uint8_t pattern_dump[4];
	
	if(xQueueReceive(lidar_uart_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
		switch(event.type){
			case UART_PATTERN_DET:
			
				/* Get actual data */
				pos = uart_pattern_pop_pos(LIDAR_PORT);
				bytes_read = uart_read_bytes(LIDAR_PORT, data, pos , 20/portTICK_RATE_MS);
				data[pos] = '\0';
				
				/* Remove pattern */
				uart_read_bytes(LIDAR_PORT, pattern_dump, 3, 20/portTICK_RATE_MS);
				pattern_dump[pos+1] = '\0';
				printf("Data buffer is %s\n", data);
				break;
				
			default:
				break;
		}
	}
	return;
}

void lidar_sendBytes(char *buffer, size_t n){
	uart_write_bytes(LIDAR_PORT, buffer, n);
}

void lidarScan(char *buffer){
	char request[2] = {0xA5,0x20};
	lidar_sendBytes(request,2);
	
	uint8_t header[10];
	uint8_t data[22];
	
	/* Test with serial info */
	getHeader(header);
	getData(data,20);
}