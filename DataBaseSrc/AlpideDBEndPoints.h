/*
 * \file AlpideDBEndPoints.h
 * \author A.Franco
 * \date 17/Mar/2017
 *
 * Copyright (C) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/
 *
 * ====================================================
 *
 *  Description : Header file for the Alpide DB EndPoint
 *  V.2.1
 *
 *  HISTORY
 *
 *  7/9/2017	-	Refine the XML parsing/reading
 *  9/11/2017   - Add ParameterID field to the parameter type struct
 *  21/02/2018  -   Add the Scientific Notation in the Activity Parameter rappresentation
 *  04/10/2018  - Version 2 : Recover errors + Improvements to error diagnosis
 *
 */
#ifndef ALPIDEDBENDPOINTS_H_
#define ALPIDEDBENDPOINTS_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "utilities.h"

using namespace std;

#define MATCHNODE(nod, tag) (nod->children && strcmp((const char *)nod->name, tag) == 0)

#define MAXNUMBEROFQUERYATTEMPTS 5
#define DELAYINQUERYRECOVER 10
#define MAXRESPONSESHISTORY 100

class AlpideDB;

// The base class
class AlpideTable {
public:
  enum ErrorCode {
    NoError           = 0,
    RecordNotFound    = 10,
    SyncQuery         = 20,
    BadXML            = 21,
    EmptyXMLroot      = 22,
    BadCreation       = 23,
    InvalidCredential = 32,
    MSIS7042          = 33,
    Unknown           = 99
  };
  enum ErrorManage { OK = 0, RETRAY = 1, ABORT = 2 };
  enum SessionCode {
    NoSession           = 0,
    CreateActivity      = 1,
    MemberAssign        = 2,
    ParameterCreate     = 3,
    AttachmentCreate    = 4,
    UriCreate           = 5,
    ChangeActivity      = 6,
    AssignComponent     = 7,
    ChangeParameter     = 8,
    CreateComponent     = 9,
    PojectGetList       = 10,
    MemberGetList       = 11,
    ComponentTypeList   = 12,
    ComponentType       = 13,
    Component           = 14,
    ComponentList       = 15,
    ComponentActivities = 16,
    ActivityTypeList    = 17,
    ActivityType        = 18,
    Activity            = 19
  };


  struct response {
    int    Session;
    int    ErrorCode;
    string ErrorMessage;
    int    ID;
  };


protected:
  AlpideDB *       theParentDB;
  response         theResponse;
  string           theGeneralBuffer;
  bool             isScienNotation;
  vector<response> theResponses;
  bool             isCreated;
  xmlDocPtr        theXMLdoc;
  xmlNode *        theRootXMLNode;
  int              theActualSession;

  map<int, string> errorCodesTbl;
  map<int, string> sessionCodesTbl;

public:
  explicit AlpideTable(AlpideDB *DBhandle);
  virtual ~AlpideTable();
  bool ExecuteQuery(string theUrl, string theQuery, bool isSOAPrequest = false,
                    const char *SOAPAction = NULL);
  // response                 GetResponse() { return theResponse; }
  AlpideTable::ErrorManage DecodeResponse(char *returnedString, const char *query, int Session = 0);
  bool                     ParseQueryResult();

  bool isParameterScientificNotation() { return (isScienNotation); }
  void setParameterScientificNotation(bool isSet = true) { isScienNotation = isSet; }

  void             SetResponse(AlpideTable::ErrorCode, int ID = 0, int Session = -1);
  response         GetResponse(unsigned int Position = -1);
  vector<response> GetResponses() { return theResponses; }
  void             ClearResponses()
  {
    theResponses.clear();
    return;
  }
  unsigned int GetResponsesNumber() { return (theResponses.size()); }
  const char * DumpResponse(response *aResponse = NULL);
  bool         GetStatus() { return isCreated; }
  string GetErrorCodeMessage(AlpideTable::ErrorCode code) { return (errorCodesTbl[(int)code]); }
  string GetSessionCodeMessage(AlpideTable::SessionCode code)
  {
    return (sessionCodesTbl[(int)code]);
  }

