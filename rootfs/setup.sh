#!/bin/ash

ifconfig eth0 192.168.3.103 netmask 255.255.255.0
echo 1 > /proc/sys/net/ipv4/ip_forward
route add default gw 192.168.3.253
export LD_LIBRARY_PATH=/lib64
