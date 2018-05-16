#!/usr/bin/env python
# -*- coding: iso8859-15 -*-
# tkex03.py - versione 1.0
import base64 
from Common import *
from myDBlib import *
  
import logging
import os

#  *** Command line argument facility
# the data structure
ComArgs = namedtuple('ComArgs', ['activity', 'result', 'component', 'parameter', 'value'])

#  Take the command line and returns a 'ComArgs' structure
def ManageCommandLineArguments(argv):
    
    helpMessage = " ITS-Upgrade Project - v.0.1 - A.Franco - INFN Bari\n ** Activity Parameter Write Utility **\n Usage: \n "
    helpMessage =  helpMessage + "ActAttach.py -a <activity Id> -r <act. result> -c <component code> -p <parameter name> -v <parameter value> -h\n"
    helpMessage =  helpMessage + "\t -a | --activity <activity Name> := the of the activity type\n"
    helpMessage =  helpMessage + "\t -r | --result <act. result> := the activity result [OK, NOK, FINE, ...]\n"
    helpMessage =  helpMessage + "\t -c | --component <component code> := the componentId of the componet linked to the activity\n"
    helpMessage =  helpMessage + "\t -p | --parameter <parameter name> := the name of the parameter to write\n"
    helpMessage =  helpMessage + "\t -v | --value <parameter value> := the value of the parameter as float number\n"
    helpMessage =  helpMessage + "\t -h := shows this help message\n"

    act =""
    com =""
    res =""
    par =""
    val =0.0

    try:
        opts, args = getopt.getopt(argv,"ha:r:c:p:v:",["activity=","result=","component=","parameter=","value="])
    except getopt.GetoptError:
        print helpMessage
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print helpMessage
            sys.exit()
        elif opt in ("-a", "--activity"):
            act = arg
        elif opt in ("-r", "--result"):
            res = arg
        elif opt in ("-c", "--component"):
            com = arg
        elif opt in ("-p", "--parameter"):
            par = arg
        elif opt in ("-v", "--value"):
            try:
                val = float(arg)
            except:
                val = 0.0
                
    par = ComArgs(act,res, com, par,val)
    return(par)

# Validate input parameters  
def ValidateInputParameters(parList):
    
    if parList.activity == "":
        print "The Activity Name is mandatory. Abort operation !"
        sys.exit()
    if parList.component == "":
        print "The ComponentID is manadatory. Abort operation !"
        sys.exit()
    if parList.result == "":
        print "The Activity Result is manadatory. Abort operation !"
        sys.exit()
    if parList.parameter == "":
        print "The Parameter Name is manadatory. Abort operation !"
        sys.exit()
                 
    return(parList)
   
# ****

#  *********************************************
# 
#     Example to Write Parameter Activity to the ITS DATA BASE
#
#  *********************************************


# Main Program
def main(argv):

    # read the Configuration
    myConf = Configuration("DBConfig.cfg")

    #logging setup    
    lg = setUpTheLogger(myConf)
    lg.info("Start the program Act Parameter Write !")

    #read the command line
    inpPar = ManageCommandLineArguments(argv)
    inpPar = ValidateInputParameters(inpPar)

    theActTag = inpPar.activity + " " + inpPar.component  # this is an extra info to create the activity : the name of an activity contains the ComponentId field
    
    # Read from the Configuration file 
    DBUser = myConf.GetItem("DBUSER")           # The User name used to open the DB connection 
    DBProject = myConf.GetItem('DBNAME')        # The DB Name used to open the DB connection 
    DBLocation = myConf.GetItem('LOCATION')     # The Location that will be used for create activities

    # connect to the DB
    itsDB = DB(project=DBProject,userName=DBUser,location=DBLocation)
    if itsDB is None:
        lg.error("DB not open !")
        return 1

    # Now we create the activity
    #
    #   compId     :=  A string that contains the Component ID
    #   activity   :=  The name of the Activity Type
    #   actTag     :=  The distinguish name of the Activity
    #   res        :=  The string containing the result
    #   close      :=  The state afer the creation True := close the activity
    #
    #   Return     :=  An Activity Result object
    actResult = itsDB.CreateCompActivity(inpPar.component, inpPar.activity, 
                                  theActTag, inpPar.result, False )
    if actResult.ErrorCode != 0:
        lg.error("Error create the activity %s -> %s!" % (inpPar.activity,actResult.ErrorMessage))
        return 1
    else:
        lg.info(" Activity %s open." % inpPar.activity)
    actID = actResult.ID

    # read the Parameter in order to decide if is a write or a change 
    isGood, Value = itsDB.ParameterRead(ActivityId = actID,
                                    ParameterName=inpPar.parameter)
    if isGood:
        # This is a change
        actResult = itsDB.ParameterChange(ActivityId = actID,
                                    ParameterName=inpPar.parameter,
                                    ParameterValue=inpPar.value)
    else: 
        # this is a Creation
        actResult = itsDB.ParameterWrite(ActivityId = actID,
                                    ParameterName=inpPar.parameter,
                                    ParameterValue=inpPar.value)
    if not actResult:
        lg.error("Error writing parameter %s!" % (inpPar.parameter))
        return 1
    else:
        lg.info(" Parameter %s = %f written." % (inpPar.parameter, inpPar.value))

    # We assign the members to the activity
    #    if we need different Members for this activity use the : itsDB.SetUpMembersForActivity(MemberNames = [])
    #
    # Example 
    #   self.membersForActivity = ['MANZARI VITO","CAMERINI PAOLO"]
    #   self.BobfpcDB.SetUpMembersForActivity(self.membersForActivity)
    #
    #  Default: automatically created in the init of the DB class with the DBUser name
    #  
    actResult = itsDB.ActivityMemberAssign(actID)
    if actResult.ErrorCode != 0:
        lg.error("Error assign member %s -> %s!" % (itsDB.selectedMembers,actResult.ErrorMessage))
        return 1
        
    # Finally close the activity
    actResult = itsDB.CloseActivity(actID, inpPar.result)
    if actResult.ErrorCode != 0:
        lg.error("Error closing the activity %s -> %s!" % (inpPar.file,actResult.ErrorMessage))
        return 1
    
    lg.info("End the program Act Parameter Write !")
    return 0

# --------------------------------

if __name__ == "__main__":
   main(sys.argv[1:])
