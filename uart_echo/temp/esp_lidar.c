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

#define ECHO_TEST_TXD  (GPIO_NUM_4)
#define ECHO_TEST_RXD  (GPIO_NUM_5)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)

void lidar_setup()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    return uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

void lidar_readBytes(char *buffer, size_t n){
	
	/*

    // Configure a temporary buffer for the incoming data
    //uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
	char *string = "Hello\n";
	const TickType_t xDelay = 500 / portTICK_PERIOD_MS;

    while (1) {
        // Read data from the UART
        uart_write_bytes(UART_NUM_1, string, 7);
		vTaskDelay( xDelay );
    }
	*/
	return;
}

void lidar_sendBytes(char *buffer, size_t n){
	uart_write_bytes(UART_NUM_1, buffer, n);
}