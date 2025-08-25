#ifndef ADS1115_H
#define ADS1115_H

#include <stdint.h>

// Initialize ADS1115 ADC, returns file descriptor or -1 on failure
int ads1115_init(const char *i2c_dev, int i2c_addr);

// Read single-ended voltage from specified channel (0 to 3)
// Returns voltage in volts
float ads1115_read_voltage(int fd, int channel);
float ads1115_read_voltage2(int fd, int channel);
#endif // ADS1115_H
