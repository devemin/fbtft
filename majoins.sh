#!/bin/bash
sudo modprobe fbtft
sudo rmmod fbtft
sudo insmod ./fbtft.ko
sudo insmod ./fb_majocairisLCD.ko
