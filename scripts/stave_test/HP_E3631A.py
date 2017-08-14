#!/usr/bin/env python2
import serial
import sys
import time


class HP_E3631A:
    def __init__(self, port_name):
        self.con=serial.Serial(port=port_name, baudrate=9600, bytesize=8, parity='N', stopbits=1, timeout=2, xonxoff=0, rtscts=0)

    def get_device_name(self):
        self.con.write(b'*IDN?\n')
        print self.con.readline()
        time.sleep(0.5)
        self.con.write(b'SYST:ERR?\n')
        print self.con.readline()

    def status(self):
        self.con.write(b'MEAS:VOLT? P6V\n')
        print self.con.readline()
        time.sleep(0.5)
        self.con.write(b'MEAS:VOLT? N25V\n')
        print self.con.readline()
        time.sleep(0.5)
        self.con.write(b'MEAS:CURR? P6V\n')
        print self.con.readline()
        time.sleep(0.5)
        self.con.write(b'MEAS:CURR? N25V\n')
        print self.con.readline()

    def init(self):
        self.con.write(b'*RST\n')
        time.sleep(2.)
        self.con.write(b'SYST:REM\n')
        time.sleep(0.5)
        self.con.write(b'APPL P6V, 5.0, 5.0\n')
        time.sleep(0.5)
        self.con.write(b'APPL N25V, -12.0, 0.5\n')
        time.sleep(0.5)
        self.con.write(b'OUTP ON\n')
        time.sleep(0.5)
        self.con.write(b'SYST:ERR?\n')
        print self.con.readline()

    def turn_off(self):
        self.con.write(b'OUTP OFF\n')

def main():
    dev = "/dev/ttyRDOPOWER"

    mode=int(sys.argv[1]) if len(sys.argv)>=2 else -1;
    
    psu=HP_E3631A(dev)
 
    # switch mode
    if mode==0:
        # read device name
        psu.get_device_name()
    elif mode==1:
        # power on
        psu.init()
    elif mode==2:
        # read voltages and currents
        psu.status()
    else:
        # power off
        psu.turn_off()

## execute the main
if __name__ == "__main__":
    sys.exit(main())
