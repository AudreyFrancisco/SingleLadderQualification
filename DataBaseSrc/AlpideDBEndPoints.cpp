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
 *
 *---------------- */

// TODO: Need to make two routines to decode the response in order to be compliant with SOAP/POST versions.

AlpideTable::response *AlpideTable::DecodeResponse(char *ReturnedString, int Session)
{
	bool bGoChildren = false;
//	cout << ">>--Returned string--->>" <<  ReturnedString << endl;
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
			if(n1->children != NULL)
				theResponse.ErrorCode = atoi( (const char*)n1->children->content);
		} else if(strcmp((const char*)n1->name, "ErrorMessage") == 0) {
			if(n1->children != NULL)
				theResponse.ErrorMessage = (const char*)n1->children->content;
		} else if(strcmp((const char*)n1->name, "ID") == 0) {
			if(n1->children != NULL)
				theResponse.ID = atoi( (const char*)n1->children->content);
		} else if(strcmp((const char*)n1->name, "text") == 0) { // we need to skip this
			// do nothing
		} else  { // we reach the parent of results
			bGoChildren = true;
		}
		if(bGoChildren) {
			n1 = n1->children;
			bGoChildren = false;
		} else {
			n1 = n1->next;
		}
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
	string theUrl = theParentDB->GetQueryDomain() + "/ProjectRead";
	string theQuery = "";
	char *result;
	project pro;


	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &result) == 0) {
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
	string theUrl = theParentDB->GetQueryDomain() + "/ProjectMemberRead";
	string theQuery = "projectID=" + std::to_string(projectID) ;
	char *result;
	member pro;

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &result) == 0) {
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
	string theUrl = theParentDB->GetQueryDomain() + "/ComponentTypeRead";
	string theQuery = "projectID=" + std::to_string(ProjectID) ;
	char *stringresult;
	componentType pro;

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
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
	string theUrl = theParentDB->GetQueryDomain() + "/ComponentTypeReadAll";
	string theQuery = "componentTypeID=" + std::to_string(ComponentTypeID) ;
	char *stringresult;
	componentType pro;

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0)  {
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
	string theUrl = theParentDB->GetQueryDomain() + "/ComponentCreate";
	string theQuery = "componentTypeID="+ComponentTypeID+"&componentID="+ComponentID+"&supplierComponentID="+SupplyCompID+
			"&description="+Description+"&lotID="+LotID+"&packageID="+PackageID+"&userID="+UserID;
	char *stringresult;

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
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
			for(unsigned int i=0;i<co->Composition.size();i++)
				ap+= "( ID="+std::to_string(co->Composition.at(i).ID)+
						",Type="+co->Composition.at(i).ComponentType+
						  ",Q.ty="+std::to_string(co->Composition.at(i).Quantity)+")";
			ap += "}\n";
			ap += "   Physical Status  : {";
			for(unsigned int i=0;i<co->PhysicalStatus.size();i++)
				ap+= "( ID="+ std::to_string(co->PhysicalStatus.at(i).ID) +
						  ",Name="+co->PhysicalStatus.at(i).Name+")";
			ap += "}\n";
			ap += "   Functional Status  : {";
			for(unsigned int i=0;i<co->FunctionalStatus.size();i++)
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
// TODO: evaluate the error conditions return policy ??

ActivityDB::response * ActivityDB::Create(activity *aActivity)
{
	char DateBuffer[40];
	char DateMask[40] = "%d/%m/%Y";
	char *stringresult;
	string theUrl;
	string theQuery;

	theUrl = theParentDB->GetQueryDomain() + "/ActivityCreate";
	theQuery = "activityTypeID="+std::to_string(aActivity->Type);
	theQuery += "&locationID=" + std::to_string(aActivity->Location);
	theQuery += "&lotID=" + aActivity->Lot;
	theQuery += "&activityName=" + aActivity->Name;
	strftime(DateBuffer, 40, DateMask,(const tm *)(localtime(&(aActivity->StartDate))));
	theQuery += "&startDate=";
	theQuery.append( DateBuffer );
	strftime(DateBuffer, 40, DateMask,(const tm *)(localtime(&(aActivity->EndDate))));
	theQuery += "&endDate=";
	theQuery.append( DateBuffer );
	theQuery += "&position=" + aActivity->Position;
	theQuery += "&resultID=" + std::to_string(aActivity->Result);
	theQuery += "&statusID=" + std::to_string(aActivity->Status);
	theQuery += "&userID=" +  std::to_string(aActivity->User);

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
		return(&theResponse);
	} else {
		DecodeResponse(stringresult);
		if(VERBOSITYLEVEL == 1) cout << "Activity creation :" << DumpResponse() << endl;
		aActivity->ID = theResponse.ID;
	}

	theUrl = theParentDB->GetQueryDomain() + "/ActivityMemberAssign";
	for(unsigned int i=0; i< aActivity->Members.size();i++) {
		theQuery = "projectMemberID="+std::to_string(aActivity->Members.at(i).ProjectMember);
		theQuery += "&activityID=" + std::to_string(aActivity->ID);
		theQuery += "&leader=" + std::to_string(aActivity->Members.at(i).Leader);
		theQuery += "&userID=" + std::to_string(aActivity->Members.at(i).User);

		if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
			SetResponse(AlpideTable::SyncQuery);
			return(&theResponse);
		} else {
			DecodeResponse(stringresult);
			if(VERBOSITYLEVEL == 1) cout << "Activity Member creation :" << DumpResponse() << endl;
			aActivity->Members.at(i).ID = theResponse.ID;
		}
	}

	theUrl = theParentDB->GetQueryDomain() + "/ActivityParameterCreate";
	for(unsigned int i=0; i< aActivity->Parameters.size();i++) {
		theQuery = "activityID=" + std::to_string(aActivity->ID);
		theQuery += "&activityParameterID=" + std::to_string(aActivity->Parameters.at(i).ActivityParameter);
		theQuery += "&value=" + std::to_string(aActivity->Parameters.at(i).Value);
		theQuery += "&userID=" + std::to_string(aActivity->Parameters.at(i).User);

		if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
			SetResponse(AlpideTable::SyncQuery);
			return(&theResponse);
		} else {
			DecodeResponse(stringresult);
			if(VERBOSITYLEVEL == 1) cout << "Activity Parameter creation :" << DumpResponse() << endl;
			aActivity->Parameters.at(i).ID = theResponse.ID;
		}
	}

	theUrl = theParentDB->GetQueryDomain() ;//+ "/ActivityAttachmentCreate";
	unsigned long theBase64Result;
	for(unsigned int i=0; i< aActivity->Attachments.size();i++) {
		theQuery = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
		if(SOAPVERSION == 11) {
			theQuery += "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">";
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
		} else {
			theQuery += "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">";
			theQuery += "<soap12:Body><ActivityAttachmentCreate xmlns=\"http://tempuri.org/\"><activityID>";
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
		if(VERBOSITYLEVEL == 1) cout << "Base64 encoding of Attachment return a len of bytes = " <<  theBase64Result << endl;

		if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult, true, "http://tempuri.org/ActivityAttachmentCreate") == 0) {
			SetResponse(AlpideTable::SyncQuery);
			return(&theResponse);
		} else {
			DecodeResponse(stringresult);
			if(VERBOSITYLEVEL == 1) cout << "Activity Attachment creation :" << DumpResponse() << endl;
			aActivity->Attachments.at(i).ID = theResponse.ID;
		}
	}

	return(&theResponse);
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
ActivityDB::response * ActivityDB::AssignComponent(int aActivityID, int aComponentID,
													int aComponentTypeID, int aUserID)

