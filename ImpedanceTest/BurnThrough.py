#!/usr/bin/env python

import logging
import os

import serial
import sys
import time

def init4030(hameg, i_max):
    hameg.write("*IDN?\n")
    idn = hameg.readline()
    if not ("HAMEG") in idn:
        sys.stderr.write("WRONG DEVICE: %s" % idn)
        return
    #print idn
    #print "maximum current: %f %f %f" % i_max
    hameg.write("*RST\n")
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
    hameg.write("OUTP ON\n")
    # CH2
    hameg.write("INST OUT2\n");
    hameg.write("FUSE:LINK 1\n")
    hameg.write("FUSE:LINK 3\n")
    hameg.write("FUSE:DEL 100\n")
    hameg.write("FUSE on\n")
    hameg.write("SOUR:VOLT 0.0\n")
    hameg.write("SOUR:CURR %f\n" % i_max[1])
    time.sleep(0.1);
    hameg.write("OUTP ON\n")
    # CH3
    hameg.write("INST OUT3\n");
    hameg.write("FUSE:LINK 1\n")
    hameg.write("FUSE:LINK 2\n")
    hameg.write("FUSE:DEL 100\n")
    hameg.write("FUSE on\n")
    hameg.write("SOUR:VOLT 0.0\n")
    hameg.write("SOUR:CURR %f\n" % i_max[2])
    time.sleep(0.1);
    hameg.write("OUTP ON\n")   

def step_up(sour, channel, voltage):
    
    sour.write("INST OUT%d\n" % channel)
    sour.write("SOUR:VOLT %f\n" % voltage)
    time.sleep(2)
    sour.write("MEAS:CURR?\n")
    return float(sour.readline())
    
def measure_current(sour, channel):
    sour.write("INST OUT%d\n" % channel)
    sour.write("MEAS:CURR?\n")
    return float(sour.readline())
    
def main():
     # set up serial port
    dev='/dev/ttyUSB0'
    
    data_dir = "DataBT"
    
    current_limits = ([2., 2., 0.5]) 
    max_voltages = ([1.8, 1.8, 4])
    voltage = 0.0
    voltage_step=0.1
    
    continue_test = True
    
    sour=serial.Serial(dev, 9600, rtscts=True)
    init4030(sour, current_limits) 
    
    HIC_name = raw_input("Enter HIC name: ")
    HIC_name = 'OBHIC-' + HIC_name  
    
    path = data_dir + '/' + HIC_name

    if not os.path.isdir(path):
        os.makedirs(path)
    
    print 'Select the channel'
    print '1: DVDD'
    print '2: AVDD'
    print '3: BIAS'
    channel=int(raw_input())
    if channel<1 or channel>3:
        print 'Wrong channel'
        sys.exit(0)
    
    if channel==1:
        channel_name = "DVDD" 
        output_file_name = path + '/' + time.strftime("%y%m%d_%H%M%S") + '_' + HIC_name + '_DVDD.dat'
    elif channel==2:
        channel_name = "AVDD"      
        output_file_name = path + '/' + time.strftime("%y%m%d_%H%M%S") + '_' + HIC_name + '_AVDD.dat'
    elif channel==3:
        channel_name = "BIAS" 
        output_file_name = path + '/' + time.strftime("%y%m%d_%H%M%S") + '_' + HIC_name + '_BIAS.dat'
        
    output_file = open(output_file_name,'w')
    
    while continue_test:
        voltage+=voltage_step
        current = step_up(sour, channel, voltage)
        resistance = voltage/current
        print("V=%f, I=%f, R=%f" % (voltage, current, resistance))
        output_file.write("%f,%f,%f\n" % (voltage, current, resistance)) 
        
        for i in range (0, 15):
            time.sleep(2)
            current = measure_current(sour, channel)
            resistance = voltage/current
            print("V=%f, I=%f, R=%f" % (voltage, current, resistance))
            output_file.write("%f,%f,%f\n" % (voltage, current, resistance)) 
            
        user_reply = raw_input('Continue? y/n: ')
        
        if voltage>=max_voltages[channel-1] or user_reply is not 'y':
            continue_test = False

    output_file.close()
## execute the main
if __name__ == "__main__":
    sys.exit(main())
    