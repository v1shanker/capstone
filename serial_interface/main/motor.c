/*
 * @file motor.c
 * @brief Defines motor interface
 *
 * @author Vikram Shanker (vshanker@andrew.cmu.edu)
 */

#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "soc/timer_group_struct.h"
#include "driver/uart.h"
#include "definitions.h"

#define TIMER_DIVIDER             16 // Hardware time clock divider
#define TIMER_SCALE               (TIMER_BASE_CLK / TIMER_DIVIDER) // convert counter value to seconds
#define DUTY_CYCLE_TICK           (TIMER_SCALE / 100 ) // this is 1 ms


#define GPIO_PIN_BITMASK_LEFT     (SET_BIT(HBRIDGE_LEFT_IN1) | SET_BIT(HBRIDGE_LEFT_IN2) | SET_BIT(HBRIDGE_LEFT_PWM))
#define GPIO_PIN_BITMASK_RIGHT    (SET_BIT(HBRIDGE_RIGHT_IN1) | SET_BIT(HBRIDGE_RIGHT_IN2) | SET_BIT(HBRIDGE_RIGHT_PWM))
#define GPIO_PIN_BITMASK          GPIO_PIN_BITMASK_LEFT | GPIO_PIN_BITMASK_RIGHT

#define LOGIC_HIGH                1
#define LOGIC_LOW                 0


/** @brief signed magnitude representation of speed of motor
 *  positive is forward, negative is backwards
 *  should be between 0 and 100 (realisitically, 20 is more than enough) */
static volatile uint8_t speed;
static volatile pulse_state pwm_pulse_state;
static turning_directions turn_dir;

void IRAM_ATTR timer_isr(void *para) {
    int next_interrupt_time = 0;
	if (pwm_pulse_state == PULSE_LOW) {
       if (speed > 0) {
            next_interrupt_time = speed * DUTY_CYCLE_TICK;
            pwm_pulse_state = PULSE_HIGH;
            gpio_set_level(HBRIDGE_LEFT_PWM, LOGIC_HIGH);
            gpio_set_level(HBRIDGE_RIGHT_PWM, LOGIC_HIGH);
        } else {
            next_interrupt_time = 100 * DUTY_CYCLE_TICK;
        }
    } else if (pwm_pulse_state == PULSE_HIGH) {
		if (speed < 100){
			next_interrupt_time = (100 - speed) * DUTY_CYCLE_TICK;
			pwm_pulse_state = PULSE_LOW;
			gpio_set_level(HBRIDGE_LEFT_PWM, LOGIC_LOW);
			gpio_set_level(HBRIDGE_RIGHT_PWM, LOGIC_LOW);
		} else {
			next_interrupt_time = 100* DUTY_CYCLE_TICK;
		}
    }
    // clear the interrupt
    TIMERG0.int_clr_timers.t0 = 1;
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, next_interrupt_time);
    // re-enable the alarm
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;
}

static void set_dir_left( direction dir ) {
    if ( dir == FORWARD ) {
        gpio_set_level(HBRIDGE_LEFT_IN1, LOGIC_HIGH);
        gpio_set_level(HBRIDGE_LEFT_IN2, LOGIC_LOW);
    } else if ( dir == REVERSE ) { // forward
        gpio_set_level(HBRIDGE_LEFT_IN1, LOGIC_LOW);
        gpio_set_level(HBRIDGE_LEFT_IN2, LOGIC_HIGH);
    } else {
        gpio_set_level(HBRIDGE_LEFT_IN1, LOGIC_LOW);
        gpio_set_level(HBRIDGE_LEFT_IN2, LOGIC_LOW);
	}
}

static void set_dir_right( direction dir ) {
    if ( dir == FORWARD ) {
        gpio_set_level(HBRIDGE_RIGHT_IN1, LOGIC_HIGH);
        gpio_set_level(HBRIDGE_RIGHT_IN2, LOGIC_LOW);
    } else if ( dir == REVERSE){ // forward
        gpio_set_level(HBRIDGE_RIGHT_IN1, LOGIC_LOW);
        gpio_set_level(HBRIDGE_RIGHT_IN2, LOGIC_HIGH);
    } else {
        gpio_set_level(HBRIDGE_LEFT_IN1, LOGIC_LOW);
        gpio_set_level(HBRIDGE_LEFT_IN2, LOGIC_LOW);
	}
}

