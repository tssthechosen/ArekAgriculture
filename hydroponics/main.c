#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "relay.h"
#include "ads1115.h"
#include "dht22.h"

float voltage_to_ph(float voltage) {
    return 3.5f * voltage;
}

float voltage_to_tds(float voltage2) {
    float tds = (133.42f * voltage2 * voltage2 * voltage2)
              - (255.86f * voltage2 * voltage2)
              + (857.39f * voltage2);
    return tds * 0.5f; // Compensation factor
}

int main() {
    init_relays();
    int fd = ads1115_init("/dev/i2c-1", 0x48);
    if (fd < 0) {
        fprintf(stderr, "Failed to initialize ADS1115\n");
        return 1;
    }

    while (1) {
        float ph_voltage = ads1115_read_voltage(fd, 0);
        float tds_voltage = ads1115_read_voltage2(fd, 1);

        float ph_value = voltage_to_ph(ph_voltage);
        float tds_value = voltage_to_tds(tds_voltage);

        float temperature, humidity;
        int dht_status = read_dht22(&temperature, &humidity);
        if (dht_status == 0) {
            printf("Temperature: %.1f °C, Humidity: %.1f%%\n", temperature, humidity);
        } else if (dht_status == -2) {
            printf("DHT22 checksum failed\n");
        } else {
            printf("Failed to read DHT22 sensor\n");
        }
        // Relay Logic
        int ph_out_of_range = (ph_value < 1.0 || ph_value > 5.0);
        int tds_out_of_range = (tds_value < 100.0 || tds_value > 300.0);

        set_relay(GPIO_RELAY1, ph_out_of_range);
        set_relay(GPIO_RELAY2, tds_out_of_range);
        set_relay(GPIO_RELAY3, ph_out_of_range && tds_out_of_range);
 

        printf("pH: %.2f (%.3f V), TDS: %.1f ppm (%.3f V)\n\n",
               ph_value, ph_voltage, tds_value, tds_voltage);


        sleep(2);
    }

    close(fd);
    cleanup_relays();
    printf("Exiting program.\n");
    return 0;
}
