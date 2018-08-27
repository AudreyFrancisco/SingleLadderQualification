#!/usr/bin/env python
# -*- coding: iso8859-15 -*-
# tkex03.py - versione 1.0
import base64 
import logging
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
import readline
import glob


# -    ------------------------ Configuration class
class Configuration:
    confFileName = ""
    
    def __init__(self, configurationFileName = ""):
        if configurationFileName == "":
            configurationFileName = os.path.dirname(os.path.realpath(__file__)) + "/EOStransfer.cfg"
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
        itExists = False
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
                itExists = True
        if not itExists:
            conf.append(name + "=" + value + "\n")
        f = open(self.confFileName, "w")
        conf = f.writelines(conf)
        f.close()
        return

# -    --------------------- End Configuration Class Definition

def buildTheDefaultBasePath(Test):
    theScriptFileName = os.path.dirname(os.path.realpath(__file__))
    ind = theScriptFileName.find("new-alpide-software")
    if ind > -1:
        theNewAlpidePath = theScriptFileName[:(ind+19)]
    else:
        return("")
        
    if Test == "IBEndurance":
        return("")
    elif Test == "IBQualification":
        return("")
    elif Test == "OBEndurance":
        return("")
    elif Test == "OBFastPower":
        return("")
    elif Test == "OBQualification":
        return(theNewAlpidePath+"/GUI/Data")
    elif Test == "OBImpedance":
        return(theNewAlpidePath+"/ImpedanceTest/Data")
    elif Test == "OBReception":
        return("")
    elif Test == "IBStave":
        return(theNewAlpidePath+"/IBStave/Data")
    elif Test == "OBHalfStaveOL":
        return(theNewAlpidePath+"/OBHalfStaveOL/Data")
    elif Test == "OBHalfStaveML":
        return(theNewAlpidePath+"/OBHalfStaveML/Data")
    elif Test == "OBStaveOL":
        return(theNewAlpidePath+"/OBStaveOL/Data")
    elif Test == "OBStaveML":
        return(theNewAlpidePath+"/OBStaveML/Data")
    elif Test == "StaveReceptionOL":
        return(theNewAlpidePath+"/StaveReceptionOL/Data")
    elif Test == "StaveReceptionML":
        return(theNewAlpidePath+"/StaveReceptionML/Data")
    return("")

def _listFolder(path):
    if path.startswith(os.path.sep):
        # absolute path
        basedir = os.path.dirname(path)
        contents = os.listdir(basedir)
        # add back the parent
        contents = [os.path.join(basedir, d) for d in contents]
    else:
        # relative path
        contents = os.listdir(os.curdir)
    return contents

def _pathCompleter(text,state):
    options = [x for x in _listFolder(text) if x.startswith(text)]
    return options[state]

def inputTheLocalPath(theMessage, theDefault = ""):
    
    readline.set_completer(_pathCompleter)
    if sys.platform == 'darwin':
        # Apple uses libedit.
        readline.parse_and_bind("bind -e")
        readline.parse_and_bind("bind '\t' rl_complete")
    else:
        # Some tweaks for linux
        readline.parse_and_bind('tab: complete')
        readline.set_completer_delims(' \t\n`~!@#$%^&*()-=+[{]}\\|;:\'",<>?')
    
    isGood = False
    while not isGood:
        local = raw_input(theMessage + " [%s]:" % theDefault)
        if local == "":
            local = theDefault
        if not os.path.isdir(local):
            print "Local path %s not exists. !" % local
            isGood = False
        else:
            isGood = True
    
    if local[:-1] == "/":
        local = local[:-1]
                
    return(local)
  
  
def inputStringField(theDefault = ""):

    local = raw_input(theMessage + " [%s]:" % theDefault)
    if local == "":
        local = theDefault
    return(local)

def inputNumericField(theMessage, theDefault = 0):
    isGood = False
    while not isGood:
        local = raw_input(theMessage +" [%d]:" % theDefault)
        if local == "":
            local = str(theDefault)
        try:
            val = int(local) 
            isGood = True
        except:
            isGood = False
    return(val)
        

