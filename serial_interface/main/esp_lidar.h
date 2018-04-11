/*
 * @file esp_lidar.h
 * @brief Interface for LIDAR driver
 *
 * @author Naveen
 * @bug None
 */

#ifndef __ESP_LIDAR_H
#define __ESP_LIDAR_H
void lidar_setup();
void lidar_readBytes(char *buffer, size_t n);
void lidar_sendBytes(char *buffer, size_t n);
#endif
