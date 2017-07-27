#!/usr/bin/env python

#
# WARNING: RTS/CTS should be used, as otherwise the PSU might not process
#          every command
#

import serial
import sys
import time

def trip(sour):
    # check compliance
    tripped = False
    for c in range(3):
        sour.write("INST OUT%d\n" % (c+1))
        sour.write("FUSE:TRIP?\n")
        if (int(sour.readline())!=0):
            sys.stderr.write("Channel %d tripped\n" % (c+1))
            tripped = True
    return tripped

def powerDown(sour):
    sour.write("INST OUT1\n")
    sour.write("OUTP:SEL OFF\n")
    sour.write("INST OUT2\n")
    sour.write("OUTP:SEL OFF\n")
    sour.write("INST OUT3\n")
    sour.write("OUTP:SEL OFF\n")
    sour.write("OUTP:GEN OFF\n")

def measureCurr(sour):
    val=([0.0, 0.0, 0.0])
    for c in range(3):
        sour.write("INST OUT%d\n" % (c+1))
        sour.write("MEAS:CURR?\n")
        val[c] = float(sour.readline())
    print "%0.4f\t%0.4f\t%0.4f" % ( val[0], val[1], val[2] )

def init2030(hameg, i_max):
    hameg.write("*IDN?\n")
    idn = hameg.readline()
    if not ("HAMEG") in idn:
        sys.stderr.write("WRONG DEVICE: %s" % idn)
        return
    #print idn
    #print "maximum current: %f %f %f" % i_max
    hameg.write("*RST\n")
    time.sleep(0.1);

    # activate digital fuse and set outputs to 0V
    # CH1
    hameg.write("INST OUT1\n");
    hameg.write("FUSE:LINK 2\n")
    hameg.write("FUSE:LINK 3\n")
    hameg.write("FUSE:DEL 100\n")
    hameg.write("FUSE on\n")
    hameg.write("SOUR:VOLT 0.0\n")
    hameg.write("SOUR:CURR %f\n" % i_max[0])
    time.sleep(0.1);
    # CH2
    hameg.write("INST OUT2\n");
    hameg.write("FUSE:LINK 1\n")
    hameg.write("FUSE:LINK 3\n")
    hameg.write("FUSE:DEL 100\n")
    hameg.write("FUSE on\n")
    hameg.write("SOUR:VOLT 0.0\n")
    hameg.write("SOUR:CURR %f\n" % i_max[1])
    time.sleep(0.1);
    # CH3
    hameg.write("INST OUT3\n");
    hameg.write("FUSE:LINK 1\n")
    hameg.write("FUSE:LINK 2\n")
    hameg.write("FUSE:DEL 100\n")
    hameg.write("FUSE on\n")
    hameg.write("SOUR:VOLT 0.0\n")
    hameg.write("SOUR:CURR %f\n" % i_max[2])
    time.sleep(0.1);

def activate_module(sour, v, vbb):
    sour.write("OUTP:GEN OFF\n")
    time.sleep(0.1)
    sour.write("INST OUT1\n")
    sour.write("SOUR:VOLT %0.3f\n" % float(v))
    sour.write("OUTP:SEL ON\n")
    time.sleep(0.1)
    sour.write("INST OUT2\n")
    sour.write("SOUR:VOLT %0.3f\n" % float(v))
    sour.write("OUTP:SEL ON\n")
    time.sleep(0.1)
    sour.write("INST OUT3\n")
    sour.write("SOUR:VOLT %0.3f\n" % float(vbb))
    sour.write("OUTP:SEL ON\n")
    time.sleep(0.1)

    sour.write("OUTP:GEN ON\n")


def main():
    # set up serial port
    dev=sys.argv[1]
    mode=int(sys.argv[2]) if len(sys.argv)>=3 else -1;
    sour=serial.Serial(dev, 9600, rtscts=True);

    # switch mode
    if mode==0:
        # prepare the voltage source
        powerDown(sour)
        init2030(sour, (float(sys.argv[3]), float(sys.argv[4]), float(sys.argv[5])))
    elif mode==1:
        # set a voltage value
        activate_module(sour, float(sys.argv[3]), float(sys.argv[4]))
    elif mode==2:
        # measure current and check whether it is in range
        measureCurr(sour)
        return trip(sour)
    else:
        powerDown(sour)

## execute the main
if __name__ == "__main__":
    sys.exit(main())
