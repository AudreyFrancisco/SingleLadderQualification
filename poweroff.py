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
    sour.write("FUSE:DEL 100\n")
    sour.write("FUSE on\n")
    sour.write("SOUR:VOLT 0.0\n")
    sour.write("SOUR:CURR 1.0\n")
    time.sleep(0.5);
    sour.write("OUTP ON\n")
    time.sleep(2.0);
    sour.write("OUTP OFF\n")

    ## CH1
    #sour.write("INST OUT1\n");
    #sour.write("OUTP OFF\n");
    #
    ## CH3
    #sour.write("INST OUT3\n");
    #sour.write("OUTP OFF\n");


## execute the main
if __name__ == "__main__":
    sys.exit(main())
