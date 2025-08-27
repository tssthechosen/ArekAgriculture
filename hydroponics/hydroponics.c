// hydroponics.c
#include "hydroponics.h"
#include "relay.h"
#include "ads1115.h"
#include "dht22.h"

#include <unistd.h>
static int ads_fd = -1;

float voltage_to_ph(float voltage) {
    return 3.5f * voltage;
}

float voltage_to_tds(float voltage2) {
    float tds = (133.42f * voltage2 * voltage2 * voltage2)
              - (255.86f * voltage2 * voltage2)
              + (857.39f * voltage2);
    return tds * 0.5f;
}

int hydroponics_init() {
    init_relays();
    ads_fd = ads1115_init("/dev/i2c-1", 0x48);
    return (ads_fd < 0) ? -1 : 0;
}

void hydroponics_cleanup() {
    if (ads_fd >= 0) close(ads_fd);
    cleanup_relays();
}

int hydroponics_read_sensors(struct SensorData *data) {
    if (!data) return -1;

    float ph_voltage = ads1115_read_voltage(ads_fd, 0);
    float tds_voltage = ads1115_read_voltage(ads_fd, 1);

    data->ph_voltage = ph_voltage;
    data->tds_voltage = tds_voltage;
    data->ph = voltage_to_ph(ph_voltage);
    data->tds = voltage_to_tds(tds_voltage);

    float temperature = 0.0f, humidity = 0.0f;
    int status = read_dht22(&temperature, &humidity);

    if (status == 0) {
        data->temperature = temperature;
        data->humidity = humidity;
    } else {
        data->temperature = -999;
        data->humidity = -999;
    }

    return 0;
}

void hydroponics_set_relay(int relay, int state) {
    set_relay(relay, state);
}
