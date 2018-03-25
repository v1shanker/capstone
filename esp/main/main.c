/** @file main.c
 *
 *  @brief Beginning of microcontroller logic for self-driving-cart
 *
 *  @author Vikram Shanker (vshanker@andrew.cmu.edu)
 *
 *  @bug No known bugs.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logger.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "motor_controller_task.h"

/** @brief userspace main
 *
 *  @param void
 *
 *  @return void
 */
void app_main( void ) {
    ESP_LOGI(_G, "App Main Start\n");

    /* Log chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGV(_G, "This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGV(_G, "silicon revision %d, ", chip_info.revision);

    ESP_LOGV(_G, "%dMB %s flash\n",
            spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    create_motor_controller_task( void );
}
