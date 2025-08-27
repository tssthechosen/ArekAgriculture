#ifndef HYDROPONICS_H
#define HYDROPONICS_H

#ifdef __cplusplus
extern "C" {
#endif

struct SensorData {
    float temperature;
    float humidity;
    float ph;
    float ph_voltage;
    float tds;
    float tds_voltage;
};

int hydroponics_init();
void hydroponics_cleanup();
int hydroponics_read_sensors(struct SensorData *data);
void hydroponics_set_relay(int relay, int state);

#ifdef __cplusplus
}
#endif

#endif
