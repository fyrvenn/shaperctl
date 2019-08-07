# Shaper Control utility

## Description

The utility can:
* turn on/off the shaper
* set the speed limit (with the minimum permissible bias)
* set filters
* display shaper's state in the console (activity, speed limit, filters)
* this all for each of the two ports separately
* the utility should be managed by input arguments in command line (interactive mode is not necessary)

## Usage

Run in terminal following command for set filters or speed limit:
```
$ ./shpctrl --port={0|1} --set-priv={both|user|mpt|none} --set-rate={Speed rate, bps}
```
To enable/disable port:
```
$ ./shpctrl --port={0|1} --enable
$ ./shpctrl --port={0|1} --disable
```
To see status of sertain port:
```
$ ./shpctrl --port={0|1} --status
```
## Build

For build run:
```
$ make
```

## External Dependencies

* GCC compiler for arm (arm-linux-gnueabihf-gcc)

## Author

Katerina Budnikova

mail: e.budnikova@metrotek.ru

## Date

Tue, 06 Aug 2019 17:10:54 +0300