/*
 * @file definitions.h
 * @brief definitions for esp drivers
 *
 * @author Naveen
 * @author Vikram Shanker (vshanker@andrew.cmu.edu)
 * @bug None
 */

#ifndef __DEFINITIONS_H
#define __DEFINITIONS_H

#define LIDAR_PORT UART_NUM_1
#define ANDROID_PORT UART_NUM_2

#define PHONE_TXD  (GPIO_NUM_19)
#define PHONE_RXD  (GPIO_NUM_18)

#define LIDAR_TXD  (GPIO_NUM_16)
#define LIDAR_RXD  (GPIO_NUM_4)

#define SERIAL_RTS  (UART_PIN_NO_CHANGE)
#define SERIAL_CTS  (UART_PIN_NO_CHANGE)

#define L_RX_SIZE (8192)
#define L_TX_SIZE (512)

#define A_RX_SIZE (512)
#define A_TX_SIZE (0)

#define SET_BIT(n)                (1ULL << n)

#define HBRIDGE_LEFT_IN1    (GPIO_NUM_26)
#define HBRIDGE_LEFT_IN2    (GPIO_NUM_25)
#define HBRIDGE_LEFT_PWM    (GPIO_NUM_33)
#define HBRIDGE_RIGHT_IN1   (GPIO_NUM_22)
#define HBRIDGE_RIGHT_IN2   (GPIO_NUM_23)
#define HBRIDGE_RIGHT_PWM   (GPIO_NUM_21)

#define LIDAR_PWM 			(GPIO_NUM_14)

#define LIDAR_THRESHOLD 	20
#define UART_EMPTY_THRESH_DEFAULT  (10)
#define UART_TOUT_THRESH_DEFAULT   (10)

#define PATTERN_NUM 1

#define MESSAGE_LEN 20
#define OUTPUT_DATA_POINTS 6
#define POINTS_PER_SECTION 20
#define MAX_RETRY 5

TaskHandle_t lidar_handle;

QueueHandle_t android_uart_queue;
QueueHandle_t lidar_uart_queue;

QueueHandle_t motor_in_queue;
QueueHandle_t lidar_in_queue;
QueueHandle_t android_out_queue;

typedef enum pulse_state_definitions {PULSE_HIGH, PULSE_LOW} pulse_state;
typedef enum direction_states {FORWARD, REVERSE} direction;

typedef struct compact_lidar_data {
	uint32_t data[OUTPUT_DATA_POINTS];
	uint8_t size;
	char type;
	char outcome;
} __attribute__((packed)) output_info;

void lidar_main();
void motor_main();
void IRAM_ATTR lidar_uart_isr();

#endif
