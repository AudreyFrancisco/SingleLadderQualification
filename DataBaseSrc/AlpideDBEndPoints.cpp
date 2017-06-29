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
 *  Description : Alpide DB  Class *
 *  HISTORY
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <ctime>


#include "AlpideDBEndPoints.h"
#include "AlpideDB.h"
#include "utilities.h"

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
	theParentDB = DBhandle;
}

/* -----------------
 *    Destructor
 *---------------- */
AlpideTable::~AlpideTable()
{
}

/* -----------------
 *    DecodeResponse := analyze the returned XML and fills the
 *                      response structure with the returned errors info
 *		Params :  ReturnedString := the XML sgtring received from the WebDB server
 *				  Session := a Session Index Value (reserved for future use)
 *		returns : a response struct that contains the error code
 *
 *---------------- */
AlpideTable::response *AlpideTable::DecodeResponse(char *ReturnedString, int Session)
{
	xmlDocPtr doc;
	doc = xmlReadMemory(ReturnedString, strlen(ReturnedString), "noname.xml", NULL, 0); // parse the XML
	if (doc == NULL) {
	    cerr << "Failed to parse document" << endl;
	    SetResponse(AlpideTable::BadXML, 0, Session);
		return(&theResponse);
	}
	xmlNode *root_element = NULL;
	root_element = xmlDocGetRootElement(doc);
	if(root_element == NULL) {
	    cerr << "Failed to parse document: No root element" << endl;
	    SetResponse(AlpideTable::BadXML, 0, Session);
		return(&theResponse);
	}
	xmlNode *n1 = root_element->children;
	while (n1 != NULL) {
		if(strcmp((const char*)n1->name, "ErrorCode") == 0) {
			theResponse.ErrorCode = atoi( (const char*)n1->children->content);
		} else if(strcmp((const char*)n1->name, "ErrorMessage") == 0) {
			theResponse.ErrorMessage = (const char*)n1->children->content;
		} else if(strcmp((const char*)n1->name, "ID") == 0) {
			theResponse.ID = atoi( (const char*)n1->children->content);
		}
		n1 = n1->next;
	}
	theResponse.Session = Session;
	return(&theResponse);
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
	theResponse.ErrorCode = (int)ErrNum;
	theResponse.ID = ID;
	theResponse.Session = Session;

	switch(ErrNum) {
	case AlpideTable::BadXML:
		theResponse.ErrorMessage = "Invalid XML format";
		break;
	case AlpideTable::SyncQuery:
		theResponse.ErrorMessage = "The Sync DB Query returns error";
		break;
	case AlpideTable::NoError:
		theResponse.ErrorMessage = "No error !";
		break;
	default:
		theResponse.ErrorMessage = "Unknown Error";
	}
	return;
}

/* -----------------

 *---------------- */
const char *AlpideTable::DumpResponse()
{
	theGeneralBuffer = "Return : Message =" + theResponse.ErrorMessage + "(" + std::to_string(theResponse.ErrorCode) + ") " ;
	theGeneralBuffer += "ID=" + std::to_string(theResponse.ID) + " Session=" +std::to_string(theResponse.Session);
	return(theGeneralBuffer.c_str());
}

/* --------------------------------------------------------
 *    ProjectDB   Class to manage the Projects Table
 *
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *---------------- */
ProjectDB::ProjectDB(AlpideDB * DBhandle) : AlpideTable(DBhandle)
{
}

ProjectDB::~ProjectDB()
{
}

/* -----------------
 *    GetList := get the complete list of projects
 *
 *		Out Param : a Reference to a vector of Project struct that will contain all the projects
 *		returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *ProjectDB::GetList(vector<project> *Result)
{
	string theUrl = theParentDB->GetQueryDomain() + "ProjectRead";
	string theQuery = "";
	char *result;
	project pro;


	if( theParentDB->GetManageHandle()->makeDBQuery(theUrl, theQuery.c_str(), &result) == 0) {
	    cerr << "Failed to execut the Query" << endl;
	    SetResponse(AlpideTable::SyncQuery, 0,0);
		return(&theResponse);
	}
	xmlDocPtr doc;
	doc = xmlReadMemory(result, strlen(result), "noname.xml", NULL, 0); // parse the XML
	if (doc == NULL) {
	    cerr << "Failed to parse document" << endl;
	    SetResponse(AlpideTable::BadXML, 0,0);
		return(&theResponse);
	}

	// Get the root element node
	xmlNode *root_element = NULL;
	root_element = xmlDocGetRootElement(doc);
	if(root_element == NULL) {
	    SetResponse(AlpideTable::BadXML, 0,0);
		return(&theResponse);
	}

	Result->clear();
	xmlNode *nod = root_element->children;
	while (nod != NULL) {
		if(strcmp((const char*)nod->name, "Project") == 0) {
			xmlNode *n1 = nod->children;
			while(n1 != NULL) {
				if(strcmp((const char*)n1->name, "ID") == 0) pro.ID = atoi( (const char*)n1->children->content);
				else if (strcmp((const char*)n1->name, "Name") == 0) pro.Name.assign( (const char *)n1->children->content);
				n1 = n1->next;
			}
			Result->push_back(pro);
		}
		nod = nod->next;
	}

	free(result);
	xmlFreeDoc(doc);       // free document
	xmlCleanupParser();

	SetResponse(AlpideTable::NoError, 0,0);
	return(&theResponse);
}



/* --------------------------------------------------------
 *    MemberDB   Class to manage the Members Table
 *
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *---------------- */
MemberDB::MemberDB(AlpideDB * DBhandle) : AlpideTable(DBhandle)
{
}

