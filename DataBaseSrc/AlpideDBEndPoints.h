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
public:
	struct composition {
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
	struct component {
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

	int Get(int ComponentTypeID , component *Result);
	int GetList(int ProjectID , vector<component> *Result);
	response *Create(string ComponentTypeID, string ComponentID, string SupplyCompID,
			string Description, string lotID, string PackaageID, string userID );

	string print(component *co) {
		string ap;
		ap = "Component : ID=" + std::to_string(co->ID) +
			" Name="+ co->Name + " Code=" + co->Code + " Description=" + co->Description + "\n";
		ap += "   Composition : {";
		for(int i=0;i<co->Composition.size();i++)
			ap+= "( Type="+co->Composition.at(i).ComponentType+
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

private:
	void extractTheComponent(xmlNode *n1, component *pro);

};

#endif /* ALPIDEDBENDPOINTS_H_ */
