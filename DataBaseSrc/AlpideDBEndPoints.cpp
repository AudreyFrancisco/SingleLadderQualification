/*
 * \file AlpideDBEndPoints.cpp
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
 *  Description : Alpide DB  Classes for access tables
 *
 *
 *  Ver 2.1 -
 *
 *  HISTORY
 *
 *  7/9/2017	-	Refine the XML parsing/reading function
 *  9/11/2017   -   Modify the GetParameterList method
 *  11/1/2018   -   Add Assign URIs method
 *  21/02/2018  -   Add the Scientific Notation in the Activity Parameter rappresentation
 *  20/08/2018  -   Free the XML Doc allocation, prevents memory leak (Paul suggestion)
 *  20/08/2018  -   Dump the parser Error in to a Debug File
 *  30/09/2018  -   Add the Management of the XML files
 *  04/10/2018  - Version 2 : Recover errors + Improvements to error diagnosis
 *  13/10/2018  - Fix History Activity bug
 *  20/12/2018  - Cut the log output for Query result
 *  12/02/2019  - Add the Method ReadParents()
 *
 */

#include <ctime>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include "AlpideDB.h"
#include "AlpideDBEndPoints.h"

/* --------------------------------------------------------
 *    AlpideDB Table Base class
 *
 *    set the pointer to the linked DB and expose
 *    basic methods for Encode/Decode responses
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *    set the pointer to the linked DB
 *---------------- */
AlpideTable::AlpideTable(AlpideDB *DBhandle)
{
  dbLogSetLevel(LOGLEVEL); // LOG_TRACE);
  theParentDB      = DBhandle;
  isScienNotation  = false;
  theXMLdoc        = NULL;
  theRootXMLNode   = NULL;
  theActualSession = AlpideTable::NoSession;
  isCreated        = false;

  createCodesDictionary();
}

/* -----------------
 *    Destructor
 *---------------- */
AlpideTable::~AlpideTable() {}


/* -----------------
 *    ExecuteQuery := Execute the Query and manage errors
 *
 *		In Param : the Url and the Query
 *		returns : a bool
 *
 *      Description:
 *      Make the DB query, and decode the result.
 *      In case of fail it redo the query some times.
 *
 *      Error conditions : return boolean
 *                         theResponse member
 *                         theResponses array member
 *                         output on the standard error
 *
 *---------------- */


bool AlpideTable::ExecuteQuery(string theUrl, string theQuery, bool isSOAPrequest,
                               const char *SOAPAction)
{
  FUNCTION_TRACE;
  char *                   stringresult;
  AlpideTable::ErrorManage errorType = OK;
  int                      attempts  = MAXNUMBEROFQUERYATTEMPTS;
  unsigned int             secDelay  = DELAYINQUERYRECOVER;

  while (attempts > 0) {
    DEBUG("Request a Query :%s / %s", theUrl.c_str(), theQuery.c_str());
    if (theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult,
                                                     isSOAPrequest, SOAPAction) == 0) {
      SetResponse(AlpideTable::SyncQuery, 0, theActualSession);
      cerr << "Activity DB query execution Error : Unable to relize the DB connection ! (Sync "
              "Query Error)"
           << endl;
      ERROR("DB connection error, %s !", "Sync Error");
    }
    else {
      DEBUG("Obtains %d char of response :%.450s", strlen(stringresult), stringresult);
      errorType = DecodeResponse(stringresult, theQuery.c_str());
      switch (errorType) {
      case 2:
        TRACE("Exit for unrecoverable error (%d)", errorType);
        pushResponse();
        return (false);
        break;
      case 1:
        TRACE("New query attempt after %d seconds ...", secDelay);
        sleep(secDelay);
        attempts--;
        break;
      case 0:
        TRACE("OK, now exit (%d)", errorType);
        return (true);
        break;
      }
    }
  }
  TRACE("Exit for end of attempts loop !(%d)", attempts);
  pushResponse();
  return (false);
}


/* -----------------
 *    DecodeResponse := analyze the returned XML and fills the
 *                      response structure with the returned errors info
 *		Params :  ReturnedString := the XML sgtring received from the WebDB server
 *				  Session := a Session Index Value (reserved for future use)
 *		returns : a response Index := 0 = OK, 1 = RETRAY, 2 = FAIL
 *
 *      Description:
 *      Analyze the XML content of the response, and stores the XML doc into
 *      two members : theXMLdoc, the complite xml tree
 *                    theRootXMLNode, a pointer to the root node
 *
 *      In case of error : provides a respoonse index
 *                         Dumps the wrong XML result into a file
 *                         sets theResponse member
 *
 *---------------- */
AlpideTable::ErrorManage AlpideTable::DecodeResponse(char *ReturnedString, const char *aQuery,
                                                     int Session)
{
  FUNCTION_TRACE;
  ErrorManage responseIndex = OK;

  if (theXMLdoc != NULL) { // the xml doc exists ! deallocate the previous
    TRACE("De-alloc XML document %d !", theXMLdoc);
    xmlFreeDoc(theXMLdoc);
    theXMLdoc = NULL;
  }
  theRootXMLNode = NULL;

  theXMLdoc =
      xmlReadMemory(ReturnedString, strlen(ReturnedString), "noname.xml", NULL,
                    XML_PARSE_NOERROR + XML_PARSE_NOWARNING + XML_PARSE_PEDANTIC); // parse the XML
  if (theXMLdoc == NULL) {
    AlpideTable::ErrorCode HTMLerr = ParseTheNonXMLResponse(ReturnedString);
    SetResponse(HTMLerr, 0, theActualSession);
    responseIndex = RETRAY;
    dumpXMLError("Failed to parse document", aQuery, ReturnedString);
    ERROR("The XML Parser fails to decode the response. HTML code=%d", HTMLerr);
  }
  else {
    TRACE("Allocated XML document %d !", theXMLdoc);
    theRootXMLNode = xmlDocGetRootElement(theXMLdoc);
    if (theRootXMLNode == NULL) {
      SetResponse(AlpideTable::EmptyXMLroot, 0, theActualSession);
      WARNING("The XML document has an empty root node ! ResponseIndex=%d", responseIndex);
      responseIndex = RETRAY;
    }
    else {
      DEBUG("The Root Node is '%s'", theRootXMLNode->name);
      SetResponse(AlpideTable::NoError, 0, theActualSession);
      responseIndex = OK;
    }
  }
  free(ReturnedString);
  DEBUG("Exit with responseIndex = %d", responseIndex);
  return (responseIndex);
}

/* -----------------
 *    ParseQueryResult := analyze the XML doc that contains the standard
 *                          DB response (ID, Errorcode, Errormessage)
 *		returns : a bool
 *
 *      Description:
 *      Analyze the XML doc and fills theResponse member
 *
 *     In case of DB error traces the error on stderr
 *     and add the response into theResponses member
 *
 *
 *---------------- */

bool AlpideTable::ParseQueryResult()
{
  FUNCTION_TRACE;
  if (theRootXMLNode != NULL) {
    xmlNode *n1 = theRootXMLNode->children;
    while (n1 != NULL) {
      if (MATCHNODE(n1, "ErrorCode")) {
        theResponse.ErrorCode = atoi((const char *)n1->children->content);
      }
      else if (MATCHNODE(n1, "ErrorMessage")) {
        theResponse.ErrorMessage = (const char *)n1->children->content;
      }
      else if (MATCHNODE(n1, "ID")) {
        theResponse.ID = atoi((const char *)n1->children->content);
      }
      n1 = n1->next; // skip to the brother
    }
    theResponse.Session = theActualSession;
    TRACE("Response Code=%d, Mess='%s', ID=%d", theResponse.ErrorCode,
          theResponse.ErrorMessage.c_str(), theResponse.ID);
    if (theResponse.ErrorCode != 0) {
      cerr << "DB query execution Error :" << DumpResponse() << endl;
      pushResponse();
      DEBUG("Response with error: %s (%d)", theResponse.ErrorMessage.c_str(),
            theResponse.ErrorCode);
      return (false);
    }
    else {
      TRACE("Response without error: %s (%d)", theResponse.ErrorMessage.c_str(),
            theResponse.ErrorCode);
      return (true);
    }
  }
  SetResponse(AlpideTable::EmptyXMLroot, 0, theActualSession);
  pushResponse();
  WARNING("Response EMPTY !  (%d)", theRootXMLNode);
  return (false);
}

/* -----------------
 *    SetResponse := preset the response struct with a certain number
 *    				 of standard errors
 *
 *		Params :  ErrorCode := the standard error code
 *				  ID := the ID ...
 *				  Session := a Session Index Value (reserved for future use)
 *
 *---------------- */
void AlpideTable::SetResponse(AlpideTable::ErrorCode ErrNum, int ID, int Session)
{
  FUNCTION_TRACE;
  theResponse.ErrorCode = (int)ErrNum;
  theResponse.ID        = ID;
  if (Session == -1) {
    theResponse.Session = theActualSession;
  }
  else {
    theResponse.Session = Session;
  }
  theResponse.ErrorMessage = errorCodesTbl[(int)ErrNum];
  TRACE("Set the response %s (%d) ID=%d Session=%d", theResponse.ErrorMessage.c_str(),
        theResponse.ErrorCode, theResponse.ID, theResponse.Session);
  return;
}

/* -----------------

 *---------------- */
const char *AlpideTable::DumpResponse(response *aResponse)
{
  FUNCTION_TRACE;
  if (aResponse == NULL) {
    aResponse = &theResponse;
  }
  theGeneralBuffer = "DB Query response : '" + aResponse->ErrorMessage + "' (" +
                     std::to_string(aResponse->ErrorCode) + ") ";
  theGeneralBuffer +=
      "ID=" + std::to_string(aResponse->ID) + " Session=" + std::to_string(aResponse->Session);
  return (theGeneralBuffer.c_str());
}

AlpideTable::response AlpideTable::GetResponse(unsigned int Position)
{
  FUNCTION_TRACE;
  unsigned int n = theResponses.size();
  if (Position < 0 or n == 0) return (theResponse);
  if (Position >= n) Position = n - 1;
  DEBUG("The response array position is %d/%d of the array", Position, n);
  return (theResponses.at(Position));
}

