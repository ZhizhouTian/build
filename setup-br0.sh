#!/bin/bash

sudo ip link add br0 type bridge
sudo ip link set br0 up
sudo ip link set eth0 master br0
sudo killall dhclient
sudo ip addr flush dev eth0
sudo dhclient br0
sudo route add default gw 192.168.3.253
