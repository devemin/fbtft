#!/usr/bin/bash


sudo modprobe fb_sys_fops
sudo modprobe syscopyarea
sudo modprobe sysfillrect
sudo modprobe sysimgblt


sudo insmod ./fbtft.ko dma

sudo insmod ./fb_s6d1121.ko

sudo insmod ./fbtft_device.ko name=itdb24 fps=20 width=640 height=48 bgr=1 buswidth=8 speed=10000000 gpios=reset:4,dc:3,wr:23,cs:2,db00:5,db01:6,db02:13,db03:19,db04:26,db05:12,db06:16,db07:20
