#include <unistd.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "USBHelpers.h"
#include "TConfig.h"

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
	bool BIsVerbose		;
	string StrFile		;
protected:
	bool BIsValid;
};








CProgParams::CProgParams(int& argc, char* argv[]) {
	BIsValid=true;
	BIsVerbose=false;
	GetPot args(argc, argv);
	if((argc<=1)||(args.search(2, "--help","-h"))) {
		cout<<" * "<<argv[0]<<" [mode=auto] [input=0] [calinput=0] [icomp=3] [rampspeed=1]"<<endl;
		cout<<" * "<<endl;
		cout<<" * "<<"Modifiers:"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -h or --help    === this help display"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -v or --verbose === activate the printout of results"<<endl;
		cout<<" * "<<"  "<< argv[0]<<" -f file.txt or --file file.txt === save output into an ASCII file"<<endl;
		cout<<" * "<<endl;
		cout<<" * "<<argv[0]<<" Inputs:"<<endl;
		cout<<" * "<<"   mode        === can be automatic (or auto), manual (or man), calibration (or cal), supermanual (or su)."<<endl;
		cout<<" * "<<"   input       === integer from 0 to 21 (see below)."<<endl;
		cout<<" * "<<"   calinput    === integer from 0 to 21 (see below)."<<endl;
		cout<<" * "<<"   icomp       === Comparator current (0:163μA - 1:190μA - 2:296μA (nominal) - 3:410 μA)."<<endl;
		cout<<" * "<<"   rampspeed   === Ramp Speed (0:500ns/spl - 1:1us/spl (nominal) - 2:2us/spl - 3:4us/spl)."<<endl;
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
	StrMode				=args(	"mode"				,"auto"		);
	if (!(
			(StrMode=="auto")||(StrMode=="automatic") 		||
			(StrMode=="cal")||(StrMode=="calibration") 		||
			(StrMode=="man")||(StrMode=="manual")			||
			(StrMode=="su")||(StrMode=="supermanual")		)) {
		cout<<"autorized values for Mode are: auto, automatic, cal, calibration, man, manual, su, supermanual"<<endl;
		BIsValid=false;
	}
	NInputSel		=args(	"input"		,0			);
	if(
			(NInputSel<0) ||
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

	if(args.search(2, "-f", "--file") ) {
		StrFile=args.next("adcmon_acq.txt");
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
	
	
	
	
int main(int argc, char* argv[]) {
	CProgParams lparams(argc,argv);
	if(!lparams.IsValid()) {
		return(0);
	}


  uint16_t          status;
  uint32_t          version;
  int               overflow;
  TReadoutBoardDAQ  *myDAQBoard;
  TConfig *config = new TConfig (16);

  // if (config->BoardType == DAQBoard)
  // 
  //  The following code searches the USB bus for DAQ boards, creates them and adds them to the readout board vector: 
  //  TReadoutBoard *readoutBoard = new TReadoutBoardDAQ(device, config);
  //  board.push_back (readoutBoard);
  std::vector <TReadoutBoard *> boards;
  InitLibUsb(); 
  FindDAQBoards (config, boards);

  std::cout << "found " << boards.size() << " DAQ boards" << std::endl;
  
  if (boards.size() == 1) {
    TAlpide *chip   = new TAlpide (config->GetChipConfig(16));
    chip         -> SetReadoutBoard (boards.at(0));
    boards.at(0) -> AddChip (0, 0, 0);
    
    myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (boards.at(0));
    if (myDAQBoard) {
      if (myDAQBoard -> PowerOn (overflow)) std::cout << "LDOs are on" << std::endl;
      else std::cout << "LDOs are off" << std::endl;
      myDAQBoard->ReadRegister (0x602, version); // read firmware version
      std::cout << "Version = " << std::hex << version << std::dec << std::endl;
      myDAQBoard->WriteRegister (0x402, 3); // disable manchester encoding
      //myDAQBoard->WriteRegister (0x500, 0x0220);
      myDAQBoard -> SendOpCode (Alpide::OPCODE_GRST);
      sleep(1);
      std::cout << "Analog Current = " << myDAQBoard-> ReadAnalogI() << std::endl;
      std::cout << "Digital Current = " << myDAQBoard-> ReadDigitalI() << std::endl;
      
      chip -> WriteRegister (Alpide::REG_MODECONTROL, 0x20);
      chip -> WriteRegister (0xc, 0x40); // disable manchester encoding 
     
/*
	  //std::cout << "After Write register " << std::endl;
      chip -> ReadRegister (Alpide::REG_IBIAS, status);
      std::cout << "IBias register value: 0x" << std::hex << status << std::dec << std::endl;
      chip->WriteRegister (Alpide::REG_IBIAS, 0);
      sleep(1);
      std::cout << "Analog Current = " << myDAQBoard-> ReadAnalogI() << std::endl;

      AlpideConfig::ApplyStandardDACSettings (chip, 0);
      chip -> ReadRegister (Alpide::REG_VRESETD, status);

      std::cout << "Control register value: 0x" << std::hex << status << std::dec << std::endl;

      chip -> ReadRegister (Alpide::REG_IBIAS, status);
      std::cout << "IBias register value: 0x" << std::hex << status << std::dec << std::endl;
*/



		CAdcMon lAdcMon(myDAQBoard,chip,lparams.NIcomp,lparams.NRampSpd);
		lAdcMon.BIsVerbose=lparams.BIsVerbose;
		if((lparams.StrMode=="auto")||(lparams.StrMode=="automatic")) {
			lAdcMon.AutomaticMeasure(lparams.NCalInputSel);
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
		if(!lparams.StrFile.empty()) {
			lAdcMon.SaveToFile(lparams.StrFile);
		}

      //libusb_exit(fContext);
    }
    else {
      std::cout << "Type cast failed" << std::endl;
    }

  }

  return 0;
}
