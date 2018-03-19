#!/usr/bin/env python

#
# WARNING: RTS/CTS should be used, as otherwise the PSU might not process
#          every command
# 

import base64
from Common import *
from myDBlib import *

import logging
import os

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

def ramp(sour, v, nsteps):
    for step in range(0, nsteps+1):
        sour.write("INST OUT1\n")
        sour.write("SOUR:VOLT %f\n" % (float(v[0])*step/nsteps))
        sour.write("INST OUT2\n")
        sour.write("SOUR:VOLT %f\n" % (float(v[1])*step/nsteps))
        sour.write("INST OUT3\n")
        sour.write("SOUR:VOLT %f\n" % (float(v[2])*step/nsteps))
        time.sleep(0.1);

def rampFromTo(sour, v, nsteps):
    for step in range(0, nsteps+1):
        sour.write("INST OUT3\n")
        sour.write("SOUR:VOLT %f\n" %
                   (float(v[0]+(float(v[1]-float(v[0]))*step/nsteps))))
        time.sleep(0.1);


def powerDown(sour):
    sour.write("INST OUT1\n")
    sour.write("OUTP OFF\n")
    sour.write("INST OUT2\n")
    sour.write("OUTP OFF\n")
    sour.write("INST OUT3\n")
    sour.write("OUTP OFF\n")

def measureCurr(sour):
    val=([0.0, 0.0, 0.0])
    for c in range(3):
        sour.write("INST OUT%d\n" % (c+1))
        sour.write("MEAS:CURR?\n")
        val[c] = float(sour.readline())
    print "%0.4fA\t%0.4fA\t%0.4fA" % ( val[0], val[1], val[2]) 
    
def doIVcurve(sour, channel, max_volt, nsteps, output_file, test_ok, resistances):
        
    if channel==0:
        channel_name = "DVDD"
    elif channel==1:
        channel_name = "AVDD"
    elif channel==2:
        channel_name = "BIAS"
    print("Scanning %s" % channel_name)
    output_file.write("V,I(%s),R(%s)\n" % (channel_name, channel_name))

    resistance = 1000000
    resistance_average = 0

    for step in range(0, nsteps):
        voltage = (max_volt/nsteps)*(step+1)
        sour.write("INST OUT%d\n" % (channel+1))
        sour.write("SOUR:VOLT %f\n" % voltage)
        time.sleep(2)
        #sour.write("FUSE:TRIP?\n")
        #if (int(sour.readline())==0):
        if not trip(sour):
            sour.write("INST OUT%d\n" % (channel+1))
            sour.write("MEAS:CURR?\n")
            current = float(sour.readline())
            if current>0:
                resistance = voltage/current
                resistance_average+=resistance
            print("V=%f, I=%f, R=%f" % (voltage, current, resistance))
            output_file.write("%f,%f,%f\n" % (voltage, current, resistance));
        else:
            #print("Channel #%d tripped. Scan is stopped" % (channel+1))
            init4030(sour,(0.1,0.1,0.01))
            break;

    resistance_average/=nsteps
    print("Impedance of %s is %f" %(channel_name, resistance_average)) 
    resistances[channel] = resistance_average
    if resistance_average>100:
        print("Test OK")  
        test_ok = test_ok*1
    else:
        print("Test failed") 
        test_ok = test_ok*0
    sour.write("SOUR:VOLT 0.0\n")

def init4030(hameg, i_max):
    hameg.write("*IDN?\n")
    idn = hameg.readline()
    if not ("HAMEG") in idn:
        sys.stderr.write("WRONG DEVICE: %s" % idn)
        return
    #print idn
    print "maximum current: %f %f %f" % i_max
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
    
def saveToDB(HIC_name, test_ok, resistances):
    # read the Configuration
    myConf = Configuration("DBConfig.cfg")

    #logging setup    
    lg = setUpTheLogger(myConf)
    lg.info("Start to save results in DB")

    #read the command line
    #inpPar = ManageCommandLineArguments(argv)
    #inpPar = ValidateInputParameters(inpPar)

    #theActTag = inpPar.activity + " " + inpPar.component  # this is an extra info to create the activity : the name of an activity contains the ComponentId field
    
    # Read from the Configuration file 
    DBUser = myConf.GetItem("DBUSER")           # The User name used to open the DB connection 
    DBProject = myConf.GetItem('DBNAME')        # The DB Name used to open the DB connection 
    DBLocation = myConf.GetItem('LOCATION')     # The Location that will be used for create activities

    # connect to the DB
    itsDB = DB(project=DBProject,userName=DBUser,location=DBLocation)
    if itsDB is None:
        lg.error("DB not open !")  
    else:
        lg.info("DB is open")    
       
    activityResult = ''
                         
    if test_ok:
        activityResult='OK'
    else:
        activityResult='NOK'
    
    actResult = itsDB.CreateCompActivity(HIC_name, "OB-HIC Impedance Test", 
                                  "Impedance test of ", activityResult, False )
    if actResult.ErrorCode != 0:
        lg.error("Error create the activity %s -> %s!" % ("OB-HIC Impedance Test",actResult.ErrorMessage))
        return 1
    else:
        lg.info(" Activity %s open." % "OB-HIC Impedance Test")
    actID = actResult.ID   
    
    parameter_name = ''
    
    for channel in range(3):    
       if channel==0:
        parameter_name = 'OB-HIC DVDD impedance'
       elif channel==1:
        parameter_name = 'OB-HIC AVDD impedance'
       elif channel==2:
        parameter_name = 'OB-HIC BIAS impedance'   
        
       actResult = itsDB.ParameterOverwrite(actID, parameter_name, resistances[channel])
    
       if not actResult:
           lg.error("Error writing parameter %s!" % (parameter_name))
           return 1
       else:
           lg.info(" Parameter %s = %f written." % (parameter_name, resistances[channel]))    
               
    actResult = itsDB.ActivityMemberAssign(actID)
    if actResult.ErrorCode != 0:
        lg.error("Error assigning member %s: %s!" % (itsDB.selectedMembers,actResult.ErrorMessage))
        return 1
        
    # Finally close the activity
    actResult = itsDB.CloseActivity(actID, activityResult)
    if actResult.ErrorCode != 0:
        lg.error("Error closing the activity: %s!" % (actResult.ErrorMessage))
        return 1
    
    lg.info("Finished to save results in DB")
    return 0

def main():
    # set up serial port
    dev='/dev/ttyUSB0'
    current_limit = 0.1
    current_limit_bias = 0.01
    max_voltage = 0.2
    max_voltage_bias = 4.
    nsteps = 20 
    nsteps_bias = 50    
    
    resistances = ([0., 0., 0.])  
    test_ok = 1

    sour=serial.Serial(dev, 9600, rtscts=True);
    
    init4030(sour, (current_limit, current_limit, current_limit_bias))

    HIC_name = raw_input("Enter HIC name: ")
    HIC_name = 'OBHIC-' + HIC_name
    output_file_name = HIC_name + '.dat'
    output_file = open(output_file_name,'w')
    
    max_voltages = ([max_voltage, max_voltage, max_voltage_bias])
    nstepss = ([nsteps, nsteps, nsteps_bias])

    for channel in range(3):    
        doIVcurve(sour, channel, max_voltages[channel], nstepss[channel], output_file, test_ok, resistances) 
      
    saveToDB(HIC_name, test_ok, resistances)  
    
    output_file.close()
    
## execute the main
if __name__ == "__main__":
    sys.exit(main())