void AlpideTable::dumpXMLError(const char *aDescription, const char *aQuery, const char *aResponse)
{
  FUNCTION_TRACE;
  char theErrorFileName[FILENAME_MAX];
  sprintf(theErrorFileName, "Data/DBXMLError_%s.dat", getTimeStamp().c_str());
  FILE *fh;
  fh = fopen(theErrorFileName, "w");
  if (fh == NULL) {
    cerr << "Failed to open the file " << theErrorFileName << endl;
    ERROR("Failed to open output file '%s' for XML error dump !", theErrorFileName);
    return;
  }
  fprintf(fh, "XML Error : %s\n", aDescription);
  fprintf(fh, "The DB Query : '%s'\n", aQuery);
  fprintf(fh, "The Returned string :\n");
  fprintf(fh, "%s\n", aResponse);
  fprintf(fh, "------ EOF -----\n");
  fclose(fh);
  return;
}

void AlpideTable::createCodesDictionary(void)
{
  errorCodesTbl.insert(std::pair<int, string>((int)NoError, "OK"));
  errorCodesTbl.insert(std::pair<int, string>((int)RecordNotFound, "Query returned no record"));
  errorCodesTbl.insert(std::pair<int, string>((int)SyncQuery, "The Sync DB Query returns error"));
  errorCodesTbl.insert(std::pair<int, string>((int)BadXML, "Invalid XML format"));
  errorCodesTbl.insert(
      std::pair<int, string>((int)EmptyXMLroot, "Query returned nothing. Empty XML tree"));
  errorCodesTbl.insert(
      std::pair<int, string>((int)BadCreation, "Wrong Activity Creation, it is incomplete"));
  errorCodesTbl.insert(std::pair<int, string>(
      (int)InvalidCredential, "CERN SSO fails. Invalid credential for authentication"));
  errorCodesTbl.insert(std::pair<int, string>((int)MSIS7042, "ISS authentication error..."));
  errorCodesTbl.insert(std::pair<int, string>((int)Unknown, "Unknown Error"));

  sessionCodesTbl.insert(std::pair<int, string>((int)NoSession, "Undefined Session"));
  sessionCodesTbl.insert(std::pair<int, string>((int)CreateActivity, "Create Activity"));
  sessionCodesTbl.insert(std::pair<int, string>((int)MemberAssign, "Assign Member to Activity"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)ParameterCreate, "Create a Parameter for Activity"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)AttachmentCreate, "Create an Attachment file"));
  sessionCodesTbl.insert(std::pair<int, string>((int)UriCreate, "Create a URI reference"));
  sessionCodesTbl.insert(std::pair<int, string>((int)ChangeActivity, "Change Activiti status"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)AssignComponent, "Assign a Component to Activity"));
  sessionCodesTbl.insert(std::pair<int, string>((int)ChangeParameter, "Change a Parameter Value"));
  sessionCodesTbl.insert(std::pair<int, string>((int)CreateComponent, "Create a new Component"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)PojectGetList, "Get the list of all Projects"));
  sessionCodesTbl.insert(std::pair<int, string>((int)MemberGetList, "Get the list of all Members"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)ComponentTypeList, "Get the Component Type list"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)ComponentType, "Get Component Type definition"));
  sessionCodesTbl.insert(std::pair<int, string>((int)Component, "Get a Component"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)ComponentList, "Get a list of Components of such type"));
  sessionCodesTbl.insert(std::pair<int, string>((int)ComponentActivities,
                                                "Get the list of activities of a component"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)ActivityTypeList, "Get the Activity Type list"));
  sessionCodesTbl.insert(
      std::pair<int, string>((int)ActivityType, "Get the Activity Type definition"));
  sessionCodesTbl.insert(std::pair<int, string>((int)Activity, "Get one Activity"));
}
/* --------------------
 *
 *    Formats the parameter value in Scientific/General notation
 *    and verify the represantation interval...
 *
 *
  -------------------- */
string AlpideTable::formatTheParameterValue(float value)
{
  FUNCTION_TRACE;
  char charBuffer[100];
  if (value > 7.921e+28) {
    cerr << "DB Format Parameter : value exceded the rappresantation interval: " << value
         << " cutted to 7.921e+28 !" << endl;
    WARNING("Parameter value %f > 7.9e+28", value);
    value = 7.921e+28;
  }
  else if (value < -7.921e+28) {
    cerr << "DB Format Parameter : value exceded the rappresantation interval: " << value
         << " cutted to -7.921e+28 !" << endl;
    WARNING("Parameter value %f < -7.9e+28", value);
    value = -7.921e+28;
  }
  else if (value < 7.921e-28 and value > 0) {
    cerr << "DB Format Parameter : value exceded the rappresantation interval: " << value
         << " cutted to 7.921e-28 !" << endl;
    WARNING("Parameter value %f < 7.9e-28", value);
    value = 7.921e-28;
  }
  else if (value > -7.921e-28 and value < 0) {
    cerr << "DB Format Parameter : value exceded the rappresantation interval: " << value
         << " cutted to -7.921e-28 !" << endl;
    WARNING("Parameter value %f > -7.9e-28", value);
    value = -7.921e-28;
  }
  if (isScienNotation) {
    double ap = (double)(value);
    sprintf(charBuffer, "%e", ap);
    TRACE("The parameter in scientific notation %f -> %s", ap, charBuffer);
    return (std::string(charBuffer));
  }
  else {
    TRACE("The parameter in float notation %f -> %s", value, float2str(value).c_str());
    return (float2str(value));
  }
}

/* --------------------------------------------------------
 *    Stores a response into the history of responses
 *
 *-------------------------------------------------------- */
void AlpideTable::pushResponse(AlpideTable::response *aResponse)
{
  FUNCTION_TRACE;
  if (aResponse == NULL) {
    theResponses.push_back(theResponse);
  }
  else {
    theResponses.push_back(*aResponse);
  }
  TRACE("The actual dimension of the responses vector is %d", theResponses.size());
  if (theResponses.size() > MAXRESPONSESHISTORY) {
    theResponses.erase(theResponses.begin());
    DEBUG("Deleted one element in the responses vector (%d)", theResponses.size());
  }
  return;
}

/* --------------------------------------------------------
 *    Parse the wrong XML response (an HTML page)
 *    in order to debug the error ....
 * TODO : !!!
 *
 *-------------------------------------------------------- */
AlpideTable::ErrorCode AlpideTable::ParseTheNonXMLResponse(char *ThePage)
{
  FUNCTION_TRACE;
  if (strstr(ThePage, "<h1>401 - Unauthorized</h1>") != NULL) {
    return InvalidCredential;
  }
  if (strstr(ThePage, "MSIS7042") != NULL) {
    return MSIS7042;
  }
  return (BadXML);
}
/* --------------------------------------------------------
 *    ProjectDB   Class to manage the Projects Table
 *
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *---------------- */
ProjectDB::ProjectDB(AlpideDB *DBhandle) : AlpideTable(DBhandle) {}

ProjectDB::~ProjectDB() {}

/* -----------------
 *   GetList := get the complete list of projects
 *
 *  Out Param : a Reference to a vector of Project struct that will contain all the
 *              projects
 *    returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *ProjectDB::GetList(vector<project> *Result)
{
  FUNCTION_TRACE;
  string theUrl   = theParentDB->GetQueryDomain() + "/ProjectRead";
  string theQuery = "";

  project pro;
  setResponseSession(AlpideTable::PojectGetList);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (theRootXMLNode == NULL) {
    return (&theResponse);
  }
  xmlNode *nod = theRootXMLNode->children;
  Result->clear();
  while (nod != NULL) {
    if (strcmp((const char *)nod->name, "Project") == 0) {
      xmlNode *n1 = nod->children;
      while (n1 != NULL) {
        if (MATCHNODE(n1, "ID"))
          pro.ID = atoi((const char *)n1->children->content);
        else if (MATCHNODE(n1, "Name"))
          pro.Name.assign((const char *)n1->children->content);
        n1 = n1->next;
      }
      Result->push_back(pro);
    }
    nod = nod->next;
  }
  DEBUG("Found %d defined projects.", Result->size());
  SetResponse(AlpideTable::NoError);
  return (&theResponse);
}

/* --------------------------------------------------------
 *    MemberDB   Class to manage the Members Table
 *
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *---------------- */
MemberDB::MemberDB(AlpideDB *DBhandle) : AlpideTable(DBhandle) {}

MemberDB::~MemberDB() {}

/* -----------------
 *    GetList := get the complete list of members
 *
 *    Out Param : a Reference to a vector of Member struct that will contain all the
 *                member
 *      returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *MemberDB::GetList(int projectID, vector<member> *Result)
{
  FUNCTION_TRACE;
  string theUrl   = theParentDB->GetQueryDomain() + "/ProjectMemberRead";
  string theQuery = "projectID=" + std::to_string(projectID);

  member pro;
  setResponseSession(AlpideTable::MemberGetList);

  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (theRootXMLNode == NULL) {
    return (&theResponse);
  }
  xmlNode *nod = theRootXMLNode->children;
  Result->clear();
  while (nod != NULL) {
    if (strcmp((const char *)nod->name, "ProjectMember") == 0) {
      xmlNode *n1 = nod->children;
      while (n1 != NULL) {
        if (MATCHNODE(n1, "ID"))
          pro.ID = atoi((const char *)n1->children->content);
        else if (MATCHNODE(n1, "PersonID"))
          pro.PersonalID = atoi((const char *)n1->children->content);
        else if (MATCHNODE(n1, "FullName"))
          pro.FullName.assign((const char *)n1->children->content);
        n1 = n1->next;
      }
      Result->push_back(pro);
    }
    nod = nod->next;
  }
  DEBUG("Found %d defined members for the project ID=%d.", Result->size(), projectID);
  SetResponse(AlpideTable::NoError);
  return (&theResponse);
}

/* --------------------------------------------------------
 *    ComponentDB   Class to manage the Components Table
 *
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *---------------- */
ComponentDB::ComponentDB(AlpideDB *DBhandle) : AlpideTable(DBhandle) {}

ComponentDB::~ComponentDB() {}

