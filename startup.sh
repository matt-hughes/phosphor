#!/bin/bash

echo 18 > /sys/class/gpio/export
boot_gpio="$(cat /sys/class/gpio/gpio18/value)"

if [ "$boot_gpio" == "1" ]; then
	tvservice -e "CEA 16 HDMI"
	fbset -xres 1920 -yres 1080
	stty -F /dev/ttyAMA0 raw speed 1500000
	sleep 1
	stty -F /dev/ttyAMA0 raw speed 1500000
	sleep 1
	make run
else
	tvservice -p
fi
