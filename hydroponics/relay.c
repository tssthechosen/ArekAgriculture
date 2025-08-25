// relay.c
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include "relay.h"

static struct gpiod_chip *chip;
static struct gpiod_line *relay1, *relay2, *relay3;

void init_relays() {
    chip = gpiod_chip_open_by_name("gpiochip4"); // Adjust based on your GPIO pin mapping
    if (!chip) {
        perror("Failed to open GPIO chip");
        exit(1);
    }

    relay1 = gpiod_chip_get_line(chip, GPIO_RELAY1);
    relay2 = gpiod_chip_get_line(chip, GPIO_RELAY2);
    relay3 = gpiod_chip_get_line(chip, GPIO_RELAY3);

    if (gpiod_line_request_output(relay1, "relay", 0) ||
        gpiod_line_request_output(relay2, "relay", 0) ||
        gpiod_line_request_output(relay3, "relay", 0)) {
        perror("Failed to request output lines for relays");
        exit(1);
    }
}

void set_relay(int gpio, int state) {
    struct gpiod_line *line = NULL;
    switch (gpio) {
        case GPIO_RELAY1: line = relay1; break;
        case GPIO_RELAY2: line = relay2; break;
        case GPIO_RELAY3: line = relay3; break;
        default: return;
    }
    gpiod_line_set_value(line, state);
}

void cleanup_relays() {
    gpiod_line_release(relay1);
    gpiod_line_release(relay2);
    gpiod_line_release(relay3);
    gpiod_chip_close(chip);
}
