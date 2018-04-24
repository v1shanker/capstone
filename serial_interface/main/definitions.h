/*
 * @file definitions.h
 * @brief definitions for esp drivers
 *
 * @author Naveen
 * @bug None
 */

#ifndef __DEFINITIONS_H
#define __DEFINITIONS_H

#define LIDAR_PORT UART_NUM_2
#define ANDROID_PORT UART_NUM_1
#define MOTOR_PORT UART_NUM_0

#define PHONE_TXD  (GPIO_NUM_5)
#define PHONE_RXD  (GPIO_NUM_17)

#define LIDAR_TXD  (GPIO_NUM_16)
#define LIDAR_RXD  (GPIO_NUM_4)

#define SERIAL_RTS  (UART_PIN_NO_CHANGE)
#define SERIAL_CTS  (UART_PIN_NO_CHANGE)

#define HBRIDGE_LEFT_IN1   (GPIO_NUM_12)
#define HBRIDGE_LEFT_IN2   (GPIO_NUM_14)
#define HBRIDGE_LEFT_PWM   (GPIO_NUM_27)
#define HBRIDGE_RIGHT_IN1   (GPIO_NUM_26)
#define HBRIDGE_RIGHT_IN2   (GPIO_NUM_25)
#define HBRIDGE_RIGHT_PWM   (GPIO_NUM_33)

#define BUF_SIZE (1024)

#define PATTERN_NUM 1

QueueHandle_t android_uart_queue;
QueueHandle_t lidar_uart_queue;

QueueHandle_t motor_in_queue;
QueueHandle_t lidar_in_queue;
QueueHandle_t android_out_queue;

typedef enum pulse_state_definitions {PULSE_HIGH, PULSE_LOW} pulse_state;
typedef enum direction_states {FORWARD, REVERSE} direction;

void lidarScan(char *buffer);
void lidar_main();
void motor_main();
#endif
