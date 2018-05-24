/*
 * \file main_DBQueueConsumer.cpp
 * \author A.Franco
 * \date 16/Maggio/2018
 *
 * Copyright (C) 2018
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
 *  Description : An Activity Creation program that reads the
 *                activity structure from JSON format files
 *                and calls the DB API
 *
 *  HISTORY
 *
 *
 */
#include "DBQueryQue.h"

#include <unistd.h>
#include <iostream>
#include <thread>


int main()
{
    bool fDatabasetype = true;
    DBQueueConsumer* theQueue = new DBQueueConsumer(fDatabasetype);

    // Configuration
    theQueue->cfgExtension = ".dbq";
    theQueue->cfgBasePath = "/tmp";
    theQueue->cfgSpecificPath = "NewAlpide";
    theQueue->cfgStatusFile = "/tmp/DBQueueConsumerState.dat";
    theQueue->cfgMaxReconAtt = 9999;

    // Run the thread engine that consumes the queue
    thread engine(&DBQueueConsumer::Run, theQueue);

    // Start the console
    vector<string> list;
    char ch;
    do {
        cout<<"\nDBQ>";
        ch = getchar();
        switch(ch) {
        case 'c':
            cout << "** DBQueueConsumer Configuration **" << endl;
            cout << " DataBaseType :" << (!theQueue->bDatabasetype ?"PRODUCTION" : "TEST") << endl;
            cout << " Running on the Project : " << theQueue->GetProjectId() << endl;
            cout << " Status filename :" << theQueue->cfgStatusFile << endl;
            cout << " Queue file extension :" << theQueue->cfgExtension << endl;
            cout << " Queue files base path :" << theQueue->cfgBasePath << endl;
            cout << " Queue files specific path :" << theQueue->cfgSpecificPath << endl;
            cout << " Maximum reconnection attempts :" << theQueue->cfgMaxReconAtt << endl;
            break;
        case 's':
            cout << "** DBQueueConsumer Status **" << endl;
            cout << " File :" << theQueue->theFileName << endl;
            cout << " State :" << theQueue->theState << endl;
            cout << " The Queue has " << theQueue->GetQueueLenght() << " elements" << endl;
            break;
        case 'q':
            cout << "** DBQueueConsumer Queue **" << endl;
            list = theQueue->GetQueue();
            for(int i=0;i<list.size();i++)
                cout << list.at(i) << endl;
            break;
        case 'h':
        	case '?':
            cout << "** DBQueueConsumer Help **" << endl;
            cout << "c := print the configuration" << endl;
            cout << "s := print the status" << endl;
            cout << "q := dump the list of queued files" << endl;
            cout << "e := exit" << endl;
        }
    }
    while(ch!='e');

    theQueue->executeTheRun = false
    engine.join();

    cout <<  "Bye bye !" << endl;
    return(0);
}