  AlpideTable::ErrorCode ParseTheNonXMLResponse(char *ThePage);

protected:
  void setResponseSession(AlpideTable::SessionCode aSession)
  {
    theActualSession = aSession;
    return;
  }
  void   dumpXMLError(const char *aDescription, const char *aQuery, const char *aResponse);
  string formatTheParameterValue(float value);
  void   pushResponse(response *aResponse = NULL);
  void   createCodesDictionary(void);
};

class ProjectDB : public AlpideTable {

  // Members
public:
  struct project {
    int    ID;
    string Name;
  };

private:
  project proj;

  // Methods
public:
  ProjectDB(AlpideDB *DBhandle);
  ~ProjectDB();

public:
  AlpideTable::response *GetList(vector<ProjectDB::project> *Result);
  string                 Print(ProjectDB::project *pr)
  {
    return ("Project : ID=" + std::to_string(pr->ID) + " Name=" + pr->Name);
  };
};

class MemberDB : public AlpideTable {

  // Members
public:
  struct member {
    int    ID;
    int    PersonalID;
    string FullName;
  };

private:
  member memb;

  // Methods
public:
  MemberDB(AlpideDB *DBhandle);
  ~MemberDB();

  AlpideTable::response *GetList(int projectID, vector<member> *Result);
  string                 Print(member *me)
  {
    return ("Member : ID=" + std::to_string(me->ID) +
            " Personal ID=" + std::to_string(me->PersonalID) + " Name=" + me->FullName);
  };
};

class ComponentDB : public AlpideTable {

  // Members
private:
  string ap;

public:
  struct composition {
    int    ID;
    string ComponentType;
    int    Quantity;
  };
#define zCOMPOSITION(a)                                                                            \
  a.ID            = 0;                                                                             \
  a.ComponentType = "";                                                                            \
  a.Quantity      = 0

  struct statusphysical {
    int    ID;
    string Name;
  };
#define zSTATUSPHYSICAL(a)                                                                         \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct statusfunctional {
    int    ID;
    string Name;
  };
#define zSTATUSFUNCTIONAL(a)                                                                       \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct componentType {
    int                      ID;
    string                   Name;
    string                   Code;
    string                   Description;
    vector<composition>      Composition;
    vector<statusphysical>   PhysicalStatus;
    vector<statusfunctional> FunctionalStatus;
  };
#define zCOMPONENTTYPE(a)                                                                          \
  a.ID          = 0;                                                                               \
  a.Name        = "";                                                                              \
  a.Code        = "";                                                                              \
  a.Description = "";                                                                              \
  a.Composition.clear();                                                                           \
  a.PhysicalStatus.clear();                                                                        \
  a.FunctionalStatus.clear()

  struct compType {
    int    ID;
    string Name;
  };
#define zCOMPTYPE(a)                                                                               \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct compComponent {
    int      ID;
    string   ComponentID;
    compType ComponentType;
  };
#define zCOMPCOMPONENT(a)                                                                          \
  a.ID          = 0;                                                                               \
  a.ComponentID = "";                                                                              \
  zCOMPTYPE(a.ComponentType)

  struct compComposition {
    int           ID;
    string        Position;
    compComponent Component;
  };
#define zCOMPCOMPOSITION(a)                                                                        \
  a.ID       = 0;                                                                                  \
  a.Position = "";                                                                                 \
  zCOMPCOMPONENT(a.Component)

  struct componentLong {
    int                     ID;
    string                  ComponentID;
    string                  SupplierComponentID;
    string                  Description;
    string                  LotID;
    string                  PackageID;
    compType                Type;
    statusphysical          PhysicalState;
    statusfunctional        FunctionalState;
    vector<compComposition> Composition;
  };
#define zCOMPONENTL(a)                                                                             \
  a.ID                  = 0;                                                                       \
  a.ComponentID         = "";                                                                      \
  a.SupplierComponentID = "";                                                                      \
  a.Description         = "";                                                                      \
  a.LotID               = "";                                                                      \
  a.PackageID           = "";                                                                      \
  zCOMPTYPE(a.Type);                                                                               \
  zSTATUSPHYSICAL(a.PhysicalState);                                                                \
  zSTATUSFUNCTIONAL(a.FunctionalState);                                                            \
  a.Composition.clear()

  struct componentShort {
    int              ID;
    string           ComponentID;
    string           SupplierComponentID;
    string           Description;
    string           LotID;
    string           PackageID;
    statusphysical   PhysicalState;
    statusfunctional FunctionalState;
  };
