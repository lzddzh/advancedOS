#!/usr/bin/sudo bash
make
sudo rmmod one
sudo insmod one.ko
#cat /dev/one
