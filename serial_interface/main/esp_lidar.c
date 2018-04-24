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

typedef struct _rplidar_response {
	uint8_t sync_quality;
	uint16_t angle_checkbit;
	uint16_t distance;
} __attribute__((packed)) rplidar_data;

void getData(uint8_t *buffer, size_t bytes_required){
	size_t buffered_len;
	uart_event_t event;

	uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);

	/* See if enough data already in queue */
	if (buffered_len >= bytes_required){
		uart_read_bytes(LIDAR_PORT, buffer, bytes_required,20/portTICK_RATE_MS);
		if (!(buffer[1] & 0x1)){
			printf("Check bit failure!\n");
		}
		return;
	}

	for (;;){
		if(xQueueReceive(lidar_uart_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
			switch(event.type){
				case UART_DATA:

					uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);

					if (buffered_len >= bytes_required){
						uart_read_bytes(LIDAR_PORT, buffer, bytes_required,20/portTICK_RATE_MS);
						return;
					}

					printf("Buffered len is %d\n", buffered_len);
					break;

				default:
					break;
			}
		}
	}
}

void getHeader(uint8_t *buffer, size_t bytes_required){

	size_t pos;
	uart_event_t event;

	uint8_t pattern_dump[2];

	for (;;){
		if(xQueueReceive(lidar_uart_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
			switch(event.type){
				case UART_PATTERN_DET:

						/* Get actual data */
						pos = uart_pattern_pop_pos(LIDAR_PORT);
						printf("pos is %d\n",pos);
						uart_read_bytes(LIDAR_PORT, buffer, pos , 20/portTICK_RATE_MS);
						buffer[pos] = 0xAA;
						/* Remove pattern */
						uart_read_bytes(LIDAR_PORT, pattern_dump, 1, 20/portTICK_RATE_MS);
						return;

				default:
					break;
			}
		}
	}
}

void lidar_sendBytes(char *buffer, size_t n){
	printf("Sending bytes\n");
	uart_write_bytes(LIDAR_PORT, buffer, n);
}

void lidarScan(rplidar_data *buffer){
	printf("Begin scan\n");

	char request[2] = {0xA5,0x20};
	char stop[2] = {0xA5,0x25};
	uint8_t header[20];


	/* Stop scan */
	lidar_sendBytes(stop,2);
	uart_flush(LIDAR_PORT);
	lidar_sendBytes(request,2);


	/* Test with serial info */
	printf("Looking for data\n");
	getHeader(header,20);

	for (int a = 0; a < 20; a++){
		printf("header[%d] = %x\n", a, header[a]);
	}

	for (int i=0; i < 297; i++){
		//printf("i is %d\n", i);
		getData((uint8_t *)&buffer[i], 5);
	}

	lidar_sendBytes(stop,2);

	printf("Finished retreiving data\n");
	float angle_f;
	float distance_f;
	uint8_t quality;

	for (int pos = 0; pos < 297 ; pos++) {

		quality = buffer[pos].sync_quality >> 2;
		angle_f = (float)(buffer[pos].angle_checkbit >> 1)/(64.0f);
		distance_f = (float)(buffer[pos].distance)/(4.0f);

		printf("theta: %03.2f Dist: %08.2f Q: %d\n", angle_f, distance_f, quality);
	}
}

void lidar_main(){
	rplidar_data data[297];
	char message[20];

	lidarScan(data);
	printf("done!\n");

	for (;;){
		if (xQueueReceive(lidar_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){
			printf("LIDAR Task: Message from android: %s\n\n",message);
		}
	}
}
