#!/usr/bin/env python2

#
# WARNING: RTS/CTS should be used, as otherwise the PSU might not process
#          every command
#

import serial
import sys
import time

class hameg:
    def __init__(self):
        ### CONFIGURATION ###
        self.dev = [ serial.Serial("/dev/ttyHAMEG3", 9600, rtscts=True) ]

        self.voltage = [ [ 1.80, 1.80, 0.00, 0.000 ] ]   # 3

        self.current = [ [ 1.30, 0.40, 0.03, 0.005 ] ]   # 3

        self.enable  = [ [ True, True, True, True  ] ]   # 3

        self.channels = [ int(4) ]

    def init(self):
        ### INITIALISATION ###
        for i_dev in range(len(self.dev)):
            self.dev[i_dev].write("*IDN?\n")
            idn = self.dev[i_dev].readline()
            if not ("HAMEG") in idn:
                sys.stderr.write("WRONG DEVICE: %s" % idn)
                return
            #print idn
            self.dev[i_dev].write("*RST\n")
            time.sleep(0.1);

            self.dev[i_dev].write("OUTP:GEN OFF\n")
            for i_ch in range(self.channels[i_dev]):
                self.dev[i_dev].write("INST OUT%d\n" % (i_ch+1))
                self.dev[i_dev].write("OUTP:SEL OFF\n")
                self.dev[i_dev].write("FUSE on\n")
                self.dev[i_dev].write("FUSE:DEL 200\n")
                for j_ch in range(self.channels[i_dev]):
                    if i_ch != j_ch:
                        self.dev[i_dev].write("FUSE:LINK %d\n" % (j_ch+1))
                self.dev[i_dev].write("SOUR:VOLT %f\n" % self.voltage[i_dev][i_ch])
                self.dev[i_dev].write("SOUR:CURR %f\n" % self.current[i_dev][i_ch])
            time.sleep(2)
            self.dev[i_dev].write("OUTP:GEN ON\n")

 

    def trip(self):
        # check compliance
        tripped = False
        for i_dev in range(len(self.dev)):
            for i_ch in range(self.channels[i_dev]):
                self.dev[i_dev].write("INST OUT%d\n" % (i_ch+1))
                self.dev[i_dev].write("FUSE:TRIP?\n")
                if (int(self.dev[i_dev].readline())!=0):
                    sys.stderr.write("Device %d, channel %d tripped\n" % (i_dev+1, i_ch+1))
                    tripped = True
                    return tripped

    def powerOnCh(self, i_dev, i_ch):
        self.dev[i_dev].write("INST OUT%d\n" % (i_ch+1))
        self.dev[i_dev].write("OUTP:SEL ON\n")

    def powerOffCh(self, i_dev, i_ch):
        self.dev[i_dev].write("INST OUT%d\n" % (i_ch+1))
        self.dev[i_dev].write("OUTP:SEL OFF\n")

    def powerOff(self):
        for i_dev in range(len(self.dev)):
            self.dev[i_dev].write("OUTP:GEN OFF\n")
            for i_ch in range(self.channels[i_dev]):
                self.powerOffCh(i_dev, i_ch)

    def powerOn(self):
        for i_dev in range(len(self.dev)):
            self.dev[i_dev].write("OUTP:GEN ON\n")
            for i_ch in range(self.channels[i_dev]):
                self.powerOnCh(i_dev, i_ch)

    def changeCh(self, i_dev, i_ch, voltage, current):
        print "Channel: %d; voltage: %fV; current: %fA" %( i_ch, voltage, current) 
        self.dev[i_dev].write("INST OUT%d\n" % (i_ch+1))
        self.dev[i_dev].write("SOUR:VOLT %f\n" % float(voltage))
        self.dev[i_dev].write("SOUR:CURR %f\n" % float(current))

    def measureAll(self):
        val=([0.0, 0.0])
        for i_dev in range(len(self.dev)):
            for i_ch in range(self.channels[i_dev]):
                self.dev[i_dev].write("INST OUT%d\n" % (i_ch+1))
                self.dev[i_dev].write("MEAS:VOLT?\n")
                val[0] = float(self.dev[i_dev].readline())
                self.dev[i_dev].write("MEAS:CURR?\n")
                val[1] = float(self.dev[i_dev].readline())
                print "%0.4f\t%0.4f\t" % ( val[0], val[1] ),
        print ""
    

def main():
    mode=int(sys.argv[1]) if len(sys.argv)>=2 else -1;
    ch=int(sys.argv[2]) if len(sys.argv)>=3 else -1;
    voltage=float(sys.argv[3]) if len(sys.argv)>=4 else -1;
    current=float(sys.argv[4]) if len(sys.argv)>=5 else -1;

    h = hameg()
    # switch mode
    if mode==0:
        # prepare the voltage source
        h.init()
    elif mode==1:
        # power set a voltage value
        h.powerOn()
    elif mode==2:
        # change channel
        if ch<0 or voltage<0 or current<0:
            print "Did not correctly specify <mode> <channel> <voltage> <current>"
            return 1
        else:
            h.changeCh(0, ch, voltage, current)
    elif mode==3:
        # measure all channels
        h.measureAll()
        return h.trip()
    elif mode==4:
        # check whether there was a trip
        return h.trip()
    else:
        h.powerOff()
    return 0

## execute the main
if __name__ == "__main__":
    sys.exit(main())
