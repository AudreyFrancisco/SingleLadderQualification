// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e. fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules, 
// all chip accesses should be done with a loop over all elements of the chip vector. 
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);  
// For an example how to access board-specific functions see the power off at the end of main. 
//
// The functions that should be modified for the specific test are configureChip() and main()


#include <unistd.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include "TConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"


#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

#include "adcmon.h"
#include "getpot.h"

class CProgParams {
private:
	CProgParams(){}; // empty constructor is forbidden
public:
	CProgParams(int& argc, char* argv[]);
	bool IsValid(){return(BIsValid);};
	void Print();
public:
	string StrMode		;
	int NInputSel		;
	int NCalInputSel	;
	int NIcomp			;
	int NRampSpd		;
	int NScanDist		;
	bool BIsVerbose		;
	bool BKeepAlive		;
	bool BScanDac		;
	string StrFile		;
	bool BRunADCEval	;
	bool BReverse		;
	bool BPreset		;
protected:
	bool BIsValid;
};








CProgParams::CProgParams(int& argc, char* argv[]) {
	BIsValid	=true	;
	BIsVerbose	=false	;
	BKeepAlive	=false	;
	BScanDac	=false	;
	BRunADCEval	=false	;
	BReverse	=false	;
	BPreset		=false	;
	GetPot args(argc, argv);
	if((argc<=1)||(args.search(2, "--help","-h"))) {
		cout<<" * "<<argv[0]<<" [mode=auto] [input=0] [calinput=0] [icomp=3] [rampspeed=1] [scandist=1]"<<endl;
		cout<<" * "<<endl;
		cout<<" * "<<"Modifiers:"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -h or --help    === this help display"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -e or --eval    === evaluate internal ADC performances: discards any other attribute"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -v or --verbose === activate the printout of results"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -f file.txt or --file file.txt === save output into an ASCII file"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -k or --keepalive === don't shutdown power supply when exiting program"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -s or --scandac === makes a scan dac (only for mode=manual or supermanual)"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -r or --reverse === the ramps are operated in reverse order (up downto 0...)"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -p or --preset === preset input to 64"<<endl;
		cout<<" * "<<endl;
		cout<<" * "<<argv[0]<<" Inputs:"<<endl;
		cout<<" * "<<"   mode        === can be automatic (or auto), manual (or man), calibration (or cal), supermanual (or su)."<<endl;
		cout<<" * "<<"   input       === integer from 0 to 21 (see below). If -1, or not specified, scan over all inputs (discards the -s option)."<<endl;
		cout<<" * "<<"   calinput    === integer from 0 to 21 (see below)."<<endl;
		cout<<" * "<<"   icomp       === Comparator current (0:163μA - 1:190μA - 2:296μA (nominal) - 3:410 μA)."<<endl;
		cout<<" * "<<"   rampspeed   === Ramp Speed (0:500ns/spl - 1:1us/spl (nominal) - 2:2us/spl - 3:4us/spl)."<<endl;
		cout<<" * "<<"   scandist    === Step for the SCAN DAC (only used in conjonction with -s)."<<endl;
		cout<<" * "<<endl;
		cout<<" * "<<"   input & calinput shall be integers choosen as follows:"<<endl;
		cout<<" * "<<"   0:AVSS - 1:DVSS - 2:AVDD - 3:DVDD"<<endl;
		cout<<" * "<<"   4:Vbandgap through voltage scaling (the measure is known to be incorrect for this input)"<<endl;
		cout<<" * "<<"   5:VCASN - 6:VCASP - 7:VPULSEH - 8:VPULSEL - 9:VRESETP - 10:VRESETD"<<endl;
		cout<<" * "<<"   11:VCASN2 - 12:VCLIP - 13:VTEMP - 14:IRESET - 15:IAUX2 - 16:IBIAS - 17:IDB"<<endl;
		cout<<" * "<<"   18:IREF the value is not correct for this input"<<endl;
		cout<<" * "<<"   19:ITHR - 20:BANDGAP - 21:TEMPERATURE"<<endl;
		BIsValid=false;
	}
	if(args.search(2, "--eval","-e")) {
		BRunADCEval=true;
	}
	StrMode				=args(	"mode"				,"auto"		);
	if (!(
			(StrMode=="auto")||(StrMode=="automatic") 		||
			(StrMode=="cal")||(StrMode=="calibration") 		||
			(StrMode=="man")||(StrMode=="manual")			||
			(StrMode=="su")||(StrMode=="supermanual")		)) {
		cout<<"authorized values for Mode are: auto, automatic, cal, calibration, man, manual, su, supermanual"<<endl;
		BIsValid=false;
	}
	NInputSel		=args(	"input"		,-1			);
	if(
			(NInputSel<-1) ||
			(NInputSel>21)) {
		cout<<"InputSel shall be an integer between 0 and 21 (0 and 21 are allowed)"<<endl;
		BIsValid=false;
	}
	NCalInputSel		=args(	"calinput"	,0			);
	if(
			(NCalInputSel<0) ||
			(NCalInputSel>21)) {
		cout<<"CalInputSel shall be an integer between 0 and 21 (0 and 21 are allowed)"<<endl;
		BIsValid=false;
	}
	NIcomp				=args(	"icomp"				,2			);
	if(
			(NIcomp<0) ||
			(NIcomp>3)) {
		cout<<"Icomp shall be an integer between 0 and 3 (0 and 3 are allowed)"<<endl;
		BIsValid=false;
	}
	NRampSpd			=args(	"rampspeed"			,1			);
	if(
			(NRampSpd<0) ||
			(NRampSpd>3)) {
		cout<<"RampSpeed shall be an integer between 0 and 3 (0 and 3 are allowed)"<<endl;
		BIsValid=false;
	}
	NScanDist			=args(	"scandist"			,1			);
	if(
			(NScanDist<1) ||
			(NScanDist>256)) {
		cout<<"ScanDist shall be an integer between 1 and 255 (1 and 255 are allowed)"<<endl;
		BIsValid=false;
	}

	if(args.search(2, "-f", "--file") ) {
		StrFile=args.next("adcmon_acq");
	} else {
		remove("adcmon_acq.txt");
		remove("adcmon_acq.dat");
		StrFile="adcmon_acq";
	}
	// remove extension
	StrFile=StrFile.substr(0, StrFile.find_last_of("."));

	
	
	if(args.search(2, "--preset","-p")) {
		BPreset=true;
	}
	if(args.search(2, "--reverse","-r")) {
		BReverse=true;
	}
	if(args.search(2, "--keepalive","-k")) {
		BKeepAlive=true;
	}
	if(args.search(2, "--scandac","-s")) {
		if((StrMode=="auto")||(StrMode=="automatic")||(StrMode=="cal")||(StrMode=="calibration")) {
			cout<<"scan over values not allowed in calibration and automatic modes: -s option is discarded"<<endl;
			BScanDac=false;
		} else 
		if(NInputSel==-1) {
			cout<<"scan over inputs AND scan over values not allowed: -s option is discarded"<<endl;
			BScanDac=false;
		} else {
			BScanDac=true;
		}
	}
	if(args.search(2, "--verbose","-v")) {
		BIsVerbose=true;
	}
	vector<string> ufos=args.unidentified_variables();
	if(ufos.size()>0) {
		BIsValid=false;
		for(int i=0;i<ufos.size();i++) {
				cout<<"Unknown variable: "<<ufos[i]<<endl;
		}
		cout<<"NOTHING was done: Exit"<<endl;
	}
}