MemberDB::~MemberDB()
{
}

/* -----------------
 *    GetList := get the complete list of members
 *
 *		Out Param : a Reference to a vector of Member struct that will contain all the member
 *		returns : a response struct that contains the error code
 *---------------- */
AlpideTable::response *MemberDB::GetList(int projectID, vector<member> *Result)
{
	string theUrl = theParentDB->GetQueryDomain() + "ProjectMemberRead";
	string theQuery = "projectID=" + std::to_string(projectID) ;
	char *result;
	member pro;

	if( theParentDB->GetManageHandle()->makeDBQuery(theUrl, theQuery.c_str(), &result) == 0) {
	    cerr << "Failed to execute the Query" << endl;
	    SetResponse(AlpideTable::SyncQuery, 0,0);
		return(&theResponse);
	}
	xmlDocPtr doc;
	doc = xmlReadMemory(result, strlen(result), "noname.xml", NULL, 0); // parse the XML
	if (doc == NULL) {
	    cerr << "Failed to parse document" << endl;
	    SetResponse(AlpideTable::BadXML, 0,0);
		return(&theResponse);
	}

	// Get the root element node
	xmlNode *root_element = NULL;
	root_element = xmlDocGetRootElement(doc);
	if(root_element == NULL) {
	    cerr << "Failed Bad XML format no root element" << endl;
	    SetResponse(AlpideTable::BadXML, 0,0);
		return(&theResponse);
	}
	xmlNode *nod = root_element->children;
	while (nod != NULL) {
		if(strcmp((const char*)nod->name, "ProjectMember") == 0) {
			xmlNode *n1 = nod->children;
			while(n1 != NULL) {
				if(strcmp((const char*)n1->name, "ID") == 0) pro.ID = atoi( (const char*)n1->children->content);
				else if (strcmp((const char*)n1->name, "PersonID") == 0) pro.PersonalID = atoi( (const char*)n1->children->content);
				else if (strcmp((const char*)n1->name, "FullName") == 0) pro.FullName.assign( (const char *) n1->children->content);
				n1 = n1->next;
			}
			Result->push_back(pro);
		}
		nod = nod->next;
	}

	free(result);
	xmlFreeDoc(doc);       // free document
	xmlCleanupParser();

	SetResponse(AlpideTable::NoError, 0,0);
	return(&theResponse);

}


/* --------------------------------------------------------
 *    ComponentDB   Class to manage the Components Table
 *
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *---------------- */
ComponentDB::ComponentDB(AlpideDB * DBhandle) : AlpideTable(DBhandle)
{
}

ComponentDB::~ComponentDB()
{
}

/* -----------------
*    GetTypeList := get the complete list of all the component types defined
*
*		In Param : the ID of the Project
*		Out Param : a Reference to a vector of ComponentType struct that will contain all the component types
*		returns : a response struct that contains the error code
*---------------- */
AlpideTable::response * ComponentDB::GetTypeList(int ProjectID, vector<componentType> *Result)
{
	string theUrl = theParentDB->GetQueryDomain() + "ComponentTypeRead";
	string theQuery = "projectID=" + std::to_string(ProjectID) ;
	char *stringresult;
	componentType pro;

	if( theParentDB->GetManageHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
	    cerr << "Failed to execute the Query" << endl;
	    SetResponse(AlpideTable::SyncQuery, 0,0);
		return(&theResponse);
	}

	xmlDocPtr doc;
	doc = xmlReadMemory(stringresult, strlen(stringresult), "noname.xml", NULL, 0); // parse the XML
	if (doc == NULL) {
	    cerr << "Failed to parse document" << endl;
	    SetResponse(AlpideTable::BadXML, 0,0);
		return(&theResponse);
	}

	// Get the root element node
	xmlNode *root_element = NULL;
	root_element = xmlDocGetRootElement(doc);
	if(root_element == NULL) {
	    SetResponse(AlpideTable::BadXML, 0,0);
		return(&theResponse);
	}

	xmlNode *n1;
	xmlNode *nod = root_element->children;
	while (nod != NULL) {
		if(strcmp((const char*)nod->name, "ComponentType") == 0) {
			n1 = nod->children;
			extractTheComponentType(n1, &pro);
			Result->push_back(pro);
		}
		nod = nod->next;
	}

	free(stringresult);
	xmlFreeDoc(doc);       // free document
	xmlCleanupParser();

    SetResponse(AlpideTable::NoError, 0,0);
	return(&theResponse);
}


