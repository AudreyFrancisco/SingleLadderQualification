#!/usr/bin/env python
import os
import sys
import subprocess
import code
import logging
from logging.handlers import RotatingFileHandler
import time
from datetime import datetime
import errno
import getopt
from collections import namedtuple

#set up the fonts
fontN = "Helvetica", 12, "normal"
fontB = "Helvetica", 12, "bold"
fontT = "Helvetica", 14, "bold"
fontS = "Helvetica", 10, "normal"

#set up the button dimensions
butW = 8
butPadX = "2m"  ### (2)
butPadY = "1m"  ### (2)

#set up buttoPack in frame
butPakPadX = "3m"       ### (3)
butPakPadY = "1m"       ### (3)
butPakIpadX = "3m"   ### (3)
butPakIpadY = "1m"   ### (3)    
 
#set up colors
colBakWin = "gray"
colForWin = "black"
colBakBut = "gray"
colForBut = "black"
colBakLab = "gray"
colForLab = "black"

VERSIONTAG = "0.0"

TIMEMASK = "%d/%m/%y %H:%M:%S "

LOGGERNAME = "ImpedanceTest"
# LOGFILENAME = os.getcwd()+"/../log/FPCBench.log"
# LOGLEVEL = logging.DEBUG

CONFIGFILENAME = os.path.dirname(os.path.realpath(__file__)) +"/DBConfig.cfg"

#  ***  TO CHANGE IN ORDER TO SWITCH FROM TEST TO PRODUCTION DB ***

WSDLDBAPIURL = 'https://test-alucmsapi.web.cern.ch/AlucmswebAPI.asmx?WSDL'
# WSDLDBAPIURL = 'https://alucmsapi.web.cern.ch/AlucmswebAPI.asmx?WSDL'

DEFAULTPROJECT = "ITS_TEST"
# DEFAULTPROJECT = "ITS"

# ***

#convert a string-date (dd:mm:yyyy) into an integer  (yyyymmdd)
def dateToInt(date):
    try:
        i = int(date[6:] * 10000 + date[3:5] * 100 + date[0:2])
    except:
        i = 0
    return(i)

def getTimeStamp():
    return( datetime.now().strftime(TIMEMASK) )

def padNumber(sTheNumber, digit = 3):
    try:
        n = int(sTheNumber)
    except:
        n = 0
    pad = 10 ** digit + n
    return(str(pad)[-digit:])


def testTheKerberosTicket():
    return True if subprocess.call(['klist', '-s']) == 0 else False

def testTheCronJobInstallation():
    try:
        return_code = subprocess.check_output("crontab -l", stderr=subprocess.STDOUT, shell=True)  
    except:
        return(False)

    if return_code.find("fpcEOS") > 0:
        return(True)
    else:
        return(False)

def setUpTheLogger(Config):    
    lg = logging.getLogger(LOGGERNAME)
    fil = Config.GetItem("LOGFILENAME")
    lev = int(Config.GetItem("LOGLEVEL"))
    logFormatter = logging.Formatter("%(asctime)s %(filename)s(%(lineno)d): %(levelname)s %(message)s", datefmt="%d-%m-%Y %H:%M:%S")
    logFileHand = RotatingFileHandler(fil, mode='a', maxBytes=1.1e6, backupCount=2, encoding=None, delay=0)
    logFileHand.setLevel(lev)  #set verbosity to show all messages of severity >= DEBUG
    logFileHand.setFormatter(logFormatter)
    lg.addHandler(logFileHand)  
    lg.setLevel(lev)
    return(lg)

def getTheLogger():
    lg = logging.getLogger(LOGGERNAME)
    return(lg)

class Error:
    ErrorCode = 0
    ErrorMessage = "OK"
    
    def __init__(self, code=0, message="OK"): 
        self.ErrorCode = code
        self.ErrorMessage = message
        
    def Set(self, code=0, message="OK"):
        self.ErrorCode = code
        self.ErrorMessage = message
        
    def Reset(self):
        self.ErrorCode = 0
        self.ErrorMessage = 'OK'

class ActResult:
    ErrorCode = 0
    ErrorMessage = "OK"
    ID = 0

    def __init__(self, code=0, message="OK", ID=-999): 
        self.ErrorCode = code
        self.ErrorMessage = message
        self.ID = ID
        
    def Set(self, code=0, message="OK", ID=-999):
        self.ErrorCode = code
        self.ErrorMessage = message
        self.ID = ID
       
    def Reset(self):
        self.ErrorCode = 0
        self.ErrorMessage = 'OK'
        self.ID = -999

def State2Color(state):
    if state == "OK":
        return("green")
    if state == "NOK":
        return("red")
    if state == "FINE":
        return("blue")
    if state == "Undefined":
        return("dark gray")
    if state == "-":
        return("gray")
    if state == "n.a.":
        return("light gray")
    return("yellow")


class Configuration:
    
    confFileName = ""
    
    def __init__(self, configurationFileName = ""):
        if configurationFileName == "":
            configurationFileName = CONFIGFILENAME
        else:
            configurationFileName = os.path.dirname(os.path.realpath(__file__)) + "/" + configurationFileName
        self.confFileName = configurationFileName
        # open the file
        try:
            f = open(configurationFileName, "r")
        except:
            print ("Error opening the config file %s. Abort !" % configurationFileName)
            sys.exit("Error access the Configuration File . Abort !")
            
        f.close()
 
    def catchParam(self, line):
        param = line.split("=")
        if len(param) < 2:
            return "", ""
        param[1] = param[1].replace("\"","")
        return str.strip(param[0]), str.strip(param[1])
         
        
    def __catchParam(self, line):
        param = line.split("=")
        if len(param) < 2:
            return "", ""
        param[1] = param[1].replace("\"","")
        return str.strip(param[0]), str.strip(param[1])
        
        
    def GetConfFile(self):
        f = open(self.confFileName, "r")
        conf = f.readlines()
        f.close()
        return conf
         
    def GetItem(self, name=""):
        if name == "":
            return("")
  
        f = open(self.confFileName, "r")
        conf = f.readlines()
        f.close()
        for lin in conf:
            na, va = self.__catchParam(lin)
            if na == name:
                return(va)
        return ""
        
    def SetItem(self, name ="", value = ""):
        if name == "":
            return 
        f = open(self.confFileName, "r")
        conf = f.readlines()
        f.close()
        
        for index, lin in enumerate(conf):
            na, va = self.__catchParam(lin)
            if na == name:
                va = value
                conf[index] = na + "=" + va + "\n"
        
        f = open(self.confFileName, "w")
        conf = f.writelines(conf)
        f.close()
        return

