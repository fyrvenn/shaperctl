##Compiler
CC=arm-linux-gnueabihf-gcc

##Flags
SHPCTRL=shprctl
CFLAGS=
LDFLAGS=

.PHONY: all clean shprctl

all: $(SHPCTRL)

$(SHPCTRL): $(SHPCTRL).o
	$(CC) $< -o $@ $(LDFLAGS)

$(SHPCTRL).o: shprctl.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(SHPCTRL) 