/* -----------------
 *    GetTypeList := get the complete list of all the component types defined
 *
 *       In Param : the ID of the Project
 *      Out Param : a Reference to a vector of ComponentType struct that will contain all
 *                  the component types
 *        returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *ComponentDB::GetTypeList(int ProjectID, vector<componentType> *Result)
{
  FUNCTION_TRACE;
  string theUrl   = theParentDB->GetQueryDomain() + "/ComponentTypeRead";
  string theQuery = "projectID=" + std::to_string(ProjectID);

  componentType pro;
  setResponseSession(AlpideTable::ComponentTypeList);

  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (theRootXMLNode == NULL) {
    return (&theResponse);
  }

  Result->clear();
  xmlNode *n1;
  xmlNode *nod = theRootXMLNode->children;
  while (nod != NULL) {
    if (strcmp((const char *)nod->name, "ComponentType") == 0) {
      n1 = nod->children;
      extractTheComponentType(n1, &pro);
      Result->push_back(pro);
    }
    nod = nod->next;
  }
  DEBUG("Found %d component types for the project ID=%d", Result->size(), ProjectID);
  SetResponse(AlpideTable::NoError);
  return (&theResponse);
}

/* -----------------
 *
 *    extractTheComponentType := get the component types definition
 *
 *    In Param : the XML node that contains the Type definition
 *   Out Param : a Reference to a ComponentType struct that will contains the values
 *               parsed
 *
 * ---------------- */
void ComponentDB::extractTheComponentType(xmlNode *ns, componentType *pro)
{
  FUNCTION_TRACE;
  xmlNode *n1, *n2, *n3, *n4;
  n1 = ns;

  while (n1 != NULL) {
    if (MATCHNODE(n1, "ID"))
      pro->ID = atoi((const char *)n1->children->content);
    else if (MATCHNODE(n1, "Name"))
      pro->Name.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "Code"))
      pro->Code.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "Description"))
      pro->Description.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "Composition")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ComponentTypeComposition")) {
          n3 = n2->children;
          composition ap1;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ComponentType")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  ap1.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Name"))
                  ap1.ComponentType.assign((const char *)n4->children->content);
                n4 = n4->next;
              }
            }
            else if (MATCHNODE(n3, "Quantity"))
              ap1.Quantity = atoi((const char *)n3->children->content);
            n3 = n3->next;
          }
          pro->Composition.push_back(ap1);
        }
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "PhysicalStatus")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "StatusPhysical")) {
          n3 = n2->children;
          statusphysical ap1;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              ap1.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Name"))
              ap1.Name.assign((const char *)n3->children->content);
            n3 = n3->next;
          }
          pro->PhysicalStatus.push_back(ap1);
        }
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "FunctionalStatus")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "StatusFunctional")) {
          n3 = n2->children;
          statusfunctional ap1;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              ap1.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Name"))
              ap1.Name.assign((const char *)n3->children->content);
            n3 = n3->next;
          }
          pro->FunctionalStatus.push_back(ap1);
        }
        n2 = n2->next;
      }
    }
    n1 = n1->next;
  }
}

/* -----------------
 *    GetType := get the complete definition of the requested component
 *
 *      In Param : the ID of the Component Type
 *     Out Param : a Reference to a ComponentType struct that will contain the component
 *                 type definition
 *       returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *ComponentDB::GetType(int ComponentTypeID, componentType *Result)
{
  FUNCTION_TRACE;
  string theUrl   = theParentDB->GetQueryDomain() + "/ComponentTypeReadAll";
  string theQuery = "componentTypeID=" + std::to_string(ComponentTypeID);

  componentType pro;
  setResponseSession(AlpideTable::ComponentType);

  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (theRootXMLNode == NULL) {
    return (&theResponse);
  }
  xmlNode *n1 = theRootXMLNode->children;
  extractTheComponentType(n1, Result);
  if (Result->ID == 0) {
    SetResponse(AlpideTable::RecordNotFound);
    pushResponse();
    WARNING("Component type ID=%d not found !", ComponentTypeID);
  }
  else {
    SetResponse(AlpideTable::NoError, Result->ID);
    DEBUG("Component type ID=%d found !", Result->ID);
  }
  return (&theResponse);
}

/* -----------------
 *    Create := A component type
 *
 *		In Param : ... all the items describing ...
 *		returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *ComponentDB::Create(string ComponentTypeID, string ComponentID,
                                           string SupplyCompID, string Description, string LotID,
                                           string PackageID, string UserID)
{
  FUNCTION_TRACE;
  string theUrl   = theParentDB->GetQueryDomain() + "/ComponentCreate";
  string theQuery = "componentTypeID=" + ComponentTypeID + "&componentID=" + ComponentID +
                    "&supplierComponentID=" + SupplyCompID + "&description=" + Description +
                    "&lotID=" + LotID + "&packageID=" + PackageID + "&userID=" + UserID;

  setResponseSession(AlpideTable::CreateComponent);

  if (!ExecuteQuery(theUrl, theQuery)) {
    isCreated = false;
    return (&theResponse);
  }
  if (!ParseQueryResult()) {
    isCreated = false;
    WARNING("Failed to create the component '%s' of type '%s' : %s", ComponentID.c_str(),
            ComponentTypeID.c_str(), theResponse.ErrorMessage.c_str());
    return (&theResponse);
  }
  if (VERBOSITYLEVEL == 1) cout << "Component creation :" << DumpResponse() << endl;
  isCreated = true;
  return (&theResponse);
}

/* -----------------
 * 	PRIVATE
 *    extractTheComponent := get the component definition
 *
 *		In Param : the XML node that contains the Component definition
 *		Out Param : a Reference to a Component struct that will contains the values parsed
 *
 * ---------------- */
void ComponentDB::extractTheComponent(xmlNode *ns, componentLong *pro)
{
  FUNCTION_TRACE;
  xmlNode *n1, *n2, *n3, *n4, *n5;
  n1 = ns;
  while (n1 != NULL) {
    DEBUG("The first node is '%s'", n1->name);
    if (MATCHNODE(n1, "ID"))
      pro->ID = atoi((const char *)n1->children->content);
    else if (MATCHNODE(n1, "ComponentID"))
      pro->ComponentID.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "SupplierComponentID"))
      pro->SupplierComponentID.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "Description"))
      pro->Description.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "LotID"))
      pro->LotID.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "PackageID"))
      pro->PackageID.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "ComponentType")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ID"))
          pro->Type.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Name"))
          pro->Type.Name.assign((const char *)n2->children->content);
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "PhysicalStatus")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ID"))
          pro->PhysicalState.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Name"))
          pro->PhysicalState.Name.assign((const char *)n2->children->content);
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "FunctionalStatus")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ID"))
          pro->FunctionalState.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Name"))
          pro->FunctionalState.Name.assign((const char *)n2->children->content);
        n2 = n2->next;
      }
    }
    if (MATCHNODE(n1, "Composition")) {
      n2 = n1->children;
      compComposition ap1;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ComponentComposition")) {
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              ap1.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Position"))
              ap1.Position.assign((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Component")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  ap1.Component.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "ComponentID"))
                  ap1.Component.ComponentID.assign((const char *)n4->children->content);
                else if (MATCHNODE(n4, "ComponentType")) {
                  n5 = n4->children;
                  while (n5 != NULL) {
                    if (MATCHNODE(n5, "ID"))
                      ap1.Component.ComponentType.ID = atoi((const char *)n5->children->content);
                    else if (MATCHNODE(n5, "Name"))
                      ap1.Component.ComponentType.Name.assign((const char *)n5->children->content);
                    n5 = n5->next;
                  }
                }
                n4 = n4->next;
              }
            }
            n3 = n3->next;
          }
          pro->Composition.push_back(ap1);
          zCOMPCOMPOSITION(ap1);
        }
        n2 = n2->next;
      }
    }
    n1 = n1->next;
  }
}

/* -----------------
 *    Read := Get a component
 *
 *		In Param : ... all the items describing ...
 *		returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *ComponentDB::Read(int ID, componentLong *Result)
{
  FUNCTION_TRACE;
  std::string sID = std::to_string(ID);
  return (readComponent(sID, "", Result));
}

AlpideTable::response *ComponentDB::Read(string ComponentID, componentLong *Result)
{
  FUNCTION_TRACE;
  return (readComponent("-999", ComponentID, Result));
}

AlpideTable::response *ComponentDB::readComponent(string ID, string ComponentID,
                                                  componentLong *Result)
{
  size_t index = ComponentID.find("&");
  if (index != std::string::npos) {
    ComponentID.replace(index, 1, "%26");
  }
  FUNCTION_TRACE;
  string theUrl   = theParentDB->GetQueryDomain() + "/ComponentReadOne";
  string theQuery = "ID=" + ID + "&componentID=" + ComponentID;

  setResponseSession(AlpideTable::Component);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (theRootXMLNode == NULL) {
    return (&theResponse);
  }
  xmlNode *n1 = theRootXMLNode->children;
  extractTheComponent(n1, Result);

  SetResponse(AlpideTable::NoError, Result->ID);
  return (&theResponse);
}

/* -----------------
 *    Read := Get a List of components
 *
 *		In Param : ...the componet type id...
 *		returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *ComponentDB::readComponents(std::string             ProjectId,
                                                   std::string             ComponentTypeID,
                                                   vector<componentShort> *compoList)
{
  FUNCTION_TRACE;
  string theUrl   = theParentDB->GetQueryDomain() + "/ComponentRead";
  string theQuery = "projectID=" + ProjectId + "&componentTypeID=" + ComponentTypeID;

  setResponseSession(AlpideTable::ComponentList);
  compoList->clear();
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (theRootXMLNode == NULL) {
    return (&theResponse);
  }
  xmlNode *n1, *n2, *n3;
  n1 = theRootXMLNode->children;
  componentShort theComponent;
  while (n1 != NULL) {
    if (MATCHNODE(n1, "Component")) {
      n2 = n1->children;
      zCOMPONENTS(theComponent);
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ID"))
          theComponent.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "ComponentID"))
          theComponent.ComponentID.assign((const char *)n2->children->content);
        else if (MATCHNODE(n2, "SupplierComponentID"))
          theComponent.SupplierComponentID.assign((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Description"))
          theComponent.Description.assign((const char *)n2->children->content);
        else if (MATCHNODE(n2, "LotID"))
          theComponent.LotID.assign((const char *)n2->children->content);
        else if (MATCHNODE(n2, "PackageID"))
          theComponent.PackageID.assign((const char *)n2->children->content);
        else if (MATCHNODE(n2, "LotID"))
          theComponent.LotID.assign((const char *)n2->children->content);
        if (MATCHNODE(n2, "PhysicalStatus")) {
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              theComponent.PhysicalState.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Name"))
              theComponent.PhysicalState.Name.assign((const char *)n3->children->content);
            n3 = n3->next;
          }
        }
        if (MATCHNODE(n2, "FunctionalStatus")) {
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              theComponent.FunctionalState.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Name"))
              theComponent.FunctionalState.Name.assign((const char *)n3->children->content);
            n3 = n3->next;
          }
        }
        n2 = n2->next;
      }
      compoList->push_back(theComponent);
    }
    n1 = n1->next;
  }
  SetResponse(AlpideTable::NoError);
  return (&theResponse);
}

/* -----------------
 *    GetListByType := Get a List of components
 *
 *
 *---------------- */