{
	char *stringresult;
	string theUrl;
	string theQuery;

	theUrl = theParentDB->GetQueryDomain() + "/ActivityComponentAssign";
	theQuery = "componentID="+std::to_string(aComponentID);
	theQuery += "&activityID=" + std::to_string(aActivityID);
	theQuery += "&actTypeCompTypeID=" + std::to_string(aComponentTypeID);
	theQuery += "&userID=" + std::to_string(aUserID);

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
		return(&theResponse);
	} else {
		DecodeResponse(stringresult);
		if(VERBOSITYLEVEL == 1) cout << "Activity creation :" << DumpResponse() << endl;
		SetResponse(AlpideTable::NoError);
	}
	return(&theResponse);
}





/* ----------------------
 * Get the list of type parameters ...
 *
 *
----------------------- */
std::vector<ActivityDB::parameterType> *ActivityDB::GetParameterTypeList(int aActivityID)
{
	vector<parameterType> *theParamList = new vector<parameterType>;


	char *stringresult;
	string theUrl;
	string theQuery;
	parameterType param;

	theUrl = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
	theQuery = "activityTypeID="+std::to_string(aActivityID);

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
		return(theParamList);
	} else {
		xmlDocPtr doc;
		doc = xmlReadMemory(stringresult, strlen(stringresult), "noname.xml", NULL, 0); // parse the XML
		if (doc == NULL) {
		    cerr << "Failed to parse document" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theParamList);
		}

		// Get the root element node
		xmlNode *root_element = NULL;
		root_element = xmlDocGetRootElement(doc);
		if(root_element == NULL) {
		    cerr << "Failed Bad XML format no root element" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theParamList);
		}
		xmlNode *nod = root_element->children;
		while (nod != NULL) {
			if(strcmp((const char*)nod->name, "Parameters") == 0) {
				xmlNode *n1 = nod->children;
				while(n1 != NULL) {

					if(strcmp((const char*)n1->name, "ActivityTypeParameter") == 0) {
						xmlNode *n2 = n1->children;
						while(n2 != NULL) {
							if(strcmp((const char*)n2->name, "Parameter") == 0) {
								xmlNode *n3 = n2->children;
								param.ID =0;
								param.Name = "";
								param.Description = "";
								while(n3 != NULL) {
									if(strcmp((const char*)n3->name, "ID") == 0) {
										if(n3->children != NULL ) param.ID = atoi( (const char*)(n3->children->content)) ;
									} else {
										if (strcmp((const char*)n3->name, "Name") == 0) {
											if(n3->children != NULL ) param.Name = (const char*)(n3->children->content);
										} else {
											if (strcmp((const char*)n3->name, "Description") == 0) {
												if(n3->children != NULL ) param.Description = (const char*)(n3->children->content);
											}
										}
									}
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

		free(stringresult);
		xmlFreeDoc(doc);       // free document
		xmlCleanupParser();

		SetResponse(AlpideTable::NoError, 0,0);
	}
	return(theParamList);
}

/* ----------------------
 * Get the list of type activity ...
 *
 *
----------------------- */
std::vector<ActivityDB::activityType> *ActivityDB::GetActivityTypeList(int aProjectID)
{
	vector<activityType> *theTypeList = new vector<activityType>;


	char *stringresult;
	string theUrl;
	string theQuery;
	activityType act;

	theUrl = theParentDB->GetQueryDomain() + "/ActivityTypeRead";
	theQuery = "projectID="+std::to_string(aProjectID);

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
		return(theTypeList);
	} else {

		xmlDocPtr doc;
		doc = xmlReadMemory(stringresult, strlen(stringresult), "noname.xml", NULL, 0); // parse the XML
		if (doc == NULL) {
		    cerr << "Failed to parse document" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theTypeList);
		}

		// Get the root element node
		xmlNode *root_element = NULL;
		root_element = xmlDocGetRootElement(doc);
		if(root_element == NULL) {
		    cerr << "Failed Bad XML format no root element" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theTypeList);
		}
		xmlNode *nod = root_element->children;
		while (nod != NULL) {
			if(strcmp((const char*)nod->name, "ActivityType") == 0) {
				xmlNode *n1 = nod->children;
				act.ID = 0;
				act.Name = "";
				act.Description = "";
				while(n1 != NULL) {
					if(strcmp((const char*)n1->name, "ID") == 0) {
						if(n1->children != NULL) act.ID = atoi( (const char*)(n1->children->content)) ;
					} else {
						if (strcmp((const char*)n1->name, "Name") == 0) {
							if(n1->children != NULL) act.Name = (const char*)(n1->children->content);
						} else {
							if (strcmp((const char*)n1->name, "Description") == 0) {
								if(n1->children != NULL) act.Description = (const char*)(n1->children->content);
							}
						}
					}
					n1 = n1->next;
				}
				theTypeList->push_back(act);
			}
			nod = nod->next;
		}

		free(stringresult);
		xmlFreeDoc(doc);       // free document
		xmlCleanupParser();

		SetResponse(AlpideTable::NoError, 0,0);
	}
	return(theTypeList);
}

/* ----------------------
 * Get the list of type location ...
 *
 *
----------------------- */
std::vector<ActivityDB::locationType> *ActivityDB::GetLocationTypeList(int aActivityID)
{
	vector<locationType> *theLocationList = new vector<locationType>;


	char *stringresult;
	string theUrl;
	string theQuery;
	locationType loc;

	theUrl = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
	theQuery = "activityTypeID="+std::to_string(aActivityID);

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
		return(theLocationList);
	} else {

		xmlDocPtr doc;
		doc = xmlReadMemory(stringresult, strlen(stringresult), "noname.xml", NULL, 0); // parse the XML
		if (doc == NULL) {
		    cerr << "Failed to parse document" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theLocationList);
		}

		// Get the root element node
		xmlNode *root_element = NULL;
		root_element = xmlDocGetRootElement(doc);
		if(root_element == NULL) {
		    cerr << "Failed Bad XML format no root element" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theLocationList);
		}
		xmlNode *nod = root_element->children;
		while (nod != NULL) {
			if(strcmp((const char*)nod->name, "Location") == 0) {
				xmlNode *n1 = nod->children;
				while(n1 != NULL) {
					if(strcmp((const char*)n1->name, "ActivityTypeLocation") == 0) {
						xmlNode *n2 = n1->children;
						loc.ID = 0;
						loc.Name = "";
						while(n2 != NULL) {
							if(strcmp((const char*)n2->name, "ID") == 0) {
								if(n2->children != NULL ) loc.ID = atoi( (const char*)(n2->children->content)) ;
							} else {
								if (strcmp((const char*)n2->name, "Name") == 0) {
									if(n2->children != NULL ) loc.Name = (const char*)(n2->children->content);
								}
							}
							n2 = n2->next;
						}
						theLocationList->push_back(loc);
					}
					n1 = n1->next;
				}
			}
			nod = nod->next;
		}

		free(stringresult);
		xmlFreeDoc(doc);       // free document
		xmlCleanupParser();

		SetResponse(AlpideTable::NoError, 0,0);
	}
	return(theLocationList);
}

