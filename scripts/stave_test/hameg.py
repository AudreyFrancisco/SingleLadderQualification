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
        self.dev = [ serial.Serial("/dev/ttyHAMEG0", 9600, rtscts=True),
                     serial.Serial("/dev/ttyHAMEG1", 9600, rtscts=True),
                     serial.Serial("/dev/ttyHAMEG2", 9600, rtscts=True),
                     serial.Serial("/dev/ttyHAMEG3", 9600, rtscts=True) ]

        self.voltage = [ [ 1.87, 1.95, 1.87 ],          # 0
                         [ 1.97, 1.94, 1.97 ],          # 1
                         [ 1.90, 2.01, 1.94, 1.96 ],    # 2
                         [ 1.90, 2.03, 1.94, 1.97 ] ]   # 3

        self.current = [ [ 0.30, 1.50, 0.30 ],          # 0
                         [ 1.50, 0.30, 1.50 ],          # 1
                         [ 0.30, 1.50, 0.30, 1.50 ],    # 2
                         [ 0.30, 1.50, 0.30, 1.50 ] ]   # 3

        self.enable  = [ [ True, True, True ],          # 0
                         [ True, True, True ],          # 1
                         [ True, True, True, True ],    # 2
                         [ True, True, True, True ] ]   # 3

        self.channels = [ int(3), int(3), int(4), int(4) ]


    def moduleToDevAndCh(self, m):
        ### CHANNEL ASSIGNMENT ###

        #    CH1 CH2 CH3 CH4
        # 0:  A1  D1  A2   x
        # 1:  D2  A3  D3   x
        # 2:  A4  D4  A5  D5
        # 3:  A6  D6  A7  D7

        #  Return value:
        #
        #  Analog          Digital
        #  ((i_dev, i_ch), (i_dev, i_ch))
        if int(m)==1:
            return ( (0, 0), (0, 1) )
        elif int(m)==2: 
            return ( (0, 2), (1, 0) )
        elif int(m)==3: 
            return ( (1, 1), (1, 2) )
        elif int(m)==4: 
            return ( (2, 0), (2, 1) )
        elif int(m)==5: 
            return ( (2, 2), (2, 3) )
        elif int(m)==6: 
            return ( (3, 0), (3, 1) )
        elif int(m)==7: 
            return ( (3, 2), (3, 3) )
        else:
            return None
    

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
                self.dev[i_dev].write("FUSE:DEL 100\n")
                for j_ch in range(self.channels[i_dev]):
                    if i_ch != j_ch:
                        self.dev[i_dev].write("FUSE:UNLINK %d\n" % (j_ch+1))
                self.dev[i_dev].write("SOUR:VOLT %f\n" % self.voltage[i_dev][i_ch])
                self.dev[i_dev].write("SOUR:CURR %f\n" % self.current[i_dev][i_ch])
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

    def powerOnModule(self, m):
        analog, digital = self.moduleToDevAndCh(m)
        self.powerOnCh(analog[0], analog[1])
        self.powerOnCh(digital[0], digital[1])

    def powerOffModule(self, m):
        analog, digital = self.moduleToDevAndCh(m)
        self.powerOffCh(analog[0], analog[1])
        self.powerOffCh(digital[0], digital[1])

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
    module=int(sys.argv[2]) if len(sys.argv)>=3 else -1;

    h = hameg()
    # switch mode
    if mode==0:
        # prepare the voltage source
        h.init()
    elif mode==1:
        # power set a voltage value
        h.powerOn()
    elif mode==2:
        # power on a module
        if module<0 or module>7:
            print "Did not specify a correct module id from 1 to 7"
        else:
            h.powerOnModule(module)
    elif mode==3:
        # power off a module
        if module<0 or module>7:
            print "Did not specify a correct module id from 1 to 7"
        else:
            h.powerOffModule(module)
    elif mode==4:
        # measure all channels
        h.measureAll()
        return h.trip()
    elif mode==5:
        # check whether there was a trip
        return h.trip()
    else:
        h.powerOff()

## execute the main
if __name__ == "__main__":
    sys.exit(main())
