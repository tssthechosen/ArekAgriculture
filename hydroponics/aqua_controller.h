#ifndef AQUA_CONTROLLER_H
#define AQUA_CONTROLLER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the controller
int aqua_controller_init();

// Cleanup resources
void aqua_controller_cleanup();

// Read sensor data
int aqua_read_sensors(float* ph_value, float* tds_value, 
                     float* temperature, float* humidity);

// Control relays
void aqua_set_relay(int relay_number, int state);
int aqua_get_relay_state(int relay_number);

// Get raw voltage readings
float aqua_read_ph_voltage();
float aqua_read_tds_voltage();

#ifdef __cplusplus
}
#endif

#endif // AQUA_CONTROLLER_H