/* ----------------------
 * Get the list of type component ...
 *
 *
----------------------- */
std::vector<ActivityDB::componentType> *ActivityDB::GetComponentTypeList(int aActivityID)
{
	vector<componentType> *theCompoList = new vector<componentType>;


	char *stringresult;
	string theUrl;
	string theQuery;
	componentType comp;

	theUrl = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
	theQuery = "activityTypeID="+std::to_string(aActivityID);

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
		return(theCompoList);
	} else {

		xmlDocPtr doc;
		doc = xmlReadMemory(stringresult, strlen(stringresult), "noname.xml", NULL, 0); // parse the XML
		if (doc == NULL) {
		    cerr << "Failed to parse document" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theCompoList);
		}

		// Get the root element node
		xmlNode *root_element = NULL;
		root_element = xmlDocGetRootElement(doc);
		if(root_element == NULL) {
		    cerr << "Failed Bad XML format no root element" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theCompoList);
		}
		xmlNode *nod = root_element->children;
		while (nod != NULL) {
			if(strcmp((const char*)nod->name, "ActivityTypeComponentType") == 0) {
				xmlNode *n1 = nod->children;
				while(n1 != NULL) {
					if(strcmp((const char*)n1->name, "ActivityTypeComponentTypeFull") == 0) {
						xmlNode *n2 = n1->children;
						while(n2 != NULL) {
							if(strcmp((const char*)n2->name, "ComponentType") == 0) {
								xmlNode *n3 = n2->children;
								comp.ID = 0;
								comp.Name = "";
								while(n3 != NULL) {
									if(strcmp((const char*)n3->name, "ID") == 0) {
										if(n3->children != NULL ) comp.ID = atoi( (const char*)(n3->children->content)) ;
									} else {
										if (strcmp((const char*)n3->name, "Name") == 0) {
											if(n3->children != NULL ) comp.Name = (const char*)(n3->children->content);
										}
									}
									n3 = n3->next;
								}
								theCompoList->push_back(comp);
							}
							n2 = n2->next;
						}
					}
					n1 = n1->next;
				}
			}
			nod = nod->next;
		}

		free(stringresult);
		xmlFreeDoc(doc);       // free document
		xmlCleanupParser();

		SetResponse(AlpideTable::NoError, 0,0);
	}
	return(theCompoList);
}

