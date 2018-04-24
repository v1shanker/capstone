/* 
 * @file motor.c
 * @brief Defines motor interface
 *
 */
 
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "definitions.h"

void getMessage(){
	char message[10];
	char *startMessage = "START";
	char *stopMessage = "STOP";
	
	for (;;){
		if (xQueueReceive(motor_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){	
			if (!strcmp(message, "MSTART")){
				printf("Starting motor\n");
				xQueueSend(android_out_queue, (void *)(startMessage), (portTickType)portMAX_DELAY);
			} else if (!strcmp(message, "MSTOP")){
				printf("Stopping motor\n");
				xQueueSend(android_out_queue, (void *)(stopMessage), (portTickType)portMAX_DELAY);
			} else {
				printf("Unknown command");
			}
		}
	}
}

void motor_main(){
	while(1){
		getMessage();
	}
}