// only expose this as the public interface because the user
// really needs to specify what all the wheels are doing at
// a given time.
void set_speed_and_dir(uint8_t new_speed, direction dir_right, direction dir_left) {
    speed = new_speed;
    set_dir_left(dir_left);
    set_dir_right(dir_right);
}

static void timer_pwm_interrupts_init() {
    timer_config_t config;

    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = 0;
    timer_init(TIMER_GROUP_0, TIMER_0, &config);


    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x0ULL));

    pwm_pulse_state = PULSE_LOW;
    int next_interrupt_ms = (100 - speed) * DUTY_CYCLE_TICK;
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, next_interrupt_ms));
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
    ESP_ERROR_CHECK(timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_isr, (void *) TIMER_0, ESP_INTR_FLAG_IRAM, NULL));
    ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0));
    return;
}

static void gpio_setup() {
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_PIN_BITMASK;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    // initialize all gpio levels
    gpio_set_level(HBRIDGE_LEFT_IN1, LOGIC_LOW);
    gpio_set_level(HBRIDGE_LEFT_IN2, LOGIC_LOW);
    gpio_set_level(HBRIDGE_LEFT_PWM, LOGIC_LOW);
    gpio_set_level(HBRIDGE_RIGHT_IN1, LOGIC_LOW);
    gpio_set_level(HBRIDGE_RIGHT_IN2, LOGIC_LOW);
    gpio_set_level(HBRIDGE_RIGHT_PWM, LOGIC_LOW);
}

void getMessage(){
	char message[MESSAGE_LEN];
	output_info response;
	int counter = 0;

	//turn(RIGHT);
	for (;;){
		if (xQueueReceive(motor_in_queue, (void *)(message),(portTickType)portMAX_DELAY)){
			if (!strcmp(message, "MFWD")){
				//printf("Starting motor\n");
				set_speed_and_dir(12,FORWARD,FORWARD);
				
				response.type = 'M';
				response.outcome = 1;
			}
			else if (!strcmp(message, "MBACK")){
				//printf("Reversing motor\n");
				set_speed_and_dir(12,REVERSE, REVERSE);
				
				response.type = 'M';
				response.outcome = 1;
				
			} else if (!strcmp(message, "MSTOP")){
				//printf("Stopping motor\n");
				set_speed_and_dir(0,FORWARD,FORWARD);
				
				response.type = 'M';
				response.outcome = 1;
				
			} else if (!strcmp(message, "MRIGHT")){
				printf("TUrn right\n");
				set_speed_and_dir(100, REVERSE, FORWARD);
				vTaskDelay(375/portTICK_PERIOD_MS);
				set_speed_and_dir(0, FORWARD, FORWARD);
				
				response.type = 'M';
				response.outcome = 1;
				
			} else if (!strcmp(message, "MLEFT")){
				printf("Turn left\n");
				
				set_speed_and_dir(100, FORWARD, REVERSE);
				vTaskDelay(375/portTICK_PERIOD_MS);
				set_speed_and_dir(0, FORWARD, FORWARD);
				
				response.type = 'M';
				response.outcome = 1;
			} else {
				printf("Unknown command\n");
				response.type = 'M';
				response.outcome = 0;
			}
			
			/*
			while (xQueueSend(android_out_queue, (void *)(&response), 1000/portTICK_PERIOD_MS)){
				counter++;
				if (counter > 5){
					printf("Can't enque response!\n");
					break;
				}
			} */
			if (!xQueueSend(android_out_queue, (void *)(&response), 2000/portTICK_PERIOD_MS)){
				printf("Failed to enq\n");
			} else {
				printf("Msend\n");
			}
		}
	}
}

void motor_main() {
    set_speed_and_dir(0, FORWARD, FORWARD);
    gpio_setup();
    timer_pwm_interrupts_init();
	getMessage();
}
