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

public:
	explicit AlpideTable(AlpideDB *DBhandle);
    virtual ~AlpideTable();
    response *DecodeResponse(char *returnedString);
    void SetResponse(AlpideTable::ErrorCode, int ID=0, int Session=0);

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
	int GetList(vector<ProjectDB::project> *Result);
	string print(ProjectDB::project *pr) { return( "Project : ID=" + std::to_string(pr->ID) +  " Name=" + pr->Name);};

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

	int GetList(int projectID, vector<member> *Result);
	string print(member *me) { return( "Member : ID=" + std::to_string(me->ID) +
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

	int GetType(int ComponentTypeID , componentType *Result);
	int GetTypeList(int ProjectID , vector<componentType> *Result);


	response *Create(string ComponentTypeID, string ComponentID, string SupplyCompID,
			string Description, string lotID, string PackaageID, string userID );

	const char *print(componentType *co);

private:
	void extractTheComponentType(xmlNode *n1, componentType *pro);

};

#endif /* ALPIDEDBENDPOINTS_H_ */