AlpideTable::response *ComponentDB::GetListByType(int ProjectID, int ComponentTypeID,
                                                  vector<componentShort> *Result)
{
  FUNCTION_TRACE;
  std::string sProject = std::to_string(ProjectID);
  std::string sTypeId  = std::to_string(ComponentTypeID);
  return (readComponents(sProject, sTypeId, Result));
}


/* -----------------
 *    ReadParents := Get the Parent component
 *
 *		In Param : the component ID of the children
 *		Out Params : a vector of IDs of all parents
 *
 *		returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *ComponentDB::ReadParents(int ID, vector<compComposition> *Parents)
{
  FUNCTION_TRACE;
  std::string sID = std::to_string(ID);

  Parents->clear();

  string theUrl   = theParentDB->GetQueryDomain() + "/ComponentParentRead";
  string theQuery = "ID=" + sID;

  setResponseSession(AlpideTable::Component);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (theRootXMLNode == NULL) {
    return (&theResponse);
  }

  compComposition comp;
  xmlNode *       n1, *n2, *n3, *n4;
  n1 = theRootXMLNode->children;
  while (n1 != NULL) {
    if (MATCHNODE(n1, "ComponentComposition")) {
      n2 = n1->children;
      zCOMPCOMPOSITION(comp);
      while (n2 != NULL) {
        if (MATCHNODE(n2, "Component")) {
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              comp.Component.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "ComponentID"))
              comp.Component.ComponentID.assign((const char *)n3->children->content);
            else if (MATCHNODE(n3, "ComponentType")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  comp.Component.ComponentType.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Name"))
                  comp.Component.ComponentType.Name.assign((const char *)n4->children->content);
                n4 = n4->next;
              }
            }
            n3 = n3->next;
          }
        }
        else if (MATCHNODE(n2, "ID"))
          comp.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Position"))
          comp.Position.assign((const char *)n2->children->content);
        n2 = n2->next;
      }
      Parents->push_back(comp);
    }
    n1 = n1->next;
  }
  SetResponse(AlpideTable::NoError);
  return (&theResponse);
}


/* -----------------
 *    Read := Get the activity list for a component activity
 *
 *		In Param : ...
 *---------------- */
AlpideTable::response *ComponentDB::GetComponentActivities(string                ComponentID,
                                                           vector<compActivity> *Result)
{
  FUNCTION_TRACE;
  componentLong          theComponent;
  AlpideTable::response *theRes = readComponent("-999", ComponentID, &theComponent);
  if (theRes->ErrorCode != 0) return (theRes);
  int ID = theComponent.ID;
  return (readComponentActivities(ID, Result));
}

AlpideTable::response *ComponentDB::GetComponentActivities(int ID, vector<compActivity> *Result)
{
  FUNCTION_TRACE;
  return (readComponentActivities(ID, Result));
}

/* -----------------
 *    extractTheActivityList := parses the doc to extract activities
 *
 *		In Param : ...
 *---------------- */
void ComponentDB::extractTheActivityList(xmlNode *ns, vector<compActivity> *actList)
{
  FUNCTION_TRACE;
  xmlNode *    n1, *n2, *n3;
  compActivity theAct;
  if (!ns->children) return;
  n1 = ns->children;
  while (n1 != NULL) {
    if (MATCHNODE(n1, "ComponentActivityHistory")) {
      n2 = n1->children;
      zCOMPACTIVITY(theAct);
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ActivityID"))
          theAct.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "ActivityName"))
          theAct.Name.assign((const char *)n2->children->content);
        else if (MATCHNODE(n2, "ActivityStartDate"))
          str2timeDate((const char *)(n2->children->content), &(theAct.StartDate));
        else if (MATCHNODE(n2, "ActivityEndDate"))
          str2timeDate((const char *)(n2->children->content), &(theAct.EndDate));
        if (MATCHNODE(n2, "ActivityResult")) {
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              theAct.Result.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Name"))
              theAct.Result.Name.assign((const char *)n3->children->content);
            n3 = n3->next;
          }
        }
        if (MATCHNODE(n2, "ActivityStatus")) {
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              theAct.Status.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Code"))
              theAct.Status.Code.assign((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Description"))
              theAct.Status.Description.assign((const char *)n3->children->content);
            n3 = n3->next;
          }
        }
        if (MATCHNODE(n2, "ActivityType")) {
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              theAct.Type = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Name"))
              theAct.Typename.assign((const char *)n3->children->content);
            n3 = n3->next;
          }
        }
        n2 = n2->next;
      }
      actList->push_back(theAct);
    }
    n1 = n1->next;
  }
}

/* -----------------
 *    readComponentActivities := read the activity history by components
 *
 *		In Param : ...
 *---------------- */
AlpideTable::response *ComponentDB::readComponentActivities(int ID, vector<compActivity> *Result)
{
  FUNCTION_TRACE;
  string   theUrl   = theParentDB->GetQueryDomain() + "/ComponentActivityHistoryRead";
  string   theQuery = "ID=" + std::to_string(ID);
  xmlNode *nod;

  setResponseSession(AlpideTable::ComponentActivities);
  Result->clear();
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (theRootXMLNode == NULL) {
    return (&theResponse);
  }
  nod = theRootXMLNode;
  extractTheActivityList(nod, Result);
  SetResponse(AlpideTable::NoError, ID); // for no root node -> empty
  return (&theResponse);
}

/* -----------------
 *    Dump := Dumps a human readable form of the Component
 *
 *		In Param : the component struct
 *		returns : a char pointer to a string buffer
 *---------------- */
string ComponentDB::Dump(componentLong *co)
{
  FUNCTION_TRACE;
  ap = "Component : ID=" + std::to_string(co->ID) + " Name=" + co->ComponentID + "\n" +
       "            Description=" + co->Description + "Type=" + co->Type.Name + "\n";
  ap += "   Composition : \n";
  for (unsigned int i = 0; i < co->Composition.size(); i++) {
    ap += "    ID = " + co->Composition.at(i).Component.ComponentID +
          " Position = " + co->Composition.at(i).Position + "\n";
  }
  ap += "    Functional status =" + co->FunctionalState.Name + "\n";
  ap += "    Physical status =" + co->PhysicalState.Name + "\n";
  return (ap);
}

/* -----------------
 *    Print := Dumps a human readable form of the Component Type definition
 *
 *		In Param : the component type struct
 *		returns : a char pointer to a string buffer
 *---------------- */
string ComponentDB::Dump(componentType *co) { return (Print(co)); }
string ComponentDB::Print(componentType *co)
{
  FUNCTION_TRACE;
  ap = "Component : ID=" + std::to_string(co->ID) + " Name=" + co->Name + " Code=" + co->Code +
       " Description=" + co->Description + "\n";
  ap += "   Composition : {";
  for (unsigned int i = 0; i < co->Composition.size(); i++)
    ap += "( ID=" + std::to_string(co->Composition.at(i).ID) +
          ",Type=" + co->Composition.at(i).ComponentType +
          ",Q.ty=" + std::to_string(co->Composition.at(i).Quantity) + ")";
  ap += "}\n";
  ap += "   Physical Status  : {";
  for (unsigned int i = 0; i < co->PhysicalStatus.size(); i++)
    ap += "( ID=" + std::to_string(co->PhysicalStatus.at(i).ID) +
          ",Name=" + co->PhysicalStatus.at(i).Name + ")";
  ap += "}\n";
  ap += "   Functional Status  : {";
  for (unsigned int i = 0; i < co->FunctionalStatus.size(); i++)
    ap += "( ID=" + std::to_string(co->FunctionalStatus.at(i).ID) +
          ",Name=" + co->FunctionalStatus.at(i).Name + ")";
  ap += "}\n";
  return (ap);
}

/* --------------------------------------------------------
 *    ActivityDB   Class to manage the Activity Table
 *
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *---------------- */
ActivityDB::ActivityDB(AlpideDB *DBhandle) : AlpideTable(DBhandle) {}

ActivityDB::~ActivityDB() {}

/* -----------------
 *    Create := Create a new activity record
 *
 *		In Param : the activity struct
 *		returns : a response struct that contains the error code
 *---------------- */

