// relay.h
#ifndef RELAY_H
#define RELAY_H

#define GPIO_RELAY1 26
#define GPIO_RELAY2 6
#define GPIO_RELAY3 5

void init_relays();
void set_relay(int gpio, int state);
void cleanup_relays();

#endif
