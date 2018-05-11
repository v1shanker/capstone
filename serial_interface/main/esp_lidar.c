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
#include "driver/timer.h"
#include "soc/timer_group_struct.h"

#define MAX_POINTS 360
#define DATA_POINTS 20

#define TIMER_DIVIDER             16 // Hardware time clock divider
#define TIMER_SCALE               (TIMER_BASE_CLK / TIMER_DIVIDER) // convert counter value to seconds
#define DUTY_CYCLE_TICK           (TIMER_SCALE / 100 ) // this is 1 ms
#define PWM_PERIOD				  100
#define SPEED 					  25

#define ON 1
#define OFF 0

int lidar_state = OFF;
int test_counter = 0;

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
					//printf("pos is %d\n",pos);
					
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
int lidarScan(rplidar_data *output){
	//printf("Begin scan\n");
	
	char request[2] = {0xA5,0x20};
	char stop[2] = {0xA5,0x25};
	
	uint8_t temp[3600];
	uint8_t header[20];
	
	int count;
	int index;
	int startIndex;
	size_t buffered_len;

	/* Stop any ongoing scan and flush RX FIFO queue */
	lidar_sendBytes(stop,2);
	uart_flush(LIDAR_PORT);
	vTaskDelay(2/portTICK_PERIOD_MS);
	lidar_sendBytes(request,2);
	
	/* Get header info */
	//printf("Looking for data\n");
	getHeader(header,20);
	
	uart_read_bytes(LIDAR_PORT, temp, 3600, portMAX_DELAY);
	lidar_sendBytes(stop,2);
	
	rplidar_data* buffer = (rplidar_data *)(temp);
	
	float angle_f;
	float distance_f;
	uint8_t quality;
	uint8_t sync;
	/*
	for (int pos = 0; pos < 3600/5; pos++) {
		sync = buffer[pos].sync_quality & 0x3;
		quality = buffer[pos].sync_quality >> 2;
		angle_f = (float)(buffer[pos].angle_checkbit >> 1)/(64.0f);
		distance_f = (float)(buffer[pos].distance)/(4.0f);
				
		printf("theta: %03.2f Dist: %08.2f Q: %d S: %x\n", angle_f, distance_f, quality, sync);
	}
	*/
	/* Avoid first syncbyte */
	index = 0;
	while ((buffer[index].sync_quality & 0x3) != 0x1){
		index++;
	}		
	
	/* Search for second sync byte */
	index++;
	while ((buffer[index].sync_quality & 0x3) != 0x1){
		index++;
	}
	startIndex = index;
	
	/* Search for third sync byte */
	index++;
	while ((buffer[index].sync_quality & 0x3) != 0x1){
		index++;
	}
	
	//printf("Start is %d, end is %d\n",  startIndex, index);
	
	count = index - startIndex;
	memcpy(output, &(buffer[startIndex]), count*5);
	
	/* Clear FIFO buffer */
	uart_flush(LIDAR_PORT);
	uart_pattern_queue_reset(LIDAR_PORT, 20);
	xQueueReset(lidar_uart_queue);
	
	uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);
	while (buffered_len != 0){
		lidar_sendBytes(stop,2);
		uart_flush(LIDAR_PORT);
		vTaskDelay(2/portTICK_PERIOD_MS);
		uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);
	}
	
	/*
	for (int pos = 0; pos < count; pos++) {
		sync = output[pos].sync_quality & 0x3;
		quality = output[pos].sync_quality >> 2;
		angle_f = (float)(output[pos].angle_checkbit >> 1)/(64.0f);
		distance_f = (float)(output[pos].distance)/(40.0f);
				
		printf("theta: %03.2f Dist: %08.2f Q: %d S: %x\n", angle_f, distance_f, quality, sync);
	}
	*/
	//printf("Count is %d\n", count);
	return count;
}

void processData(rplidar_data *data, output_info *output,size_t count){
	
	uint32_t sum;
	uint32_t average;
	uint32_t point;
	uint32_t counter;
	uint32_t distance;
	
	for (int i=0; i < OUTPUT_DATA_POINTS; i++){	
		
		sum = 0;
		counter = 0;
		/* Left side of 0 */
		if (i < 3){
			for (point = 0; point < 20; point++){
				distance = data[count - (3-i)*POINTS_PER_SECTION + point].distance;
				if (distance > 0){
					sum += distance;
					counter++;
				}
			}
		}
		
		/* Right side of 0 */
		else {
			for (point=0; point < 20; point++){
				distance = data[(i-3)*POINTS_PER_SECTION + point].distance;
				if (distance > 0){
					sum += distance;
					counter++;
				}
			}
		}
		
		average = sum/counter;
		(output->data)[i] = average;
	}
	/*
	for (int i=0; i < OUTPUT_DATA_POINTS; i++){
		printf("Average value is %f\n", (float)((output->data)[i])/40.0f);
	}
	*/
	output -> size = OUTPUT_DATA_POINTS;
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
	output_info response;
	char message[MESSAGE_LEN];
	int result;
	char stop[2] = {0xA5,0x25};
	
	lidar_sendBytes(stop,2);
	
	doScan(&response);
	response.outcome = 1;
	response.type = 'L';
	
	for (;;){
		if (xQueueReceive(lidar_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){	
			//printf("LIDAR Task: Message from android: %s\n",message);
			if (!strcmp(message,"LSCAN")){
				printf("Lsend\n");
				xQueueSend(android_out_queue, (void *)(&response), (portTickType)portMAX_DELAY);
				
				result = doScan(&response);
				
				if (result == -1){
					response.outcome = 0;
				} else {
					response.outcome = 1;
				}
				
				response.type = 'L';
				//xQueueSend(android_out_queue, (void *)(&response), (portTickType)portMAX_DELAY);
			}
		}
	}
}
