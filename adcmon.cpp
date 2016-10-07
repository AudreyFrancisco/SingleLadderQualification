#include <stdio.h>
#include <cstring>
#include <list>

#include <iostream>
#include <chrono>

#include <unistd.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "USBHelpers.h"
#include "TConfig.h"



#include<list>

#include <sstream>
#include <fstream>
#include <ios>
#include <iomanip>
#include<time.h>
#include<bitset>

using namespace std;

#include"drbug.h"
using namespace DrBug;
#include "adcmon.h"

/// Class CAdcCtrlReg for ADC Control Register (address 0x610)
class CAdcCtrlReg {
public:
	CAdcCtrlReg() : Mode(0), InputSel(0), Icomp(0), DiscriSign(0), RampSpd(0), HalfBitTrim(0), NotUsed(0), CompOut(0) {};
	// first byte(lower)
	unsigned short Mode			: 2; // 0:Manual - 1:Calib - 2:Auto - 3:SuperManual 
	unsigned short InputSel		: 4; // Select Input (0: AVSS - 1: DVSS - 2: AVDD - 3: DVDD - 4: Vbandgap through voltage scaling - 5: DACMONV - 6: DACMONI - 7: Bandgap (direct measurement) - 8: Temperature (direct measurement)
	unsigned short Icomp		: 2; // Set Comparator Current
	// second byte(higher)
	unsigned short DiscriSign	: 1; // Discriminator Sign
	unsigned short RampSpd		: 2; // Ramp Speed (0:500ns/spl - 1:1us/spl - 2:2us/spl - 3:4us/spl
	unsigned short HalfBitTrim	: 1; // Half LSB trimmer
	unsigned short NotUsed		: 3; // Not Used
	unsigned short CompOut		: 1; // Comparator Output for super manual mode
	// i.e. CompOut is the MSB !
};

/// Class CAMORReg for Analog Monitoring and Override Register (address 0x600)
class CAMORReg {
public:
	CAMORReg() : VDacSelect(0), IDacSelect(0), SWCNTLI(0), SWCNTLV(0), IRefBuf(0), NotUsed(0) {};
	// first byte(lower)
	unsigned short VDacSelect	: 4; // Voltage DAC selection 0:VCASN - 1:VCASP - 2:VPULSEH - 3:VPULSEL - 4:VRESETP - 5:VRESETD - 6:VCASN2 - 7:VCLIP - 8:VTEMP
	unsigned short IDacSelect	: 3; // Current DAC slectetion, 0:IRESET - 1:IAUX2 - 2:IBIAS - 3:IDB - 4:IREF (don't select it) - 5:ITHR -6:IREFBuffer
	unsigned short SWCNTLI		: 1; // Override Current DAC by external value when ==1

	// second byte(higher)
	unsigned short SWCNTLV		: 1; // Override Voltage DAC by external value when ==1
	unsigned short IRefBuf		: 2; // Select IRef Buffer Current
	unsigned short NotUsed		: 5; // Not Used
};

/// Class CAdcMonAcq
CAdcMonAcq::CAdcMonAcq(TAlpide * pAlpide, const int nIcomp, const int nRampSpd){ ///< default constructor
	PAlpideHdl=pAlpide;
	char lstrTime[20];
	time_t now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));
	
	ostringstream lstrStatus;
	lstrStatus<<string(lstrTime)<<" : Object Creation"<<endl;
	
	StrStatus=lstrStatus.str(); ///< Date when the object was created
	
	
	
	FTemp_beg=-273.15f; // cold isn't it?
	FTemp_end=FTemp_beg; 
	NIcomp=nIcomp;
	NRampSpd=nRampSpd;
	NDiscriSign=0;
	NHalfBitTrim=0;
	NCalValue=-1; ///< Value of Calibration procedure
	NCalInputSel=-1; ///< Input used for the Calibration procedure
	StrCalInputSel="NOTHING"; ///< Input used for the Calibration procedure
	NCalAdcInputSel=-1; ///< ADC Input used for the Calibration procedure
	NCalIFEInputSel=-1; ///< Analog Frontend Current Input used for the Calibration procedure
	NCalVFEInputSel=-1; ///< Analog Frontend Voltage Input used for the Calibration procedure
}

CAdcMonAcq::~CAdcMonAcq(){ ///< destructor
}


Status CAdcMonAcq::SetAdacValue(const int nInputSelect, const int nValue){
	Status lCurStatus=STOK;
	Alpide::TRegister lAdacReg;
	switch(nInputSelect) {
		case 0: // AVSS
			Error("no dac scan possible for selected input");lCurStatus=STERR;	break;
		case 1: // DVSS
			Error("no dac scan possible for selected input");lCurStatus=STERR;	break;
		case 2: // AVDD
			Error("no dac scan possible for selected input");lCurStatus=STERR;	break;
		case 3: // DVDD
			Error("no dac scan possible for selected input");lCurStatus=STERR;	break;
		case 4: // Vbandgap through voltage scaling (the measure is known to be incorrect for this input)
			Error("no dac scan possible for selected input");lCurStatus=STERR; 	break;
		case 5: // VCASN
			lAdacReg=Alpide::REG_VCASN;											break;
		case 6: // VCASP
			lAdacReg=Alpide::REG_VCASP;											break;
		case 7: // VPULSEH
			lAdacReg=Alpide::REG_VPULSEH;										break;
		case 8: // VPULSEL
			lAdacReg=Alpide::REG_VPULSEL;										break;
		case 9: // VRESETP
			lAdacReg=Alpide::REG_VRESETP;										break;
		case 10: // VRESETD
			lAdacReg=Alpide::REG_VRESETD;										break;
		case 11: // VCASN2
			lAdacReg=Alpide::REG_VCASN2;										break;
		case 12: // VCLIP
			lAdacReg=Alpide::REG_VCLIP;											break;
		case 13: // VTEMP
			lAdacReg=Alpide::REG_VTEMP;											break;
		case 14: // IRESET
			lAdacReg=Alpide::REG_IRESET;										break;
		case 15: // IAUX2
			lAdacReg=Alpide::REG_IAUX2;											break;
		case 16: // IBIAS
			lAdacReg=Alpide::REG_IBIAS;											break;
		case 17: // IDB
			lAdacReg=Alpide::REG_IDB;											break;
		case 18: // IREF the value is not correct for this input
			Error("no dac scan possible for selected input");lCurStatus=STERR; 	break;
		case 19: // ITHR
			lAdacReg=Alpide::REG_ITHR;											break;
		case 20: // BANDGAP
			Error("no dac scan possible for selected input");lCurStatus=STERR; 	break;
		case 21: // TEMPERATURE
			Error("no dac scan possible for selected input");lCurStatus=STERR; 	break;
		default: // error
			Error("Unkown Input Selection, nothing selected, nothing stored");
			lCurStatus=STERR;
	}
	
	if(lCurStatus==STOK) {
		if(nValue==-1) {
			PAlpideHdl->ReadRegister(lAdacReg, AdacValue[nInputSelect]); // store value
		} else
		if(nValue==-2) {
			PAlpideHdl->WriteRegister(lAdacReg, AdacValue[nInputSelect]); // restore value
		} else {
			PAlpideHdl->WriteRegister(lAdacReg, nValue); // set value
			StoreAdacScanValue(nValue);
		}
	}
	return lCurStatus;
}	