def getMenuChoice(menuName, itemList, theExit):
    isGood = False
    print "** %s  menu list   **" % menuName
    index = 0
    for i in itemList:
        index = index + 1
        print "\t%d) \t %s " % (index, i)
    print "\t0) \t %s \n" % theExit
    while not isGood:
        choice = raw_input(" \t Select the choice :")
        try:
            ind = int(choice)
            if ind >= 0 or ind <= index:
                isGood = True
        except ValueError:
            print "Invalid input !\n"
    return( ind )
    
def createTheSymLink(theLocalBasePath, theRemoteBasePath, theFolder):
    
    theLocalPath = theLocalBasePath 
    if not os.path.isdir(theLocalPath):
        print "ERROR : Path %s not exists. Abort operation !\n" % theLocalPath
        return(False)
        
    bashCommand = "mkdir -p '" + theRemoteBasePath + "'"
    try:
        return_code = subprocess.check_output(bashCommand, stderr=subprocess.STDOUT, shell=True)  
        # print ">>-> Folder creation of %s result: %s " % (theRemoteBasePath, return_code)
    except:
        print "ERROR : to create Destination dir: %s Abort operation !\n" % bashCommand
        return(False)
        
    if not os.path.isdir(theRemoteBasePath):
        print "ERROR : Remote mirror path %s not exists. Abort operation !\n" % theRemoteBasePath
        return(False)
     
    bashCommand = "ln -sfn '" + theLocalPath + "' '" + theRemoteBasePath + "/" + theFolder +"'"
    try:
        return_code = subprocess.check_output(bashCommand, stderr=subprocess.STDOUT, shell=True)  
        #print ">>-> Symlink creation of %s result: %s\n" % (theRemoteBasePath + "/" + theFolder, return_code)
    except:
        print "ERROR : to create Remote mirror symlink: %s Abort operation!\n" % bashCommand
        return(False)
  
    return(True)



def getTheSiteInfo():
    
    bashCommand = "cat /tmp/krblist" 
    try:
        return_code = subprocess.check_output(bashCommand, stderr=subprocess.STDOUT, shell=True)  
    except OSError:
        print "ERROR : to verify the Kerberos ticket. Abort ! (%s) \n" % bashCommand
        return(False, "", "")
    
    ind = return_code.find("@CERN.CH")
    if ind < 0:
        print "ERROR : no Kerberos ticket. Abort ! \n"
        return(False, "", "")
    
    i = ind
    while i >= 0:
        ch = return_code[i:i+1]
        if ch == "\n":
            service = return_code[i+1:ind]
            j = 0
            for s in ServiceAccount:
                if s == service:
                    return(True, service, Sites[j] )
                j = j + 1
        i = i - 1
    return(False, "", "")



