CC=gcc
CFLAGS=-Wall -pthread
INCLUDE=-Iinclude
LIBS=-lbluetooth -lasound

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)
TARGET=bt_ag

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)