Status CAdcMonAcq::SetInput(const int nInputSelect, const int nMode){
if(nMode>3) {
		Error("Unkown Mode requested");
		return(STERR);
	}

	Status lCurStatus=STOK;
	int lAdcInputSel, lIFEInputSel, lVFEInputSel;
	string lstrInput;
	switch(nInputSelect) {
		case 0: // AVSS
			lAdcInputSel=0; lIFEInputSel=0; lVFEInputSel=0; lstrInput="AVSS";		break;
		case 1: // DVSS
			lAdcInputSel=1; lIFEInputSel=0; lVFEInputSel=0; lstrInput="DVSS";		break;
		case 2: // AVDD
			lAdcInputSel=2; lIFEInputSel=0; lVFEInputSel=0; lstrInput="AVDD";		break;
		case 3: // DVDD
			lAdcInputSel=3; lIFEInputSel=0; lVFEInputSel=0; lstrInput="DVDD";		break;
		case 4: // Vbandgap through voltage scaling (the measure is known to be incorrect for this input)
Debug("Not Implemented Yet");			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=0; lstrInput="VBG";		break; //		lAdcInputSel=4; lIFEInputSel=0; lVFEInputSel=0; lstrInput="VBG";	break;
		case 5: // VCASN
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=0; lstrInput="VCASN";		break;
		case 6: // VCASP
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=1; lstrInput="VCASP";		break;
		case 7: // VPULSEH
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=2; lstrInput="VPULSEH";	break;
		case 8: // VPULSEL
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=3; lstrInput="VPULSEL";	break;
		case 9: // VRESETP
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=4; lstrInput="VRESETP";	break;
		case 10: // VRESETD
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=5; lstrInput="VRESETD";	break;
		case 11: // VCASN2
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=6; lstrInput="VCASN2";		break;
		case 12: // VCLIP
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=7; lstrInput="VCLIP";		break;
		case 13: // VTEMP
			lAdcInputSel=5; lIFEInputSel=0; lVFEInputSel=8; lstrInput="VTEMP";		break;
		case 14: // IRESET
			lAdcInputSel=6; lIFEInputSel=0; lVFEInputSel=0; lstrInput="IRESET";		break;
		case 15: // IAUX2
			lAdcInputSel=6; lIFEInputSel=1; lVFEInputSel=0; lstrInput="IAUX2";		break;
		case 16: // IBIAS
			lAdcInputSel=6; lIFEInputSel=2; lVFEInputSel=0; lstrInput="IBIAS";		break;
		case 17: // IDB
			lAdcInputSel=6; lIFEInputSel=3; lVFEInputSel=0; lstrInput="IDB";		break;
		case 18: // IREF the value is not correct for this input
Debug("Not Implemented Yet"); lIFEInputSel=0; lVFEInputSel=0; lstrInput="IREF";       break;//			lAdcInputSel=6; lIFEInputSel=4; lVFEInputSel=0; lstrInput="IREF";		break;
		case 19: // ITHR
			lAdcInputSel=6; lIFEInputSel=5; lVFEInputSel=0; lstrInput="ITHR";		break;
		case 20: // BANDGAP
			lAdcInputSel=7; lIFEInputSel=6; lVFEInputSel=0; lstrInput="BG";			break;
		case 21: // TEMPERATURE
			lAdcInputSel=8; lIFEInputSel=0; lVFEInputSel=0; lstrInput="TEMP";		break;
		default: // error
			Error("Unkown Input Selection, nothing selected, nothing stored");
			lCurStatus=STERR;
	}
	if(lCurStatus==STOK){
		if(nMode==2) { // do not store input number in automatic mode
		} else 
		if(nMode==1) {
			NCalInputSel=nInputSelect;
			StrCalInputSel=lstrInput;
			NCalAdcInputSel=lAdcInputSel;
			NCalIFEInputSel=lIFEInputSel;
			NCalVFEInputSel=lVFEInputSel;
		} else {
			LAdcMonInputSel.push_back(nInputSelect);
			LAdcMonStrInputSel.push_back(lstrInput);
			LAdcMonAdcInputSel.push_back(lAdcInputSel);
			LAdcMonIFEInputSel.push_back(lIFEInputSel);
			LAdcMonVFEInputSel.push_back(lVFEInputSel);
		}
	} else {
		return(lCurStatus);
	}




	CAdcCtrlReg lAdcCtrlReg; // Setup the ADC Control Register
	lAdcCtrlReg.Mode=nMode;
	lAdcCtrlReg.InputSel=lAdcInputSel; // Set ADC input according to Table 3.31
	lAdcCtrlReg.Icomp=NIcomp;
	lAdcCtrlReg.DiscriSign=NDiscriSign;
	lAdcCtrlReg.RampSpd=NRampSpd;
	lAdcCtrlReg.HalfBitTrim=NHalfBitTrim;

	ostringstream lstrdbg; // for debug info
	char lstrTime[20];
	time_t now = time(0);
	ostringstream lstrStatus;
	if((lAdcCtrlReg.InputSel==5)||(lAdcCtrlReg.InputSel==6)) {
		CAMORReg lAMORReg;
//		Debug("todo: read value of the AMORReg address 0x600");
		PAlpideHdl->ReadRegister(Alpide::REG_ANALOGMON, *(reinterpret_cast<uint16_t *>(&lAMORReg)));
		lAMORReg.VDacSelect=lVFEInputSel;
		lAMORReg.IDacSelect=lIFEInputSel;
		lAMORReg.SWCNTLI=0; // don't Override Current DAC by external
		lAMORReg.SWCNTLV=0; // don't Override Voltage DAC by external
//		lstrdbg<<"todo: write back the new value (0x"<< hex << setfill('0') << uppercase<< setw(4) << *(reinterpret_cast<uint16_t *>(&lAMORReg))<<") of the AMORReg address 0x600";
//		Debug(lstrdbg.str());
		PAlpideHdl->WriteRegister(Alpide::REG_ANALOGMON, *(reinterpret_cast<uint16_t *>(&lAMORReg)));

		now = time(0);
		strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));

		lstrStatus<<string(lstrTime)<<" : Set Analog Monitoring and Override Register "<<hex << setfill('0') << uppercase<< setw(4) << *(reinterpret_cast<uint16_t *>(&lAMORReg))<<endl;
		StrStatus+=lstrStatus.str();
	
	}
	

