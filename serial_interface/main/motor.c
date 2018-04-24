/*
 * @file motor.c
 * @brief Defines motor interface
 *
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "soc/timer_group_struct.h"
#include "driver/uart.h"
#include "definitions.h"

#define TIMER_DIVIDER         16 // Hardware time clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER) // convert counter value to seconds
#define TIMER_INTERVAL        1 // second

#define SET_BIT(n)                (1ULL << n)
#define GPIO_PIN_BITMASK_LEFT     (SET_BIT(HBRIDGE_LEFT_IN1) | SET_BIT(HBRIDGE_LEFT_IN2) | SET_BIT(HBRIDGE_LEFT_PWM))
#define GPIO_PIN_BITMASK_RIGHT    (SET_BIT(HBRIDGE_RIGHT_IN1) | SET_BIT(HBRIDGE_RIGHT_IN2) | SET_BIT(HBRIDGE_RIGHT_PWM))
#define GPIO_PIN_BITMASK          GPIO_PIN_BITMASK_LEFT | GPIO_PIN_BITMASK_RIGHT

#define LOGIC_HIGH            1
#define LOGIC_LOW             0

/** @brief signed magnitude representation of speed of motor
 *  positive is forward, negative is backwards
 *  should be between 0 and 100 (realisitically, 20 is more than enough) */
static int8_t speed_dir_left_sm;
static int8_t speed_dir_right_sm;
static int counter;

void getMessage(){
	char message[10];
	for (;;){
		if (xQueueReceive(motor_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){
			printf("Motor Task: Message from android: %s\n\n",message);
		}
	}
}

void IRAM_ATTR timer_isr(void *para) {

    // clear the interrupt
    TIMERG0.int_clr_timers.t0 = 1;
    // re-enable the alarm
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;
}


void set_speed_and_dir_left( int8_t speed_dir_left) {
    speed_dir_left_sm = speed_dir_left;
}

void set_speed_and_dir_right(int8_t speed_dir_right) {
    speed_dir_right_sm = speed_dir_right;
}


void timer_pwm_interrupts_init() {
    timer_config_t config;

    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = 1;
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x0ULL);

    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_isr, (void *) TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, TIMER_0);

}

void gpio_setup() {
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_PIN_BITMASK;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

}

void motor_main() {

    timer_pwm_interrupts_init();
    gpio_setup();

    gpio_set_level(HBRIDGE_RIGHT_PWM, LOGIC_HIGH);

	while(1){
		getMessage();
	}
}