void CProgParams::Print() {
	cout<<"Mode is "<<StrMode<<endl;
	cout<<"Input selected is "<<NInputSel<<endl;
	cout<<"Input selected for calibration is "<<NCalInputSel<<endl;
	cout<<"Comparator Current selected is "<<NIcomp<<endl;
	cout<<"Ramp Speed selected is "<<NRampSpd<<endl;
	return;
}
	









TReadoutBoardDAQ *myDAQBoard;



int configureChip(TAlpide *chip) {
  // put all chip configurations before the start of the test here
  chip->WriteRegister (Alpide::REG_MODECONTROL,   0x20);
  chip->WriteRegister (Alpide::REG_CMUDMU_CONFIG, 0x60);
}



int main(int argc, char* argv[]) {
	CProgParams lparams(argc,argv);
	if(!lparams.IsValid()) {
		return(0);
	}

  initSetup();

  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (!myDAQBoard) {
    std::cout << "This test works only with DAQ board" << std::endl;
    exit(1);
  }

  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (int i = 0; i < fChips.size(); i ++) {
      configureChip (fChips.at(i));
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

		TAlpide *chip   = fChips.at(0);

		CAdcMon lAdcMon(myDAQBoard,chip,lparams.NIcomp,lparams.NRampSpd);
		lAdcMon.BIsVerbose=lparams.BIsVerbose;
		lAdcMon.BReverse=lparams.BReverse;
		if(lparams.BPreset==true) {
			lAdcMon.Preset(lparams.NInputSel);
		}
		if(lparams.BRunADCEval==true) {
			lAdcMon.AdcEval();
			lAdcMon.VerboseIt();
			lAdcMon.SaveToTxtFile(lparams.StrFile);
			lAdcMon.SaveToDatFile(lparams.StrFile);
		} else		
		if(lparams.BScanDac==false) {
			if((lparams.StrMode=="auto")||(lparams.StrMode=="automatic")) {
Debug("Uncomment to allow this measure");lAdcMon.Calibrate(lparams.NCalInputSel);//			lAdcMon.AutomaticMeasure(lparams.NCalInputSel);
			} else
			if((lparams.StrMode=="cal")||(lparams.StrMode=="calibration")) {
				lAdcMon.Calibrate(lparams.NCalInputSel);
			} else
			if((lparams.StrMode=="man")||(lparams.StrMode=="manual")) {
				lAdcMon.ManualMeasure(lparams.NInputSel,lparams.NCalInputSel);
			} else
			if((lparams.StrMode=="su")||(lparams.StrMode=="supermanual")) {
				lAdcMon.SuperManualMeasure(lparams.NInputSel);
			}	
			lAdcMon.VerboseIt();
			lAdcMon.SaveToTxtFile(lparams.StrFile);
			lAdcMon.SaveToDatFile(lparams.StrFile);
		} else {
			if((lparams.StrMode=="man")||(lparams.StrMode=="manual")) {
				lAdcMon.ScanManual(lparams.NInputSel,lparams.NCalInputSel,lparams.NScanDist);
			} else
			if((lparams.StrMode=="su")||(lparams.StrMode=="supermanual")) {
				lAdcMon.ScanSuperManual(lparams.NInputSel,lparams.NScanDist);
			}	
			lAdcMon.VerboseIt();
			lAdcMon.SaveToTxtFile(lparams.StrFile);
			lAdcMon.SaveToDatFile(lparams.StrFile);
		}






    if (myDAQBoard) {
		if(lparams.BKeepAlive==false) {
  			std::cout << "Powering off chip" << std::endl;
			myDAQBoard->PowerOff();
		}
		delete myDAQBoard;
    }
  }

  return 0;
}