/* -----------------
 * 	PRIVATE
 *    extractTheComponentType := get the component types definition
 *
 *		In Param : the XML node that contains the Type definition
 *		Out Param : a Reference to a ComponentType struct that will contains the values parsed
 *
 * ---------------- */
void ComponentDB::extractTheComponentType(xmlNode *ns, componentType *pro)
{
	xmlNode *n1,*n2,*n3, *n4;
	n1 = ns;

	while(n1 != NULL) {
		if(strcmp((const char*)n1->name, "ID") == 0) pro->ID = atoi( (const char*)n1->children->content);
		else if (strcmp((const char*)n1->name, "Name") == 0) pro->Name.assign( (const char *)n1->children->content);
		else if (strcmp((const char*)n1->name, "Code") == 0) pro->Code.assign( (const char *)n1->children->content);
		else if (strcmp((const char*)n1->name, "Description") == 0) pro->Description.assign((const char *)n1->children->content);
		else if (strcmp((const char*)n1->name, "Composition") == 0) {
			n2 = n1->children;
			while(n2 != NULL) {
				if(strcmp((const char*)n2->name, "ComponentTypeComposition") == 0)  {
					n3 = n2->children;
					composition ap1;
					while(n3 != NULL) {
						if(strcmp((const char*)n3->name, "ComponentType") == 0)  {
							n4 = n3->children;
							while(n4 != NULL) {
								if(strcmp((const char*)n4->name, "ID") == 0) ap1.ID = atoi( (const char*)n4->children->content);
								else if(strcmp((const char*)n4->name, "Name") == 0)  ap1.ComponentType.assign( (const char *)n4->children->content);
								n4 = n4->next;
							}
						}
						else if(strcmp((const char*)n3->name, "Quantity") == 0)  ap1.Quantity = atoi( (const char*)n3->children->content);
						n3 =n3->next;
					}
					pro->Composition.push_back(ap1);
				}
				n2 =n2->next;
			}
		}
		else if (strcmp((const char*)n1->name, "PhysicalStatus") == 0) {
			n2 = n1->children;
			while(n2 != NULL) {
				if(strcmp((const char*)n2->name, "StatusPhysical") == 0)  {
					n3 = n2->children;
					statusphysical ap1;
					while(n3 != NULL) {
						if(strcmp((const char*)n3->name, "ID") == 0) ap1.ID = atoi((const char *)n3->children->content);
						else if(strcmp((const char*)n3->name, "Name") == 0)  ap1.Name.assign( (const char *)n3->children->content);
						n3 = n3->next;
					}
					pro->PhysicalStatus.push_back(ap1);
				}
				n2 =n2->next;
			}
		}
		else if (strcmp((const char*)n1->name, "FunctionalStatus") == 0) {
			n2 = n1->children;
			while(n2 != NULL) {
				if(strcmp((const char*)n2->name, "StatusFunctional") == 0)  {
					n3 = n2->children;
					statusfunctional ap1;
					while(n3 != NULL) {
						if(strcmp((const char*)n3->name, "ID") == 0)  ap1.ID = atoi((const char *)n3->children->content);
						else if(strcmp((const char*)n3->name, "Name") == 0)  ap1.Name.assign( (const char *)n3->children->content );
						n3 = n3->next;
					}
					pro->FunctionalStatus.push_back(ap1);
				}
				n2 =n2->next;
			}
		}
		n1 = n1->next;
	}
}

/* -----------------
*    GetType := get the complete definition of the requested component
*
*		In Param : the ID of the Component Type
*		Out Param : a Reference to a ComponentType struct that will contain the component type definition
*		returns : a response struct that contains the error code
*---------------- */
AlpideTable::response *ComponentDB::GetType(int ComponentTypeID, componentType *Result)
{
	string theUrl = theParentDB->GetQueryDomain() + "ComponentTypeReadAll";
	string theQuery = "componentTypeID=" + std::to_string(ComponentTypeID) ;
	char *stringresult;
	componentType pro;

	if( theParentDB->GetManageHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0)  {
	    cerr << "Failed to execute the Query" << endl;
	    SetResponse(AlpideTable::SyncQuery, 0,0);
		return(&theResponse);
	}
	xmlDocPtr doc;
	doc = xmlReadMemory(stringresult, strlen(stringresult), "noname.xml", NULL, 0); // parse the XML
	if (doc == NULL) {
	    cerr << "Failed to parse document" << endl;
	    SetResponse(AlpideTable::BadXML, 0,0);
		return(&theResponse);
	}
	// Get the root element node
	xmlNode *root_element = NULL;
	root_element = xmlDocGetRootElement(doc);
	if(root_element == NULL) {
	    SetResponse(AlpideTable::BadXML, 0,0);
		return(&theResponse);
	}

	xmlNode *n1 = root_element->children;
	extractTheComponentType(n1, Result);

    SetResponse(AlpideTable::NoError, 0,0);
	return(&theResponse);
}

