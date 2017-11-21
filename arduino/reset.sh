#!/bin/bash
# Reset arduino subsystem
/bin/stty --file /dev/ttyAMA0 -hupcl