//		lstrdbg.str("");
//		lstrdbg<<"todo: write the new value (0x"<< hex << setfill('0') << uppercase<< setw(4) << *(reinterpret_cast<uint16_t *>(&lAdcCtrlReg))<<") of the AdcCtrlReg address 0x"<<Alpide::REG_ADC_CONTROL;
//		Debug(lstrdbg.str());
		PAlpideHdl->WriteRegister(Alpide::REG_ADC_CONTROL, *(reinterpret_cast<uint16_t *>(&lAdcCtrlReg)));

		now = time(0);
		strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));
	
		lstrStatus.str("");
		lstrStatus<<string(lstrTime)<<" : Set Adc Control Register "<<hex << setfill('0') << uppercase<< setw(4) << *(reinterpret_cast<uint16_t *>(&lAdcCtrlReg))<<endl;
		StrStatus+=lstrStatus.str();
	
	


	return lCurStatus;
}


void CAdcMonAcq::StoreValue(const int nValue, const int nMode){
	if(nMode==1) {
		NCalValue=nValue;
	} else {
		LAdcMonValues.push_back(nValue);
	}
	return;
}

void CAdcMonAcq::StoreAdacScanValue(const int nValue){
	LAdacScanValues.push_back(nValue);
	return;
}

void CAdcMonAcq::StorePcbValue(const float fValue){
	LAdcPcbValues.push_back(fValue);
	return;
}

void CAdcMonAcq::StoreEvalValue(const int nValue){
	LAdcEvalValues.push_back(nValue);
	return;
}









/// class CAdcMon
CAdcMon::CAdcMon(TReadoutBoardDAQ *ts, TAlpide *pAlpide, const int nIcomp, const int nRampSpd){ ///< Constructor
	PSetupHdl=ts;
	PAlpideHdl=pAlpide;
	AcqValues.PAlpideHdl=pAlpide;
	AcqValues.FTemp_beg=PSetupHdl->ReadTemperature();
	AcqValues.FTemp_end=PSetupHdl->ReadTemperature();
	BIsVerbose=false;
}

CAdcMon::~CAdcMon(){ ///< Destructor
}	

Status CAdcMon::Set(const int nIcomp, const int nRampSpd){
	AcqValues.NIcomp=nIcomp;
	AcqValues.NRampSpd=nRampSpd;
	return(STOK);
}

Status CAdcMon::Preset(const int nInputSelect) {
	int lnInputSelectBeg, lnInputSelectEnd;
	if(nInputSelect==-1) {
		lnInputSelectBeg=5;
		lnInputSelectEnd=19;
	} else {
		lnInputSelectBeg=nInputSelect;
		lnInputSelectEnd=nInputSelect;
	}
	for(int lnInputSelect=lnInputSelectBeg;lnInputSelect<=lnInputSelectEnd;lnInputSelect++) {
		if(lnInputSelect!=18)
			AcqValues.SetAdacValue(lnInputSelect,64);
	}
	AcqValues.LAdacScanValues.clear();
	return(STOK);
}

Status CAdcMon::Calibrate(const int nInputSelect){
	Status lCurStatus;

	// first step
	AcqValues.NDiscriSign=0;
	AcqValues.NHalfBitTrim=0;
	lCurStatus=UnitCalibrateMeas(nInputSelect);
	if(lCurStatus!=STOK){
		Error("Calibration Procedure:: Failure at First Step (DISCRI_SIGN=0, HalfLSBTrim=0), stopping here.");
		return(lCurStatus);
	}
	int lnVal1=AcqValues.NCalValue;

	// second step
	AcqValues.NDiscriSign=1;
	AcqValues.NHalfBitTrim=0;
	lCurStatus=UnitCalibrateMeas(nInputSelect);
	if(lCurStatus!=STOK){
		Error("Calibration Procedure:: Failure at Second Step (DISCRI_SIGN=1, HalfLSBTrim=0), stopping here.");
		return(lCurStatus);
	}
	int lnVal2=AcqValues.NCalValue;
	if(lnVal1>lnVal2) {
		Debug("From Calibration Procedure, it appears the optimal value for DiscriSign is 0");
		AcqValues.NDiscriSign=0;
	} else {
		Debug("From Calibration Procedure, it appears the optimal value for DiscriSign is 1");
		AcqValues.NDiscriSign=1;
		lnVal1=lnVal2; // so Now in lnVal1 we have the case NDiscriSign=1 NHalfBitTrim=0
	}

	// third step
	AcqValues.NHalfBitTrim=1;
	lCurStatus=UnitCalibrateMeas(nInputSelect);
	if(lCurStatus!=STOK){
		Error("Calibration Procedure:: Failure at Third Step (HalfLSBTrim=1), stopping here.");
		return(lCurStatus);
	}
	lnVal2=AcqValues.NCalValue;
	if(lnVal1>lnVal2) {
		Debug("From Calibration Procedure, it appears the optimal value for HalfBitTrim is 0");
		AcqValues.NHalfBitTrim=0;
		AcqValues.NCalValue=lnVal1;
	} else {
		Debug("From Calibration Procedure, it appears the optimal value for HalfBitTrim is 1");
		AcqValues.NHalfBitTrim=1;
		AcqValues.NCalValue=lnVal2;
	}
	
	AcqValues.FTemp_end=PSetupHdl->ReadTemperature();
	return(STOK);
}

Status CAdcMon::ManualMeasure(const int nInputSelect, const int nCalInputSelect){
	Status lCurStatus;
	lCurStatus=Calibrate(nCalInputSelect);
	if(lCurStatus!=STOK){
		Error("Manual Measure Procedure:: Failure at Calibration step, stopping here.");
		return(lCurStatus);
	}
	int lnInputSelectBeg, lnInputSelectEnd;
	if(nInputSelect==-1) {
		lnInputSelectBeg=0;
		lnInputSelectEnd=21;
	} else {
		lnInputSelectBeg=nInputSelect;
		lnInputSelectEnd=nInputSelect;
	}
	for(int lnInputSelect=lnInputSelectBeg;lnInputSelect<=lnInputSelectEnd;lnInputSelect++) {
		lCurStatus=UnitManualMeas(lnInputSelect);
		AcqValues.StoreEvalValue(-1);
		if(lCurStatus!=STOK){
			Error("Manual Measure Procedure:: Failure when doing measure itself.");
			return(lCurStatus);
		}
		usleep(100000);
	}
	AcqValues.FTemp_end=PSetupHdl->ReadTemperature();
	return(STOK);
}

