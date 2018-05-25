/*
 * \file main_DBQueryProducer.cpp
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
 *  Description : Simple test program able to create JSON format
 *                Activity Creation files
 *
 *  HISTORY
 *
 *
 */
#include "DBHelpers.h"
#include "DBQueryQue.h"

#include <iostream>
#include <thread>
#include <unistd.h>


int main()
{
  AlpideDB *fDB;
  bool      fDatabasetype         = true;
  fDB                             = new AlpideDB(fDatabasetype);
  ActivityDB *         myactivity = new ActivityDB(fDB);
  ActivityDB::activity activ;

  myactivity->theAsyncronuosQueue->SetLocalCopyEnabled(true);
  myactivity->theAsyncronuosQueue->SetEstension(".dbq");
  myactivity->theAsyncronuosQueue->SetLocalCopyPath("/tmp/loc");
  myactivity->theAsyncronuosQueue->SetQueueBasePath("/tmp");
  myactivity->theAsyncronuosQueue->SetSpecificPath("GUIlocalQueryBackup");

  activ.Type      = 881;     // fIdofactivitytype;
  activ.Location  = 161;     // fIdoflocationtype;
  activ.User      = 1126;    // fIdofoperator;
  activ.StartDate = time(0); // date.currentDateTime().toTime_t();
  activ.EndDate   = time(0); // date.currentDateTime().toTime_t();
  activ.Lot       = " ";
  activ.Name = "Test of asyncronous act creation"; // CreateActivityName(fHICs.at(i)->GetDbId(),
                                                   // fConfig->GetScanConfig());
  activ.Position = " ";
  activ.Result = -999; // apparently has to stay open here, otherwise activity is considered closed
  activ.Status = 83;   // DbGetStatusId(fDB, fIdofactivitytype, "OPEN");
  DbAddParameter(fDB, activ, "Number of Working Chips", 10.0, "");
  DbAddParameter(fDB, activ, "IDDD", 10.1, "");
  DbAddAttachment(fDB, activ, attachLog, string("/tmp/Comment.txt"), string("Peppo"));
  DbAddMember(fDB, activ, 1126);
  DbAddUri(fDB, activ, "URI test 1", "http://www.cern.ch");
  DbAddInComp(fDB, activ, 11956, 1221);
  DbAddOutComp(fDB, activ, 11956, 1241);
  activ.Result = 822; // DbGetResultId(fDB, fIdofactivitytype, fHICs.at(i)->GetClassification());
  myactivity->CreateAsyncronous(&activ);

  return (0);
}