def generateTheEOStransferScript(ServiceAccount, LocalBasePath):

    theScriptFileName = os.path.dirname(os.path.realpath(__file__)) + "/EOStransfer.sh"
    if os.path.isfile(theScriptFileName):  # first verify the existence
        print "WARNING : File %s already exists. Overwrite !" % theScriptFileName
        try:
            os.remove( theScriptFileName +"_old")
        except:
            # skip
            i = 0
        os.rename( theScriptFileName, theScriptFileName +"_old")
        
    try:
        file = open(theScriptFileName, "w") 
    except:
        print "ERROR : to create the script file %s. Abort !" % theScriptFileName
        retrun(False)
        
    
    file.write("#!/bin/bash\n")
    file.write("# --- Get all the variables ---\n")
    file.write("SCRIPTSPATH=\"$( cd \"$( dirname \"${BASH_SOURCE[0]}\" )\" && pwd )\" \n")
    file.write(". $SCRIPTSPATH/EOStransfer.cfg\n")
    file.write("DBATTACHBASEPATH="+LocalBasePath+"\n")
    file.write("HINIBITFILE=$DBATTACHBASEPATH/StopTransfer\n")
    file.write("STARTDATE=`date`\n")
    file.write("# The remote EOS Path, with the service account name specification\n")
    file.write("DBATTACHREMOTEPATH="+ServiceAccount+"@lxplus.cern.ch:/eos/project/a/alice-its/www\n")

    file.write("echo \" ------ ALICE-ITS EOS Repo sync program - v.3.0 - A.Franco - INFN BARI Italy\" \n")
    file.write("echo \"Start execution : $STARTDATE\" \n")
    file.write("echo \"Local path = $DBATTACHBASEPATH\" \n")
    file.write("echo \"Remote path = $DBATTACHREMOTEPATH\" \n")
    file.write("echo \"Number of attempts = $SYNCATTEMPTS\" \n")
    file.write("\n")
    file.write("# --- tests if the transfer is hinibit ---\n")
    file.write("if [ -e $HINIBITFILE ]\n")
    file.write("then\n")
    file.write("     echo \"$STARTDATE : The EOS file transfer is hinibit !\"\n")
    file.write("     exit 0\n")
    file.write("fi\n")
    file.write(" \n")
    
    file.write("# --- Log Rotate ---\n")
    file.write("FILE_SIZE=`du -b $LOGFILENAME | tr -s '\t' ' ' | cut -d' ' -f1`\n")
    file.write("if [ $FILE_SIZE -gt 10240000 ];then\n")
    file.write("  rm ${LOGFILENAME}_bak\n")
    file.write("  mv $LOGFILENAME ${LOGFILENAME}_bak\n")
    file.write("  touch $LOGFILENAME\n")
    file.write("  echo \"Execute Log Rotate !\" \n")
    file.write("fi\n")
    
    file.write("# --- create the inclusion list ---\n")
    file.write("ENABLEFILENAME=DBParameters.dat\n")
    file.write("INCLUDEFILE=/tmp/includedir.txt\n")
    file.write("CLEANPATH=$(echo $DBATTACHBASEPATH | sed 's/\//\\\\\//g')\n")
    file.write("find -L $DBATTACHBASEPATH -name $ENABLEFILENAME >$INCLUDEFILE\n")
    file.write("sed -i -e \"s/${CLEANPATH}//g\"  $INCLUDEFILE\n")
    file.write("sed -i -e \"s/\/${ENABLEFILENAME}//g\" $INCLUDEFILE\n")
    file.write(" \n")
    
    file.write("# --- performs the rsync, loop for more attempts ---\n")
    file.write("while [ $SYNCATTEMPTS -ne 0 ]; do\n")
    file.write("     # - do the sync\n")
    file.write("     STOPDATE=`date`\n")
    file.write("     rsync --files-from=$INCLUDEFILE -Lravuze ssh $DBATTACHBASEPATH $DBATTACHREMOTEPATH\n")
    file.write("     # - evaluates the result\n")
    file.write("     if [[ $? -gt 0 ]] \n")
    file.write("     then\n")
    file.write("            echo \"$STOPDATE : Error to sync the remote repository $DBATTACHREMOTEPATH\" >> $LOGFILENAME\n")
    file.write("            echo \"$STOPDATE : Error to sync the remote repository $DBATTACHREMOTEPATH\" \n")
    file.write("            let \"SYNCATTEMPTS=$SYNCATTEMPTS-1\" \n")
    file.write("            sleep 10\n")
    file.write("     else\n")
    file.write("           echo \"Remote repository sync done. ($STARTDATE -> $STOPDATE)\" >> $LOGFILENAME\n")
    file.write("           echo \"Remote repository sync done at $STOPDATE !\"\n")
    file.write("           echo \" --------- \"\n")
    file.write("           exit 0\n") 
    file.write("     fi\n")
    file.write("done\n")
    file.write("STOPDATE=`date`\n")
    file.write("echo \"$STOPDATE : Exit for error. Abort !\"\n")
    file.write("echo \" --------- \"\n")
    file.write("exit 1\n")

    file.close()
    os.chmod(theScriptFileName, 0777)
    
    print ">>-> Script file %s was created !" % theScriptFileName
    return(True)

