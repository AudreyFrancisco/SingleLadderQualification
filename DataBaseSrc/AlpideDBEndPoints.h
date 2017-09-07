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
 *  Description : Header file for the
 *
 *  HISTORY
 *
 */
#ifndef ALPIDEDBENDPOINTS_H_
#define ALPIDEDBENDPOINTS_H_

#include <iostream>
#include <string>
#include <vector>


#include <libxml/tree.h>
#include <libxml/parser.h>

#include "utilities.h"

using namespace std;


class AlpideDB;

// The base class
class AlpideTable
{
public:
	enum ErrorCode { NoError=0, SyncQuery=20, BadXML=21 };

struct response {
	int Session;
	int ErrorCode;
	string ErrorMessage;
	int ID;
};

protected:
	AlpideDB * theParentDB;
	response theResponse;
	string   theGeneralBuffer;

public:
	explicit AlpideTable(AlpideDB *DBhandle);
    virtual ~AlpideTable();
    response *DecodeResponse(char *returnedString, int Session = 0);
    void SetResponse(AlpideTable::ErrorCode, int ID=0, int Session=0);
    const char *DumpResponse();

protected:
    bool _parseTheXML(char *stringresult, xmlDocPtr *doc);
    bool _getTheRootElementChildren(xmlDocPtr doc, xmlNode **nod);

};


class ProjectDB : public AlpideTable
{

// Members
public:
	struct project {
		int ID;
		string Name;
	};
private:
	project proj;

	// Methods
public:
	ProjectDB(AlpideDB * DBhandle);
    ~ProjectDB();

public:
    AlpideTable::response *GetList(vector<ProjectDB::project> *Result);
	string Print(ProjectDB::project *pr) { return( "Project : ID=" + std::to_string(pr->ID) +  " Name=" + pr->Name);};

};

class MemberDB : public AlpideTable
{

// Members
public:
	struct member {
		int ID;
		int PersonalID;
		string FullName;
	};

private:
	member memb;

// Methods
public:
	MemberDB(AlpideDB * DBhandle);
    ~MemberDB();

    AlpideTable::response *GetList(int projectID, vector<member> *Result);
	string Print(member *me) { return( "Member : ID=" + std::to_string(me->ID) +
			" Personal ID="+ std::to_string(me->PersonalID) + " Name=" + me->FullName);};

};

class ComponentDB : public AlpideTable
{

	// Members
private:
	string ap;

public:
	struct composition {
		int ID;
		string ComponentType;
		int Quantity;
	};
	struct statusphysical {
		int ID;
		string Name;
	};
	struct statusfunctional {
		int ID;
		string Name;
	};
	struct componentType {
		int ID;
		string Name;
		string Code;
		string Description;
		vector<composition> Composition;
		vector<statusphysical> PhysicalStatus;
		vector<statusfunctional> FunctionalStatus;
	} ;

// Methods
public:
	ComponentDB(AlpideDB * DBhandle);
    ~ComponentDB();

    AlpideTable::response *GetType(int ComponentTypeID , componentType *Result);
    AlpideTable::response *GetTypeList(int ProjectID , vector<componentType> *Result);

    AlpideTable::response *Create(string ComponentTypeID, string ComponentID, string SupplyCompID,
			string Description, string lotID, string PackaageID, string userID );

	string Print(componentType *co);

private:
	void extractTheComponentType(xmlNode *n1, componentType *pro);

};

// --------------------
class ActivityDB : public AlpideTable
{

	// Members
private:

public:
	struct member {
		int ID;
		int ProjectMember;
		int Leader;
		int User;
	};

	struct parameter {
		int ID;
		int ActivityParameter;
		float Value;
		int User;
	};

	struct attach {
		int ID;
		int Category;
		string RemoteFileName;
		string LocalFileName;
		int User;
	};

	struct parameterType {
		int ID;
		string Name;
		string Description;
	};
	struct activityType {
		int ID;
		string Name;
		string Description;
	};
	struct locationType {
		int ID;
		string Name;
	};
	struct componentType {
		int ID;
		string Name;
	};
	struct resultType {
		int ID;
		string Name;
	};
	struct statusType {
		int ID;
		string Code;
		string Description;
	};


	struct activity {
		int ID;
		int Type;
		int Location;
		string Lot;
		string Name;
		time_t StartDate;
		time_t EndDate;
		string Position;
		int Result;
		int Status;
		int User;
		vector<member> Members;
		vector<parameter> Parameters;
		vector<attach> Attachments;
	} ;


// Methods
public:
	ActivityDB(AlpideDB * DBhandle);
    ~ActivityDB();

    AlpideTable::response *Create(activity *aActivity);
    AlpideTable::response *AssignComponent(int aActivityID, int aComponentID, int aComponentTypeID, int aUserID);

    vector<parameterType> *GetParameterTypeList(int aActivityID);
    vector<activityType> *GetActivityTypeList(int aProjectID);
    vector<locationType> *GetLocationTypeList(int aActivityID);
    vector<componentType> *GetComponentTypeList(int aActivityID);
    vector<resultType> *GetResultList(int aActivityID);
    vector<statusType> *GetStatusList(int aActivityID);

private:
    unsigned long buildBase64Binary(string aLocalFileName, string * aBuffer);
    int buildUrlEncoded(string aLocalFileName, string *Buffer);

};








#endif /* ALPIDEDBENDPOINTS_H_ */
