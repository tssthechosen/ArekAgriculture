#include "ads1115.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>  // for TDS conversion

// ADS1115 Registers and config bits
#define ADS1115_POINTER_CONVERT   0x00
#define ADS1115_POINTER_CONFIG    0x01

#define ADS1115_CONFIG_OS_SINGLE   0x8000
#define ADS1115_CONFIG_MUX_SHIFT   12
#define ADS1115_CONFIG_GAIN_4_096  0x0200
#define ADS1115_CONFIG_MODE_SINGLE 0x0100
#define ADS1115_CONFIG_DR_128SPS   0x0080
#define ADS1115_CONFIG_COMP_QUE_DISABLE 0x0003

int ads1115_init(const char *i2c_dev, int i2c_addr) {
    int fd = open(i2c_dev, O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C device");
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, i2c_addr) < 0) {
        perror("Failed to set I2C address");
        close(fd);
        return -1;
    }
    return fd;
}

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

#define ADS1115_POINTER_CONVERT   0x00
#define ADS1115_POINTER_CONFIG    0x01

#define ADS1115_OS_SINGLE         0x8000
#define ADS1115_MODE_SINGLE       0x0100
#define ADS1115_DR_128SPS         0x0080

#define ADS1115_PGA_4_096V        0x0200  // ±4.096V range
#define ADS1115_COMP_DISABLE      0x0003

/*int ads1115_init(const char *i2c_device, int addr) {
    int fd = open(i2c_device, O_RDWR);
    if (fd < 0) return -1;
    if (ioctl(fd, I2C_SLAVE, addr) < 0) return -2;
    return fd;
}
*/
float ads1115_read_voltage(int fd, int channel) {
    if (channel < 0 || channel > 3) return -1;

    uint16_t config = ADS1115_OS_SINGLE
                    | ADS1115_MODE_SINGLE
                    | ADS1115_DR_128SPS
                    | ADS1115_PGA_4_096V
                    | ADS1115_COMP_DISABLE;

    // Set MUX for single-ended channel
    switch (channel) {
        case 0: config |= 0x4000; break; // AIN0
        case 1: config |= 0x5000; break; // AIN1
        case 2: config |= 0x6000; break; // AIN2
        case 3: config |= 0x7000; break; // AIN3
    }

    // Write config
    uint8_t write_buf[3];
    write_buf[0] = ADS1115_POINTER_CONFIG;
    write_buf[1] = config >> 8;
    write_buf[2] = config & 0xFF;
    if (write(fd, write_buf, 3) != 3) return -1;

    usleep(8000);  // Wait for conversion

    // Read conversion result
    uint8_t read_buf[2];
    write_buf[0] = ADS1115_POINTER_CONVERT;
    if (write(fd, write_buf, 1) != 1) return -1;
    if (read(fd, read_buf, 2) != 2) return -1;

    int16_t raw = (read_buf[0] << 8) | read_buf[1];

    // Convert to voltage (±4.096V range, 16-bit signed integer)
    float voltage = (float)raw * 4.096 / 32768.0;
    return voltage;
}
float ads1115_read_voltage2(int fd, int channel) {
    if (channel < 0 || channel > 3) return -1;

    uint16_t config = ADS1115_OS_SINGLE
                    | ADS1115_MODE_SINGLE
                    | ADS1115_DR_128SPS
                    | ADS1115_PGA_4_096V
                    | ADS1115_COMP_DISABLE;

    // Set MUX for single-ended channel
    switch (channel) {
        case 0: config |= 0x4000; break; // AIN0
        case 1: config |= 0x5000; break; // AIN1
        case 2: config |= 0x6000; break; // AIN2
        case 3: config |= 0x7000; break; // AIN3
    }

    // Write config
    uint8_t write_buf[3];
    write_buf[0] = ADS1115_POINTER_CONFIG;
    write_buf[1] = config >> 8;
    write_buf[2] = config & 0xFF;
    if (write(fd, write_buf, 3) != 3) return -1;

    usleep(8000);  // Wait for conversion

    // Read conversion result
    uint8_t read_buf[2];
    write_buf[0] = ADS1115_POINTER_CONVERT;
    if (write(fd, write_buf, 1) != 1) return -1;
    if (read(fd, read_buf, 2) != 2) return -1;

    int16_t raw = (read_buf[0] << 8) | read_buf[1];

    // Convert to voltage (±4.096V range, 16-bit signed integer)
    float voltage2 = (float)raw * 4.096 / 32768.0;
    return voltage2;
}

// Convert voltage to pH using typical calibration: 0V = pH 0, 3.0V = pH 14
float ads1115_voltage_to_ph(float voltage) {
    // Adjust for your sensor calibration values if needed
    float ph = 14.0f * voltage / 3.0f;
    if (ph < 0.0f) ph = 0.0f;
    if (ph > 14.0f) ph = 14.0f;
    return ph;
}

// Convert voltage to TDS (in ppm) using DFRobot Gravity analog TDS formula
float ads1115_voltage_to_tds(float voltage2, float temperature_celsius) {
    // 1. Compensation for temperature
    float compensation_coefficient = 1.0f + 0.02f * (temperature_celsius - 25.0f);
    float compensated_voltage = voltage2 / compensation_coefficient;

    // 2. Convert to ppm (based on DFRobot formula for 3.3V ADC reference)
    float tds = (133.42f * powf(compensated_voltage, 3)
               - 255.86f * powf(compensated_voltage, 2)
               + 857.39f * compensated_voltage) * 0.5f;

    if (tds < 0.0f) tds = 0.0f;
    return tds;
}