def generateTheEOSAllSyncScript(ServiceAccount, LocalBasePath):

    theScriptFileName = os.path.dirname(os.path.realpath(__file__)) + "/syncAll.sh"
    if os.path.isfile(theScriptFileName):  # first verify the existence
        print "WARNING : File %s already exists. Overwrite !" % theScriptFileName
        try:
            os.remove( theScriptFileName +"_old")
        except:
            # skip
            i = 0
        os.rename( theScriptFileName, theScriptFileName +"_old")
        
    try:
        file = open(theScriptFileName, "w") 
    except:
        print "ERROR : to create the script file %s. Abort !" % theScriptFileName
        retrun(False)
        
    
    file.write("#!/bin/bash\n")
    file.write("# --- Get all the variables ---\n")
    file.write("SCRIPTSPATH=\"$( cd \"$( dirname \"${BASH_SOURCE[0]}\" )\" && pwd )\" \n")
    file.write(". $SCRIPTSPATH/EOStransfer.cfg\n")
    file.write("DBATTACHBASEPATH="+LocalBasePath+"\n")
    file.write("HINIBITFILE=$DBATTACHBASEPATH/StopTransfer\n")
    file.write("STARTDATE=`date`\n")
    file.write("# The remote EOS Path, with the service account name specification\n")
    file.write("DBATTACHREMOTEPATH="+ServiceAccount+"@lxplus.cern.ch:/eos/project/a/alice-its/\n")

    file.write("echo \" ------ ALICE-ITS EOS Repo sync program - v.2.0 - A.Franco - INFN BARI Italy\" \n")
    file.write("echo \"Start execution : $STARTDATE\" \n")
    file.write("echo \"Local path = $DBATTACHBASEPATH\" \n")
    file.write("echo \"Remote path = $DBATTACHREMOTEPATH\" \n")
    file.write("echo \"Number of attempts = $SYNCATTEMPTS\" \n")
    file.write("\n")
    
    file.write("# --- performs the rsync, loop for more attempts ---\n")
    file.write("while [ $SYNCATTEMPTS -ne 0 ]; do\n")
    file.write("     # - do the sync\n")
    file.write("     STOPDATE=`date`\n")
    file.write("     rsync -Lravuze ssh $DBATTACHBASEPATH $DBATTACHREMOTEPATH\n")
    file.write("     # - evaluates the result\n")
    file.write("     if [[ $? -gt 0 ]] \n")
    file.write("     then\n")
    file.write("            echo \"$STOPDATE : Error to sync the remote repository $DBATTACHREMOTEPATH\" >> $LOGFILENAME\n")
    file.write("            echo \"$STOPDATE : Error to sync the remote repository $DBATTACHREMOTEPATH\" \n")
    file.write("            let \"SYNCATTEMPTS=$SYNCATTEMPTS-1\" \n")
    file.write("            sleep 10\n")
    file.write("     else\n")
    file.write("           echo \"Remote repository sync done. ($STARTDATE -> $STOPDATE)\" >> $LOGFILENAME\n")
    file.write("           echo \"Remote repository sync done at $STOPDATE !\"\n")
    file.write("           echo \" --------- \"\n")
    file.write("           exit 0\n") 
    file.write("     fi\n")
    file.write("done\n")
    file.write("STOPDATE=`date`\n")
    file.write("echo \"$STOPDATE : Exit for error. Abort !\"\n")
    file.write("echo \" --------- \"\n")
    file.write("exit 1\n")

    file.close()
    os.chmod(theScriptFileName, 0777)
    
    print ">>-> Script file %s was created !" % theScriptFileName
    return(True)

