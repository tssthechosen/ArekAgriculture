#define _POSIX_C_SOURCE 199309L  // For CLOCK_MONOTONIC

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <gpiod.h>
#include "dht22.h"
#include <unistd.h>

#define DHT_GPIO_LINE 22    // BCM GPIO22
#define CHIPNAME "gpiochip0"

// Busy wait for approximately 'us' microseconds
static void busy_wait_us(int us) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
    } while (((now.tv_sec - start.tv_sec) * 1000000L + (now.tv_nsec - start.tv_nsec) / 1000L) < us);
}

int read_dht22(float *temperature, float *humidity) {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret = -1;

    chip = gpiod_chip_open_by_name(CHIPNAME);
    if (!chip) {
        perror("Open chip failed");
        return -1;
    }

    line = gpiod_chip_get_line(chip, DHT_GPIO_LINE);
    if (!line) {
        perror("Get line failed");
        gpiod_chip_close(chip);
        return -1;
    }

    // Request line as output and drive it high
    if (gpiod_line_request_output(line, "dht22", 1) < 0) {
        perror("Request line as output failed");
        gpiod_chip_close(chip);
        return -1;
    }

    // Send start signal: pull line low for 1 ms
    gpiod_line_set_value(line, 0);
    busy_wait_us(1000);

    // Pull line high and release output request
    gpiod_line_set_value(line, 1);
    gpiod_line_release(line);

    // Request line as input to read sensor response
    if (gpiod_line_request_input(line, "dht22") < 0) {
        perror("Request line as input failed");
        gpiod_chip_close(chip);
        return -1;
    }

    // Wait for sensor to pull line low (~80us)
    int count = 0;
    while (gpiod_line_get_value(line) == 1) {
        sleep(1);
        if (++count > 100) goto cleanup;  // timeout ~100us
    }

    // Wait for sensor to pull line high (~80us)
    count = 0;
    while (gpiod_line_get_value(line) == 0) {
        sleep(1);
        if (++count > 100) goto cleanup;
    }

    // Wait for sensor to pull line low again (~80us)
    count = 0;
    while (gpiod_line_get_value(line) == 1) {
        sleep(1);
        if (++count > 100) goto cleanup;
    }

    // Read 40 bits = 5 bytes
    uint8_t data[5] = {0};
    for (int i = 0; i < 40; i++) {
        // Wait for line to go high (start of bit)
        count = 0;
        while (gpiod_line_get_value(line) == 0) {
            sleep(1);
            if (++count > 100) goto cleanup;
        }

        // Measure length of the high signal to distinguish 0 vs 1
        int len = 0;
        while (gpiod_line_get_value(line) == 1) {
            sleep(1);
            len++;
            if (len > 100) break;  // max ~100us
        }

        int byte_idx = i / 8;
        data[byte_idx] <<= 1;
        if (len > 40)  // > ~40us means bit=1
            data[byte_idx] |= 1;
    }

    // Verify checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ret = -2;  // checksum failed
        goto cleanup;
    }

    // Convert data to humidity and temperature
    *humidity = ((data[0] << 8) | data[1]) * 0.1f;
    int temp_raw = ((data[2] & 0x7F) << 8) | data[3];
    *temperature = temp_raw * 0.1f;
    if (data[2] & 0x80)  // negative temperature
        *temperature = -*temperature;

    ret = 0;  // success

cleanup:
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return ret;
}
