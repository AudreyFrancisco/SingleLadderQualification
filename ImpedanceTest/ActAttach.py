#!/usr/bin/env python
# -*- coding: iso8859-15 -*-
# tkex03.py - versione 1.0
import base64 
from Common import *
from myDBlib import *
  
import logging
import os
#  *********************************************
# 
#     Example to Attach Document to the ITS DATA BASE
#
#  *********************************************

#  *** Command line argument facility
# the data structure
ComArgs = namedtuple('ComArgs', ['activity', 'component', 'result', 'file', 'prefix', 'subfolder', 'type'])

#  Take the command line and returns a 'ComArgs' structure
def ManageCommandLineArguments(argv):
    
    helpMessage = " ITS-Upgrade Project - v.0.1 - A.Franco - INFN Bari\n ** Activity Attachment Utility **\n Usage: \n "
    helpMessage =  helpMessage + "ActAttach.py -a <activity name> -r <act. result> -c <component code> -f <attch.filename> -p <file prefix> -s <subfolder> -h\n"
    helpMessage =  helpMessage + "\t -a | --activity <activity name> := the string that define the activity to attach the file\n"
    helpMessage =  helpMessage + "\t -r | --result <act. result> := the activity result [OK, NOK, FINE, ...]\n"
    helpMessage =  helpMessage + "\t -c | --component <component code> := the componentId of the componet linked to the activity\n"
    helpMessage =  helpMessage + "\t -f | --file <attch.filename> := the name and path of the local file to attach\n"
    helpMessage =  helpMessage + "\t -p | --prefix <file prefix> := a short string to prefix to the file name on the remote repository (EOS only, could be null)\n"
    helpMessage =  helpMessage + "\t -s | --subfolder <subfolder> := the remote path where the file will be stored (EOS only)\n"
    helpMessage =  helpMessage + "\t -t | --type <attach.type> := the type of the attached file [def. 'GeneralInformation']\n"
    helpMessage =  helpMessage + "\t -h := shows this help message\n"

    act =""
    com =""
    res =""
    fil =""
    pre ="Test"
    sub ="www/"
    typ ="GeneralInformation"

    try:
        opts, args = getopt.getopt(argv,"ha:r:c:f:p:s:t:",["activity=","result=","component=","file=","prefix=","subfolder=","type="])
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
        elif opt in ("-f", "--file"):
            fil = arg
        elif opt in ("-p", "--prefix"):
            pre = arg
        elif opt in ("-s", "--subfolder"):
            sub = arg
        elif opt in ("-y", "--type"):
            typ = arg
    par = ComArgs(act,com,res,fil,pre,sub,typ)
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
    if parList.file == "" or not os.path.isfile(parList[3]):
        print "The Local File NOT Exists. Abort operation !"
        sys.exit()
         
    return(parList)
   
# ****


# Main Program
def main(argv):

    # read the Configuration
    myConf = Configuration("DBConfig.cfg")

    #logging setup    
    lg = setUpTheLogger(myConf)
    lg.info("Start the program Act Attach !")

    #read the command line and take all parameters
    inpPar = ManageCommandLineArguments(argv)
    inpPar = ValidateInputParameters(inpPar)
    theActTag = inpPar.activity + " " + inpPar.component  # this is an extra info to create the activity : the name of an activity contains the ComponentId field
    
     
    # set up of DB
    itsDB = None
    
    # Read from the Configuration file 
    DBUser = myConf.GetItem("DBUSER")           # The User name used to open the DB connection 
    DBProject = myConf.GetItem('DBNAME')        # The DB Name used to open the DB connection 
    DBLocation = myConf.GetItem('LOCATION')     # The Location that will be used for create activities

    # connect to the DB
    itsDB = DB(project=DBProject,userName=DBUser,location=DBLocation)
    if itsDB is None:
        lg.error("DB not open !")
        return 1

    # Set up the attachment mechanism 
    # this is mandatory to attach files !
    DBATTLimit = int(myConf.GetItem("DBATTACHLIMIT"))      # The limit to perform EOS transfer or BLOB
    DBATTBasepath = myConf.GetItem("DBATTACHBASEPATH")     # The base path of the Local Repository (Es. /tmp/www/repo )
    DBATTCommand = myConf.GetItem("DBATTACHCOMMAND")       # The bash command used to copy files into local repo
    DBATTMkdir = myConf.GetItem("DBANEWDIRCOMMAND")        # The bash command used to create the subfolders into local repo
    DBATTUripath = myConf.GetItem("DBATTACHURIBASEPATH")   # The URI base path to prefix the DB links to attached files
    itsDB.SetUpAttachments(DBATTLimit,DBATTBasepath,DBATTUripath,DBATTCommand,DBATTMkdir)     

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

    # save the ID of the activity
    actID = actResult.ID   # The long that identify the created activity

    # prepare the list of files to attach (list of strings that are complete Posix file name)
    fileList = []
    fileList.append(inpPar.file)
    
    #  Now we Attach the files
    #
    #  files         := list of filenames to attach (with the complete path)
    #  ActivityId    := the ID af the Activity
    #  AttCatName    := string, the name of the attacment Category
    #  subFolder     := string, the Path in the Local Repository (starting from Base Path)
    #  prefix        := string, a prefix that will be add to the file name in the Local Repository
    #
    #  Return := an Activity Result Object
    # 
    actResult = itsDB.AttachDocumentsToActivity(fileList, 
                                                 actID, 
                                                 inpPar.type,
                                                 inpPar.subfolder, 
                                                 inpPar.prefix)
    if actResult.ErrorCode != 0:
        lg.error("Error attaching documents %s -> %s!" % (inpPar.file,actResult.ErrorMessage))
        return 1
    else:
        lg.info(" File %s attached." % inpPar.file)
    
    
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
    
    lg.info("End the program Act Attach !")
    return 0

# --------------------------------

if __name__ == "__main__":
   main(sys.argv[1:])
