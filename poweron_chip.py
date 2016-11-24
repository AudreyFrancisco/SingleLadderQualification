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
    sour.write("FUSE OFF\n")
    sour.write("SOUR:VOLT 1.895\n")
    sour.write("SOUR:CURR 2.0\n")
    time.sleep(0.5);
    sour.write("OUTP ON\n")
    time.sleep(1.5);
    sour.write("SOUR:CURR 0.15\n")
    sour.write("FUSE ON\n")


    #val=([0.0, 0.0, 0.0])
    #for c in range(3):
    #    sour.write("INST OUT%d\n" % (c+1))
    #    sour.write("MEAS:VOLT?\n")
    #    val[c] = float(sour.readline())
    #print "%0.4fV\t%0.4fV\t%0.4fV" % ( val[0], val[1], val[2])
    #
    #
    #for c in range(3):
    #    sour.write("INST OUT%d\n" % (c+1))
    #    sour.write("MEAS:CURR?\n")
    #    val[c] = float(sour.readline())
    #print "%0.4fA\t%0.4fA\t%0.4fA" % ( val[0], val[1], val[2])
    #
    #tripped = False
    #for c in range(3):
    #    sour.write("INST OUT%d\n" % (c+1))
    #    sour.write("FUSE:TRIP?\n")
    #    if (int(sour.readline())!=0):
    #        sys.stderr.write("Channel %d tripped\n" % (c+1))
    #        tripped = True
    #return tripped


## execute the main
if __name__ == "__main__":
    sys.exit(main())
    