/* -----------------
*    Create := A component type ... TODO: need a better interface (with the struct parameter)
*
*		In Param : ... all the items describing ...
*		returns : a response struct that contains the error code
*---------------- */
AlpideTable::response * ComponentDB::Create(string ComponentTypeID, string ComponentID, string SupplyCompID,
		string Description, string LotID, string PackageID, string UserID )
{
	string theUrl = theParentDB->GetQueryDomain() + "ComponentCreate";
	string theQuery = "componentTypeID="+ComponentTypeID+"&componentID="+ComponentID+"&supplierComponentID="+SupplyCompID+
			"&description="+Description+"&lotID="+LotID+"&packageID="+PackageID+"&userID="+UserID;
	char *stringresult;

	if( theParentDB->GetManageHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
	} else {
		DecodeResponse(stringresult);
	}
	return(&theResponse);
}

/* -----------------
*    Print := Dumps a human readable form of the Component Type definition
*
*		In Param : the component type struct
*		returns : a char pointer to a string buffer
*---------------- */
string ComponentDB::Print(componentType *co)
{
			ap = "Component : ID=" + std::to_string(co->ID) +
				" Name="+ co->Name + " Code=" + co->Code + " Description=" + co->Description + "\n";
			ap += "   Composition : {";
			for(int i=0;i<co->Composition.size();i++)
				ap+= "( ID="+std::to_string(co->Composition.at(i).ID)+
						",Type="+co->Composition.at(i).ComponentType+
						  ",Q.ty="+std::to_string(co->Composition.at(i).Quantity)+")";
			ap += "}\n";
			ap += "   Physical Status  : {";
			for(int i=0;i<co->PhysicalStatus.size();i++)
				ap+= "( ID="+ std::to_string(co->PhysicalStatus.at(i).ID) +
						  ",Name="+co->PhysicalStatus.at(i).Name+")";
			ap += "}\n";
			ap += "   Functional Status  : {";
			for(int i=0;i<co->FunctionalStatus.size();i++)
				ap+= "( ID="+ std::to_string(co->FunctionalStatus.at(i).ID) +
						  ",Name="+co->FunctionalStatus.at(i).Name+")";
			ap += "}\n";
			return(ap);
}


/* --------------------------------------------------------
 *    ActivityDB   Class to manage the Activity Table
 *
 *
 *-------------------------------------------------------- */

/* -----------------
 *    Constructor
 *---------------- */
ActivityDB::ActivityDB(AlpideDB * DBhandle) : AlpideTable(DBhandle)
{
}

ActivityDB::~ActivityDB()
{
}

/* -----------------
*    Create := Create a new activity record  TODO: complete the activity with all the info
*
*		In Param : the activity struct
*		returns : a char pointer to a string buffer
*---------------- */
ActivityDB::response * ActivityDB::Create(activity *aActivity)
{
	char DateBuffer[40];
	char DateMask[40] = "%Y.%m.%d %H.%M.%S";
	char *stringresult;

	string theUrl = theParentDB->GetQueryDomain() + "ActivityCreate";
	string theQuery = "activityTypeID="+std::to_string(aActivity->Type);
	theQuery += "&locationID=" + std::to_string(aActivity->Location);
	theQuery += "&lotID=" + aActivity->Lot;
	theQuery += "&activityName=" + aActivity->Name;
	strftime(DateBuffer, 40, DateMask,(const tm *)(localtime(&(aActivity->StartDate))));
	theQuery += "&startDate=" + DateBuffer;
	strftime(DateBuffer, 40, DateMask,(const tm *)(localtime(&(aActivity->EndDate))));
	theQuery += "&endDate=" + DateBuffer;
	theQuery += "&position=" + aActivity->Position;
	theQuery += "&resultID=" + std::to_string(aActivity->Result);
	theQuery += "&statusID=" + std::to_string(aActivity->Status);
	theQuery += "&userID=" +  std::to_string(aActivity->User);

	if( theParentDB->GetManageHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
	} else {
		DecodeResponse(stringresult);
	}
	return(&theResponse);
}

