#!/usr/bin/env python

#
# WARNING: RTS/CTS should be used, as otherwise the PSU might not process
#          every command
#

import serial
import sys
import time

def main():
    # set up serial port
    dev="/dev/ttyHAMEG0"
    sour=serial.Serial(dev, 9600, rtscts=True);

    sour.write("*IDN?\n")
    idn = sour.readline()
    if not ("HAMEG") in idn:
        sys.stderr.write("WRONG DEVICE: %s" % idn)
        return

    # CH2
    sour.write("INST OUT2\n");
    sour.write("FUSE:LINK 1\n")
    sour.write("FUSE:LINK 3\n")
    sour.write("FUSE:LINK 4\n")
    sour.write("FUSE:DEL 100\n")
    sour.write("FUSE on\n")
    sour.write("SOUR:VOLT 0.0\n")
    sour.write("SOUR:CURR 0.150\n")
    sour.write("OUTP ON\n")


    # CH4
    sour.write("INST OUT4\n");
    sour.write("FUSE:LINK 1\n")
    sour.write("FUSE:LINK 2\n")
    sour.write("FUSE:LINK 3\n")
    sour.write("FUSE:DEL 100\n")
    sour.write("FUSE on\n")
    sour.write("SOUR:VOLT 0.0\n")
    sour.write("SOUR:CURR 0.150\n")
    sour.write("OUTP ON\n")
    time.sleep(2.0);


## execute the main
if __name__ == "__main__":
    sys.exit(main())
