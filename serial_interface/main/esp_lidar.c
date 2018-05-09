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
	//printf("Begin scan\n");
	
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
	uart_enable_pattern_det_intr(LIDAR_PORT, (char)0x81, PATTERN_NUM, 10000, 10, 10);

    /* Reset the pattern queue length to record at most 20 pattern positions. */
    uart_pattern_queue_reset(LIDAR_PORT, 20);
	
	/* Get header info */
	printf("Looking for data\n");
	getHeader(header,20);
	uart_disable_pattern_det_intr(LIDAR_PORT);
	printf("Got here\n");
	start = xTaskGetTickCount();
	while (1){
		uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);
		printf("Length is %d\n", buffered_len);
		if (buffered_len > 4000){
			lidar_sendBytes(stop,2);
			break;
		}
		
		if ((xTaskGetTickCount() - start) > 2000/portTICK_PERIOD_MS){
			printf("Timeout!\n");
			return -1;
		}
		vTaskDelay(500/portTICK_PERIOD_MS);
	} 
	
	printf("Done waiting\n");
	/* Wait for LIDAR to stop sending */
	/*prev_len = 0;
	while (prev_len != buffered_len){
		prev_len = buffered_len;
		uart_get_buffered_data_len(LIDAR_PORT, &buffered_len);
	}*/	
	
	printf("Converged\n");
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
	//printf("Count is %d\n", count);
	return count;
}

void processData(rplidar_data *data, output_info *output,size_t count){
	
	float sum;
	float average;
	int point;
	
	for (int i=0; i < OUTPUT_DATA_POINTS; i++){	
		
		sum = 0;
		
		/* Left side of 0 */
		if (i < 3){
			for (point = 0; point < 20; point++){
				sum += data[count - (3-i)*POINTS_PER_SECTION + point].distance;
			}
			
			average = sum/POINTS_PER_SECTION;
			(output->data)[i] = (uint32_t)average;
		}
		
		/* Right side of 0 */
		else {
			for (point=0; point < 20; point++){
				sum += data[(i-3)*POINTS_PER_SECTION + point].distance;
			}
			
			average = sum/POINTS_PER_SECTION;
			(output->data)[i] = (uint32_t)average;
		}
	}
	
	for (int i=0; i < OUTPUT_DATA_POINTS; i++){
		printf("Average value is %f\n", (float)((output->data)[i])/4.0f);
	}
	
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

void IRAM_ATTR lidar_isr(){

	int next_interrupt_time = 0;
	
    if (lidar_state == OFF) {
		next_interrupt_time = (SPEED) * DUTY_CYCLE_TICK;
		gpio_set_level(LIDAR_PWM, ON);
		lidar_state = ON;
    } else if (lidar_state == ON) {
		if (SPEED == PWM_PERIOD){
			next_interrupt_time = (SPEED) * DUTY_CYCLE_TICK;
		}
		
		else {
			next_interrupt_time = (PWM_PERIOD - SPEED) * DUTY_CYCLE_TICK;
			gpio_set_level(LIDAR_PWM, OFF);
			lidar_state = OFF;
		}
    }
	
    // clear the interrupt
    TIMERG1.int_clr_timers.t0 = 1;
    timer_set_alarm_value(TIMER_GROUP_1, TIMER_0, next_interrupt_time);
    // re-enable the alarm
    TIMERG1.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;
	
}

void pwm_setup(){
	
	/* Set up PWM pin */
	gpio_config_t io_conf;
	timer_config_t config;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = SET_BIT(LIDAR_PWM);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
	
    gpio_set_level(LIDAR_PWM, 1);
	
	/* Set up timer */
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = 0;
    timer_init(TIMER_GROUP_1, TIMER_0, &config);

    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_1, TIMER_0, 0x0ULL));

    lidar_state = OFF;
	
    int next_interrupt_ms = (PWM_PERIOD - SPEED) * DUTY_CYCLE_TICK;
	
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_1, TIMER_0, next_interrupt_ms));
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_1, TIMER_0));
    ESP_ERROR_CHECK(timer_isr_register(TIMER_GROUP_1, TIMER_0, lidar_isr, NULL, ESP_INTR_FLAG_IRAM, NULL));
    ESP_ERROR_CHECK(timer_start(TIMER_GROUP_1, TIMER_0));
    return;

}

void lidar_main(){
	output_info response;
	char message[MESSAGE_LEN];
	int result;
	char stop[2] = {0xA5,0x25};
	
	pwm_setup();
	
	lidar_sendBytes(stop,2);
	
	/*
	while (1){
		printf("%d\n", test_counter);
	}
	*/
	//doScan(&response);
	//response.outcome = 1;
	//response.type = 'L';
	//xQueueSend(android_out_queue, (void *)(&response), (portTickType)portMAX_DELAY);
	
	for (;;){
		if (xQueueReceive(lidar_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){	
			printf("LIDAR Task: Message from android: %s\n",message);
			if (!strcmp(message,"LSCAN")){
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
