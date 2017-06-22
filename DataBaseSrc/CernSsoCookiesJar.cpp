/*
 * \file CernSsoCookiesJar.cpp
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
 *  Description : Cookie Jar Class extension
 *                for manage the CERN SSO autentication method
 *
 *  HISTORY
 *
 *
 */
#include "utilities.h"
#include "CernSsoCookiesJar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
// / #include <string>


CernSsoCookieJar::CernSsoCookieJar() //: QNetworkCookieJar(parent)
{
    if( !testTheCERNSSO()) {
        exit(1);
    }
    theCliCert = "";
    theCliKey = "";
    theSslUrl = "";
}

CernSsoCookieJar::~CernSsoCookieJar()
{

}

/* -----------------------------------
 * examin if the Jar is valid
 * ----------------------------------- */
bool CernSsoCookieJar::isJarValid()
{
	/*
    QList<QNetworkCookie> ActualJar;
    ActualJar = allCookies();
    if(ActualJar.size() == 0) { // it is empty
        return(false);
    }
    for(int i=0;i<ActualJar.size();i++) { // test the expiration datetime
        QDateTime expire;
        expire = ActualJar.at(i).expirationDate();
        if(expire < QDateTime::currentDateTime()) {
            qDebug() << "A cookie is expired !";
            return(false);
        }
    }
    */
    return(true);
}




/* -----------------------------------
 * gets the CERN SSO Cookies and fills
 * the cookie jar
 * ----------------------------------- */
bool CernSsoCookieJar::fillTheJar(string aCliCert, string aCliKey, string aSslUrl)
{
    theCliCert = aCliCert;
    theCliKey = aCliKey;
    theSslUrl = aSslUrl;
    return(fillTheJar());
}

bool CernSsoCookieJar::fillTheJar()
{
    // run the CERN SSO in order to obtain the cookies file
    string CookieFileName =  "/tmp/cerncookie.txt";
    if(remove(CookieFileName.c_str())) {//the file exists... delete!
        cout << "The " << CookieFileName << " file deleted.";
    }
    string Command = "cern-get-sso-cookie ";
    Command += " --cert ";
    Command += theCliCert;
    Command += " --key ";
    Command += theCliKey;
    Command += " -r -u ";
    Command += theSslUrl;
    Command += " -o ";
    Command += CookieFileName;

    system(Command.c_str());
    cout << "Execute the bash :" << Command;
    if(!fileExists(CookieFileName)) { //the file doesn't exists. ACH !
        cerr << "Error to obtain the CERN SSO Cookies pack file. Abort !";
        return(false);
    }
/*
    // Parse the CERN SSO file
    QList<QNetworkCookie> *CernCookiePack = new QList<QNetworkCookie>;
    QNetworkCookie *Cookie = new QNetworkCookie();

    QFile file(CookieFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Error to open the CERN SSO Cookies pack file. Abort !";
        return(false);
    }
    QTextStream in(&file);
    QString line;
    QDateTime exp;
    QStringList fields;

    in.setCodec("UTF-8"); // change the file codec to UTF-8.
    while(!in.atEnd()) {
        line = in.readLine();
        if(line.size() == 0) continue;
        if(line.mid(0,1) == "#") continue;
        fields = line.split('\t');
        Cookie->setDomain(fields.at(0).toAscii());
        exp.setTime_t(fields.at(4).toUInt());
        Cookie->setExpirationDate(exp);
        Cookie->setHttpOnly(true);
        Cookie->setName(fields.at(5).toAscii());
        Cookie->setPath(fields.at(2).toAscii());
        Cookie->setSecure(fields.at(3) == "TRUE" ? true : false);
        Cookie->setValue(fields.at(6).toAscii());
        qDebug() << "Cookie : " << Cookie->toRawForm();
        CernCookiePack->push_back(*Cookie);
    }
    file.close();

    this->setAllCookies(*CernCookiePack);

 */
    return(true);
}



/* ------------------------------------
 * tests the presence of CERNSSO
 * ------------------------------------ */
bool CernSsoCookieJar::testTheCERNSSO()
{

    remove("/tmp/exitus.txt");

    string Command = "type cern-get-sso-cookie > /tmp/exitus.txt";
    system(Command.c_str());
    cout << "Execute : " << Command << endl;

    FILE *result;
    result = fopen("/tmp/exitus.txt", "r");
    if (result == NULL) {
        cerr << "Error to access /tmp folder ! Abort" << endl;
        return(false);
    }

    char buffer[512];
    if( fgets(buffer,511,result) != NULL && strstr(buffer,"cern-get-sso-cookie is") !=  NULL) {
    	cout << "The CERN-SSO package is installed !" << endl;
	fclose(result);    	
	return(true);
    } else {
    	cerr << "CERN-SSO package not Found ! Abort" << endl;
	fclose(result);    	
	return(false);
    }
    return(false);
}