ActivityDB::response *ActivityDB::Create(activity *aActivity)
{
  FUNCTION_TRACE;
  char   DateBuffer[40];
  char   DateMask[40] = "%d/%m/%Y";
  bool   hasFailed    = false;
  string theUrl;
  string theQuery;
  // theResponses.clear();
  isCreated = true;
  theUrl    = theParentDB->GetQueryDomain() + "/ActivityCreate";
  theQuery  = "activityTypeID=" + std::to_string(aActivity->Type);
  theQuery += "&locationID=" + std::to_string(aActivity->Location);
  theQuery += "&lotID=" + aActivity->Lot;
  theQuery += "&activityName=" + aActivity->Name;
  strftime(DateBuffer, 40, DateMask, (const tm *)(localtime(&(aActivity->StartDate))));
  theQuery += "&startDate=";
  theQuery.append(DateBuffer);
  strftime(DateBuffer, 40, DateMask, (const tm *)(localtime(&(aActivity->EndDate))));
  theQuery += "&endDate=";
  theQuery.append(DateBuffer);
  theQuery += "&position=" + aActivity->Position;
  theQuery += "&resultID=" + std::to_string(aActivity->Result);
  theQuery += "&statusID=" + std::to_string(aActivity->Status);
  theQuery += "&userID=" + std::to_string(aActivity->User);

  setResponseSession(AlpideTable::CreateActivity);
  if (!ExecuteQuery(theUrl, theQuery)) {
    isCreated = false;
    return (&theResponse);
  }
  if (!ParseQueryResult()) {
    isCreated = false;
    return (&theResponse);
  }
  if (VERBOSITYLEVEL == 1) cout << "Activity creation :" << DumpResponse() << endl;
  aActivity->ID = theResponse.ID;
  isCreated     = true;

  setResponseSession(AlpideTable::MemberAssign);
  theUrl = theParentDB->GetQueryDomain() + "/ActivityMemberAssign";
  for (unsigned int i = 0; i < aActivity->Members.size(); i++) {
    theQuery = "projectMemberID=" + std::to_string(aActivity->Members.at(i).ProjectMember);
    theQuery += "&activityID=" + std::to_string(aActivity->ID);
    theQuery += "&leader=" + std::to_string(aActivity->Members.at(i).Leader);
    theQuery += "&userID=" + std::to_string(aActivity->Members.at(i).User);

    if (!ExecuteQuery(theUrl, theQuery)) {
      hasFailed = true;
    }
    else {
      if (!ParseQueryResult()) {
        hasFailed = true;
      }
      else {
        if (VERBOSITYLEVEL == 1) cout << "Activity Member creation  :" << DumpResponse() << endl;
        aActivity->Members.at(i).ID = theResponse.ID;
      }
    }
  }

  setResponseSession(AlpideTable::ParameterCreate);
  theUrl = theParentDB->GetQueryDomain() + "/ActivityParameterCreate";
  for (unsigned int i = 0; i < aActivity->Parameters.size(); i++) {
    theQuery = "activityID=" + std::to_string(aActivity->ID);
    theQuery +=
        "&activityParameterID=" + std::to_string(aActivity->Parameters.at(i).ActivityParameter);
    theQuery += "&value=";
    theQuery += formatTheParameterValue(aActivity->Parameters.at(i).Value);
    theQuery += "&userID=" + std::to_string(aActivity->Parameters.at(i).User);

    if (!ExecuteQuery(theUrl, theQuery)) {
      hasFailed = true;
      cerr << "Activity Parameter Error (id " << aActivity->Parameters.at(i).ActivityParameter
           << "=" << aActivity->Parameters.at(i).Value << ")  : " << DumpResponse() << endl;
    }
    else {
      if (!ParseQueryResult()) {
        hasFailed = true;
      }
      else {
        if (VERBOSITYLEVEL == 1) cout << "Activity Parameter creation :" << DumpResponse() << endl;
        aActivity->Parameters.at(i).ID = theResponse.ID;
      }
    }
  }

  setResponseSession(AlpideTable::AttachmentCreate);
  theUrl = theParentDB->GetQueryDomain(); //+ "/ActivityAttachmentCreate";
  unsigned long theBase64Result;
  for (unsigned int i = 0; i < aActivity->Attachments.size(); i++) {
    theQuery = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
    if (SOAPVERSION == 11) {
      theQuery += "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                  "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
                  "xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">";
      theQuery += "<soap:Body><ActivityAttachmentCreate xmlns=\"http://tempuri.org/\"><activityID>";
      theQuery += std::to_string(aActivity->ID);
      theQuery += "</activityID><attachmentCategoryID>";
      theQuery += std::to_string(aActivity->Attachments.at(i).Category);
      theQuery += "</attachmentCategoryID><file>";
      theBase64Result = buildBase64Binary(aActivity->Attachments.at(i).LocalFileName, &theQuery);
      theQuery += "</file><fileName>";
      theQuery += aActivity->Attachments.at(i).RemoteFileName;
      theQuery += "</fileName><userID>";
      theQuery += std::to_string(aActivity->Attachments.at(i).User);
      theQuery += "</userID></ActivityAttachmentCreate></soap:Body></soap:Envelope>";
    }
    else {
      theQuery += "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                  "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
                  "xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">";
      theQuery +=
          "<soap12:Body><ActivityAttachmentCreate xmlns=\"http://tempuri.org/\"><activityID>";
      theQuery += std::to_string(aActivity->ID);
      theQuery += "</activityID><attachmentCategoryID>";
      theQuery += std::to_string(aActivity->Attachments.at(i).Category);
      theQuery += "</attachmentCategoryID><file>";
      theBase64Result = buildBase64Binary(aActivity->Attachments.at(i).LocalFileName, &theQuery);
      theQuery += "</file><fileName>";
      theQuery += aActivity->Attachments.at(i).RemoteFileName;
      theQuery += "</fileName><userID>";
      theQuery += std::to_string(aActivity->Attachments.at(i).User);
      theQuery += "</userID></ActivityAttachmentCreate></soap12:Body></soap12:Envelope>";
    }
    if (VERBOSITYLEVEL == 1)
      cout << "Base64 encoding of Attachment return a len of bytes = " << theBase64Result << endl;

    if (!ExecuteQuery(theUrl, theQuery, true, "http://tempuri.org/ActivityAttachmentCreate")) {
      hasFailed = true;
    }
    else {
      if (!ParseQueryResult()) {
        hasFailed = true;
      }
      else {
        if (VERBOSITYLEVEL == 1) cout << "Activity Attachment creation :" << DumpResponse() << endl;
        aActivity->Attachments.at(i).ID = theResponse.ID;
      }
    }
  }

  setResponseSession(AlpideTable::CreateActivity);
  if (hasFailed) {
    SetResponse(AlpideTable::BadCreation, aActivity->ID);
  }
  else {
    SetResponse(AlpideTable::NoError, aActivity->ID);
  }
  return (&theResponse);
}

/* -----------------
 *    AssignUris := Create/Remove change Uris list
 *
 *		In Param : the activity ID
 *				   the update list of URIs
 *		returns : a char pointer to a string buffer
 *---------------- */
AlpideTable::response *ActivityDB::AssignUris(int aActivityID, int aUserId,
                                              vector<ActivityDB::actUri> *aUris)
{
  FUNCTION_TRACE;
  string theUrl;
  string theQuery;

  bool                   hasFailed = false;
  activityLong           theActivity;
  AlpideTable::response *theResult;

  setResponseSession(AlpideTable::UriCreate);
  // First : read the activity and obtain the actual URIs list
  theResult = Read(aActivityID, &theActivity);
  if (theResult->ErrorCode != AlpideTable::NoError) {
    if (VERBOSITYLEVEL == 1) cout << "Invalid activity ID !" << DumpResponse() << endl;
    return (&theResponse);
  }

  // Search for URIs to remove or change
  bool bToDelete;
  for (unsigned int i = 0; i < theActivity.Uris.size(); i++) {
    bToDelete = true;
    for (unsigned int j = 0; j < aUris->size(); j++) {
      if (aUris->at(j).Path == theActivity.Uris.at(i).Path) {                 // Find a URI
        if (aUris->at(j).Description != theActivity.Uris.at(i).Description) { // It is to change
          theUrl   = theParentDB->GetQueryDomain() + "/ActivityUriChange";
          theQuery = "activitysUriID=" + std::to_string(theActivity.Uris.at(i).ID);
          theQuery += "&uriPath=" + aUris->at(j).Path;
          theQuery += "&uriDescription=" + aUris->at(j).Description;
          theQuery += "&userID=" + std::to_string(aUserId);

          if (!ExecuteQuery(theUrl, theQuery)) {
            return (&theResponse);
          }
          if (!ParseQueryResult()) {
            hasFailed = true;
          }
        }
        bToDelete = false; // mark this for not delete !
        break;
      }
    }
    if (bToDelete) { // Trigger URI remove
      theUrl   = theParentDB->GetQueryDomain() + "/ActivityUriRemove";
      theQuery = "uriID=" + std::to_string(theActivity.Uris.at(i).ID);
      if (!ExecuteQuery(theUrl, theQuery)) {
        return (&theResponse);
      }
      if (!ParseQueryResult()) {
        hasFailed = true;
      }
    }
  }

  // Search for URIs to create
  bool bToCreate;
  for (unsigned int j = 0; j < aUris->size(); j++) {
    bToCreate = true;
    for (unsigned int i = 0; i < theActivity.Uris.size(); i++) {
      if (aUris->at(j).Path == theActivity.Uris.at(i).Path) { // we find, not create it !
        bToCreate = false;
        break;
      }
    }
    if (bToCreate) {
      theUrl   = theParentDB->GetQueryDomain() + "/ActivityUriCreate";
      theQuery = "activityID=" + std::to_string(aActivityID);
      theQuery += "&uriPath=" + aUris->at(j).Path;
      theQuery += "&uriDescription=" + aUris->at(j).Description;
      theQuery += "&userID=" + std::to_string(aUserId);
      if (!ExecuteQuery(theUrl, theQuery)) {
        return (&theResponse);
      }
      if (!ParseQueryResult()) {
        hasFailed = true;
      }
    }
  }
  if (hasFailed)
    SetResponse(AlpideTable::BadCreation, aActivityID);
  else
    SetResponse(AlpideTable::NoError, aActivityID);
  if (VERBOSITYLEVEL == 1) cout << "Activity URIs change Done !" << DumpResponse() << endl;
  return (&theResponse);
}

