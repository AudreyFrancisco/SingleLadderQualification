/*
 * \file AlpideDB.cpp
 * \author A.Franco
 * \date 16/Mar/2017
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
#include "AlpideDB.h"
#include "utilities.h"

#include <curl/curl.h>

AlpideDB::AlpideDB()
{
	curl_global_init( CURL_GLOBAL_ALL );

	theQueryDomain = "https://test-alucmsapi.web.cern.ch/AlucmswebAPI.asmx/";

    theDBmanager = new AlpideDBManager();
    theDBmanager->Init("/home/fap/.globus/usercert.pem",
    					"/home/fap/.globus/userkey.key",
						"https://test-alucmsapi.web.cern.ch",
						"/etc/ssl/certs/");

}


AlpideDB::~AlpideDB()
{
}

/*
int AlpideDB::XMLResultsDecode(QByteArray RecordTag, QByteArray *SourceString,  QList<QList<QVariant> > *queryResult, QList<int> *FieldsFormat)
{
	QDomDocument doc;
	QList<QVariant> appo;
	QDomNode node;
	int nFields =0;
	QString buf;
	if(FieldsFormat == 0) FieldsFormat = new QList<int>;

	doc.setContent(*SourceString, true); // parse the XML
	QDomNodeList nodesList=doc.elementsByTagName(RecordTag); // extracts all the nodes tagged

	for(int i=0; i<nodesList.size(); i++) { // for each record
		QDomNodeList elem = nodesList.at(i).childNodes(); // take all the fields
		if(i==0) { // take the headers, the fields type and the record dimension
			nFields = elem.size();
			appo.clear();
			for(int j=0;j<nFields;j++) {
				appo.append( elem.item(j).nodeName());
				if(FieldsFormat->size() <= j) FieldsFormat->append(StringFormat);
			}
			queryResult->append(appo);
		}
		appo.clear();
		for(int j=0;j<nFields;j++) { // now take the records
			buf = elem.item(j).firstChild().nodeValue();
			switch(FieldsFormat->at(j)) {
			case IntFormat:
				appo.append( QVariant( buf.toLongLong()) );
				break;
			case FloatFormat:
				appo.append( QVariant( buf.toDouble()) );
				break;
			case StringFormat:
				appo.append( QVariant( buf ) );
				break;
			case CharFormat:
				appo.append( QVariant( buf.mid(0,1) ) );
				break;
			case BoolFormat:
				appo.append( QVariant( (buf.toUpper() == "TRUE" ? true : false) ) );
				break;
			default:
				appo.append( QVariant( buf ) );
				break;
			}
		}
		queryResult->append(appo);
	}
	return(queryResult->size()-1);
}

int AlpideDB::XMLResultsDecode(char *RecordTag, char *SourceString,  vector<vector<string> > *queryResult)
{
//	QDomDocument doc;
//	QList<QString> appo;
//	QDomNode node;

	int nFields =0;
	QString buf;
	qDebug() << *SourceString;
	doc.setContent(*SourceString, true); // parse the XML
	QDomNodeList nodesList=doc.elementsByTagName(RecordTag); // extracts all the nodes tagged
	qDebug() << " Woh many " << nodesList.size();
	for(int i=0; i<nodesList.size(); i++) { // for each record
		QDomNodeList elem = nodesList.at(i).childNodes(); // take all the fields
		if(i==0) { // take the headers, the fields type and the record dimension
			nFields = elem.size();
			appo.clear();
			for(int j=0;j<nFields;j++) {
				appo.append( elem.item(j).nodeName());
			}
			queryResult->append(appo);
		}
		appo.clear();
		for(int j=0;j<nFields;j++) { // now take the records
			appo.append( elem.item(j).firstChild().nodeValue());
		}
		queryResult->append(appo);
	}
	return(queryResult->size()-1);
}

*/
