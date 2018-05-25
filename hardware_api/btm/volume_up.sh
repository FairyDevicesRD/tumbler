#!/bin/bash
# GPIO39 -> BT_PIO18
/usr/bin/gpio mode 39 out;
/usr/bin/gpio write 39 1;
sleep 1.0; # > 800 msec
/usr/bin/gpio write 39 0;
