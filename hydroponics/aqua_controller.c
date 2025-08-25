#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "relay.h"
#include "ads1115.h"
#include "dht22.h"
#include "aqua_controller.h"

// Global file descriptor for ADS1115
static int ads1115_fd = -1;

float voltage_to_ph(float voltage) {
    return 3.5f * voltage;
}

float voltage_to_tds(float voltage2) {
    float tds = (133.42f * voltage2 * voltage2 * voltage2)
              - (255.86f * voltage2 * voltage2)
              + (857.39f * voltage2);
    return tds * 0.5f; // Compensation factor
}

int aqua_controller_init() {
    init_relays();
    
    ads1115_fd = ads1115_init("/dev/i2c-1", 0x48);
    if (ads1115_fd < 0) {
        fprintf(stderr, "Failed to initialize ADS1115\n");
        return 0; // Failure
    }
    
    return 1; // Success
}

void aqua_controller_cleanup() {
    if (ads1115_fd >= 0) {
        close(ads1115_fd);
        ads1115_fd = -1;
    }
    cleanup_relays();
}

int aqua_read_sensors(float* ph_value, float* tds_value, 
                     float* temperature, float* humidity) {
    if (ads1115_fd < 0) {
        return 0; // Not initialized
    }

    float ph_voltage = ads1115_read_voltage(ads1115_fd, 0);
    float tds_voltage = ads1115_read_voltage2(ads1115_fd, 1);

    *ph_value = voltage_to_ph(ph_voltage);
    *tds_value = voltage_to_tds(tds_voltage);

    int dht_status = read_dht22(temperature, humidity);
    return dht_status == 0 ? 1 : 0; // Return 1 on success, 0 on failure
}

void aqua_set_relay(int relay_number, int state) {
    set_relay(relay_number, state);
}

int aqua_get_relay_state(int relay_number) {
    // You'll need to implement this function in relay.h/relay.c
    // to read the current state of a relay
    return 0; // Placeholder
}

float aqua_read_ph_voltage() {
    if (ads1115_fd < 0) return 0.0f;
    return ads1115_read_voltage(ads1115_fd, 0);
}

float aqua_read_tds_voltage() {
    if (ads1115_fd < 0) return 0.0f;
    return ads1115_read_voltage2(ads1115_fd, 1);
}