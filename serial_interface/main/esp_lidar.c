/*
 * @file esp_uart.c
 * @brief Implements uart code for LIDAR driver
 *
 * @author Naveen
 * @bug None
 */


#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "definitions.h"

#define MAX_POINTS 360

typedef struct _rplidar_response {
	uint8_t sync_quality;
	uint16_t angle_checkbit;
	uint16_t distance;
} __attribute__((packed)) rplidar_data;

typedef struct _rplidar_small {
	uint16_t angle;
	uint16_t distance;
} lidar_small;

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
						if (!(buffer[1] & 0x1)){
							printf("Check bit failure!\n");
						}
						
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

int lidarScan(rplidar_data *buffer){
	printf("Begin scan\n");
	
	char request[2] = {0xA5,0x20};
	char stop[2] = {0xA5,0x25};
	uint8_t header[20];
	int count = 0;

	/* Stop any ongoing scan and flush RX FIFO queue */
	lidar_sendBytes(stop,2);
	uart_flush(LIDAR_PORT);
	lidar_sendBytes(request,2);
	
	
	/* Get header info */
	printf("Looking for data\n");
	getHeader(header,20);
	
	for (int a = 0; a < 20; a++){
		printf("header[%d] = %x\n", a, header[a]);
	}
	
	/* Get actual data */
	
	buffer[0].sync_quality = 0;
	
	/* Wait for sync bit */
	while ((buffer[0].sync_quality & 0x3) != 0x01){
		getData((uint8_t *)&buffer[0], 5);
	}

	count++;
	
	do{
		getData((uint8_t *)&buffer[count], 5);
		count++;	
	}
	while ((buffer[count-1].sync_quality & 0x3) != 0x01);
	
	/* Remove the last packet as it's start of new scan */
	count --;	
	
	lidar_sendBytes(stop,2); 
	
	printf("Finished count is %d\n", count);
	float angle_f;
	float distance_f;
	uint8_t quality;
	uint8_t sync;
	
	for (int pos = 0; pos < count; pos++) {
		sync = buffer[pos].sync_quality & 0x3;
		quality = buffer[pos].sync_quality >> 2;
		angle_f = (float)(buffer[pos].angle_checkbit >> 1)/(64.0f);
		distance_f = (float)(buffer[pos].distance)/(4.0f);
				
		printf("theta: %03.2f Dist: %08.2f Q: %d S: %x\n", angle_f, distance_f, quality, sync);
	} 
	
	return count;
}

int begin_scan(lidar_small *output){
	rplidar_data data[MAX_POINTS];
	int count;
	int total;
	int index;
	
	for (int i = 0; i < 3; i++){
		count = lidarScan(data);
		if (count > 260) {
			break;
		}
		else {
			printf("Scan not successful. Retrying...\n");
		}
	}
	
	if (count < 260){
		printf("Failed to acquire scan");
		return -1;
	}
	
	total = 0;
	index = 0;
	
	/* Take best 10 points from 0 */
	while (total != 10){
		if ((data[index].sync_quality >> 2) != 0){
			output[total].angle = data[index].angle_checkbit >> 1;
			output[total].distance = data[index].distance;
			total++;
		}
		
		index++;
	}
	
	/* Take 10 points from other side */
	
	index = count - 1;
	
	while (total != 20){
		if ((data[index].sync_quality >> 2) != 0){
			output[total].angle = data[index].angle_checkbit >> 1;
			output[total].distance = data[index].distance;
			total++;
		}
		index--;
	}
	
	printf("Scan successful!\n");
	return 0;
}

void lidar_main(){
	lidar_small small[20];
	char message[20];
	
	for (;;){
		if (xQueueReceive(lidar_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){	
			printf("LIDAR Task: Message from android: %s\n\n",message);
			if (!strcmp(message,"LSCAN")){
				begin_scan(small);
				//xQueueSend(android_out_queue, (void *)(stopMessage), (portTickType)portMAX_DELAY);
			}
		}
	}
}