def generateTheCronJobControlScript(ServiceAccount, LocalBasePath):

    theScriptFileName = os.path.dirname(os.path.realpath(__file__)) + "/autosyncOn.sh"
    if os.path.isfile(theScriptFileName):  # first verify the existence
        print "WARNING : File %s already exists. Overwrite !" % theScriptFileName
        try:
            os.remove( theScriptFileName +"_old")
        except:
            # skip
            i = 0
        os.rename( theScriptFileName, theScriptFileName +"_old")
        
    try:
        file = open(theScriptFileName, "w") 
    except:
        print "ERROR : to create the script file %s. Abort !" % theScriptFileName
        retrun(False)
        
    
    file.write("#!/bin/bash\n")
    file.write("# --- Get all the variables ---\n")
    file.write("SCRIPTSPATH=\"$( cd \"$( dirname \"${BASH_SOURCE[0]}\" )\" && pwd )\" \n")
    file.write(". $SCRIPTSPATH/EOStransfer.cfg\n")
    file.write("DBATTACHBASEPATH="+LocalBasePath+"\n")
    file.write("HINIBITFILE=$DBATTACHBASEPATH/StopTransfer\n")
    file.write("STARTDATE=`date`\n")
    file.write("echo \" ------ ALICE-ITS EOS Repo sync CronJob control - v.1.0 - A.Franco - INFN BARI Italy\" \n")
    file.write("\n")
    file.write("rm $HINIBITFILE\n")
    file.write("exit 0\n")
    file.write(" \n")
    file.close()
    os.chmod(theScriptFileName, 0777)
    
    print ">>-> Script file %s was created !" % theScriptFileName
    
    theScriptFileName = os.path.dirname(os.path.realpath(__file__)) + "/autosyncOff.sh"
    if os.path.isfile(theScriptFileName):  # first verify the existence
        print "WARNING : File %s already exists. Overwrite !" % theScriptFileName
        try:
            os.remove( theScriptFileName +"_old")
        except:
            # skip
            i = 0
        os.rename( theScriptFileName, theScriptFileName +"_old")
        
    try:
        file = open(theScriptFileName, "w") 
    except:
        print "ERROR : to create the script file %s. Abort !" % theScriptFileName
        retrun(False)
        
    
    file.write("#!/bin/bash\n")
    file.write("# --- Get all the variables ---\n")
    file.write("SCRIPTSPATH=\"$( cd \"$( dirname \"${BASH_SOURCE[0]}\" )\" && pwd )\" \n")
    file.write(". $SCRIPTSPATH/EOStransfer.cfg\n")
    file.write("DBATTACHBASEPATH="+LocalBasePath+"\n")
    file.write("HINIBITFILE=$DBATTACHBASEPATH/StopTransfer\n")
    file.write("STARTDATE=`date`\n")
    file.write("echo \" ------ ALICE-ITS EOS Repo sync CronJob control - v.1.0 - A.Franco - INFN BARI Italy\" \n")
    file.write("\n")
    file.write("touch $HINIBITFILE\n")
    file.write("exit 0\n")
    file.write(" \n")
    file.close()
    os.chmod(theScriptFileName, 0777)
    
    print ">>-> Script file %s was created !" % theScriptFileName

    return(True)

def generateTheCronJobLockFile(ServiceAccount, LocalBasePath):

    theFileName = LocalBasePath + "/StopTransfer"
    try:
        file = open(theFileName, "w") 
    except:
        print "ERROR : to create the  file %s. Abort !" % theFileName
        retrun(False)
    
    file.write("AF\n")
    file.close()
    print ">>-> The file %s was created !" % theFileName
    return(True)

# Main Program

# constant define
Sites = ["CERN", "Pusan",  "Bari", "Strasbourg", "Liverpool",  "Wuhan", 
             "Trieste", "Catania", "Torino", "Berkeley", "Daresbury", "Frascati", "Nikhef"]
ServiceAccount = ["aliceits", "itspusan",  "aliceitsbari", "aliceitssbg", "aliceitslpool",   "aliceitswuhan",
                      "aliceitstrieste", "aliceitscatania", "aliceitstorino","aliceitslbl", "aliceitsdl", "aliceitslnf", "itsnik"]