#define zCOMPONENTS(a)                                                                             \
  a.ID                  = 0;                                                                       \
  a.ComponentID         = "";                                                                      \
  a.SupplierComponentID = "";                                                                      \
  a.Description         = "";                                                                      \
  a.LotID               = "";                                                                      \
  a.PackageID           = "";                                                                      \
  zSTATUSPHYSICAL(a.PhysicalState);                                                                \
  zSTATUSFUNCTIONAL(a.FunctionalState)

  struct compActResult {
    int    ID;
    string Name;
  };
#define zCOMPACTRESULT(a)                                                                          \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct compActStatus {
    int    ID;
    string Code;
    string Description;
  };
#define zCOMPACTSTATUS(a)                                                                          \
  a.ID          = 0;                                                                               \
  a.Code        = "";                                                                              \
  a.Description = ""

  struct compActivity {
    int           ID;
    string        Name;
    time_t        StartDate;
    time_t        EndDate;
    compActResult Result;
    compActStatus Status;
    int           Type;
    string        Typename;
  };
#define zCOMPACTIVITY(a)                                                                           \
  a.ID   = 0;                                                                                      \
  a.Name = "";                                                                                     \
  a.Type = 0;                                                                                      \
  zCOMPACTRESULT(a.Result);                                                                        \
  zCOMPACTSTATUS(a.Status)

  // Methods
public:
  ComponentDB(AlpideDB *DBhandle);
  ~ComponentDB();

  AlpideTable::response *GetType(int ComponentTypeID, componentType *Result);
  AlpideTable::response *GetTypeList(int ProjectID, vector<componentType> *Result);

  AlpideTable::response *Create(string ComponentTypeID, string ComponentID, string SupplyCompID,
                                string Description, string lotID, string PackageID, string userID);

  AlpideTable::response *Read(int ID, componentLong *Result);
  AlpideTable::response *Read(string ComponentID, componentLong *Result);
  AlpideTable::response *ReadParents(int ID, vector<compComposition> *Parents);

  AlpideTable::response *GetComponentActivities(string ComponentID, vector<compActivity> *Result);
  AlpideTable::response *GetComponentActivities(int ID, vector<compActivity> *Result);

  AlpideTable::response *GetListByType(int ProjectID, int ComponentTypeID,
                                       vector<componentShort> *Result);
  string                 Print(componentType *co);
  string                 Dump(componentType *co);
  string                 Dump(componentLong *co);

private:
  void                   extractTheComponentType(xmlNode *n1, componentType *pro);
  AlpideTable::response *readComponent(string ID, string ComponentID, componentLong *Result);
  AlpideTable::response *readComponents(std::string ProjectId, std::string ComponentTypeID,
                                        vector<componentShort> *compoList);
  void                   extractTheComponent(xmlNode *ns, componentLong *pro);
  AlpideTable::response *readComponentActivities(int ID, vector<compActivity> *Result);
  void                   extractTheActivityList(xmlNode *ns, vector<compActivity> *actList);
};

// --------------------
class ActivityDB : public AlpideTable {

  // Members
private:
public:
  struct member {
    int ID;
    int ProjectMember;
    int Leader;
    int User;
  };
#define zMEMBER(a)                                                                                 \
  a.ID            = 0;                                                                             \
  a.ProjectMember = 0;                                                                             \
  a.Leader        = 0;                                                                             \
  a.User          = 0

  struct parameter {
    int   ID;
    int   ActivityParameter;
    float Value;
    int   User;
  };
#define zPARAMETER(a)                                                                              \
  a.ID                = 0;                                                                         \
  a.ActivityParameter = 0;                                                                         \
  a.Value             = 0.0;                                                                       \
  a.User              = 0

  struct attach {
    int    ID;
    int    Category;
    string RemoteFileName;
    string LocalFileName;
    int    User;
  };
#define zATTACH(a)                                                                                 \
  a.ID             = 0;                                                                            \
  a.Category       = 0;                                                                            \
  a.RemoteFileName = "";                                                                           \
  a.LocalFileName  = "";                                                                           \
  a.User           = 0

