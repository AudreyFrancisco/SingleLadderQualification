/* ---------------
 * Example of MOSAIC use
 *
 ----------------- */

#include <iostream>
#include <unistd.h>
#include "TReadoutBoard.h"
#include "TReadoutBoardMOSAIC.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "TPowerBoard.h"
#include "TConfig.h"
#include "TAlpide.h"
#include <exception>

#include <strings.h>

using namespace std;


// Global Variables
TPowerBoard *thePowerBoard;


// Display the Status of the Power BOARD
void DisplayStatus(int numModules)
{
	float AV,AI,DV,DI;
	bool BiON,AChON,DChON;

	cout << endl << "---- POWER BOARD : Defined Modules :" << numModules << " -----" << endl;
	cout << "The board is connected and ready : " << (thePowerBoard->IsOK() ? "YES" : "NOT") << endl;
	cout << "Back Bias   Voltage = " << thePowerBoard->GetBiasVoltage() << "    Current = " << thePowerBoard->GetBiasCurrent() << endl;
	cout << "Temperature = " << thePowerBoard->GetTemperature() << "C" << endl;
	for(int i=0;i<numModules; i++) {
		thePowerBoard->GetModule(i, &AV,&AI,&DV,&DI,&BiON,&AChON,&DChON);
		cout << " ------------ " << endl;
		cout << " Module :" << i << "    Back Bias :" << (BiON ? "ON" : "off") << endl;
		cout << "    Analogic channel :  Voltage: " << AV << "   Current:" << AI << "    State =:" << (AChON ? "ON" : "off") << endl;
		cout << "    Digital channel :  Voltage: " << DV << "   Current:" << DI << "    State =:" << (DChON ? "ON" : "off") << endl;
	}
	cout << " ------------ " << endl;
	return;
}

// Display the menu of commands
void DisplayMenu(int numModules)
{
	cout << endl << "****  POWER BOARD MENU  ****" << endl;
	cout << " 1) \t Display status" << endl;
	cout << " 2) \t Set module" << endl;
	cout << " 3) \t Switch one module" << endl;
	cout << " 4) \t Switch ON all modules" << endl;
	cout << " 5) \t Switch OFF all modules" << endl;
	cout << " 6) \t Save Configuration file" << endl;
	cout << " 7) \t Load Configuration File" << endl;
	cout << endl;

	cout << " 0) \t Exit" << endl;

	return;
}

int GetChoice()
{
	int cho;
	cout << endl << "Type the choice : " ;
	scanf("%d",&cho);
	if(cho == 0) cho = -1;
	cout << endl;
	return(cho);
}

void DoJob(int choice, int numModules)
{
	int mod;
	float AV,AI,DV,DI;
	bool BiON,AChON,DChON;
	char buf[255];
	TPowerBoardConfig *theConfig;

	switch(choice) {
	case 1:
		DisplayStatus(numModules);
		break;
	case 2:
		cout << endl << "What module :" ;scanf("%d",&mod);
		if(mod <0 && mod >= numModules) return;
		cout << endl << "Analog voltage :" ;scanf("%f",&AV);
		cout << endl << "Analog max current :" ;scanf("%f",&AI);
		cout << endl << "Digital voltage :" ;scanf("%f",&DV);
		cout << endl << "Digital max current :" ;scanf("%f",&DI);
		cout << endl << "Back Bias [ON,OFF] :" ;scanf("%s",buf);
		BiON = (strcasecmp((const char *)buf, "ON") == 0) ?  true : false;
		thePowerBoard->SetModule(mod,AV,AI,DV,DI,BiON);
		break;
	case 3:
		cout << endl << "What module to switch:" ;scanf("%d",&mod);
		if(mod <0 && mod >= numModules) return;
		cout << endl << "Module [ON,OFF] :" ;scanf("%s",buf);
		AChON = (strcasecmp((const char *)buf, "ON") == 0) ?  true : false;
		thePowerBoard->SwitchModule(mod, AChON);
		break;
	case 4:
		thePowerBoard->SwitchON();
		break;
	case 5:
		thePowerBoard->SwitchOFF();
		break;
	case 6:
		cout << endl << "The Configuration File name to write :" ;scanf("%s",buf);
		theConfig = thePowerBoard->GetConfigurationHandler();
		theConfig->WriteToFile(buf);
		break;
	case 7:
		cout << endl << "The Configuration File name to read:" ;scanf("%s",buf);
		theConfig = thePowerBoard->GetConfigurationHandler();
		theConfig->ReadFromFile(buf);
		break;
	default:
		break;
	}
	return;
}


int main()
{
	TBoardConfigMOSAIC 	*theBoardConfiguration;
	TReadoutBoardMOSAIC 	*theBoard;

	TConfig *config = new TConfig ("Config.cfg");
	theBoard = new TReadoutBoardMOSAIC(config, (TBoardConfigMOSAIC*)config->GetBoardConfig(0));

	// Create an Object : Power Board
	thePowerBoard = new TPowerBoard(theBoard);
	if(thePowerBoard == NULL) {
		cerr << "Error to create Power Board Object ! Abort" << endl;
	}

	int numModules = 2; // just 2 of the 8 modules

	DisplayStatus(numModules);
	int choice = 0;
	while(choice != -1) {
		DisplayMenu(numModules);
		choice = GetChoice();
		DoJob(choice, numModules);
	}

    return 0;
}