/* ----------------------
 * Get the list of type results ...
 *
 *
----------------------- */
std::vector<ActivityDB::resultType> *ActivityDB::GetResultList(int aActivityID)
{
	vector<resultType> *theResultList = new vector<resultType>;


	char *stringresult;
	string theUrl;
	string theQuery;
	resultType resu;

	theUrl = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
	theQuery = "activityTypeID="+std::to_string(aActivityID);

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
		return(theResultList);
	} else {

		xmlDocPtr doc;
		doc = xmlReadMemory(stringresult, strlen(stringresult), "noname.xml", NULL, 0); // parse the XML
		if (doc == NULL) {
		    cerr << "Failed to parse document" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theResultList);
		}

		// Get the root element node
		xmlNode *root_element = NULL;
		root_element = xmlDocGetRootElement(doc);
		if(root_element == NULL) {
		    cerr << "Failed Bad XML format no root element" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theResultList);
		}
		xmlNode *nod = root_element->children;
		while (nod != NULL) {
			if(strcmp((const char*)nod->name, "Result") == 0) {
				xmlNode *n1 = nod->children;
				while(n1 != NULL) {
					if(strcmp((const char*)n1->name, "ActivityTypeResultFull") == 0) {
						xmlNode *n2 = n1->children;
						resu.ID = 0;
						resu.Name = "";
						while(n2 != NULL) {
							if(strcmp((const char*)n2->name, "ID") == 0) {
								if(n2->children != NULL) resu.ID = atoi( (const char*)(n2->children->content)) ;
							} else {
								if (strcmp((const char*)n2->name, "Name") == 0) {
									if(n2->children != NULL)  resu.Name = (const char*)(n2->children->content);
								}
							}
							n2 = n2->next;
						}
						theResultList->push_back(resu);
					}
					n1 = n1->next;
				}
			}
			nod = nod->next;
		}

		free(stringresult);
		xmlFreeDoc(doc);       // free document
		xmlCleanupParser();

		SetResponse(AlpideTable::NoError, 0,0);
	}
	return(theResultList);
}


