/** @file motor_controller_task.c
 *
 *  @brief Motor controller task logic lives here
 *
 *  @author Vikram Shanker (vshanker@andrew.cmu.edu)
 *
 *  @bug No known bugs.
*/

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "motor_controller_task.h"
#include "util_i2c.h"

#define DELAY_MS                           5000             /*!< Task delay of 5 seconds */

#define MOTOR_CONTROLLER_ADDR              0x4              /*!< Motor Controller slave address */

#define START1                             0x53             /*!< magic start byte 1 */
#define START2                             0x01             /*!< magic start byte 2 */
#define END1                               0x2F             /*!< magic end byte 1 */
#define END2                               0x45             /*!< magic end byte 2 */
#define SPEED_80_FORWARD                   0x50             /*!< signed magnitude representatin of +80 */
#define SPEED_80_REVERSE                   0xD0             /*!< signed magnitude representatin of -80 */

#define MOTOR_CONTROLLER_DATA_SIZE         6
static uint8_t motor_controller_data[MOTOR_CONTROLLER_SIZE] =
        { START1, START2, SPEED_80_FORWARD, SPEED_80_FORWARD, END1, END2 }

static void motor_controller_task_fn( void ) {
    i2c_master_init( void );
    while (1) {
        ESP_LOGE(_V, "I2C write start\n");
        i2c_motor_controller_write(
                I2C_MASTER_NUM,
                MOTOR_CONTROLLER_ADDR,
                &motor_controller_data,
                MOTOR_CONTROLLER_DATA_SIZE);
        ESP_LOGE(_V, "I2C write end\n");

       vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
    }
}

void create_motor_controller_task( void ) {

    BaseType_t x_returned;
    TaskHandle_t x_task_handle;

    x_returned = TaskCreate(motor_controller_task_fn, /* task function */
                            "Motor Controller Task", /* Task Name */
                            MOTOR_CONTROLLER_STACK_DEPTH, /* usStackDepth - number of words */
                            NULL, /* pvParameters */
                            MOTOR_CONTROLLER_TASK_PRIO, /* uxPriority */
                            &x_task_handle /* reference to the task created */
                           );

    if (x_returned != pdPass) {
      ESP_LOGE(_G, "Failed to start motor controller task");
    }
}
