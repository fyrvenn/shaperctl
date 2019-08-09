##Compiler
CROSS_COMPILE=arm-linux-gnueabihf-
CC=$(CROSS_COMPILE)gcc
##Flags
CFLAGS?=-Wall -Werror -Wpedantic
LDFLAGS?=
TARGET=shprctl

.PHONY: all clean distclean

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $< -o $@ $(LDFLAGS)

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET).o

distclean: clean
	rm -rf $(TARGET) 