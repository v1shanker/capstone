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

void getMessage(){
	char message[10];
	for (;;){
		if (xQueueReceive(motor_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){	
			printf("Motor Task: Message from android: %s\n\n",message);
		}
	}
}

void motor_main(){
	while(1){
		getMessage();
	}
}