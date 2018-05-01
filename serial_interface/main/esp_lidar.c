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
#define DATA_POINTS 20

typedef struct _rplidar_response {
	uint8_t sync_quality;
	uint16_t angle_checkbit;
	uint16_t distance;
} __attribute__((packed)) rplidar_data;

void getHeader(uint8_t *buffer, size_t bytes_required){
	
	int pos;
	uart_event_t event;
	
	uint8_t pattern_dump[2];
	
	for (;;){
		if(xQueueReceive(lidar_uart_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
			switch(event.type){
				case UART_PATTERN_DET:
					
					/* Get actual data */
					pos = uart_pattern_pop_pos(LIDAR_PORT);
					printf("pos is %d\n",pos);
					
					if (pos == -1){
						continue;
					}
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
	uart_write_bytes(LIDAR_PORT, buffer, n);
}

/*
 * @brief Send scan command to LIDAR and collects result
 */
int lidarScan(rplidar_data *buffer){
	printf("Begin scan\n");
	
	char request[2] = {0xA5,0x20};
	char stop[2] = {0xA5,0x25};
	uint8_t header[20];
	int count;
	size_t buffered_len;
	size_t prev_len;
	TickType_t start;

	/* Stop any ongoing scan and flush RX FIFO queue */
	lidar_sendBytes(stop,2);
	uart_flush(LIDAR_PORT);
	vTaskDelay(2/portTICK_PERIOD_MS);
	lidar_sendBytes(request,2);
	
	/* Get header info */
	printf("Looking for data\n");
	getHeader(header,20);
	
	start = xTaskGetTickCount();
	while (1){
		uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);
		
		if (buffered_len > 4800){
			lidar_sendBytes(stop,2);
			printf("Length is %d\n", buffered_len);
			break;
		}
		
		if ((xTaskGetTickCount() - start) > 2000/portTICK_PERIOD_MS){
			printf("Timeout!\n");
			return -1;
		}
	} 
	
	/* Wait for LIDAR to stop sending */
	prev_len = 0;
	while (prev_len != buffered_len){
		prev_len = buffered_len;
		uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);
	}	
	
	/* Ignore first set of data */
	for (int i = 0; i < 2; i++){
		buffer[0].sync_quality = 0;
		while ((buffer[0].sync_quality & 0x3) != 0x1){
			uart_read_bytes(LIDAR_PORT, (uint8_t *)&buffer[0], 5,20/portTICK_RATE_MS);
		}
	}
	
	uart_read_bytes(LIDAR_PORT, (uint8_t *)&buffer[0], 5,20/portTICK_RATE_MS);
	count = 1;
	do{
		uart_read_bytes(LIDAR_PORT, (uint8_t *)&buffer[count], 5,20/portTICK_RATE_MS);
		count++;
	}
	while ((buffer[count-1].sync_quality & 0x3) != 0x01);
	
	/* Clear FIFO buffer */
	uart_flush(LIDAR_PORT);
	uart_pattern_queue_reset(LIDAR_PORT, 20);
	xQueueReset(lidar_uart_queue);
	/*
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
	*/
	printf("Count is %d\n", count);
	return count;
}

void processData(rplidar_data *data, output_info *output,size_t count){
	
	float sum;
	float average;
	int point;
	uint32_t *A = (uint32_t *)(output -> data);
	
	for (int i=0; i < OUTPUT_DATA_POINTS; i++){	
		
		sum = 0;
		
		/* Left side of 0 */
		if (i < 3){
			for (point = 0; point < 20; point++){
				sum += data[count - (3-i)*POINTS_PER_SECTION + point].distance;
			}
			
			average = sum/POINTS_PER_SECTION;
			A[i] = average;
		}
		
		/* Right side of 0 */
		else {
			for (point=0; point < 20; point++){
				sum += data[(i-3)*POINTS_PER_SECTION + point].distance;
			}
			
			average = sum/POINTS_PER_SECTION;
			A[i] = average;
		}
	}
	
	for (int i=0; i < OUTPUT_DATA_POINTS; i++){
		printf("Average value is %f\n", (float)A[i]/4.0f);
	}
	
	output -> size = sizeof(uint32_t) * OUTPUT_DATA_POINTS;
	return;
}

/* 
 * Scans until required amount of LIDAR data received, then processes it
 */
int doScan(output_info *output){
	rplidar_data data[MAX_POINTS];
	int count;
	
	for (int i = 0; i < MAX_RETRY; i++){
		count = lidarScan(data);
		if (count > 260) {
			break;
		}
		else {
			printf("Scan not successful. Retrying...\n");
		}
	}
	
	if (count < 260){
		printf("Failed to acquire scan\n");
		return -1;
	}
	
	processData(data, output, count);
	
	return 0;
}


void lidar_main(){
	output_info small[DATA_POINTS];
	char message[MESSAGE_LEN];
	
	for (;;){
		if (xQueueReceive(lidar_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){	
			printf("LIDAR Task: Message from android: %s\n",message);
			if (!strcmp(message,"LSCAN")){
				doScan(small);
				//xQueueSend(android_out_queue, (void *)(stopMessage), (portTickType)portMAX_DELAY);
			}
		}
	}
}