ActivityDB::response *ActivityDB::Change(activity *aActivity)
{
  FUNCTION_TRACE;
  char DateBuffer[40];
  char DateMask[40] = "%d/%m/%Y";

  string theUrl;
  string theQuery;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityChange";
  theQuery = "ID=" + std::to_string(aActivity->ID);
  theQuery += "&activityTypeID=" + std::to_string(aActivity->Type);
  theQuery += "&locationID=" + std::to_string(aActivity->Location);
  theQuery += "&lotID=" + aActivity->Lot;
  theQuery += "&activityName=" + aActivity->Name;
  strftime(DateBuffer, 40, DateMask, (const tm *)(localtime(&(aActivity->StartDate))));
  theQuery += "&startDate=";
  theQuery.append(DateBuffer);
  strftime(DateBuffer, 40, DateMask, (const tm *)(localtime(&(aActivity->EndDate))));
  theQuery += "&endDate=";
  theQuery.append(DateBuffer);
  theQuery += "&position=" + aActivity->Position;
  theQuery += "&resultID=" + std::to_string(aActivity->Result);
  theQuery += "&statusID=" + std::to_string(aActivity->Status);
  theQuery += "&userID=" + std::to_string(aActivity->User);

  setResponseSession(AlpideTable::ChangeActivity);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  if (ParseQueryResult()) {
    aActivity->ID = theResponse.ID;
  }
  return (&theResponse);
}
/* -----------------
 *    AssignComponent := Add a new component to a defined activity
 *
 *		In Param : the activity ID
 *					the Component ID
 *					the Key for the COmponent Type
 *					The ID of the USER
 *		returns : a char pointer to a response type
 *---------------- */
ActivityDB::response *ActivityDB::AssignComponent(int aActivityID, int aComponentID,
                                                  int aComponentTypeID, int aUserID)
{
  FUNCTION_TRACE;
  string theUrl;
  string theQuery;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityComponentAssign";
  theQuery = "componentID=" + std::to_string(aComponentID);
  theQuery += "&activityID=" + std::to_string(aActivityID);
  theQuery += "&actTypeCompTypeID=" + std::to_string(aComponentTypeID);
  theQuery += "&userID=" + std::to_string(aUserID);

  setResponseSession(AlpideTable::AssignComponent);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (&theResponse);
  }
  ParseQueryResult();
  return (&theResponse);
}

/* ----------------------
 * Get the list of type parameters ...
 *
 *
----------------------- */
std::vector<ActivityDB::parameterType> *ActivityDB::GetParameterTypeList(int aActivityTypeID)
{
  vector<parameterType> *theParamList = new vector<parameterType>;

  string        theUrl;
  string        theQuery;
  parameterType param;
  xmlNode *     nod;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
  theQuery = "activityTypeID=" + std::to_string(aActivityTypeID);

  setResponseSession(AlpideTable::ActivityType);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (theParamList);
  }
  if (theRootXMLNode != NULL) {
    nod = theRootXMLNode->children;
    while (nod != NULL) {
      if (MATCHNODE(nod, "Parameters")) {
        xmlNode *n1 = nod->children;
        while (n1 != NULL) {
          if (MATCHNODE(n1, "ActivityTypeParameter")) {
            xmlNode *n2 = n1->children;
            zPARAMETERTYPE(param);
            while (n2 != NULL) {
              if (MATCHNODE(n2, "ID"))
                param.ParameterID = atoi((const char *)(n2->children->content));
              else if (MATCHNODE(n2, "Parameter")) {
                xmlNode *n3 = n2->children;
                while (n3 != NULL) {
                  if (MATCHNODE(n3, "ID"))
                    param.ID = atoi((const char *)(n3->children->content));
                  else if (MATCHNODE(n3, "Name"))
                    param.Name = (const char *)(n3->children->content);
                  else if (MATCHNODE(n3, "Description"))
                    param.Description = (const char *)(n3->children->content);
                  n3 = n3->next;
                }
                theParamList->push_back(param);
              }
              n2 = n2->next;
            }
          }
          n1 = n1->next;
        }
      }
      nod = nod->next;
    }
  }
  SetResponse(AlpideTable::NoError, aActivityTypeID);
  return (theParamList);
}

/* ----------------------
 * Get the list of type activity ...
 *
 *
----------------------- */
std::vector<ActivityDB::activityType> *ActivityDB::GetActivityTypeList(int aProjectID)
{
  FUNCTION_TRACE;
  vector<activityType> *theTypeList = new vector<activityType>;

  string       theUrl;
  string       theQuery;
  activityType act;
  xmlNode *    nod;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityTypeRead";
  theQuery = "projectID=" + std::to_string(aProjectID);

  setResponseSession(AlpideTable::ActivityTypeList);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (theTypeList);
  }
  if (theRootXMLNode != NULL and MATCHNODE(theRootXMLNode, "ArrayOfActivityType")) {
    DEBUG("We have a list of activity types... (%d)", theRootXMLNode);
    nod = theRootXMLNode->children;
    while (nod != NULL) {
      if (MATCHNODE(nod, "ActivityType")) {
        TRACE("Inspect node name %s", nod->name);
        xmlNode *n1 = nod->children;
        zACTIVITYTYPE(act);
        while (n1 != NULL) {
          if (MATCHNODE(n1, "ID"))
            act.ID = atoi((const char *)(n1->children->content));
          else if (MATCHNODE(n1, "Name"))
            act.Name = (const char *)(n1->children->content);
          else if (MATCHNODE(n1, "Description"))
            act.Description = (const char *)(n1->children->content);
          n1 = n1->next;
        }
        theTypeList->push_back(act);
      }
      nod = nod->next;
    }
  }
  SetResponse(AlpideTable::NoError, aProjectID);
  return (theTypeList);
}

/* ----------------------
 * Get the list of type location ...
 *
 *
----------------------- */
std::vector<ActivityDB::locationType> *ActivityDB::GetLocationTypeList(int aActivityTypeID)
{
  FUNCTION_TRACE;
  vector<locationType> *theLocationList = new vector<locationType>;

  string       theUrl;
  string       theQuery;
  locationType loc;
  xmlNode *    nod;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
  theQuery = "activityTypeID=" + std::to_string(aActivityTypeID);

  setResponseSession(AlpideTable::ActivityType);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (theLocationList);
  }
  if (theRootXMLNode != NULL) {
    nod = theRootXMLNode->children;
    while (nod != NULL) {
      if (MATCHNODE(nod, "Location")) {
        xmlNode *n1 = nod->children;
        while (n1 != NULL) {
          if (MATCHNODE(n1, "ActivityTypeLocation")) {
            xmlNode *n2 = n1->children;
            zLOCATIONTYPE(loc);
            while (n2 != NULL) {
              if (MATCHNODE(n2, "ID"))
                loc.ID = atoi((const char *)(n2->children->content));
              else if (MATCHNODE(n2, "Name"))
                loc.Name = (const char *)(n2->children->content);
              n2 = n2->next;
            }
            theLocationList->push_back(loc);
          }
          n1 = n1->next;
        }
      }
      nod = nod->next;
    }
  }
  SetResponse(AlpideTable::NoError, aActivityTypeID);
  return (theLocationList);
}

/* ----------------------
 * Get the list of type attachment ...
 *
 *
----------------------- */
std::vector<ActivityDB::attachmentType> *ActivityDB::GetAttachmentTypeList()
{
  FUNCTION_TRACE;
  vector<attachmentType> *theAttachmentList = new vector<attachmentType>;

  string         theUrl;
  string         theQuery;
  attachmentType att;
  xmlNode *      nod;

  theUrl   = theParentDB->GetQueryDomain() + "/AttachmentCategoryRead";
  theQuery = "";

  setResponseSession(AlpideTable::ActivityType);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (theAttachmentList);
  }
  if (theRootXMLNode != NULL and MATCHNODE(theRootXMLNode, "ArrayOfAttachmentCatagory")) {
    nod = theRootXMLNode->children;
    while (nod != NULL) {
      if (MATCHNODE(nod, "AttachmentCatagory")) {
        xmlNode *n1 = nod->children;
        while (n1 != NULL) {
          if (MATCHNODE(n1, "ID"))
            att.ID = atoi((const char *)(n1->children->content));
          else if (MATCHNODE(n1, "Category"))
            att.Category = (const char *)(n1->children->content);
          else if (MATCHNODE(n1, "Description"))
            att.Description = (const char *)(n1->children->content);
          n1 = n1->next;
        }
        theAttachmentList->push_back(att);
      }
      nod = nod->next;
    }
  }
  SetResponse(AlpideTable::NoError, 0);
  return (theAttachmentList);
}

/* ----------------------
 * Get the list of type component ...
 *
 *
----------------------- */
std::vector<ActivityDB::actTypeCompType> *ActivityDB::GetComponentTypeList(int aActivityTypeID)
{
  FUNCTION_TRACE;
  vector<actTypeCompType> *theCompoList = new vector<actTypeCompType>;

  string          theUrl;
  string          theQuery;
  actTypeCompType comp;
  xmlNode *       nod;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
  theQuery = "activityTypeID=" + std::to_string(aActivityTypeID);

  setResponseSession(AlpideTable::ActivityType);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (theCompoList);
  }
  if (theRootXMLNode != NULL) {
    nod = theRootXMLNode->children;
    while (nod != NULL) {
      if (MATCHNODE(nod, "ActivityTypeComponentType")) {
        xmlNode *n1 = nod->children;
        while (n1 != NULL) {
          if (MATCHNODE(n1, "ActivityTypeComponentTypeFull")) {
            xmlNode *n2 = n1->children;
            zACTTYPECOMPTYPE(comp);
            while (n2 != NULL) {
              if (MATCHNODE(n2, "ID"))
                comp.ID = atoi((const char *)(n2->children->content));
              else if (MATCHNODE(n2, "Quantity"))
                comp.Quantity = atoi((const char *)(n2->children->content));
              else if (MATCHNODE(n2, "Direction"))
                comp.Direction = (const char *)(n2->children->content);
              else if (MATCHNODE(n2, "ComponentType")) {
                xmlNode *n3 = n2->children;
                while (n3 != NULL) {
                  if (MATCHNODE(n3, "ID"))
                    comp.Type.ID = atoi((const char *)(n3->children->content));
                  else if (MATCHNODE(n3, "Name"))
                    comp.Type.Name = (const char *)(n3->children->content);
                  n3 = n3->next;
                }
              }
              n2 = n2->next;
            }
            theCompoList->push_back(comp);
          }
          n1 = n1->next;
        }
      }
      nod = nod->next;
    }
  }
  SetResponse(AlpideTable::NoError, aActivityTypeID);
  return (theCompoList);
}