Status CAdcMon::ScanManual(const int nInputSelect, const int nCalInputSelect, const int nSplDist){
	Status lCurStatus;
	lCurStatus=Calibrate(nCalInputSelect);
	if(lCurStatus!=STOK){
		Error("Manual Measure Procedure:: Failure at Calibration step, stopping here.");
		return(lCurStatus);
	}
	int lnInputSelect=nInputSelect;

	if(AcqValues.SetAdacValue(nInputSelect,-1)!=STOK) { // store value to be able to restore it after work
		return(STERR);
	}

	for(int lnAdacValue=0;lnAdacValue<256;lnAdacValue+=nSplDist) {
		if(BReverse) {
			AcqValues.SetAdacValue(lnInputSelect,(255-lnAdacValue));
		} else {
			AcqValues.SetAdacValue(lnInputSelect,lnAdacValue);
		}
		lCurStatus=UnitManualMeas(lnInputSelect);
		if(lCurStatus!=STOK){
			Error("Manual Measure Procedure:: Failure when doing measure itself.");
			return(lCurStatus);
		}
		usleep(100000);
	}
	AcqValues.SetAdacValue(nInputSelect,-2); // restore value
	
	AcqValues.FTemp_end=PSetupHdl->ReadTemperature();
	return(STOK);
}

Status CAdcMon::AutomaticMeasure(const int nCalInputSelect){
	Status lCurStatus;
	lCurStatus=Calibrate(nCalInputSelect);
	if(lCurStatus!=STOK){
		Error("Automatic Measure Procedure:: Failure at Calibration step, stopping here.");
		return(lCurStatus);
	}
	
	char lstrTime[20];
	time_t now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));
	
	ostringstream lstrStatus;
	lstrStatus<<string(lstrTime)<<" : Automatic Measure"<<endl;
	AcqValues.StrStatus+=lstrStatus.str();
	
	
	lCurStatus=AcqValues.SetInput(0, 2); // Mode=2
	if(lCurStatus!=STOK){
		Error("could not set ADC and analog frontend switches according to input selection.");
		return(lCurStatus);
	}
	
//	Debug("todo: Send a START_MEASURE pulse 0xFF20");
	PAlpideHdl->WriteRegister(Alpide::REG_COMMAND,Alpide::OPCODE_ADCMEASURE);

	usleep(100000); // wait for 100ms
	
	int lnAddr=Alpide::REG_ADC_AVSS;
	for(int nInputSelect=0; nInputSelect<22; nInputSelect++){
		if(nInputSelect!=5) { // input for 5 is not sampled by the automatic sequence
			uint16_t lnValue=65535;
			float lfPcbValue=-1;
			ostringstream lstrdbg; // for debug info
//			lstrdbg<<"todo: Read the value register address 0x"<<  hex << setfill('0') << uppercase<< setw(4) <<lnAddr;
//			Debug(lstrdbg.str());
			PAlpideHdl->ReadRegister(lnAddr, lnValue);
			lCurStatus=AcqValues.SetInput(nInputSelect,0); // we lie and say it is a manual measure as we are scanning inputs for an external ADC measurement...
			if(lCurStatus!=STOK){
				Error("could not set ADC and analog frontend switches according to input selection.");
				return(lCurStatus);
			}
			AcqValues.StoreValue(lnValue,2); // Mode=2;
	
			usleep(10000); // wait for 10ms

			if(AcqValues.LAdcMonAdcInputSel.back()==5) {
				lfPcbValue=PSetupHdl->ReadMonV();
		   	} else
			if(AcqValues.LAdcMonAdcInputSel.back()==6) {
				lfPcbValue=PSetupHdl->ReadMonI();
			} 
			
			AcqValues.StorePcbValue(lfPcbValue);
			AcqValues.StoreEvalValue(-1);
			
			lnAddr++;
		}
	}
	AcqValues.FTemp_end=PSetupHdl->ReadTemperature();
	return(STOK);
}
	
Status CAdcMon::ScanSuperManual(const int nInputSelect, const int nSplDist){
	Status lCurStatus;
	if(AcqValues.SetAdacValue(nInputSelect,-1)!=STOK) { // store value to be able to restore it after work
		return(STERR);
	}
	int lnInputSelect=nInputSelect;
	for(int lnAdacValue=0;lnAdacValue<256;lnAdacValue+=nSplDist) {
		if(BReverse) {
			AcqValues.SetAdacValue(lnInputSelect,(255-lnAdacValue));
		} else {
			AcqValues.SetAdacValue(lnInputSelect,lnAdacValue);
		}
		lCurStatus=SuperManualMeasure(lnInputSelect);
		if(lCurStatus!=STOK){
			Error("Super Manual Measure Procedure:: Failure when doing measure itself.");
			return(lCurStatus);
		}
		usleep(100000);
	}
	AcqValues.SetAdacValue(nInputSelect,-2); // restore value
	return STOK;
}

Status CAdcMon::UnitCalibrateMeas(const int nInputSelect){
	char lstrTime[20];
	time_t now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));
	
	ostringstream lstrStatus;
	lstrStatus<<string(lstrTime)<<" : Calibration step for input "<<dec<<nInputSelect<<endl;
	AcqValues.StrStatus+=lstrStatus.str();
	
	
	Status lCurStatus;
	lCurStatus=AcqValues.SetInput(nInputSelect,1); // Mode=1
	if(lCurStatus!=STOK){
		Error("could not set ADC and analog frontend switches according to input selection for calibration.");
		return(lCurStatus);
	}
	usleep(5000); // wait for 5ms
	
//	Debug("todo: Send a START_MEASURE pulse 0xFF20");
	PAlpideHdl->WriteRegister(Alpide::REG_COMMAND,Alpide::OPCODE_ADCMEASURE);

	usleep(20000); // wait for 20ms

	uint16_t lnValue=65535;
//	Debug("todo: Read the CAL register 0x612");
	 PAlpideHdl->ReadRegister(Alpide::REG_ADC_CALIB, lnValue);

			
	AcqValues.StoreValue(lnValue,1); // Mode=1;

	return(STOK);
}

Status CAdcMon::UnitManualMeas(const int nInputSelect){
	char lstrTime[20];
	time_t now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));
	
	ostringstream lstrStatus;
	lstrStatus<<string(lstrTime)<<" : Manual Measure for input "<<dec<<nInputSelect<<endl;
	AcqValues.StrStatus+=lstrStatus.str();
	
	
	Status lCurStatus;
	lCurStatus=AcqValues.SetInput(nInputSelect, 0); // Mode=0
	if(lCurStatus!=STOK){
		Error("could not set ADC and analog frontend switches according to input selection.");
		return(lCurStatus);
	}
	
	usleep(5000); // wait for 5ms
//	Debug("todo: Send a START_MEASURE pulse 0xFF20");
	PAlpideHdl->WriteRegister(Alpide::REG_COMMAND,Alpide::OPCODE_ADCMEASURE);

	usleep(20000); // wait for 20ms
	
