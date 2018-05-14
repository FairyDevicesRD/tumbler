#!/bin/bash
# GPIO42 -> BT_VREG_EN
/usr/bin/gpio mode 42 out;
/usr/bin/gpio write 42 1;
sleep 0.2; # > 100 msec
/usr/bin/gpio write 42 0;
sleep 0.2;
/usr/bin/gpio write 42 1;
sleep 0.2;
/usr/bin/gpio write 42 0;

# monitor
/usr/bin/gpio mode 34 in; # LED0
/usr/bin/gpio mode 35 in; # LED1
/usr/bin/gpio read 35;
