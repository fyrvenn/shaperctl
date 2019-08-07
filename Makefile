##Compiler
CROSS_COMPILE=arm-linux-gnueabihf-
CC=$(CROSS_COMPILE)gcc
##Flags
TARGET=shprctl

.PHONY: all clean distclean

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $< -o $@

$(TARGET).o: $(TARGET).c
	$(CC) -c $< -o $@

clean:
	rm -rf $(TARGET).o

distclean:
	rm -rf $(TARGET) 