/* ----------------------
 * Get the list of type results ...
 *
 *
----------------------- */
std::vector<ActivityDB::statusType> *ActivityDB::GetStatusList(int aActivityID)
{
	vector<statusType> *theStatusList = new vector<statusType>;


	char *stringresult;
	string theUrl;
	string theQuery;
	statusType stat;

	theUrl = theParentDB->GetQueryDomain() + "/ActivityTypeReadAll";
	theQuery = "activityTypeID="+std::to_string(aActivityID);

	if( theParentDB->GetManagerHandle()->makeDBQuery(theUrl, theQuery.c_str(), &stringresult) == 0) {
		SetResponse(AlpideTable::SyncQuery);
		return(theStatusList);
	} else {

		xmlDocPtr doc;
		doc = xmlReadMemory(stringresult, strlen(stringresult), "noname.xml", NULL, 0); // parse the XML
		if (doc == NULL) {
		    cerr << "Failed to parse document" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theStatusList);
		}

		// Get the root element node
		xmlNode *root_element = NULL;
		root_element = xmlDocGetRootElement(doc);
		if(root_element == NULL) {
		    cerr << "Failed Bad XML format no root element" << endl;
		    SetResponse(AlpideTable::BadXML, 0,0);
			return(theStatusList);
		}
		xmlNode *nod = root_element->children;
		while (nod != NULL) {
			if(strcmp((const char*)nod->name, "Status") == 0) {
				xmlNode *n1 = nod->children;
				while(n1 != NULL) {
					if(strcmp((const char*)n1->name, "ActivityStatus") == 0) {
						xmlNode *n2 = n1->children;
						stat.ID = 0;
						stat.Code = "";
						stat.Description = "";
						while(n2 != NULL) {
							if(strcmp((const char*)n2->name, "ID") == 0) {
								if(n2->children != NULL)  stat.ID = atoi( (const char*)(n2->children->content)) ;
							} else {
								if (strcmp((const char*)n2->name, "Code") == 0) {
									if(n2->children != NULL)  stat.Code = (const char*)(n2->children->content);
								} else {
									if (strcmp((const char*)n2->name, "Description") == 0) {
										if(n2->children != NULL) stat.Description = (const char*)(n2->children->content);
									}
								}
							}
							n2 = n2->next;
						}
						theStatusList->push_back(stat);
					}
					n1 = n1->next;
				}
			}
			nod = nod->next;
		}

		free(stringresult);
		xmlFreeDoc(doc);       // free document
		xmlCleanupParser();

		SetResponse(AlpideTable::NoError, 0,0);
	}
	return(theStatusList);
}