//	Debug("todo: Read the AVSS register 0x613");
	uint16_t lnValue=65535;
	PAlpideHdl->ReadRegister(Alpide::REG_ADC_AVSS, lnValue);

	AcqValues.StoreValue(lnValue,0); // Mode=0;


	float lfPcbValue=-1;
	if(AcqValues.LAdcMonAdcInputSel.back()==5) {
//		lCurStatus=AcqValues.SetInput(0, 2); // do not store value
		usleep(5000); // wait for 5ms
		lfPcbValue=PSetupHdl->ReadMonV();
	} else
	if(AcqValues.LAdcMonAdcInputSel.back()==6) {
//		lCurStatus=AcqValues.SetInput(0, 2); // do not store value
		usleep(5000); // wait for 5ms
		lfPcbValue=PSetupHdl->ReadMonI();
	} 

	AcqValues.StorePcbValue(lfPcbValue);

	return(STOK);
}

class CDacCmd {
public:
	unsigned short NCol : 8; // LSByte
	unsigned short NRow : 8; // MSByte
};

int CAdcMon::DoRamp(){
	CDacCmd lDacCmd;
	int lnDacValue=0;
	
	
	lDacCmd.NRow=0;
	lDacCmd.NCol=1;

//	Debug("todo: Write the value 0 into register address 0x611");
	PAlpideHdl->WriteRegister(Alpide::REG_ADC_DAC_INPUT, *(reinterpret_cast<uint16_t *>(&lDacCmd)));

	usleep(1000000); // wait for 1s

	bool lbUpNdn=true; // up when == true
	for(int lnRow=0; lnRow<8;lnRow++){
		lDacCmd.NRow=lnRow;
		while(1) {
			ostringstream lstrdbg; // for debug info
//			bitset<12> y(*(reinterpret_cast<unsigned short *>(&lDacCmd)));
//			lstrdbg<<"todo: Write the value "<< y <<" into register address 0x"<<hex << setfill('0') << uppercase<< setw(4) <<Alpide::REG_ADC_DAC_INPUT;
//			Debug(lstrdbg.str());
			PAlpideHdl->WriteRegister(Alpide::REG_ADC_DAC_INPUT, *(reinterpret_cast<uint16_t *>(&lDacCmd)));

			usleep(1000); // wait for 1ms

			CAdcCtrlReg lAdcCtrlReg;
//			Debug("todo: Read the ADCCtrlReg register 0x610");
			PAlpideHdl->ReadRegister(Alpide::REG_ADC_CONTROL, *(reinterpret_cast<uint16_t *>(&lAdcCtrlReg)));
	
			if(lAdcCtrlReg.CompOut==lAdcCtrlReg.DiscriSign) 
				return(lnDacValue);

			lnDacValue++;

			if(lnDacValue==1056)
				return(-1);

			if(lDacCmd.NCol==0) {
				lbUpNdn=true;
			} else if (lDacCmd.NCol==132) {
				lbUpNdn=false;
			} // else keep value

			if(lbUpNdn==true) {
				lDacCmd.NCol+=1;
			} else {
				lDacCmd.NCol-=1;
			}

			if( (lbUpNdn==true) && (lDacCmd.NCol==132) )
				break;

			if( (lbUpNdn==false) && (lDacCmd.NCol==0) )
				break;

		}
	}	
	return(lnDacValue);
}

Status CAdcMon::SuperManualMeasure(const int nInputSelect){
	char lstrTime[20];
	int lnInputSelectBeg, lnInputSelectEnd;
	if(nInputSelect==-1) {
		lnInputSelectBeg=0;
		lnInputSelectEnd=21;
	} else {
		lnInputSelectBeg=nInputSelect;
		lnInputSelectEnd=nInputSelect;
	}
	for(int lnInputSelect=lnInputSelectBeg;lnInputSelect<=lnInputSelectEnd;lnInputSelect++) {
		time_t now = time(0);
		strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));
	
		ostringstream lstrStatus;
		lstrStatus<<string(lstrTime)<<" : Super Manual Measure for input "<<dec<<lnInputSelect<<endl;
		AcqValues.StrStatus+=lstrStatus.str();

		Status lCurStatus;
		lCurStatus=AcqValues.SetInput(lnInputSelect, 3); // Mode=3
		if(lCurStatus!=STOK){
			Error("could not set ADC and analog frontend switches according to input selection.");
			return(lCurStatus);
		}
		usleep(5000); // wait for 5ms

		int lnDacValue=0;
		lnDacValue=DoRamp();
		ostringstream lstrdbg; // for debug info
		lstrdbg<<"toggle when lnDacValue = "<< dec << lnDacValue;
		Debug(lstrdbg.str());
	
		AcqValues.StoreValue(lnDacValue,3); // Mode=3;
		
		AcqValues.FTemp_end=PSetupHdl->ReadTemperature();


		float lfPcbValue=-1;
		if(AcqValues.LAdcMonAdcInputSel.back()==5) {
			lCurStatus=AcqValues.SetInput(0, 2); // do not store value
			usleep(5000); // wait for 5ms
			lfPcbValue=PSetupHdl->ReadMonV();
   		} else
		if(AcqValues.LAdcMonAdcInputSel.back()==6) {
			lCurStatus=AcqValues.SetInput(0, 2); // do not store value
			usleep(5000); // wait for 5ms
			lfPcbValue=PSetupHdl->ReadMonI();
		} 

		AcqValues.StorePcbValue(lfPcbValue);
		AcqValues.StoreEvalValue(-1);
		usleep(100000);
	}
	return(STOK);
}

	void CAdcMon::AdcEval(){

	AcqValues.SetInput(1, 3); // Mode=3

	CAMORReg lAMORReg;

	// 1- send ADCDAC value out of the ALPIDE

//	Debug("todo: read value of the AMORReg address 0x600");
	PAlpideHdl->ReadRegister(Alpide::REG_ANALOGMON, *(reinterpret_cast<uint16_t *>(&lAMORReg)));
	lAMORReg.VDacSelect=9;
	lAMORReg.IDacSelect=0;
	lAMORReg.SWCNTLI=0; // don't Override Current DAC by external
	lAMORReg.SWCNTLV=0; // don't Override Voltage DAC by external
//	lstrdbg<<"todo: write back the new value (0x"<< hex << setfill('0') << uppercase<< setw(4) << *(reinterpret_cast<uint16_t *>(&lAMORReg))<<") of the AMORReg address 0x600";
//	Debug(lstrdbg.str());
	PAlpideHdl->WriteRegister(Alpide::REG_ANALOGMON, *(reinterpret_cast<uint16_t *>(&lAMORReg)));

	char lstrTime[20];
	time_t now = time(0);
	ostringstream lstrStatus;
	now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));
	lstrStatus<<string(lstrTime)<<" : Set Analog Monitoring and Override Register "<<hex << setfill('0') << uppercase<< setw(4) << *(reinterpret_cast<uint16_t *>(&lAMORReg))<<endl;
	AcqValues.StrStatus+=lstrStatus.str();
	cout<<lstrStatus.str();

	 // 2- do ramp and store extarnal ADC value
	
	CDacCmd lDacCmd;
	int lnDacValue=0;
	
	
	lDacCmd.NRow=0;
	lDacCmd.NCol=1;

