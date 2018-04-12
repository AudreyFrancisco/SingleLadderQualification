#!/usr/bin/env python

# to produce the sso.txt cookie jar:
# http://linux.web.cern.ch/linux/docs/cernssocookie.shtml
# kinit aliceits
# cern-get-sso-cookie --krb -r -u https://test-alucmsapi.web.cern.ch/AlucmswebAPI.asmx -o /home/alice/sso.txt

from zeep import Client
from zeep.transports import Transport
import cookielib
import base64
from datetime import datetime
import sys
import os.path
import subprocess
import re
import logging
import ntpath
from pickle import FALSE
from collections import namedtuple
import copy

from Common import *
from pty import CHILD

#    Version 2.7 - 12/04/2018 - A.franco - INFN BARI ITALY


# ---------------------------------------------
#   CLASS TO ACCESS THE ITS DATABASE
#
class DB:

    # Members
    LOCATION='CERN'
    WSDL=WSDLDBAPIURL
    #SSO='/home/alice/sso.txt'
    #SSO='/home/palpidefs/probestation/sso.txt'
    SSO='/tmp/sso_fpc.txt'
    USERID  =-3
    PROJECT=DEFAULTPROJECT

    OPEN = 0
    CLOSED = 1
    
    DATEMASK = "%Y/%m/%d"
    
    # Constructor
    def __init__(self,\
                 project=PROJECT,\
                 userName=USERID,\
                 wsdl=WSDL,\
                 sso=SSO,\
                 location=LOCATION):
        
        # set up the logger
        self.lg = getTheLogger()

        # in order to realize the connection get the cookies ...
        try:
            subprocess.call(["cern-get-sso-cookie", "--krb", "-r", "-u", wsdl, "-o", sso])
        except:
            self.lg.critical("CERN SSO not installed. Abort !") 
            sys.exit("CERN SSO Error. Abort !")
        
        try:      
            jar=cookielib.MozillaCookieJar(sso)
            jar.load()
        except:
            self.lg.critical("Cookie Lib not installed. Abort !") 
            sys.exit("Cookie Lib Error. Abort !")
            
        transport=Transport(cache=False)
        transport.session.cookies.update(jar)
        
        # establish the connection 
        self.DB=Client(wsdl,transport=transport)
        if self.DB is None:
            self.lg.critical("DB connection not established. Abort !")
            sys.exit("DB Error. Abort !")
            
        # common to all tables access variables
        try:
            self.project=next(p for p in self.DB.service.ProjectRead() if p.Name==project)
        except:
            self.lg.critical("DB - Project NAME %s doesn't exists into DB. (set it in Presets. Abort !" % project)
            sys.exit("DB Project Name Error. Abort !")

        # first we get the complete Member List
        self.MemberList = self.DB.service.ProjectMemberRead(projectID=self.project.ID)
        # and Set the Member for the activities
        self.selectedMembers = [self.GetMemberByName(userName)]
        
        # now we associate to the userID the -22
        factory = self.DB.type_factory('ns0')
        self.user = factory.ProjectMember(ID=-22,FullName="Fake User", PersonID= -22)
                
        if self.selectedMembers[0] is None:
            self.lg.warning("Get the selected Member %s return an ERROR !" % userName)
        else:
            self.lg.debug("DB User selection User %d %s  Selected %d %s" % (self.user.ID, self.user.FullName, self.selectedMembers[0].ID, self.selectedMembers[0].FullName))
        
        # set the attachment manager
        self.AttachCategory = self.DB.service.AttachmentCategoryRead()
        self.myAttach = AttchMan()
        self.SetUpAttachments()
        
        #save this for further actions
        self.locationName = location
        self.locationID = -999

    # -- Setup the Attachments Manager
    # 
    #   Limit    :=    The minimal dimension for select the attachment mechanism: Blob or EOS
    #   Base     :=    The Local repository base Path where the files to attach will be stored
    #   Url      :=    The Base URL used to compose the URI link to the attached file (EOS)
    #   AttCom   :=    The Bash command used to move/copy the file into the Local Repository
    #   MkCom    :=    The Bash Command used to create subfolders into the Local Repository
    def SetUpAttachments(self, Limit = 10000,
                         Base = "/tmp/", 
                         Url = "https://cernbox.cern.ch/remote.php/webdav/%20%20project%20alice-its/",
                         AttCom = "cp '%1/%2' '%3'",
                         MkCom = "mkdir -p '%1'"):
        self.myAttach.SetUp( Limit, Base, Url, AttCom, MkCom )
        return
 
    # -- Read the Activity Type and stores info into the hinstance 
    # 
    #   ActivityId    :=    The ID of an activity
    #   Return     :=   True/False
    #
    def AquireActivityTypeByActId(self, activityId):
        # first get the activitiy
        activity = self.DB.service.ActivityReadOne(ID = activityId)
        if activity is None:
            return(False)
        return( self._AcquireActivityTypeByActivityTypeId(activity.ActivityType.ID) )

    # -- Read the Activity Type and stores info into the hinstance 
    # 
    #   ActivityId    :=    The ID of an activity Type
    #   Return     :=   True/False
    #
    def AquireActivityTypeByActTypId(self, activityTypeId):
        # first get the activitiy
        return( self._AcquireActivityTypeByActivityTypeId(activityTypeId) )
        
    # -- Read the Activity Type and stores info into the hinstance 
    # 
    #   activityName    :=    The Name of an activity Type
    #   Return     :=   True/False
    #
    def AquireActivityType(self, activityName):    
        # Get the Activity spec.
        ActiTypeSummary =next(t for t in self.DB.service.ActivityTypeRead(projectID=self.project.ID) if t.Name==activityName)
        return( self._AcquireActivityTypeByActivityTypeId(ActiTypeSummary.ID) )
  
  
    # -- Private Method : Read the Activity Type  
    # 
    #   activityTypeId    :=    The Activity Type ID
    #   Return     :=   True/False
    #
    def _AcquireActivityTypeByActivityTypeId(self, activityTypeId):
        self.ActivityType =self.DB.service.ActivityTypeReadAll(activityTypeID=activityTypeId)
        # Now that we have the activity type we can extract the location ID 
        # Get the Location ID
        try:
            location = next( loc for loc in self.ActivityType.Location.ActivityTypeLocation if loc.Name == self.locationName)
        except:
            self.lg.critical("DB - Location NAME doesn't exists into DB. (correct it in CONFIGURATION file. Abort !")
            return(False)

        self.locationID = location.ID

        # and the ID for the OPEN & CLOSED status
        self.stOpen = next(status for status in self.ActivityType.Status.ActivityStatus if status.Code=="OPEN")
        self.stClosed = next(status for status in self.ActivityType.Status.ActivityStatus if status.Code=="CLOSED")
        return(True)
        
    # --- Get the list of possible results for the actual stored activity type
    #
    #   Return     :=   A list of strings conteins the results name
    #
    def GetActResults(self):
        reslist = []
        #print self.ActivityType.Result
        if self.ActivityType.Result is None:
            return reslist
        else:
            for res in self.ActivityType.Result.ActivityTypeResultFull:
                reslist.append(res.Name)
            return(reslist)


    # --- Create one Component
    # 
    #   compId     :=  A string that contains the ComponentId
    #   Return     :=   True or False
    #
    def CreateComponent(self, compTypeID, compID, suppID, description, lotid, package, user ):

        compo = self.DB.service.ComponentCreate(
                                    componentTypeID    = compTypeID,
                                    componentID        = compID,
                                    supplierComponentID= suppID, 
                                    description        = description,
                                    lotID              = lotid,
                                    packageID          = package,
                                    userID             = user
                                    )

        if compo.ErrorCode != 0:
            self.lg.debug("Error to create Component: %s -> %s !" % (compID,compo.ErrorMessage) )
            return(False)

        self.lg.debug("Component: %s (%d) CREATED !" % (compID,compo.ID) )
        return(True)
 
    # --- Read one Component
    # 
    #   compId     :=  A string that contains the ComponentId
    #   Return     :=   A component Object
    #
    def ReadComponent(self, compId):
        compo=self.DB.service.ComponentReadOne(ID=-999,componentID=compId)
        if compo is None:
            self.lg.debug("Component Not found ! (%s)" % compId ) 
        return(compo)
        
        
    # --- Remove one Component
    # 
    # 
    #   compId     :=  A string that contains the ComponentId
    #   comptypeId     :=  the ID of the component type
    #   Return     :=   True/False
    #
    def RemoveComponent(self, compId, comptypeId = -999):
        # Read the component
        compo=self.DB.service.ComponentReadOne(ID=-999,componentID=compId)
        if compo is None:
            self.lg.debug("Component Not found ! (%s)" % compId ) 
            return(False)

        if comptypeId > 0:
            if compo.ComponentType.ID != comptypeId:
                self.lg.debug("Component found but type mismatch ! (%s)" % compId ) 
                return(False)

        compo=self.DB.service.ComponentRemove(ID = compo.ID)
        if compo.ErrorCode != 0:
            self.lg.debug("Error to remove Component: %s -> %s !" % (compId,compo.ErrorMessage) )
            return(False)

        self.lg.debug("Component: %s (%d) REMOVED !" % (compId,compo.ID) )
        return(True)

    # --- Get the list of components of a specified Type
    # 
    #   componentTypeCode     :=  A string that contains the Component Type Code (Name)
    #   Return     :=   A list of component Object
    #
    def ListComponent(self, componentTypeCode = ""):
        self.lg.debug("List component of type : %s ..." % (componentTypeCode) )
        
        # first verify that the component type exists
        compType = next(typ for typ in self.DB.service.ComponentTypeRead(projectID = self.project.ID) if typ.Code==componentTypeCode)
        if compType is None:
            self.lg.debug("Error no Component Type '%s' defined  !" % (componentTypeCode) )
            return(None)
        self.lg.debug("Find component type : %d  %s %s" % (compType.ID,compType.Code,compType.Name) )
        
        compoList = self.DB.service.ComponentRead(projectID = self.project.ID, componentTypeID = compType.ID)
        if compoList is None:
            self.lg.debug("Return NONE ! %d  %d" % (self.project.ID,compType.ID) )
            
        return(compoList)

    # --- Get the Component Composition (all the children)
    # 
    #   compId     :=  A string that contains the Component ID
    #   Return     :=   Three arrays: Component IDs, Pysical State, Functional State
    #
    def GetComponentComposition(self,compId):
        self.lg.debug("Read the children of the  component  %s" % (compId) )
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID = compId) 
        c = []
        p = []
        f = []
        
        if compo is None:
            c1 = []
            p1 = []
            f1 = []
            c.append(c1)
            p.append(p1)
            f.append(f1)
            return(c,p,f)
        comID = compo.ID
        children = self.DB.service.ComponentChildrenRead(ID = comID)
        if children is not None:
            for child in children:
                comcode = child.Component.ComponentID
                if child.Component.PhysicalStatus is None:
                    py = "n.a"
                else:
                    py = child.Component.PhysicalStatus.Name
                if child.Component.FunctionalStatus is None:
                    fu = "n.a"
                else:
                    fu = child.Component.FunctionalStatus.Name
                c1 = [comcode]
                p1 = [py]
                f1 = [fu]
                c.append(c1)
                p.append(p1)
                f.append(f1)
            # --- FIXME : TODO subchil = component.Composition
        return(c,p,f)

    # --- Get the Component Composition Parents (all the parents)
    # 
    #   compId     :=  A string that contains the Component ID
    #   Return     :=  a list  of Component objects
    #
    def GetParentComponents(self, compId):
        self.lg.debug("Read the parents of Component  %s" % (compId,) )
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID = compId)
        if compo is None:
            return( [] ) 
        
        compocomp = self.DB.service.ComponentParentRead(ID = compo.ID) 
        if compocomp is None:
            return( [] )
  
        return( compocomp )
    
    # --- Verify that the component is used in composition 
    # 
    #   compId     :=  A string that contains the Component ID
    #   parentComponentType     :=  the Type ID of the parent
    #   Return    :=  True /False
    #
    def IsComponentUsedInComposition(self, compId, parentComponentType):
        compocomp = self.GetParentComponents( compId )
        if len(compocomp) == 0 :
            return( False )
        for comp in compocomp:
            if( comp.Component is not None):
                if (comp.Component.ComponentType.ID == parentComponentType) :
                    return ( True )
        return( False )
    
    # --- Get Parents of a type in a composition
    # 
    #   compId     :=  A string that contains the Component ID
    #   parentComponentType     :=  the Type ID of the parent
    #   Return    :=  a list of Component objects
    #
    def GetParentOfAType(self, compId, parentComponentType):
        compocomp = self.GetParentComponents( compId )
        res = []
        if len(compocomp) == 0 :
            return( res )
        for comp in compocomp:
            if( comp.Component is not None):
                if (comp.Component.ComponentType.ID == parentComponentType) :
                    res.append( comp.Component )
        return( res )

    # --- Get the Component Activities for one component and one activity type
    # 
    #   compId     :=  A string that contains the Component ID
    #   activityType     :=  the Type ID of the activity
    #   Return    :=  a list of Activity
    #
    def GetComponentActivities(self, compID, activityType):
        actList = []
        self.lg.debug("Read the activity for Component  %s of type %d" % (compID, activityType) )
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID = compID) 
        if compo is None:
            self.lg.debug("Component %s not found !" % (compID) )
            return actList
        cID = compo.ID
        history = self.DB.service.ComponentActivityHistoryRead(ID = cID)
        if history is None:
            return actList
        for act in history:
            if act.ActivityType.ID == activityType:
                actList.append(act)          
        return actList

    # --- Get the Component Activities Attach list for one component and one activity type
    #     either the Files and Uris are read
    #
    #   compId     :=  A string that contains the Component ID
    #   activityType     :=  the Type ID of the activity
    #   Return    :=  a list of records := [AttachmentID, FileNAme/Path, Category/Description, Type , ActivityID]
    #
    def GetComponentActivityAttachments(self, compID, activityType):
        attachList =[]
        self.lg.debug("Read the activity for Component  %s of type %d" % (compID, activityType) )
        actList = self.GetComponentActivities(compID, activityType)
        if len(actList) == 0:
            self.lg.debug("Activity %d not found !" % (activityType) )
            return attachList
        for act in actList:
            compo = self.DB.service.ActivityReadOne(ID=act.ActivityID) 
            if compo is None:
                self.lg.debug("Activity %d not found !" % (act.ActivityID) )
                return attachList
            if compo.Attachments is not None:
                if compo.Attachments.ActivityAttachment is not None:
                    for att in compo.Attachments.ActivityAttachment :
                        attachList.append([att.ID,att.FileName, att.AttachmentCatagory.Category, "File", act.ActivityID])

            if compo.Uris is not None:
                if compo.Uris.ActivityUri is not None:
                    for att in compo.Uris.ActivityUri  :
                        attachList.append([att.ID,att.Path, att.Description, "URI", act.ActivityID])
        return attachList

    # -- Erase the component composition
    #
    #   compId     :=  A string that contains the Component ID
    #
    def EraseComponentComposition(self, compId):
        self.lg.debug("Remove the component composition  %s" % (compId) )
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID = compId) 
        if compo is None:  
            self.lg.debug("Component not found (%s)" % (compId) )
            return
        
        composition = self.DB.service.ComponentChildrenRead(compo.ID)
        if composition is None:  
            self.lg.debug("Component Composition not found (%s)" % (compId) )
            return
        
        for c in composition:
            resu = self.DB.service.ComponentCompositionRemove(c.ID)
            if resu.ErrorCode != 0:
                self.lg.warning("Component Composition Remove error = %s" % (resu.ErrorMessage) )
        self.lg.debug("Component composition for %s REMOVED !" % (compId) )
        return

    # -- Get all the Activities related to a Component
    #
    #   compId     :=  A string that contains the Component ID
    #
    #   Return    :=  a list of descriptors (strings) := "<start_date> <activity_name> <activity_result> <activity_state>"
    #  
    #    
    def GetComponentActivitHistory(self,compId):
        self.lg.debug("Read the activity history of component  %s" % (compId) )
        
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID = compId) 
        if compo is None:
            return([])
        cID = compo.ID
        history = self.DB.service.ComponentActivityHistoryRead(ID = cID)
        if history is None:
            return([])
        history.sort(key = lambda c: dateToInt(c.ActivityStartDate))
        ls = []
        for act in history:
            if act.ActivityResult is None:
                n = ""
            else:
                n = act.ActivityResult.Name
            if act.ActivityStatus is None:
                c = ""
            else:
                c = act.ActivityStatus.Code
            s = act.ActivityStartDate + " " + act.ActivityName + " " + n + " " + c
            ls.append(s)   
        self.lg.debug("Find activities : %d" % (len(ls)) )
        return(ls)

    # -- Get all the Activities related to a Component
    #
    #   compId     :=  A string that contains the Component ID
    #
    #   Return    :=  a list of descriptors (strings) := "<start_date> <activity_name> <activity_result> <activity_state>"
    #  
    #    
    def GetComponentActivityList(self,compId):
        self.lg.debug("Read the activity list of component  %s" % (compId) )
        
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID = compId) 
        if compo is None:
            return([])
        cID = compo.ID
        history = self.DB.service.ComponentActivityHistoryRead(ID = cID)
        if history is None:
            return([])
        history.sort(key = lambda c: dateToInt(c.ActivityStartDate))
        ls = []
        for act in history:
            ls.append(act)   
        self.lg.debug("Find activities : %d" % (len(ls)) )
        return(ls)



    # -- Get the (Physical, Functional) state of a component
    #
    #   compId     :=  A string that contains the Component ID
    #
    #   Return     :=  A couple (Physical, Functional)
    #                      ( '-','-' ) := Component not found
    #                      ( n.a. , n.a.) := No activity performed
    #    
    def GetComponentState(self,compId,  componentTypeCode = ""):
        self.lg.debug("Read the status of component  %s (%s)" % (compId, componentTypeCode) )
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID = compId) 
        if compo is None:
            return("-","-")
        if compo.PhysicalStatus is None:
            py = "n.a."
        else:
            py = compo.PhysicalStatus.Name
            
        if compo.FunctionalStatus is None:
            fu = "n.a."
        else:
            fu = compo.FunctionalStatus.Name
        self.lg.debug("Find component : %s  State %s %s" % (compId , py,fu) )
        return(py,fu)
    
    # *********
    #     Getters on the Component Object Structure
    #
    def CompGetPyState(self, comp):
        try:
            return(comp.PhysicalStatus.Name)
        except:
            return("-")
       
    def CompGetFuState(self, comp):
        try:
            return(comp.FunctionalStatus.Name)
        except:
            return("-")
        
    def CompGetID(self, comp):
        if(comp.ID is None):
            return(-999)
        else:
            return(comp.ID)
    
    def CompGetCompID(self, comp):
        if(comp.ComponentID is None):
            return("")
        else:
            return(comp.ComponentID)
    
    def CompGetSupplID(self, comp):
        if(comp.SupplierComponentID is None):
            return("")
        else:
            return(comp.SupplierComponentID)
    
    def CompGetDescription(self, comp):
        if(comp.Description is None):
            return("")
        else:
            return(comp.Description)
    
    def CompGetLotID(self, comp):
        if(comp.LotID is None):
            return("")
        else:
            return(comp.LotID)
    
    def CompGetPackID(self, comp):
        if(comp.PackageID is None):
            return("")
        else:
            return(comp.PackageID)


    # -- Get the ResultID of a specified Activit result name
    def _GetTheResultID(self, ResultName):
        if ResultName == "":
            return(-999)
        for result in self.ActivityType.Result.ActivityTypeResultFull:
            if result.Name==ResultName:
                return(result.ID)
        return(-999)


    # **************************************************
    #
    #  Create/OPEN an activity : Duplication optional
    #  
    #  seems that the re-open of an activity isn't avoided ! 
    #
    def _CreateOrOpenAnActivity(self, ComponentId, ActTypeId=-999, lotId=0, actName="Noname" , duplication= True):
        # verify if the Activity exists
        compActy = self.GetComponentActivities( ComponentId, ActTypeId)
        ts=datetime.now().strftime(self.DATEMASK)
        if( len(compActy) == 0 ): # ok create
            # Create the activity 
            te=datetime.now().strftime(self.DATEMASK)
            actCreRes = self._CreateAnActivity(DB.OPEN,ActTypeId,lotId,actName,ts,te)
            return(actCreRes, ts)
        for act in compActy: # search for an open activity
            if act.ActivityStatus is None:
                continue
            if act.ActivityStatus.Code == "OPEN":
                return(ActResult(0,"Open activity", act.ActivityID), act.ActivityStartDate)
        if duplication:
            te=datetime.now().strftime(self.DATEMASK)
            actCreRes = self._CreateAnActivity(DB.OPEN,ActTypeId,lotId,actName,ts,te)
            return(actCreRes, ts)
        else:
            self.lg.debug("Activity Duplication error : %s -> %s" % (actName,ComponentId))
            return(ActResult(0,"Duplication Activity ", compActy[0].ActivityID), ts) 

        return(ActResult(-1,"Strange ", -999), ts)


    # Create an activity
    def _CreateAnActivity(self, Status, ActTypeId=-999, lotId=0, name="Noname", staTim=0, endTim=0, resultId =-999 ):
 
        if((Status != self.OPEN and Status != self.CLOSED) or ActTypeId <= 0):
            self.lg.warning("Some parameters are wrong. Create Activity rejected")
            return(ActResult(-1,"Parameter Wrong", ActTypeId))

        #Create the activity 
        activityCreateResult = self.DB.service.ActivityCreate(
          activityTypeID = ActTypeId,
          locationID = self.locationID,
          lotID = lotId,
          activityName = name,
          startDate = staTim,
          endDate = endTim,
          position = 0,
          statusID = self.stOpen.ID if (Status == self.OPEN) else self.stClosed.ID ,
          resultID = resultId,
          userID = self.user.ID
        ) 
        if activityCreateResult.ErrorCode != 0:
            self.lg.warning("Activity creation error : %s -> %s" % (name,activityCreateResult.ErrorMessage))
            return(activityCreateResult) 

        self.lg.debug("Activity: %d of type %d created !" % (activityCreateResult.ID,ActTypeId) )
        return(activityCreateResult)

    # *********************
    #
    # Change an Activity: principally to close, (open doesn't work ?)
    #
    def _ChangeAnActivity(self, Status, activityID, ActTypeId=-999, lotId=0, name="Noname", staTim=0, endTim=0, resultId =-999 ):
 
        if((Status != self.OPEN and Status != self.CLOSED) or ActTypeId <= 0):
            self.lg.warning("Some parameters are wrong. Change Activity rejected")
            return(ActResult(-1,"Parameter Wrong", activityID))
 
        #Change the activity 
        activityCreateResult = self.DB.service.ActivityChange(
          ID=activityID,
          activityTypeID = ActTypeId,
          locationID = self.locationID,
          lotID = lotId,
          activityName = name,
          startDate = staTim,
          endDate = endTim,
          position = 0,
          statusID = self.stOpen.ID if (Status == self.OPEN) else self.stClosed.ID ,
          resultID = resultId,
          userID = self.user.ID
        ) 
        if activityCreateResult.ErrorCode != 0:
            self.lg.warning("Activity change error : %s -> %s" % (name,activityCreateResult.ErrorMessage))
            return(activityCreateResult) 
        
        self.lg.debug("Activity of type %d changed !" % (ActTypeId) )
        return(activityCreateResult)

    # -- Create one activity without Component Assignment
    #
    #   activity     :=  The name of the Activity Type
    #   actName     :=  The distinguish name of the Activity
    #   res     :=  The string containing the result
    #
    #   Return     :=  An Activity Result object
    def CreateActivity(self, compId, activity, actName, res="OK" ):
        
        # Get the Activity spec.
        if not self.AquireActivityType(activity):
            return(ActResult(9,"Activity not enabled for the specified location"))
       
        # prepare the common data 
        te=datetime.now().strftime(self.DATEMASK)
        resultID = self._GetTheResultID(res)
        actTypId = self.ActivityType.ID
        
        # Create the activity 
        actCreRes, ts = self._CreateOrOpenAnActivity(compId, actTypId, 0, actName)
        if actCreRes.ErrorCode != 0:
            self.lg.warning("Error creating the activity: %s. (%s)" % (actName, actCreRes.ErrorMessage))
            return actCreRes  
      
        self.lg.debug("Activity ID=%d created and manteined Open ! (%s)" % (actCreRes.ID, actName ))            
        return actCreRes

    # -- Create one activity
    #
    #   compId     :=  A string that contains the Component ID
    #   activity     :=  The name of the Activity Type
    #   actTag     :=  The distinguish name of the Activity
    #   res     :=  The string containing the result
    #   close     :=  The state afer the creation True ;= close the activity
    #
    #   Return     :=  An Activity Result object
    def CreateCompActivity(self, compId, activity, actTag, res="OK", close=False, withoutComponent=False ):
        # Read the component
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID=compId)
        if compo is None:
            if withoutComponent :
                return( self.CreateActivity(compId, activity, actTag, res) )
            else:
                self.lg.warning("Component Not found ! (%s)" % compId ) 
                return(ActResult(-1,"Component not found"))
        
        # Get the Activity spec.
        if not self.AquireActivityType(activity):
            return(ActResult(9,"Activity not enabled for the specified location"))
       
        # prepare the common data 
        te=datetime.now().strftime(self.DATEMASK)
        resultID = self._GetTheResultID(res)
        actTypId = self.ActivityType.ID
        actName = actTag + " " + compo.ComponentID
        
        # Create the activity 
        actCreRes, ts = self._CreateOrOpenAnActivity(compo.ComponentID, actTypId, 0, actName)
        if actCreRes.ErrorCode != 0:
            return actCreRes  
        
        InCompType, OutCompType = self._GetInOutComponentFromActivity(compo.ComponentType.ID)
        self._AssignComponentToActivity(compo.ID, actCreRes.ID, InCompType[0].ID )
        self._AssignComponentToActivity(compo.ID, actCreRes.ID, OutCompType[0].ID )

        if close :
            actCreRes = self._ChangeAnActivity(DB.CLOSED,actCreRes.ID,actTypId,0,actName,ts,te,resultID)
            self.lg.debug("Activity Created and Closed ! (%s)" % compId )
        else:
            self.lg.debug("Activity Created and manteined Open ! (%s)" % compId )            
            
        return actCreRes

    # -- Open one activity
    #
    #   *** DISABLED ***
    #
    #   activityID     :=  The ID of an Activity 
    #   Return     :=  An Activity Result object
    def OpenActivity(self,activityID):
        if self._OpenCloseActivity(activityID,-999, True):
            return ActResult(0,"OK !", activityID)
        else:
            return ActResult(1,"Error to open the activity!", activityID)
    
    # -- Close one activity
    #
    #   activityID     :=  The ID of an Activity 
    #   Result       :=  The result of Activity
    #   Return     :=  An Activity Result object
    #
    def CloseActivity(self,activityID, Result):
        resultID = self._GetTheResultID(Result)
        if self._OpenCloseActivity(activityID, resultID, False):
            return ActResult(0,"OK !", activityID)
        else:
            return ActResult(1,"Error to close the activity!", activityID)
 
    # -- Close all the activities of a specified Type and Component
    #
    #   ComponetID     :=  The ComponentID
    #   ActivityTypeID :=  The ID of an Activity Type
    #   Result       :=  The result of Activity
    #   Return     :=  An Activity Result object
    #
    def CloseCompActivity(self, ComponetID, ActivityTypeID, Result):
        # Get the Open activity
        resultID = self._GetTheResultID(Result)
        compActyList = self.GetComponentActivities( ComponetID, ActivityTypeID)
        if len(compActyList) == 0:
            return(ActResult(-1,"No Activities !"))
        for compActy in compActyList:
            if compActy.ActivityStatus.Code == "OPEN":
                actCreRes = self._ChangeAnActivity(DB.CLOSED,compActy.ActivityID, ActivityTypeID,0,
                                                   compActy.ActivityName,
                                                   compActy.ActivityStartDate,
                                                   compActy.ActivityEndDate,
                                                   resultID)
                self.lg.debug("Activity Closed ! (%s)" % compActy.ActivityName )
                return actCreRes
        return(ActResult(1,"No open activity found"))
    
    def _OpenCloseActivity(self, activityID, ResultID, isOpen = True):
        activity = self.DB.service.ActivityReadOne(ID=activityID)
        if activity is None or activity.ID == 0:
            self.lg.warning("Activity not found error : %d " % (activityID)) 
            return(False)
        else:
            if isOpen:
                sta = self.stOpen.ID
                staD = "OPEN"
            else:
                sta = self.stClosed.ID
                staD = "CLOSE"
            activityResult = self.DB.service.ActivityChange(
                        ID=activity.ID,
                        activityTypeID = activity.ActivityType.ID,
                        locationID = activity.ActivityLocation.ID,
                        lotID = activity.LotID,
                        activityName = activity.Name,
                        startDate = activity.StartDate,
                        endDate = activity.EndDate,
                        position = 0,
                        statusID = sta,
                        resultID = ResultID,
                        userID = -999
                        )
            if activityResult.ErrorCode != 0:
                self.lg.warning("Activity %s error : %s -> %s" % (staD, activity.Name,activityResult.ErrorMessage))
                return(False)
        return(True)    
        
    # -- Assign a Component to an Activity
    #
    #   compId     :=  A string that contains the Component key (Name)
    #   activityID     :=  The ID of the Activity 
    #
    #   Return     :=  An Activity Result object
    def AssignComponentToActivity(self,compId, activityID):
        # Read the component
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID=compId)
        if compo is None:
            self.lg.warning("Component Not found ! (%s)" % compId ) 
            return(ActResult(-1,"Component not found"))
    
        InCompType, OutCompType = self._GetInOutComponentFromActivity(compo.ComponentType.ID)
        self._AssignComponentToActivity(compo.ID, activityID, InCompType[0].ID )
        self._AssignComponentToActivity(compo.ID, activityID, OutCompType[0].ID )

        self.lg.debug("Component %s assigned to activity ID=%d !" % (compId,activityID) ) 
        return(ActResult(0,"Component assigned",activityID))

 
    # -- Assign a Component to an Activity
    #
    #   compID     :=  The ComponentID
    #   activityID     :=  The ID of the Activity 
    #   ActTypCompType   :=  The ID of the Activity Type Component Type
    #   Return     :=  Activity result object
    #
    def AssignComponentToActivityByType(self, compId, ActivityID, ActTypCompType=-999 ):
        # Read the component
        compo = self.DB.service.ComponentReadOne(ID=-999, componentID=compId)
        if compo is None:
            self.lg.warning("Component Not found ! (%s)" % compId ) 
            return(ActResult(-1,"Component not found"))
        
        activity = self.GetActivity(ActivityID)
        if activity is None:
            self.lg.warning("Activity Not found ! (%d)" % ActivityID ) 
            return(ActResult(-1,"Activity not found"))
        if self._AssignComponentToActivity(compo.ID, ActivityID, ActTypCompType):
            self.lg.debug("Component %s assigned to activity %d !" % (compId, ActivityID ) )
            return(ActResult(0,"OK")) 
        else:
            self.lg.warning("Failed to assign Componet  %s to Activity %d !" % (compId, ActivityID )  ) 
            return(ActResult(-1,"Failed to assign Componet to Activity !"))
        
    # ********************
    #
    # Assign a component to an Activity
    #
    def _AssignComponentToActivity(self, ComponentId=-999, ActivityId=-999, ActTypCompType=-999 ):
        # Verify if there are errors in the parameters
        if(ComponentId <= 0 or ActivityId <= 0 or ActTypCompType <= 0):
            self.lg.debug("Some parameters are wrong. Assign rejected")
            return(False)
        
        # Now Associate the component
        activityAssociateResult = self.DB.service.ActivityComponentAssign(
            componentID = ComponentId,
            activityID = ActivityId,
            actTypeCompTypeID = ActTypCompType,
            userID = self.user.ID
        )  
        if activityAssociateResult.ErrorCode != 0:
            self.lg.debug("Assign componet to Activity error : %s -> %s" % (ActivityId,activityAssociateResult.ErrorMessage))
            return(False)
  
        self.lg.debug("Assign done : Component:%d => Activity:%d " % (ComponentId,ActivityId) )
        return(True)

    # ******************
    #
    # Get the lists of Input, output components for an activity
    #
    def _GetInOutComponentFromActivity(self, ComponentTypeId=-999):
        InTy = []
        OutTy = []
        #print self.ActivityType.ActivityTypeComponentType
        # Verify if there are errors in the parameters
        if(ComponentTypeId <= 0):
            self.lg.debug("Some parameters are wrong. Extraction rejected")
        else:
            #Now build the In Out component list
            for Component in self.ActivityType.ActivityTypeComponentType.ActivityTypeComponentTypeFull:
                if Component.ComponentType.ID == ComponentTypeId:
                    if Component.Direction == "in":
                        InTy.append(Component)
                    elif Component.Direction == "out":
                        OutTy.append(Component)
            self.lg.debug("Get Component in/out type Activity:%d In=%d Out=%d" % (self.ActivityType.ID, len(InTy), len(OutTy)) )
        return InTy, OutTy


    # -- Attach a list of Documents to one Activity
    #
    #  files         := list of filenames to attach (with the complete path)
    #  ActivityId    := the ID af the Activity
    #  AttCatName    := string, the name of the attacment Category
    #  subFolder     := string, the Path in the Local Repository (starting from Base Path)
    #  prefix        := string, a prefix that will be add to the file name in the Local Repository
    #
    #  Return := an Activity Result Object
    #
    def AttachDocumentsToActivity(self, files = [], ActivityId = -999, AttCatName ="", subFolder = "various/", prefix = ""):
        filesStruct = []
        for li in files:
            ap = [0,-1, li, li, 'text', ActivityId]
            filesStruct.append(ap)
        res = self._AttachADocumentToActivity(filesStruct, ActivityId, AttCatName, subFolder, prefix)
        return res

    # -- Read one activity specified by its ID
    #
    #  ActivityId    := the ID af the Activity
    #
    #  Return := an Activity Result Object
    #
    def GetActivity(self, ActivityId = -999):
        activity = self.DB.service.ActivityReadOne(ID = ActivityId)
        if activity is None:
            self.lg.warning("Activity Not found ! (%d)" % ActivityId ) 
            return (None)
      
        return (activity)
        
        
    # *******************
    #
    # Attach a File to an Activity
    #
    #    the parameter files is a list of = ( action = { remove == -1 , insert >= 0 }, 
    #                                         attachId = {new == -1, already created > 0 },
    #                                         fileName/Uri,
    #                                         Description,
    #                                         type,
    #                                         activityID )
    
    #                                         subFolder ,
    #                                         activityprefix
    #                                        
    #                                            
    def _AttachADocumentToActivity(self, files = [], ActivityId = -999, AttCatName ="", subFolder = "various/", prefix = ""):
        # Verify if there are errors in the parameters
        if(ActivityId <= 0 or files == []):
            self.lg.debug("Some parameters are wrong. Attachment rejected")
            return(ActResult(1,"Wrong parameter"))
        elif AttCatName == "":
            AttCatName = "TestReport"
   
        # get the Category ID        
        attCat = next(categ for categ in self.AttachCategory if categ.Category == AttCatName)
       
        # read the document from the file and store into a variable
        for fil in files:
            fileName = fil[2]
            if fil[0] == -1 and fil[1] > 0:  #this is to remove...
                self.lg.debug("Remove the attach file = %s" % (fileName))
                actID = fil[5]
                
                activityAttachResult = self.DB.service.ActivityAttachmentRemove(
                        ID = fil[1]
                        )  
                if activityAttachResult.ErrorCode != 0:
                    self.lg.warning("Remove attach file to Activity error : %s -> %s" % (fileName,activityAttachResult.ErrorMessage))
                else:
                    self.lg.debug("File %s attach REMOVED from the Activity:%d" % (fileName, ActivityId) )
                #self.CloseActivity(actID);
                continue
            
            if fil[1] > 0:# this already exists
                self.lg.debug("Attach file already exists = %s" % (fileName))
                continue
            
            # this is good for insertion !
            if not os.path.isfile(fileName):  # first verify the existence
                self.lg.warning("File %s not exists. Ignored !" % fileName)
                continue
            else:
                size = os.path.getsize(fileName) 
                self.lg.debug("The attach file = %s has size of = %d (max=%d)" % (fileName,size,self.myAttach.GetLimit()))
                # then decide if use the BLOB or EOS
                if (size > self.myAttach.GetLimit()) :
                    self.lg.debug("Attach as URI !")
                    # if you want the TS prefix
                    #      prefix = prexif + getFileNameTimeStamp()
                    #
                    dest = self.myAttach.ExecuteTheTransfer(fileName, subFolder, prefix) 
                    if dest == "":
                        self.lg.warning("Error transferring the file to remote. Attachment rejected")
                        continue
                    else:
                        self.lg.info("File %s transferred " % (fileName) )
                    
                    activityAttachResult = self.DB.service.ActivityUriCreate(
                        activityID = ActivityId,
                        uriPath = dest,
                        uriDescription = self.myAttach.GetURIDescription(fileName),
                        userID = self.user.ID
                        )  
                else:
                    self.lg.debug("Attach as Blob !")
                    try:
                        f = open(fileName, "rb")
                        data = f.read()
                        data = data.encode("base64")
                        self.lg.debug("File %s read %d bytes in base64 code" % (fileName, len(data) ) )
                    except:
                        self.lg.warning("Error reading the file to attach. Attachment rejected")
                        continue
        
                    activityAttachResult = self.DB.service.ActivityAttachmentCreate(
                        activityID = ActivityId,
                        attachmentCategoryID = attCat.ID,
                        file = data,
                        fileName = ntpath.basename(fileName),
                        userID = self.user.ID
                        ) 
                    
                if activityAttachResult.ErrorCode != 0:
                    self.lg.warning("Attach file to Activity error : %s -> %s" % (fileName,activityAttachResult.ErrorMessage))
                else:
                    self.lg.debug("File %s attached to the Activity:%d" % (fileName, ActivityId) )
                continue
        return(ActResult(0,"OK", ActivityId))


    # **************** 
    # Attach a URI to an Activity
    #
    def _AttachAUriToActivity(self, uri="",  uriName = "", ActivityId = -999):
        # Verify if there are errors in the parameters
        if(ActivityId <= 0 or uri == ""):
            self.lg.debug("Some parameters are wrong. URI Attachment rejected")
            return(False)
        activityAttachURIResult = self.DB.service.ActivityUriCreate(
            activityID = ActivityId,
            uriPath = uri,
            uriDescription = uriName,
            userID = self.user.ID
        )  
        if activityAttachURIResult.ErrorCode != 0:
            self.lg.warning("Attach URI to Activity error : %s -> %s" % (uri,activityAttachURIResult.ErrorMessage))
            return(False) 
        self.lg.debug("URI %s attached to the Activity:%d" % (uri, ActivityId) )
        return(True)

    # *************************************
    #   Paramaeters Methods 
    #
   
    # -- Write or Change one Activity Parameter
    #
    #  ActivityId    := the ID af the Activity
    #  ParameterName := string, the name of the Parameter
    #  ParameterValue:= float, the value to write
    #
    #  Return := True / False
    #
    def ParameterOverwrite(self, ActivityId = -999, ParameterName="", ParameterValue=0.0):
        # Get the Activity spec.
        if not self.AquireActivityTypeByActId(ActivityId):
            self.lg.warning("Activity not enabled for the specified location : %d " % (ActivityId))
            return(False)
        # prepare the common data
        Flag = False 
        for ActTypeParam in self.ActivityType.Parameters.ActivityTypeParameter:
            if ActTypeParam.Parameter.Name==ParameterName:
                Flag = True
                break
        if not Flag:
            self.lg.warning("Parameter :'%s' not found for activity %d !" % (ParameterName,ActivityId))
            return(False) 

        strParamValue = str(ParameterValue)   
        
        Param = self._GetParameterByName(ActivityId,ParameterName)
        if Param is None:
            activityParameterResult = self.DB.service.ActivityParameterCreate(
                activityID = ActivityId,
                activityParameterID = ActTypeParam.ID,
                value = strParamValue, # ParameterValue,
                userID = self.user.ID
            )  
            if activityParameterResult.ErrorCode != 0:
                self.lg.warning("Create Parameter to Activity error : '%s' -> %s" % (ParameterName,activityParameterResult.ErrorMessage))
                return(False) 
            self.lg.debug("Create Parameter to the Activity: '%s'=%f" % (ParameterName, ParameterValue) )
        else:
            activityParameterResult = self.DB.service.ActivityParameterChange(
                ID = Param.ID,
                value = strParamValue, #ParameterValue,
                userID = self.user.ID
            )  
            if activityParameterResult.ErrorCode != 0:
                self.lg.warning("Change Parameter to Activity error : '%s' -> %s" % (ParameterName,activityParameterResult.ErrorMessage))
                return(False) 
            self.lg.debug("Change Parameter to the Activity: '%s'=%f" % (ParameterName, ParameterValue) )
        return(True)
    
    # -- Write one Activity Parameter
    #
    #  ActivityId    := the ID af the Activity
    #  ParameterName := string, the name of the Parameter
    #  ParameterValue:= float, the value to write
    #
    #  Return := True / False
    #
    def ParameterWrite(self, ActivityId = -999, ParameterName="", ParameterValue=0.0):
        strParamValue = str(ParameterValue)   
        return( self.ParameterWriteString(ActivityId, ParameterName, strParamValue) )

