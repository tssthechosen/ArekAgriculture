CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lm   # link math library if needed (TDS calc uses pow sometimes)

# source files
SRCS = main.c hydroponics.c ads1115.c relay.c dht22.c
OBJS = $(SRCS:.c=.o)

# output binary
TARGET = hydro_test 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