//	Debug("todo: Write the value 0 into register address 0x611");
	PAlpideHdl->WriteRegister(Alpide::REG_ADC_DAC_INPUT, *(reinterpret_cast<uint16_t *>(&lDacCmd)));

	usleep(1000000); // wait for 1s

//	std::list<int> llDacValue		;
//	std::list<float> llPcbValue		;
	bool lbUpNdn=true; // up when == true
	for(int lnRow=0; lnRow<8;lnRow++){
		lDacCmd.NRow=lnRow;
		while(1) {
			ostringstream lstrdbg; // for debug info
//			bitset<12> y(*(reinterpret_cast<unsigned short *>(&lDacCmd)));
//			lstrdbg<<"todo: Write the value "<< y <<" into register address 0x"<<hex << setfill('0') << uppercase<< setw(4) <<Alpide::REG_ADC_DAC_INPUT;
//			Debug(lstrdbg.str());
			PAlpideHdl->WriteRegister(Alpide::REG_ADC_DAC_INPUT, *(reinterpret_cast<uint16_t *>(&lDacCmd)));

			usleep(1000); // wait for 1ms

			float lfPcbValue=PSetupHdl->ReadMonV();

//			cout<<dec<<setfill('0')<<setw(4)<<lnDacValue<<" (0x"<<hex << setfill('0') << uppercase<<setw(4)<<*(reinterpret_cast<uint16_t *>(&lDacCmd))<<dec<<") -> "<<lfPcbValue<<endl;
			AcqValues.StoreEvalValue(lnDacValue);
			AcqValues.StoreValue(lnDacValue,2);
			AcqValues.StorePcbValue(lfPcbValue);

			list<string>::reverse_iterator end(AcqValues.LAdcMonStrInputSel.rbegin());
			AcqValues.LAdcMonStrInputSel.push_back(*end);

			AcqValues.LAdcMonAdcInputSel.push_back(*(AcqValues.LAdcMonAdcInputSel.end()));
			AcqValues.LAdcMonIFEInputSel.push_back(*(AcqValues.LAdcMonIFEInputSel.end()));
			AcqValues.LAdcMonVFEInputSel.push_back(*(AcqValues.LAdcMonVFEInputSel.end()));

			lnDacValue++;

			if(lnDacValue==1056)
				break;

			if(lDacCmd.NCol==0) {
				lbUpNdn=true;
			} else if (lDacCmd.NCol==132) {
				lbUpNdn=false;
			} // else keep value

			if(lbUpNdn==true) {
				lDacCmd.NCol+=1;
			} else {
				lDacCmd.NCol-=1;
			}

			if( (lbUpNdn==true) && (lDacCmd.NCol==132) )
				break;

			if( (lbUpNdn==false) && (lDacCmd.NCol==0) )
				break;

		}
	} 
	// 3- go back to a failsafe state

	lAMORReg.VDacSelect=0;
	PAlpideHdl->WriteRegister(Alpide::REG_ANALOGMON, *(reinterpret_cast<uint16_t *>(&lAMORReg)));

	now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"#[%Y%m%d:%H%M%S]", localtime(&now));
	lstrStatus<<string(lstrTime)<<" : Set Analog Monitoring and Override Register "<<hex << setfill('0') << uppercase<< setw(4) << *(reinterpret_cast<uint16_t *>(&lAMORReg))<<endl;
	AcqValues.StrStatus+=lstrStatus.str();

	return;
}















float CAdcMon::GetRealValue(const int nValue, const int nAdcInputSel) {
	float lfValue=nValue;
	switch(nAdcInputSel) {
		case 5:
//			lfValue=lfValue*2.0f*0.816f/1000.0f;
			lfValue=lfValue*2.0f*1.068f/1000.0f;
			break;
		case 6:
//			lfValue=lfValue*0.816f/5.0f;
			lfValue=lfValue*1.068f/5.0f;
			break;
		case 8:
			lfValue=(lfValue+51.5f)*0.147f; Debug("Modified formula for temperature");
//			lfValue=lfValue*0.147f-51.5f; //original formula
			break;
	}
	return lfValue;
}