  struct uriattach {
    int    ID;
    string UriPath;
    string UriDescription;
    int    User;
  };
#define zURIATTACH(a)                                                                              \
  a.ID             = 0;                                                                            \
  a.UriPath        = "";                                                                           \
  a.UriDescription = "";                                                                           \
  a.User           = 0

  struct parameterType {
    int    ID;
    int    ParameterID;
    string Name;
    string Description;
  };
#define zPARAMETERTYPE(a)                                                                          \
  a.ID          = 0;                                                                               \
  a.ParameterID = 0;                                                                               \
  a.Name        = "";                                                                              \
  a.Description = ""

  struct activityType {
    int    ID;
    string Name;
    string Description;
  };
#define zACTIVITYTYPE(a)                                                                           \
  a.ID          = 0;                                                                               \
  a.Name        = "";                                                                              \
  a.Description = ""

  struct locationType {
    int    ID;
    string Name;
  };
#define zLOCATIONTYPE(a)                                                                           \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct attachmentType {
    int    ID;
    string Category;
    string Description;
  };
#define zATTACHMENTTYPE(a)                                                                         \
  a.ID          = 0;                                                                               \
  a.Category    = "";                                                                              \
  a.Description = "";

  struct componentType {
    int    ID;
    string Name;
  };
#define zCOMPOTYPE(a)                                                                              \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct resultType {
    int    ID;
    string Name;
  };
#define zRESULTTYPE(a)                                                                             \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct statusType {
    int    ID;
    string Code;
    string Description;
  };
#define zSTATUSTYPE(a)                                                                             \
  a.ID          = 0;                                                                               \
  a.Code        = "";                                                                              \
  a.Description = ""

  struct statusphysical {
    int    ID;
    string Name;
  };
#define zSTATUSPHYSICAL(a)                                                                         \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct statusfunctional {
    int    ID;
    string Name;
  };
#define zSTATUSFUNCTIONAL(a)                                                                       \
  a.ID   = 0;                                                                                      \
  a.Name = ""

  struct activity {
    int               ID;
    int               Type;
    int               Location;
    string            Lot;
    string            Name;
    time_t            StartDate;
    time_t            EndDate;
    string            Position;
    int               Result;
    int               Status;
    int               User;
    vector<member>    Members;
    vector<parameter> Parameters;
    vector<attach>    Attachments;
  };

  // ---------------
  struct activityShort {
    int          ID;
    string       Name;
    time_t       StartDate;
    time_t       EndDate;
    activityType Type;
    statusType   Status;
  };
#define zACTIVITYSHORT(a)                                                                          \
  a.ID        = 0;                                                                                 \
  a.Name      = "";                                                                                \
  a.Type.ID   = 0;                                                                                 \
  a.Status.ID = 0

  struct actTypeCompType {
    int           ID;
    int           Quantity;
    string        Direction;
    componentType Type;
  };
#define zACTTYPECOMPTYPE(a)                                                                        \
  a.ID        = 0;                                                                                 \
  a.Quantity  = 0;                                                                                 \
  a.Direction = "in";                                                                              \
  zCOMPOTYPE(a.Type)

  struct actComp {
    int           ID;
    string        ComponentID;
    componentType Type;
  };
#define zACTCOMP(a)                                                                                \
  a.ID          = 0;                                                                               \
  a.ComponentID = "";                                                                              \
  zCOMPOTYPE(a.Type)

  struct actComponent {
    int              ID;
    int              Conformity;
    actTypeCompType  ActivityComponentType;
    actComp          Component;
    statusphysical   PhysicalStatus;
    statusfunctional FunctionalStatus;
  };
#define zACTCOMPONENT(a)                                                                           \
  a.ID         = 0;                                                                                \
  a.Conformity = 0;                                                                                \
  zACTTYPECOMPTYPE(a.ActivityComponentType);                                                       \
  zACTCOMP(a.Component);

  struct actParParameter {
    int    ID;
    string Name;
    string Description;
  };
#define zACTPARPARAMETER(a)                                                                        \
  a.ID          = 0;                                                                               \
  a.Name        = "";                                                                              \
  a.Description = ""

  struct actParameterType {
    int             ID;
    actParParameter Parameter;
  };