/* ----------------------
 * Get the list of type results ...
 *
 *
----------------------- */
std::vector<ActivityDB::resultType> *ActivityDB::GetResultList(int aActivityTypeID)
{
  FUNCTION_TRACE;
  vector<resultType> *theResultList = new vector<resultType>;

  string     theUrl;
  string     theQuery;
  resultType resu;
  xmlNode *  nod;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
  theQuery = "activityTypeID=" + std::to_string(aActivityTypeID);

  setResponseSession(AlpideTable::ActivityType);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (theResultList);
  }
  if (theRootXMLNode != NULL) {
    nod = theRootXMLNode->children;
    while (nod != NULL) {
      if (MATCHNODE(nod, "Result")) {
        xmlNode *n1 = nod->children;
        while (n1 != NULL) {
          if (MATCHNODE(n1, "ActivityTypeResultFull")) {
            xmlNode *n2 = n1->children;
            zRESULTTYPE(resu);
            while (n2 != NULL) {
              if (MATCHNODE(n2, "ID"))
                resu.ID = atoi((const char *)(n2->children->content));
              else if (MATCHNODE(n2, "Name"))
                resu.Name = (const char *)(n2->children->content);
              n2 = n2->next;
            }
            theResultList->push_back(resu);
          }
          n1 = n1->next;
        }
      }
      nod = nod->next;
    }
  }
  SetResponse(AlpideTable::NoError, aActivityTypeID);
  return (theResultList);
}

/* ----------------------
 * Get the list of type results ...
 *
 *
----------------------- */
std::vector<ActivityDB::statusType> *ActivityDB::GetStatusList(int aActivityTypeID)
{
  FUNCTION_TRACE;
  vector<statusType> *theStatusList = new vector<statusType>;

  string     theUrl;
  string     theQuery;
  statusType stat;
  xmlNode *  nod;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
  theQuery = "activityTypeID=" + std::to_string(aActivityTypeID);

  setResponseSession(AlpideTable::ActivityType);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (theStatusList);
  }
  if (theRootXMLNode != NULL) {
    nod = theRootXMLNode->children;
    while (nod != NULL) {
      if (MATCHNODE(nod, "Status")) {
        xmlNode *n1 = nod->children;
        while (n1 != NULL) {
          if (MATCHNODE(n1, "ActivityStatus")) {
            xmlNode *n2 = n1->children;
            zSTATUSTYPE(stat);
            while (n2 != NULL) {
              if (MATCHNODE(n2, "ID"))
                stat.ID = atoi((const char *)(n2->children->content));
              else if (MATCHNODE(n2, "Code"))
                stat.Code = (const char *)(n2->children->content);
              else if (MATCHNODE(n2, "Description"))
                stat.Description = (const char *)(n2->children->content);
              n2 = n2->next;
            }
            theStatusList->push_back(stat);
          }
          n1 = n1->next;
        }
      }
      nod = nod->next;
    }
  }
  SetResponse(AlpideTable::NoError, aActivityTypeID);
  return (theStatusList);
}

/* ----------------------
 * Get the list of activities ...
 *
 *
----------------------- */
std::vector<ActivityDB::activityShort> *ActivityDB::GetActivityList(int aProjectID,
                                                                    int aActivityTypeID)
{
  FUNCTION_TRACE;
  vector<activityShort> *theActList = new vector<activityShort>;

  string        theUrl;
  string        theQuery;
  activityShort act;
  xmlNode *     nod;

  theUrl   = theParentDB->GetQueryDomain() + "/ActivityRead";
  theQuery = "projectID=" + std::to_string(aProjectID) +
             "&activityTypeID=" + std::to_string(aActivityTypeID);

  setResponseSession(AlpideTable::ActivityTypeList);
  if (!ExecuteQuery(theUrl, theQuery)) {
    return (theActList);
  }
  if (theRootXMLNode != NULL and MATCHNODE(theRootXMLNode, "ArrayOfActivity")) {
    nod = theRootXMLNode->children;
    DEBUG("We have a list of activity ... (%d)", theRootXMLNode);
    while (nod != NULL) {
      if (MATCHNODE(nod, "Activity")) {
        xmlNode *n1 = nod->children;
        zACTIVITYSHORT(act);
        while (n1 != NULL) {
          if (MATCHNODE(n1, "ID"))
            act.ID = atoi((const char *)(n1->children->content));
          else if (MATCHNODE(n1, "Name"))
            act.Name = (const char *)(n1->children->content);
          else if (MATCHNODE(n1, "StartDate"))
            str2timeDate((const char *)(n1->children->content), &act.StartDate);
          else if (MATCHNODE(n1, "EndDate"))
            str2timeDate((const char *)(n1->children->content), &act.EndDate);
          else if (MATCHNODE(n1, "ActivityType")) {
            xmlNode *n2 = n1->children;
            while (n2 != NULL) {
              if (MATCHNODE(n2, "ID"))
                act.Type.ID = atoi((const char *)(n2->children->content));
              else if (MATCHNODE(n2, "Name"))
                act.Type.Name = (const char *)(n2->children->content);
              else if (MATCHNODE(n2, "Description"))
                act.Type.Description = (const char *)(n2->children->content);
              n2 = n2->next;
            }
          }
          if (MATCHNODE(n1, "ActivityStatus")) {
            xmlNode *n2 = n1->children;
            while (n2 != NULL) {
              if (MATCHNODE(n2, "ID"))
                act.Status.ID = atoi((const char *)(n2->children->content));
              else if (MATCHNODE(n2, "Code"))
                act.Status.Code = (const char *)(n2->children->content);
              else if (MATCHNODE(n2, "Description"))
                act.Status.Description = (const char *)(n2->children->content);
              n2 = n2->next;
            }
          }
          n1 = n1->next;
        }
        theActList->push_back(act);
      }
      nod = nod->next;
    }
  }
  SetResponse(AlpideTable::NoError, aActivityTypeID);
  return (theActList);
}

/* -----------------
 *    Read := Get an activity
 *
 *		In Param : ...
 *		returns : a response struct that contains the error code
 *---------------- */
void ActivityDB::extractTheActivity(xmlNode *ns, activityLong *act)
{
  FUNCTION_TRACE;
  xmlNode *n1, *n2, *n3, *n4, *n5;
  n1 = ns;
  while (n1 != NULL) {
    if (MATCHNODE(n1, "ID"))
      act->ID = atoi((const char *)n1->children->content);
    else if (MATCHNODE(n1, "Name"))
      act->Name.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "LotID"))
      act->LotID.assign((const char *)n1->children->content);
    else if (MATCHNODE(n1, "StartDate"))
      str2timeDate((const char *)(n1->children->content), &act->StartDate);
    else if (MATCHNODE(n1, "EndDate"))
      str2timeDate((const char *)(n1->children->content), &act->EndDate);
    else if (MATCHNODE(n1, "ActivityResult")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ID"))
          act->Result.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Name"))
          act->Result.Name.assign((const char *)n2->children->content);
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "ActivityType")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ID"))
          act->Type.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Name"))
          act->Type.Name.assign((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Description"))
          act->Type.Description.assign((const char *)n2->children->content);
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "ActivityStatus")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ID"))
          act->Status.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Code"))
          act->Status.Code.assign((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Description"))
          act->Status.Description.assign((const char *)n2->children->content);
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "ActivityLocation")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ID"))
          act->Location.ID = atoi((const char *)n2->children->content);
        else if (MATCHNODE(n2, "Name"))
          act->Location.Name.assign((const char *)n2->children->content);
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "Components")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ActivityComponent")) {
          actComponent com;
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              com.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Conformity"))
              com.Conformity = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "ActivityTypeComponentType")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  com.ActivityComponentType.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Quantity"))
                  com.ActivityComponentType.Quantity = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Direction"))
                  com.ActivityComponentType.Direction.assign((const char *)n4->children->content);
                n4 = n4->next;
              }
            }
            else if (MATCHNODE(n3, "Component")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  com.Component.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "ComponentID"))
                  com.Component.ComponentID.assign((const char *)n4->children->content);
                else if (MATCHNODE(n4, "ComponentType")) {
                  n5 = n4->children;
                  while (n5 != NULL) {
                    if (MATCHNODE(n5, "ID"))
                      com.Component.Type.ID = atoi((const char *)n5->children->content);
                    else if (MATCHNODE(n5, "Name"))
                      com.Component.Type.Name.assign((const char *)n5->children->content);
                    n5 = n5->next;
                  }
                }
                n4 = n4->next;
              }
            }
            else if (MATCHNODE(n3, "PhysicalStatus")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  com.PhysicalStatus.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Name"))
                  com.PhysicalStatus.Name.assign((const char *)n4->children->content);
                n4 = n4->next;
              }
            }
            else if (MATCHNODE(n3, "FunctionalStatus")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  com.FunctionalStatus.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Name"))
                  com.FunctionalStatus.Name.assign((const char *)n4->children->content);
                n4 = n4->next;
              }
            }
            n3 = n3->next;
          }
          act->Components.push_back(com);
          zACTCOMPONENT(com);
        }
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "Parameters")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ActivityParameter")) {
          actParameter par;
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              par.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Value"))
              par.Value = atof((const char *)n3->children->content);
            else if (MATCHNODE(n3, "ActivityTypeParameter")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  par.Type.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Parameter")) {
                  n5 = n4->children;
                  while (n5 != NULL) {
                    if (MATCHNODE(n5, "ID"))
                      par.Type.Parameter.ID = atoi((const char *)n5->children->content);
                    else if (MATCHNODE(n5, "Description"))
                      par.Type.Parameter.Description.assign((const char *)n5->children->content);
                    else if (MATCHNODE(n5, "Name"))
                      par.Type.Parameter.Name.assign((const char *)n5->children->content);
                    n5 = n5->next;
                  }
                }
                n4 = n4->next;
              }
            }
            n3 = n3->next;
          }
          act->Parameters.push_back(par);
          zACTPARAMETER(par);
        }
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "Attachments")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ActivityAttachment")) {
          actAttachment att;
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              att.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "FileName"))
              att.FileName.assign((const char *)n3->children->content);
            else if (MATCHNODE(n3, "AttachmentCatagory")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  att.Type.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Category"))
                  att.Type.Category.assign((const char *)n4->children->content);
                else if (MATCHNODE(n4, "Description"))
                  att.Type.Description.assign((const char *)n4->children->content);
                n4 = n4->next;
              }
            }
            n3 = n3->next;
          }
          act->Attachments.push_back(att);
          zACTATTACHMENT(att);
        }
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "Members")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ActivityMember")) {
          actMember mem;
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              mem.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Leader"))
              mem.Leader = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Member")) {
              n4 = n3->children;
              while (n4 != NULL) {
                if (MATCHNODE(n4, "ID"))
                  mem.Member.ID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "PersonID"))
                  mem.Member.PersonID = atoi((const char *)n4->children->content);
                else if (MATCHNODE(n4, "FullName"))
                  mem.Member.FullName.assign((const char *)n4->children->content);
                n4 = n4->next;
              }
            }
            n3 = n3->next;
          }
          act->Members.push_back(mem);
          zACTMEMBER(mem);
        }
        n2 = n2->next;
      }
    }
    else if (MATCHNODE(n1, "Uris")) {
      n2 = n1->children;
      while (n2 != NULL) {
        if (MATCHNODE(n2, "ActivityUri")) {
          actUri uri;
          n3 = n2->children;
          while (n3 != NULL) {
            if (MATCHNODE(n3, "ID"))
              uri.ID = atoi((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Path"))
              uri.Path.assign((const char *)n3->children->content);
            else if (MATCHNODE(n3, "Description"))
              uri.Description.assign((const char *)n3->children->content);
            n3 = n3->next;
          }
          act->Uris.push_back(uri);
          zACTURI(uri);
        }
        n2 = n2->next;
      }
    }
    n1 = n1->next;
  }
}

