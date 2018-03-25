/** @file util_i2c.h
 *
 *  @brief Contains motor controller i2c interfacing prototypes.
 *
 *  @author Vikram Shanker (vshanker@andrew.cmu.edu)
 *
 *  @bug No known bugs.
*/

#ifndef _UTIL_I2C_H_
#define _UTIL_I2C_H_

#include "driver/i2c.h"

#define I2C_MASTER_NUM                     I2C_NUM_1        /*!< I2C port number for master dev */

/** @brief i2c master initialization
 *
 *  @param void
 *
 *  @return ESP_OK on success, esp_err_t error code otherwise
 */
esp_err_t i2c_master_init( void );

/** @brief Write to motor controller using I2C
 *
 *  @param i2c_num   the i2c port number
 *  @param data      the data to be written to the slave
 *  @param size      the size of the buffer
 *
 *  @return ESP_OK on success and ESP_ERR_INVALID_ARG on parameter error
 */
esp_err_t i2c_motor_controller_write (
        i2c_port_t i2c_num,
        uint8_t addr,
        uint8_t *data,
        size_t size);

#endif /* _UTIL_I2C_H_ */