Status CAdcMon::VerboseIt(){
	char lstrTime[20];
	time_t now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"#%Y%m%d:%H%M%S", localtime(&now));
	
	Print("################");
	ostringstream lstrParams;
	lstrParams<<"#Parameters"<<endl;
	lstrParams<<"#Time [YYYYMMDD:HHMMSS]\t"<<string(lstrTime)<<endl;
	lstrParams<<"#Temp_begin\t"<<AcqValues.FTemp_beg-273.15f<<endl;
	lstrParams<<"#Temp_last\t"<<AcqValues.FTemp_end-273.15f<<endl;
	lstrParams<<"#IComp\t"<<dec<<AcqValues.NIcomp<<endl;
	switch(AcqValues.NIcomp) {
		case 0:
			lstrParams<<"#IComp\t163 uA"<<endl;break;
		case 1:
			lstrParams<<"#IComp\t190 uA"<<endl;break;
		case 2:
			lstrParams<<"#IComp\t296 uA"<<endl;break;
		case 3:
			lstrParams<<"#IComp\t410 uA"<<endl;break;
	}
	lstrParams<<"#RampSpeed\t"<<dec<<AcqValues.NRampSpd<<endl;
	switch(AcqValues.NRampSpd) {
		case 0:
			lstrParams<<"#RampSpeed\t500 ns/sample";break;
		case 1:
			lstrParams<<"#RampSpeed\t1 us/sample";break;
		case 2:
			lstrParams<<"#RampSpeed\t2 us/sample";break;
		case 3:
			lstrParams<<"#RampSpeed\t4 us/sample";break;
	}
	Print(lstrParams.str());
	Print("################");

	ostringstream lstrCal;
	lstrCal<<"#Calibration"<<endl;
	lstrCal<<"#HalfLsbTrim\t"<<dec<<AcqValues.NHalfBitTrim<<endl;
	lstrCal<<"#DiscriSign\t"<<AcqValues.NDiscriSign<<endl;
	lstrCal				<<"#Input      "<<dec<<"\t"<<AcqValues.NCalInputSel<<endl;
	lstrCal				<<"#Input      "<<dec<<"\t"<<AcqValues.StrCalInputSel<<endl;
	lstrCal				<<"#ADC Input  "<<dec<<"\t"<<AcqValues.NCalAdcInputSel<<endl;
	lstrCal				<<"#IFE Input  "<<dec<<"\t"<<AcqValues.NCalIFEInputSel<<endl;
	lstrCal				<<"#VFE Input  "<<dec<<"\t"<<AcqValues.NCalVFEInputSel<<endl;
	lstrCal				<<"#Value      "<<dec<<"\t"<<AcqValues.NCalValue<<endl;
	lstrCal				<<"#Float Value"<<dec<<"\t"<<GetRealValue(AcqValues.NCalValue, AcqValues.NCalAdcInputSel);
	Print(lstrCal.str());
	Print("################");
	
	if(!(AcqValues.LAdcMonInputSel.empty())) {
		bool lbScanActivated;
		if(!AcqValues.LAdacScanValues.empty()) {
			lbScanActivated=true;
		} else {
			lbScanActivated=false;
		}
		ostringstream lstrInput, lstrStrInput;
		ostringstream lstrAdcInput, lstrVFEInput, lstrIFEInput;
		ostringstream lstrValue, lstrPcbValue;
		ostringstream lstrAdacScan, lstrFloatValue;
		lstrInput		<<"#Measures"<<endl;
		lstrInput		<<"#Input      "<<dec;
		lstrStrInput	<<"#Input      "<<dec;
		lstrAdcInput	<<"#ADC Input  "<<dec;
		lstrIFEInput	<<"#IFE Input  "<<dec;
		lstrVFEInput	<<"#VFE Input  "<<dec;
		lstrAdacScan	<<"#ADAC Value "<<dec;
		lstrValue		<<"#Value      "<<dec;
		lstrFloatValue	<<"#Float Value"<<dec;
		lstrPcbValue	<<"#PcbValue   "<<dec;
		list<int>::iterator litInput	=AcqValues.LAdcMonInputSel.begin();
		list<string>::iterator litStrInput	=AcqValues.LAdcMonStrInputSel.begin();
		list<int>::iterator litAdcInput	=AcqValues.LAdcMonAdcInputSel.begin();
		list<int>::iterator litVFEInput	=AcqValues.LAdcMonVFEInputSel.begin();
		list<int>::iterator litIFEInput	=AcqValues.LAdcMonIFEInputSel.begin();
		list<int>::iterator litAdacScanValue	=AcqValues.LAdacScanValues.begin();
		list<int>::iterator litValue	=AcqValues.LAdcMonValues.begin();
		list<float>::iterator litPcbValue	=AcqValues.LAdcPcbValues.begin();


while(litValue!=AcqValues.LAdcMonValues.end()) {
   			lstrInput		<<"\t"	<< *litInput		;
   			lstrStrInput	<<"\t"	<< *litStrInput		;
	   		lstrAdcInput	<<"\t"	<< *litAdcInput		;
   			lstrVFEInput	<<"\t"	<< *litVFEInput		;
   			lstrIFEInput	<<"\t"	<< *litIFEInput		;
			if(lbScanActivated) {
				lstrAdacScan<<"\t"	<< *litAdacScanValue	;
			}
			lstrValue		<<"\t"	<< *litValue		;
			lstrFloatValue	<<"\t"	<< GetRealValue(*litValue,*litAdcInput)	;
			lstrPcbValue	<<"\t"	<< *litPcbValue		;

			if(!lbScanActivated) { // for scanning we repeat the input number...
				litInput++;
				litStrInput++;
				litAdcInput++;
				litVFEInput++;
				litIFEInput++;
			} else {
				litAdacScanValue++;
			}
			litValue++;
			litPcbValue++;
		}
		Print(lstrInput.str());
		Print(lstrStrInput.str());
		Print(lstrAdcInput.str());
		Print(lstrIFEInput.str());
		Print(lstrVFEInput.str());
		if(lbScanActivated) {
			Print(lstrAdacScan.str());
		}
		Print(lstrValue.str());
		Print(lstrFloatValue.str());
		Print(lstrPcbValue.str());
	} else {
		Print("No measure stored");
	}
	
//	Print("################");
//	Print("Comments");
//	Print(AcqValues.StrStatus);
	return(STOK);
} 

void CAdcMon::Print(const string str){
	if(BIsVerbose) {
		cout<<str<<endl;
	}
	return;
}