TestsName = ["HicTests", "fpc"] # , "hic"]
HicTestsName = ["IBEndurance", "IBQualification", "OBEndurance", "OBFastPower","OBQualification","OBReception", "OBImpedance","IBStave","OBHalfStaveOL","OBHalfStaveML","OBStaveOL","OBStaveML","StaveReceptionOL","StaveReceptionML"]

def main(argv):
    # read the Configuration
    myConf = Configuration("EOStransfer.cfg")
  
    # --- print the Header ...
    print " ******************************************************* "
    print " *  ALICE ITS : EOS transfer configuration program     * "
    print " *  ver. 1.0 - 15/03/2018    Auth : A.Franco INFN Bari * "
    print " *                                                     * "
    print " ******************************************************* "
    print " "
    
    isGood, Service, Site = getTheSiteInfo()
    if not isGood:
        print "Service account invalid. Abort !\n"
        return 1
    
#    cho = getMenuChoice("Select the Site Name:", Sites, "Quit")
#    if cho == 0:
#        print "Bye Bye !\n"
#        return 1
#    Site = Sites[cho -1]
#    Service = ServiceAccount[cho -1] 
    myConf.SetItem("SITENAME", Site)
    myConf.SetItem("SERVICEACCOUNT", Service)
    print ">>-> You are at %s site. The related service account is  '%s' \n" % (Site, Service)
    
#    theMirrorBasePath = myConf.GetItem("SYNCBASEPATH") 
#    local = inputTheLocalPath("Specify the base path of sync", theMirrorBasePath)
    local = "/var/aliceits"
    myConf.SetItem("SYNCBASEPATH", local)
    theMirrorBasePath = local + "/www"

    theAttempts = myConf.GetItem("SYNCATTEMPTS") 
    num = inputNumericField("The number of attempts to rsync :", int(theAttempts))
    myConf.SetItem("SYNCATTEMPTS", str(num))
    print "\n"
    
    cho = getMenuChoice("Select the Activity :", TestsName, "Quit")
    if cho == 0:
        print "Bye Bye !\n"
        return 1
    Activity = TestsName[cho -1]
    print ">>-> You select %s activity\n" % (Activity)

    isGood = False
    
    if Activity == "fpc":
        if Site == "Trieste" or Site == "Catania":
            isGood = True
            LocalBasePath = inputTheLocalPath("Specify the position of the sources path")
            RemoteBasePath = theMirrorBasePath + "/fpc"
            theFolder = "TRIESTE"
            if not createTheSymLink(LocalBasePath, RemoteBasePath, theFolder):
                print "Bye Bye !\n"
                return 1
            
    elif Activity == "HicTests":
        cho = -1
        while cho != 0:
            cho = getMenuChoice("Select one or more Hic Test :", HicTestsName, "Done")
            if cho > 0:
                Test = HicTestsName[cho -1]
                print ">>-> You select %s test \n" % (Test)
                LocalBasePath = inputTheLocalPath("Specify the position of the sources path", buildTheDefaultBasePath(Test))
                isGood = True
                RemoteBasePath = theMirrorBasePath + "/HicTests/" + Test
                theFolder = Site
                if not createTheSymLink(LocalBasePath, RemoteBasePath, theFolder):
                    print "Bye Bye !\n"
                    return 1
                
    elif Activity == "hic":
        isGood = False
        
    else:
        print "Bye Bye !/n"
        return 1
 
    #  ------ 
    if isGood:
        generateTheEOStransferScript(Service, theMirrorBasePath)
        generateTheCronJobControlScript(Service, theMirrorBasePath)
        generateTheEOSAllSyncScript(Service, theMirrorBasePath)
        generateTheCronJobLockFile(Service, theMirrorBasePath)
        
    print "EOS configuration program. Done !\n"

    return 0

# --------------------------------

if __name__ == "__main__":
   main(sys.argv[1:])