AlpideTable::response *ActivityDB::Read(int ActivityID, activityLong *Result)
{
  FUNCTION_TRACE;
  std::string sID = std::to_string(ActivityID);
  return (readActivity(sID, Result));
}

AlpideTable::response *ActivityDB::readActivity(string ID, activityLong *Result)
{
  FUNCTION_TRACE;
  string theUrl   = theParentDB->GetQueryDomain() + "/ActivityReadOne";
  string theQuery = "ID=" + ID;

  setResponseSession(AlpideTable::Activity);
  if (ExecuteQuery(theUrl, theQuery)) {
    if (theRootXMLNode != NULL) {
      extractTheActivity(theRootXMLNode->children, Result);
      SetResponse(AlpideTable::NoError, Result->ID);
    }
  }
  return (&theResponse);
}

/* ----------------------
 * Change the value of one parameter
 *
 *
----------------------- */
AlpideTable::response *ActivityDB::ChangeParameter(int ActivityID, string parameterName,
                                                   float newValue, int userID)
{
  FUNCTION_TRACE;
  activityLong Activity;
  setResponseSession(AlpideTable::ChangeParameter);
  Read(ActivityID, &Activity);
  if (theResponse.ErrorCode == 0) {
    int   parameterID = 0;
    float oldParameterValue;
    for (unsigned int i = 0; i < Activity.Parameters.size(); i++) {
      if (Activity.Parameters.at(i).Type.Parameter.Name == parameterName) {
        parameterID       = Activity.Parameters.at(i).ID;
        oldParameterValue = Activity.Parameters.at(i).Value;
        DEBUG("The parameter '%s' ID=%d has the value = %f", parameterName.c_str(), parameterID,
              oldParameterValue);

        string theUrl   = theParentDB->GetQueryDomain() + "/ActivityParameterChange";
        string theQuery = "ID=" + std::to_string(parameterID);
        theQuery += "&value=" + formatTheParameterValue(newValue);
        theQuery += "&userID=" + std::to_string(userID);

        if (!ExecuteQuery(theUrl, theQuery)) {
          return (&theResponse);
        }
        if (!ParseQueryResult()) {
          return (&theResponse);
        }
        if (VERBOSITYLEVEL == 1) cout << "Parameter changed :" << DumpResponse() << endl;
        DEBUG("The parameter '%s' ID=%d has the new value = %f", parameterName.c_str(), parameterID,
              newValue);
        return (&theResponse);
      }
    }
    WARNING("The parameter '%s' isn't defined in the activity ID=%d !", parameterName.c_str(),
            ActivityID);
    SetResponse(AlpideTable::RecordNotFound, ActivityID);
    pushResponse();
    return (&theResponse);
  }
  else {
    cerr << "Change Parameter : Activity ID=" << ActivityID << "not found !" << endl;
    SetResponse(AlpideTable::RecordNotFound, ActivityID);
    pushResponse();
    return (&theResponse);
  }
}


/* ----------------------
 * Dump an activity long in readble format
 *
 *
----------------------- */
void ActivityDB::DumpActivity(activityLong *Act)
{
  FUNCTION_TRACE;
  cout << endl << "----Activity Dump ---- " << endl;
  cout << "Activity ID:" << Act->ID << endl;

  cout << "Name:" << Act->Name << endl;
  cout << "Lot ID:" << Act->LotID << endl;
  cout << "Location:" << Act->Location.ID << "\t" << Act->Location.Name << endl;
  cout << "Start Date:" << Act->StartDate << endl;
  cout << "End Date:" << Act->EndDate << endl;
  cout << "Result:" << Act->Result.ID << "\t" << Act->Result.Name << endl;
  cout << "Status:" << Act->Status.ID << "\t" << Act->Status.Code << "\t" << Act->Status.Description
       << endl;
  cout << "Type:" << Act->Type.ID << "\t" << Act->Type.Name << "\t" << Act->Type.Description
       << endl;
  cout << "Members:";
  for (unsigned int i = 0; i < Act->Members.size(); i++) {
    cout << Act->Members.at(i).ID << " \t" << Act->Members.at(i).Member.FullName << endl
         << "        ";
  }
  cout << endl << "Components:";
  for (unsigned int i = 0; i < Act->Components.size(); i++) {
    cout << Act->Components.at(i).ID << "\t" << Act->Components.at(i).Component.ComponentID << "\t"
         << Act->Components.at(i).FunctionalStatus.Name << "\t"
         << Act->Components.at(i).PhysicalStatus.Name << endl
         << "          ";
  }
  cout << endl << "Parameters:";
  for (unsigned int i = 0; i < Act->Parameters.size(); i++) {
    cout << Act->Parameters.at(i).ID << "\t" << Act->Parameters.at(i).Type.Parameter.Name << "="
         << Act->Parameters.at(i).Value << endl
         << "          ";
  }
  cout << endl << "Attachments:";
  for (unsigned int i = 0; i < Act->Attachments.size(); i++) {
    cout << Act->Attachments.at(i).ID << "\t" << Act->Attachments.at(i).FileName << "\t"
         << Act->Attachments.at(i).Type.Description << endl
         << "          ";
  }
  cout << endl << "Uris:";
  for (unsigned int i = 0; i < Act->Uris.size(); i++) {
    cout << Act->Uris.at(i).ID << "\t" << Act->Uris.at(i).Description << "\t"
         << Act->Uris.at(i).Path << "\t" << endl
         << "          ";
  }
  cout << endl << "------------------------" << endl;
}

/* ----------------------
 * Converts the string into the URI encoding
 *
 *  not used !
----------------------- */
int ActivityDB::buildUrlEncoded(string aLocalFileName, string *Buffer)
{
  FUNCTION_TRACE;
  FILE *fh = fopen(aLocalFileName.c_str(), "rb");
  if (fh == NULL) {
    cerr << "Failed to open the file :" << aLocalFileName << "Abort !" << endl;
    return (0);
  }

  char exa[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  unsigned char ch;

  ch = (unsigned char)fgetc(fh);
  while (!feof(fh)) {
    if (isalnum(ch) || ch == '~' || ch == '-' || ch == '.' || ch == '_') { // rfc3986
      *Buffer += ch;
    }
    else {
      *Buffer += '%';
      *Buffer += exa[ch >> 4];
      *Buffer += exa[ch & 4]; // YCM:FIXME if fix was not intended by [ch && 4]
    }
    ch = (unsigned char)fgetc(fh);
  }
  fclose(fh);
  return (Buffer->size());
}

/* ----------------------
 * Converts the string into the Base64 coding
 *
 * must be improved !
----------------------- */
unsigned long ActivityDB::buildBase64Binary(string aLocalFileName, string *Buffer)
{
  FUNCTION_TRACE;
  static const string base64chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned long theBufferLength = Buffer->size();

  FILE *fh = fopen(aLocalFileName.c_str(), "rb");
  if (fh == NULL) {
    cerr << "Failed to open the file :" << aLocalFileName << "Abort !" << endl;
    return (0);
  }

  unsigned char cBufferIn[3];
  unsigned char cBufferOut[4];

  int           i = 0;
  int           j = 0;
  unsigned char ch;

  ch = (unsigned char)fgetc(fh);
  while (!feof(fh)) {
    cBufferIn[i++] = ch; // *(bytes_to_encode++);
    if (i == 3) {
      cBufferOut[0] = (cBufferIn[0] & 0xfc) >> 2;
      cBufferOut[1] = ((cBufferIn[0] & 0x03) << 4) + ((cBufferIn[1] & 0xf0) >> 4);
      cBufferOut[2] = ((cBufferIn[1] & 0x0f) << 2) + ((cBufferIn[2] & 0xc0) >> 6);
      cBufferOut[3] = cBufferIn[2] & 0x3f;
      for (i = 0; (i < 4); i++)
        *Buffer += base64chars[cBufferOut[i]];
      i = 0;
    }
    ch = (unsigned char)fgetc(fh);
  }
  if (i) {
    for (j = i; j < 3; j++)
      cBufferIn[j] = '\0';
    cBufferOut[0] = (cBufferIn[0] & 0xfc) >> 2;
    cBufferOut[1] = ((cBufferIn[0] & 0x03) << 4) + ((cBufferIn[1] & 0xf0) >> 4);
    cBufferOut[2] = ((cBufferIn[1] & 0x0f) << 2) + ((cBufferIn[2] & 0xc0) >> 6);
    cBufferOut[3] = cBufferIn[2] & 0x3f;
    for (j = 0; (j < i + 1); j++)
      *Buffer += base64chars[cBufferOut[j]];
    while ((i++ < 3))
      *Buffer += '=';
  }
  theBufferLength = Buffer->size() - theBufferLength;
  fclose(fh);
  return (theBufferLength);
}