/* ----------------------
 * Converts the string into the URI encoding
 *
 *  not used !
----------------------- */
int ActivityDB::buildUrlEncoded(string aLocalFileName, string *Buffer)
{
	FILE *fh = fopen(aLocalFileName.c_str(), "rb");
	if(fh == NULL) {
		cerr << "Failed to open the file :" << aLocalFileName << "Abort !" << endl;
		return(0);
	}

	char exa[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	unsigned char ch;

	ch =  (unsigned char)fgetc(fh);
	while(!feof(fh)) {
		if(isalnum(ch) || ch == '~' || ch == '-' || ch == '.' || ch == '_') {  //rfc3986
			*Buffer += ch;
		} else {
			*Buffer += '%';
			*Buffer += exa[ch >> 4];
			*Buffer += exa[ch &  4]; //YCM:FIXME if fix was not intended by [ch && 4]
		}
		ch = (unsigned char)fgetc(fh);
	}
	return(Buffer->size());
}

/* ----------------------
 * Converts the string into the Base64 coding
 *
 * must be improved !
----------------------- */
unsigned long ActivityDB::buildBase64Binary(string aLocalFileName, string *Buffer)
{

	static const string base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned long theBufferLength = Buffer->size();

	FILE *fh = fopen(aLocalFileName.c_str(), "rb");
	if(fh == NULL) {
		cerr << "Failed to open the file :" << aLocalFileName << "Abort !" << endl;
		return(0);
	}

	unsigned char cBufferIn[3];
	unsigned char cBufferOut[4];

	int i = 0;
	int j = 0;
	unsigned char ch;

	ch =  (unsigned char)fgetc(fh);
    while(!feof(fh)) {
    	cBufferIn[i++] = ch;// *(bytes_to_encode++);
    	if (i == 3) {
    		cBufferOut[0] = (cBufferIn[0] & 0xfc) >> 2;
    		cBufferOut[1] = ((cBufferIn[0] & 0x03) << 4) + ((cBufferIn[1] & 0xf0) >> 4);
    		cBufferOut[2] = ((cBufferIn[1] & 0x0f) << 2) + ((cBufferIn[2] & 0xc0) >> 6);
    		cBufferOut[3] = cBufferIn[2] & 0x3f;
			for(i = 0; (i<4) ; i++) *Buffer += base64chars[cBufferOut[i]];
			i = 0;
		}
		ch = (unsigned char)fgetc(fh);
	}
    if (i) {
    	for(j = i; j < 3; j++) cBufferIn[j] = '\0';
    	cBufferOut[0] = ( cBufferIn[0] & 0xfc) >> 2;
    	cBufferOut[1] = ((cBufferIn[0] & 0x03) << 4) + ((cBufferIn[1] & 0xf0) >> 4);
    	cBufferOut[2] = ((cBufferIn[1] & 0x0f) << 2) + ((cBufferIn[2] & 0xc0) >> 6);
    	cBufferOut[3] =   cBufferIn[2] & 0x3f;
	    for (j = 0; (j < i + 1); j++) *Buffer += base64chars[cBufferOut[j]];
	    while((i++ < 3)) *Buffer += '=';
    }
    theBufferLength = Buffer->size() - theBufferLength;
	return(theBufferLength);
}