# -- Write one Activity Parameter String
    #
    #  ActivityId    := the ID af the Activity
    #  ParameterName := string, the name of the Parameter
    #  ParameterValue:= string, the value to write
    #
    #  Return := True / False
    #
    def ParameterWriteString(self, ActivityId = -999, ParameterName="", strParameterValue=""):
        # Get the Activity spec.
        if not self.AquireActivityTypeByActId(ActivityId):
            self.lg.warning("Activity not enabled for the specified location : %d " % (ActivityId))
            return(False)
        # prepare the common data
        Flag = False 
        for ActTypeParam in self.ActivityType.Parameters.ActivityTypeParameter:
            if ActTypeParam.Parameter.Name==ParameterName:
                Flag = True
                break
        if not Flag:
            self.lg.warning("Parameter :'%s' not found for activity %d !" % (ParameterName,ActivityId))
            return(False) 
         
        activityParameterResult = self.DB.service.ActivityParameterCreate(
            activityID = ActivityId,
            activityParameterID = ActTypeParam.ID,
            value = strParameterValue, # ParameterValue,
            userID = self.user.ID
        )  
        if activityParameterResult.ErrorCode != 0:
            self.lg.warning("Create Parameter to Activity error : '%s' -> %s" % (ParameterName,activityParameterResult.ErrorMessage))
            return(False) 
        self.lg.debug("Create Parameter to the Activity: '%s'=%s" % (ParameterName, strParameterValue) )
        return(True)


    # -- Change one Activity Parameter
    #
    #  ActivityId    := the ID af the Activity
    #  ParameterName := string, the name of the Parameter
    #  ParameterValue:= float, the value to write
    #
    #  Return := True / False
    #
    def ParameterChange(self, ActivityId = -999, ParameterName="", ParameterValue=0.0):
        strParamValue = str(ParameterValue)
        return( self.ParameterChangeString(ActivityId, ParameterName, strParamValue) )

    # -- Change one Activity Parameter String
    #
    #  ActivityId    := the ID af the Activity
    #  ParameterName := string, the name of the Parameter
    #  ParameterValue:= string, the value to write
    #
    #  Return := True / False
    #
    def ParameterChangeString(self, ActivityId = -999, ParameterName="", strParameterValue=""):
        ParamId = self._GetParameterIdByName(ActivityId,ParameterName)
        if ParamId < 0:
            self.lg.warning("Parameter not found : '%s'" % (ParameterName))
            return(False)
        # change the param
        activityParameterResult = self.DB.service.ActivityParameterChange(
            ID = ParamId,
            value = strParameterValue, #ParameterValue,
            userID = self.user.ID
        )  
        if activityParameterResult.ErrorCode != 0:
            self.lg.warning("Change Parameter to Activity error : '%s' -> %s" % (ParameterName,activityParameterResult.ErrorMessage))
            return(False) 
        self.lg.debug("Change Parameter to the Activity: '%s'=%s" % (ParameterName, strParameterValue) )
        return(True)

    # -- Read one Activity Parameter
    #
    #  ActivityId    := the ID af the Activity
    #  ParameterName := string, the name of the Parameter
    #
    #  Return := a couple := [ True / False, Value (float) ]
    #
    def ParameterRead(self, ActivityId = -999, ParameterName=""):
        flag, strVal = self.ParameterReadString(ActivityId, ParameterName)
        return(flag, float(strVal))

    # -- Read one Activity Parameter String
    #
    #  ActivityId    := the ID af the Activity
    #  ParameterName := string, the name of the Parameter
    #
    #  Return := a couple := [ True / False, Value (string) ]
    #
    def ParameterReadString(self, ActivityId = -999, ParameterName=""):
        Param = self._GetParameterByName(ActivityId,ParameterName)
        if Param is None:
            return(False, 0.0)
            
        self.lg.debug("Parameter '%s' = %f " % (ParameterName, float(Param.Value)) )
        return(True, Param.Value)
    
    
    # -- Delete one Activity Parameter
    #
    #  ActivityId    := the ID af the Activity
    #  ParameterName := string, the name of the Parameter
    #
    #  Return := True / False
    #
    def ParameterDelete(self, ActivityId = -999, ParameterName=""):
        ParamId = self._GetParameterIdByName(ActivityId,ParameterName)
        if ParamId < 0:
            self.lg.warning("Parameter not found : '%s'" % (ParameterName))
            return(False)
        # change the param
        activityParameterResult = self.DB.service.ActivityParameterRemove(
            ID = ParamId) 
        if activityParameterResult.ErrorCode != 0:
            self.lg.warning("Delete Parameter to Activity error : '%s' -> %s" % (ParameterName,activityParameterResult.ErrorMessage))
            return(False) 
        self.lg.debug("Delete Parameter to the Activity: '%s'" % (ParameterName) )
        return(True)            
         
         
    def _GetParameterIdByName(self, ActivityId = -999, ParameterName=""):
        act = self.GetActivity(ActivityId)
        if act is None:
            self.lg.warning("Activity %d not found !" % (ActivityId))
            return(-999)
        if act.Parameters is None:
            self.lg.warning("Activity %d have not defined parameters !" % (ActivityId))
            return(-999)
        for ActParam in act.Parameters.ActivityParameter:
            if ActParam.ActivityTypeParameter.Parameter.Name==ParameterName:
                return(ActParam.ID)
        self.lg.warning("Parameter '%s' not found !" % (ParameterName))
        return(-999)
    
    def _GetParameterByName(self, ActivityId = -999, ParameterName=""):
        # Get the Activity spec.
        act = self.GetActivity(ActivityId)
        if act is None:
            self.lg.warning("Activity %d not found !" % (ActivityId))
            return(None)
        if act.Parameters is None:
            self.lg.warning("Activity %d have not defined parameters !" % (ActivityId))
            return(None)
        for ActParam in act.Parameters.ActivityParameter:
            if ActParam.ActivityTypeParameter.Parameter.Name==ParameterName:
                return(ActParam)
        self.lg.warning("Parameter '%s' not found !" % (ParameterName))
        return(None)
    
    
    
    # *************************************
    #   Members Methods 
    #
      
    # -- Get the list of Members Name for a specified Project
    #
    #  projectName        := string, the name of the project
    #  Return := string list, the Members name
    #
    def GetMembersByProjectName(self, projectName):
        membersList = []
        projId = self.GetProjectIdByName(projectName)
        if projId > 0:
            result = self.DB.service.ProjectMemberRead(
                projectID = projId
            )
            if result is None: 
                self.lg.warning("Member list for project %s is empty !" % projectName)
            else:
                for mr in result:
                    membersList.append(mr.FullName)
                self.lg.debug("Member list contains %d items !" % len(membersList))
        return(membersList)
        
    # -- Get the List of all Project Members
    #
    #  Return := A list of Memeber Objects
    #
    def GetMemberList(self):
        return( self.MemberList )

    # -- Get one Member
    #
    #  MemberFullName    := string, the full name of the member
    #  Return := A Member Objects
    #
    def GetMemberByName(self, MemberFullName):
        for p in self.MemberList:
            if p.FullName == MemberFullName:
                return(p)
        self.lg.warning("The Member %s is not found in the list of %d elements !" % (MemberFullName,len(self.MemberList)))
        return(None)

    def GetMemberIdByName(self, MemberFullName):
        for p in self.MemberList:
            if p.FullName == MemberFullName:
                return(p.ID)
        return(-999)

    def GetMemberNameById(self, MemberId):
        for p in self.MemberList:
            if p.ID == MemberId:
                return(p.FullName)
        return("")
  
    # -- Set the list of the Members selected for the Activity
    #
    #  MemberIds    := list of int, the ID of the members
    #  Return := A List of Member Objects
    #
    def SetSelectedMemberByIds(self, MemberIds):
        self.selectedMembers = []
        for memberId in MemberIds:
            for member in self.MemberList:
                if member.ID == memberId:
                    self.selectedMembers.append(member)
        return(self.selectedMembers)

    # -- Get the list of the Members selected for the Activity
    #
    #  Return := A List of Memeber Objects
    #
    def GetSelectedMember(self):
        return(self.SelectedMembers)

 
    # --- Stores the list of Member that will be used with activities 
    # 
    #   MemberNames    :=    The list of Member objects
    #   Return     :=    
    #   
    def SetUpMembersForActivity(self, MemberNames = []):
        self.lg.debug("Sets %d members for activity." % len(MemberNames))
        self.selectedMembers = []
        for name in MemberNames:
            member = self.GetMemberByName(name)
            if(member is not None):
                self.selectedMembers.append(member)
        if( len(self.selectedMembers) <= 0):
            self.lg.warning("No members ar defined for this activity !")
        return
 
    # --- Records the Members to an Activity
    # 
    #   ActivityId    :=    The ID of an open activity
    #   Return     :=    the result as ActivityResult Object
    #   
    def ActivityMemberAssign(self, ActivityId):
        result = self._ActivityMemberAssign(ActivityId, self.selectedMembers)
        return(result)

    def _ActivityMemberAssign(self, ActivityId, Members = [], isLeader = False):
        if isLeader :
            isTheLeader = 1
        else:
            isTheLeader = 0
        #print MemberId, ActivityId,   isTheLeader , self.user.ID 
        for member in Members:
            activityResult = self.DB.service.ActivityMemberAssign(
                projectMemberID = member.ID,
                activityID = ActivityId,
                leader = isTheLeader,
                userID = self.user.ID
                )  
            if activityResult.ErrorCode != 0:
                self.lg.warning("Assign member %s to Activity %d error : %s" % (member.FullName,ActivityId,activityResult.ErrorMessage))
            else:
                self.lg.debug("Assign member %s ID=%d to Activity ID=%d !" % (member.FullName,member.ID,ActivityId))
        return(activityResult)
        
    def _ActivityMemberRemove(self, ActivityMemberId):
        activityResult = self.DB.service.ActivityMemberRemove(
            ID = ActivityMemberId
        )  
        if activityResult.ErrorCode != 0:
            self.lg.warning("Remove member to Activity error : %s -> %s" % (ActivityMemberId,activityResult.ErrorMessage))
        return(activityResult) 
  
    def _GetActivityMembersID(self, ActivityId):
        act = self.GetActivity(ActivityId)
        if act is None:
            self.lg.warning("Activity %d not found !" % (ActivityId))
            return([])
        IdList = []
        for ActMember in act.Members :
            IdList.append(ActMember.ActivityMember.Member.ID)
        return(IdList)
    
    def _GetActivityMemberIDbyMemberID(self, ActivityId, MemberId):
        act = self.GetActivity(ActivityId)
        if act is None:
            self.lg.warning("Activity %d not found !" % (ActivityId))
            return(-22)
        for ActMember in act.Members :
            if ActMember.ActivityMember.Member.ID == MemberId:
                return(ActMember.ActivityMember.ID)
        return(-22)

    def _GetActivityMemberIDbyMemberName(self, ActivityId, MemberName):
        act = self.GetActivity(ActivityId)
        if act is None:
            self.lg.warning("Activity %d not found !" % (ActivityId))
            return(-22)
        for ActMember in act.Members :
            if ActMember.ActivityMember.Member.FullName == MemberName:
                return(ActMember.ActivityMember.ID)
        return(-22)


    # **********************************
    #    Project Management
    #
    
    # -- Get the list of all projects
    #
    #  Return := A List of Project Names
    #
    def GetProjects(self):
        projectList =[]
        result = self.DB.service.ProjectRead()
        if result is None: 
            self.lg.warning("Projects list into the DB error !")
        else:
            for pr in result:
                projectList.append(pr.Name)
            self.lg.debug("Projects list contains %d items !" % len(projectList))
        return(projectList) 

    # -- Get the ID of the specified Project
    #
    #  projectName        := string, the name of the project
    #  Return := int, the project ID
    #
    def GetProjectIdByName(self, projectName):
        result = self.DB.service.ProjectRead()
        if result is None: 
            self.lg.warning("Projects list into the DB error !")
            return(-999)
        else:
            proj = next(p for p in result if p.Name==projectName)
            self.lg.debug("From project %s get the Id %d !" % (projectName,proj.ID))
            return(proj.ID)

 
        