Status CAdcMon::SaveToTxtFile(const string strFileName){
	std::ofstream lofsFile;
	string lstrFileName=strFileName+".txt";
	lofsFile.open (lstrFileName, std::ofstream::out | std::ofstream::app);
	
	char lstrTime[20];
	time_t now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"#%Y%m%d:%H%M%S", localtime(&now));
	lofsFile<<"################"<<endl;
	ostringstream lstrParams;
	lstrParams<<"#Parameters"<<endl;
	lstrParams<<"#Time [YYYYMMDD:HHMMSS]\t"<<string(lstrTime)<<endl;
	lstrParams<<"#Temp_begin\t"<<AcqValues.FTemp_beg-273.15f<<endl;
	lstrParams<<"#Temp_last\t"<<AcqValues.FTemp_end-273.15f<<endl;
	lstrParams<<"#IComp\t"<<dec<<AcqValues.NIcomp<<endl;
	switch(AcqValues.NIcomp) {
		case 0:
			lstrParams<<"#IComp\t163 uA"<<endl;break;
		case 1:
			lstrParams<<"#IComp\t190 uA"<<endl;break;
		case 2:
			lstrParams<<"#IComp\t296 uA"<<endl;break;
		case 3:
			lstrParams<<"#IComp\t410 uA"<<endl;break;
	}
	lstrParams<<"#RampSpeed\t"<<dec<<AcqValues.NRampSpd<<endl;
	switch(AcqValues.NRampSpd) {
		case 0:
			lstrParams<<"#RampSpeed\t500 ns/sample";break;
		case 1:
			lstrParams<<"#RampSpeed\t1 us/sample";break;
		case 2:
			lstrParams<<"#RampSpeed\t2 us/sample";break;
		case 3:
			lstrParams<<"#RampSpeed\t4 us/sample";break;
	}
	lofsFile<<lstrParams.str()<<endl;
	lofsFile<<"################"<<endl;
	ostringstream lstrCal;
	lstrCal<<"#Calibration"<<endl;
	lstrCal<<"#HalfLsbTrim\t"<<dec<<AcqValues.NHalfBitTrim<<endl;
	lstrCal<<"#DiscriSign\t"<<AcqValues.NDiscriSign<<endl;
	lstrCal				<<"#Input      "<<dec<<"\t"<<AcqValues.NCalInputSel<<endl;
	lstrCal				<<"#Input      "<<dec<<"\t"<<AcqValues.StrCalInputSel<<endl;
	lstrCal				<<"#ADC Input  "<<dec<<"\t"<<AcqValues.NCalAdcInputSel<<endl;
	lstrCal				<<"#IFE Input  "<<dec<<"\t"<<AcqValues.NCalIFEInputSel<<endl;
	lstrCal				<<"#VFE Input  "<<dec<<"\t"<<AcqValues.NCalVFEInputSel<<endl;
	lstrCal				<<"#Value      "<<dec<<"\t"<<AcqValues.NCalValue<<endl;
	lstrCal				<<"#Float Value"<<dec<<"\t"<<GetRealValue(AcqValues.NCalValue, AcqValues.NCalAdcInputSel);
	lofsFile<<lstrCal.str()<<endl;
	lofsFile<<"################"<<endl;
	
	
	if(!(AcqValues.LAdcMonInputSel.empty())) {
		bool lbScanActivated;
		if(!AcqValues.LAdacScanValues.empty()) {
			lbScanActivated=true;
		} else {
			lbScanActivated=false;
		}
		
		ostringstream lstrInput, lstrStrInput;
		ostringstream lstrAdcInput, lstrVFEInput, lstrIFEInput;
		ostringstream lstrValue, lstrPcbValue;
		ostringstream lstrAdacScan, lstrFloatValue;
		lstrInput		<<"#Measures"<<endl;
		lstrInput		<<"#Input      "<<dec;
		lstrStrInput	<<"#Input      "<<dec;
		lstrAdcInput	<<"#ADC Input  "<<dec;
		lstrIFEInput	<<"#IFE Input  "<<dec;
		lstrVFEInput	<<"#VFE Input  "<<dec;
		lstrAdacScan	<<"#ADAC Value "<<dec;
		lstrValue		<<"#Value      "<<dec;
		lstrFloatValue	<<"#Float Value"<<dec;
		lstrPcbValue	<<"#PcbValue   "<<dec;
		list<int>::iterator litInput	=AcqValues.LAdcMonInputSel.begin();
		list<string>::iterator litStrInput	=AcqValues.LAdcMonStrInputSel.begin();
		list<int>::iterator litAdcInput	=AcqValues.LAdcMonAdcInputSel.begin();
		list<int>::iterator litVFEInput	=AcqValues.LAdcMonVFEInputSel.begin();
		list<int>::iterator litIFEInput	=AcqValues.LAdcMonIFEInputSel.begin();
		list<int>::iterator litAdacScanValue	=AcqValues.LAdacScanValues.begin();
		list<int>::iterator litValue	=AcqValues.LAdcMonValues.begin();
		list<float>::iterator litPcbValue	=AcqValues.LAdcPcbValues.begin();
		while(litValue!=AcqValues.LAdcMonValues.end()) {
   			lstrInput		<<"\t"	<< *litInput		;
			lstrStrInput	<<"\t"	<< *litStrInput		;
	   		lstrAdcInput	<<"\t"	<< *litAdcInput		;
   			lstrVFEInput	<<"\t"	<< *litVFEInput		;
   			lstrIFEInput	<<"\t"	<< *litIFEInput		;
			if(lbScanActivated) {
				lstrAdacScan<<"\t"	<< *litAdacScanValue	;
			}
			lstrValue		<<"\t"	<< *litValue		;
			lstrFloatValue	<<"\t"	<< GetRealValue(*litValue,*litAdcInput)	;
			lstrPcbValue	<<"\t"	<< *litPcbValue		;

			if(!lbScanActivated) { // for scanning we repeat the input number...
				litInput++;
				litStrInput++;
				litAdcInput++;
				litVFEInput++;
				litIFEInput++;
			} else {
				litAdacScanValue++;
			}
			litValue++;
			litPcbValue++;
		}
		lofsFile<<lstrInput.str()<<endl;
		lofsFile<<lstrStrInput.str()<<endl;
		lofsFile<<lstrAdcInput.str()<<endl;
		lofsFile<<lstrIFEInput.str()<<endl;
		lofsFile<<lstrVFEInput.str()<<endl;
		if(lbScanActivated) {
			lofsFile<<lstrAdacScan.str()<<endl;
		}
		lofsFile<<lstrValue.str()<<endl;
		lofsFile<<lstrFloatValue.str()<<endl;
		lofsFile<<lstrPcbValue.str()<<endl;
	} else {
		lofsFile<<"No measured stored"<<endl;
	}
	
	lofsFile<<"################"<<endl;
	lofsFile<<"#Comments"<<endl;
	lofsFile<<AcqValues.StrStatus<<endl;
	lofsFile.close();
	return(STOK);
} 





Status CAdcMon::SaveToDatFile(const string strFileName){
	std::ofstream lofsFile;
	
	char lstrTime[20];
	time_t now = time(0);
	strftime(lstrTime,sizeof(lstrTime),"%Y%m%d_%H%M%S", localtime(&now));
	
	
	if(!(AcqValues.LAdcMonInputSel.empty())) {
		bool lbScanActivated;
		if(!AcqValues.LAdacScanValues.empty()) {
			lbScanActivated=true;
		} else {
			lbScanActivated=false;
		}

		
		list<int>::iterator litInput	=AcqValues.LAdcMonInputSel.begin();
		list<string>::iterator litStrInput	=AcqValues.LAdcMonStrInputSel.begin();
		list<int>::iterator litAdcInput	=AcqValues.LAdcMonAdcInputSel.begin();
		list<int>::iterator litAdacScanValue	=AcqValues.LAdacScanValues.begin();
		list<int>::iterator litEvalValue	=AcqValues.LAdcEvalValues.begin();
		list<int>::iterator litValue	=AcqValues.LAdcMonValues.begin();
		list<float>::iterator litPcbValue	=AcqValues.LAdcPcbValues.begin();
		int lnCurrentInput=*litInput;
		string lstrFileName=strFileName;
		lstrFileName+="_";
		lstrFileName+=lstrTime;
		lstrFileName+="_";
		lstrFileName+=*litStrInput;
		lstrFileName+=".dat";
		lofsFile.open (lstrFileName, std::ofstream::out);
		while(litValue!=AcqValues.LAdcMonValues.end()) {
			if(lnCurrentInput!=*litInput) {
				lnCurrentInput=*litInput;
				lofsFile.close();
				lstrFileName=strFileName;
				lstrFileName+="_";
				lstrFileName+=lstrTime;
				lstrFileName+="_";
				lstrFileName+=*litStrInput;
				lstrFileName+=".dat";
				lofsFile.open (lstrFileName, std::ofstream::out | std::ofstream::app);
			}
			if(lbScanActivated) {
				lofsFile<<*litAdacScanValue<<"\t"<<*litValue<<"\t"<<*litPcbValue<<"\t"<<GetRealValue(*litValue,*litAdcInput)<<endl ;
			} else {
				lofsFile<<*litEvalValue<<"\t"<<*litValue<<"\t"<<*litPcbValue<<"\t"<<GetRealValue(*litValue,*litAdcInput)<<endl ;
			}
			
			if(!lbScanActivated) { // for scanning we repeat the input number...
				litInput++;
				litStrInput++;
				litAdcInput++;
				litEvalValue++;
			} else {
				litAdacScanValue++;
			}
			litValue++;
			litPcbValue++;
		}
	}
	lofsFile.close();
	
	return(STOK);
} 

