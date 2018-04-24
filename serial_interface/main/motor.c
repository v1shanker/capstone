/*
 * @file motor.c
 * @brief Defines motor interface
 *
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "definitions.h"

/** @brief signed magnitude representation of speed of motor
 *  positive is forward, negative is backwards
 *  should be between 0 and 100 (realisitically, 20 is more than enough) */
static int8_t speed_dir_left_sm;
static int8_t speed_dir_right_sm;

void getMessage(){
	char message[10];
	for (;;){
		if (xQueueReceive(motor_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){
			printf("Motor Task: Message from android: %s\n\n",message);
		}
	}
}


void set_speed_and_dir_left( int8_t speed_dir_left) {
    speed_dir_left_sm = speed_dir_left;
}

void set_speed_and_dir_right(int8_t speed_dir_right) {
    speed_dir_right_sm = speed_dir_right;
}

void motor_main(){
	while(1){
		getMessage();
	}
}
