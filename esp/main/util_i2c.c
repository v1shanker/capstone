/** @file util_i2c.c
 *
 *  @brief Contains motor controller i2c interfacing logic
 *
 *  @author Vikram Shanker (vshanker@andrew.cmu.edu)
 *
 *  @bug No known bugs.
*/

#include "driver/i2c.h"
#include "logger.h"
#include "util_i2c.h"

#define I2C_MASTER_SCL                     19               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA                     18               /*!< gpio number for I2C master data  */
#define I2C_MASTER_TX_BUF_DISABLE          0                /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE          0                /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ                 100000           /*!< I2C master clock frequency */

#define MOTOR_CONTROLLER_ADDR              0x4              /*!< Motor Controller slave address */
#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

/** @brief i2c master initialization
 *
 *  @param void
 *
 *  @return ESP_OK on success, esp_err_t error code otherwise
 */
esp_err_t i2c_master_init( void ) {

    int i2c_master_port;
    i2c_config_t conf;
    esp_err_t ret;

    i2c_master_port = I2C_MASTER_NUM;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    i2c_param_config(i2c_master_port, &conf);
    ret = i2c_driver_install(i2c_master_port,
                       conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE,
                       0 /* intr_alloc_flags */
                      );

    ESP_LOGE(_V, "i2c returns code : %d", ret);

    return ret;
}

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
        uint8_t slave_addr,
        uint8_t *data,
        size_t size) {

    i2c_cmd_handle_t cmd;
    esp_err_t ret;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slave_addr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    ESP_LOGE(_V, "i2c returns code : %d", ret);

    return ret;
}
