# Makefile

CC = gcc
CFLAGS = -Wall -O2
TARGET = hydro_test
SRCS = aqua_controller.c ads1115.c
LIBS = -li2c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
