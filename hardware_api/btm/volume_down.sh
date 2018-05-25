#!/bin/bash
# GPIO33 -> BT_PIO7
/usr/bin/gpio mode 33 out;
/usr/bin/gpio write 33 1;
sleep 1.0; # > 800 msec
/usr/bin/gpio write 33 0;
sleep 1.0;