#define zACTPARAMETERTYPE(a)                                                                       \
  a.ID = 0;                                                                                        \
  zACTPARPARAMETER(a.Parameter)

  struct actParameter {
    int              ID;
    float            Value;
    actParameterType Type;
  };
#define zACTPARAMETER(a)                                                                           \
  a.ID    = 0;                                                                                     \
  a.Value = 0.0;                                                                                   \
  zACTPARAMETERTYPE(a.Type)

  struct actAttachment {
    int            ID;
    string         FileName;
    attachmentType Type;
  };
#define zACTATTACHMENT(a)                                                                          \
  a.ID       = 0;                                                                                  \
  a.FileName = "";                                                                                 \
  zATTACHMENTTYPE(a.Type)

  struct actMemMemmber {
    int    ID;
    int    PersonID;
    string FullName;
  };
#define zACTMEMMEMBER(a)                                                                           \
  a.ID       = 0;                                                                                  \
  a.PersonID = 0;                                                                                  \
  a.FullName = ""

  struct actMember {
    int           ID;
    int           Leader;
    actMemMemmber Member;
  };
#define zACTMEMBER(a)                                                                              \
  a.ID     = 0;                                                                                    \
  a.Leader = 0;                                                                                    \
  zACTMEMMEMBER(a.Member)

  struct actUri {
    int    ID;
    string Path;
    string Description;
  };
#define zACTURI(a)                                                                                 \
  a.ID          = 0;                                                                               \
  a.Path        = "";                                                                              \
  a.Description = ""

  struct activityLong {
    int                   ID;
    string                Name;
    string                LotID;
    time_t                StartDate;
    time_t                EndDate;
    resultType            Result;
    activityType          Type;
    statusType            Status;
    locationType          Location;
    vector<actComponent>  Components;
    vector<actParameter>  Parameters;
    vector<actAttachment> Attachments;
    vector<actMember>     Members;
    vector<actUri>        Uris;
  };
#define zACTIVITYLOND(a)                                                                           \
  a.ID    = 0;                                                                                     \
  a.Name  = "";                                                                                    \
  a.LotID = "";                                                                                    \
  zRESULTTYPE(a.Result);                                                                           \
  zACTIVITYTYPE(a.Type);                                                                           \
  zSTATUSTYPE(a.Status);                                                                           \
  zLOCATIONTYPE(a.Location);                                                                       \
  Components.clear();                                                                              \
  Parameters.clear();                                                                              \
  Attachments.clear();                                                                             \
  Members.clear();                                                                                 \
  Uris.clear()

  // Methods
public:
  ActivityDB(AlpideDB *DBhandle);
  ~ActivityDB();

  AlpideTable::response *Create(activity *aActivity);
  AlpideTable::response *Change(activity *aActivity);
  AlpideTable::response *AssignComponent(int aActivityID, int aComponentID, int aComponentTypeID,
                                         int aUserID);

  AlpideTable::response *AssignUris(int aActivityID, int aUserId, vector<actUri> *aUris);

  vector<parameterType> *  GetParameterTypeList(int aActivityTypeID);
  vector<activityType> *   GetActivityTypeList(int aProjectID);
  vector<locationType> *   GetLocationTypeList(int aActivityTypeID);
  vector<actTypeCompType> *GetComponentTypeList(int aActivityTypeID);
  vector<resultType> *     GetResultList(int aActivityTypeID);
  vector<statusType> *     GetStatusList(int aActivityTypeID);
  vector<attachmentType> * GetAttachmentTypeList();
  vector<activityShort> *  GetActivityList(int aProjectID, int aActivityTypeID);
  AlpideTable::response *  ChangeParameter(int ActivityID, string parameterName, float newValue,
                                           int userID);
  AlpideTable::response *  Read(int ActivityID, activityLong *Result);
  void                     DumpActivity(activityLong *Act);

private:
  unsigned long          buildBase64Binary(string aLocalFileName, string *aBuffer);
  int                    buildUrlEncoded(string aLocalFileName, string *Buffer);
  AlpideTable::response *readActivity(string ID, activityLong *Result);
  void                   extractTheActivity(xmlNode *ns, activityLong *pro);
};

#endif /* ALPIDEDBENDPOINTS_H_ */