# =======================================================
# Attachment manager
#
class AttchMan:

    lg = None
    theAttachLimit = 100000
    theUriBasePath = "https://cernbox.cern.ch/remote.php/webdav/%20%20project%20alice-its/"

#   Set for the use with rsync
    theAttachCommand = "cp %1/%2 %3"
    theMakeDirCommand = "mkdir -p %1"
    theBasePath = "/tmp/"

#    Set for the use with ssh/scp
#   theAttachCommand = "sleep 1 && scp %1/%2 aliceitsbari@lxplus.cern.ch:%3 && sleep 1"
#   theMakeDirCommand = "ssh aliceitsbari@lxplus.cern.ch 'mkdir -p %1'"
#   theBasePath = "/eos/project/a/alice-its/"

    def __init__(self):
        
        # set up the logger
        self.lg = logging.getLogger(LOGGERNAME)
        return

    def SetUp(self, Limit, Base, Uri, AttCom, MkCom): 
        # get the setting for the attachment mechanism
        self.theAttachLimit = Limit
        self.theBasePath = Base
        self.theUriBasePath = Uri
        self.theAttachCommand = AttCom
        self.theMakeDirCommand = MkCom

    def SetLimit(self, limit):
        self.theAttachLimit = limit
        
    def GetLimit(self):
        return(self.theAttachLimit)
    
    def SetCommand(self, command):
        self.theAttachCommand = command
        
    def GetCommand(self):
        return(self.theAttachCommand)
    
    def ExecuteTheTransfer(self, fileName, subfolder, prefix):
        Path, Name = os.path.split(fileName)
        if Name == "":
            return(False)
        if Path != "":
            Path = Path +"/"
            
        destination = self._BuildTheEosPath(subfolder) + self._BuildTheEosName(Name, prefix)
        destURI = self._BuildTheUriPath(subfolder) + self._BuildTheEosName(Name, prefix)
        
        # Build the destination path
        destPathDir = os.path.dirname(destination)
        try:
            aNewCommand = self.theMakeDirCommand
            aNewCommand = aNewCommand.replace("%1", destPathDir)
        except:
            self.lg.error("File Attachment : Error to format the Make New Dir command ! (%s)" % self.theMakeDirCommand)
            return("")
            
        self.lg.debug("Destination creation command : '%s'" % aNewCommand)
        try:
            return_code = subprocess.check_output(aNewCommand, stderr=subprocess.STDOUT, shell=True)  
            self.lg.debug("Creation result: %s" % return_code)
        except OSError:
            self.lg.error("Error to create Destination dir: %s Abort transfer!", aNewCommand)
            return("")

        # Executes the transfer
        try:
            aTranfCommand = self.theAttachCommand
            aTranfCommand = aTranfCommand.replace("%1/", Path)
            aTranfCommand = aTranfCommand.replace("%2", Name)
            aTranfCommand = aTranfCommand.replace("%3", destination )
        except:
            self.lg.error("File Attachment : Error to format the Transfer File command ! (%s)" % self.theAttachCommand)
            return("")

        self.lg.debug("Transfer command: '%s'" % aTranfCommand)
        try:
            return_code = subprocess.check_output(aTranfCommand, stderr=subprocess.STDOUT, shell=True)  
            self.lg.debug("Transfer result: %s" % return_code)
        except subprocess.CalledProcessError, ex: # error code <> 0 
            self.lg.error("Error to transfer Attach file. %s !", ex.message)
            self.lg.error("Output: %s ", ex.output)
            destURI = ""
        return(destURI)
         
    def GetURIPath(self, fileName):
        Path, Name = os.path.split(fileName)
        return(Name)

    def GetURIDescription(self, fileName):
        return(fileName)
    
    def _BuildTheEosPath(self, subFolder):
        path = self.theBasePath + "/" + subFolder + "/"
        path = path.replace("//","/")
        return(path)

    def _BuildTheUriPath(self, subFolder):
        subFolder = "/" + subFolder +"/"
        subFolder = subFolder.replace("//","/")
        if self.theUriBasePath[-1:] == "/":
            self.theUriBasePath = self.theUriBasePath[:-1]
        path = self.theUriBasePath + subFolder
        return(path)

    def _BuildTheEosName(self, fileName, prefix):
        if fileName == "":
            path = "NoNameFile.txt"
        elif prefix == "":
            path = fileName
        else:
            path = prefix + "_" + fileName
        if path[:1] == "/":
            path = path[1:]
        return(path)




    # *****************************   EOF  ***************************************
