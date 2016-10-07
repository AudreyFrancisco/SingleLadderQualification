// needed:
// string.h
// list.h
// drbug.h

#ifndef __pALPIDEfs_software__adcmon__
#define __pALPIDEfs_software__adcmon__

#include "drbug.h"
using namespace DrBug;
#include <list>

extern char OutputPath[];
extern char Suffix[];
extern int USB_id;

using namespace std::chrono;
class CAdcMonAcq
{
public:
	CAdcMonAcq(TAlpide *pAlpide=NILPTR, const int nIcomp=2, const int nRampSpd=1); ///< default constructor
	~CAdcMonAcq(); ///< destructor 
	DrBug::Status SetInput(const int nInputSelect, const int nMode); ///< select and stores the right input for ADC and analog frontend
	DrBug::Status SetAdacValue(const int nInputSelect, const int nValue); ///< sets Analog Dac value (stores value if value=-1, restores it if -2, and set if >=0)
	void StoreValue(const int nValue, const int nMode); ///< stores the value obtained after calibration procedure
	void StorePcbValue(const float fValue); ///< stores the value obtained by the ADC onboard the pCB
	void StoreAdacScanValue(const int nValue); ///< stores the value sent to the Analogue DAC when a scanning is performed
	void StoreEvalValue(const int nValue); ///< stores the value of the ramp sent to the internal DAC inside the ADC
public:
	TAlpide *PAlpideHdl						; ///< Handle to chip
	std::string StrStatus					; ///< a string containing some informations about the life of the object
	float FTemp_beg							; ///< Temperature at creation
	float FTemp_end							; ///< Temperature (last temperature got)_
	int NIcomp								; ///< Comparator current (0:163μA - 1:190μA - 2:296μA (nominal) - 3:410 μA)
	int NRampSpd							; ///< Ramp Speed (0:500ns/spl - 1:1us/spl (nominal) - 2:2us/spl - 3:4us/spl)
	int NDiscriSign							; ///< Comparator Sign
	int NHalfBitTrim						; ///< Half Bit LSB trimmer
	int NCalValue							; ///< Value of Calibration procedure
	int NCalInputSel						; ///< Adc Input used for the Calibration procedure (parameter given to this software)
	std::string StrCalInputSel				; ///< Adc Input used for the Calibration procedure (human readable)
	int NCalAdcInputSel						; ///< Adc Input used for the Calibration procedure (0: AVSS - 1: DVSS - 2: AVDD - 3: DVDD - 4: Vbandgap through voltage scaling - 5: DACMONV - 6: DACMONI - 7: Bandgap (direct measurement) - 8: Temperature (direct measurement)
	int NCalIFEInputSel						; ///< Current Frontend Input used for the Calibration procedure (0:IRESET - 1:IAUX2 - 2:IBIAS - 3:IDB - 4:IREF (don't select it) - 5:ITHR -6:IREFBuffer)
	int NCalVFEInputSel						; ///< Voltage Frontend Input used for the Calibration procedure (0:VCASN - 1:VCASP - 2:VPULSEH - 3:VPULSEL - 4:VRESETP - 5:VRESETD - 6:VCASN2 - 7:VCLIP - 8:VTEMP - 9:ADCDAC)
	std::list<int> LAdacScanValues			; ///< Values set on Analog DAC when performing a scan DAC
	std::list<int> LAdcMonValues			; ///< Values from ADC monitor
	std::list<int> LAdcMonInputSel			; ///< Input Selected from this soft (0 to 21)
	std::list<std::string> LAdcMonStrInputSel; ///< Input Selected from this soft (human readable)
	std::list<int> LAdcMonAdcInputSel		; ///< Input Selected inside ADCMon (0: AVSS - 1: DVSS - 2: AVDD - 3: DVDD - 4: Vbandgap through voltage scaling - 5: DACMONV - 6: DACMONI - 7: Bandgap (direct measurement) - 8: Temperature (direct measurement)
	std::list<int> LAdcMonIFEInputSel		; ///< Current Frontend Selected in Analog Frontend (0:IRESET - 1:IAUX2 - 2:IBIAS - 3:IDB - 4:IREF (don't select it) - 5:ITHR -6:IREFBuffer)
	std::list<int> LAdcMonVFEInputSel		; ///< Voltage Frontend Selected in Analog Frontend (0:VCASN - 1:VCASP - 2:VPULSEH - 3:VPULSEL - 4:VRESETP - 5:VRESETD - 6:VCASN2 - 7:VCLIP - 8:VTEMP - 9:ADCDAC)
	std::list<float> LAdcPcbValues			; ///< Values from ADC onboard PCB 
	std::list<int> LAdcEvalValues			; ///< value sent to the ramp when testing internal DAC of the ADC
protected:
	uint16_t AdacValue[22]					; ///< Adac value storage to remind what was the value before operating a scandac
};
class ofstream;

class CAdcMon
{
public:
	CAdcMon(TReadoutBoardDAQ *ts, TAlpide *pAlpide, const int nIcomp=2, const int nRampSpd=1); ///< Constructor
	~CAdcMon(); ///< Destructor
	
	DrBug::Status Set(const int nIcomp=2, const int nRampSpd=1); ///< Modify paramteres
/*
 * Selected Input
	0. AVSS
	1. DVSS
	2. AVDD
	3. DVDD
	4. Vbandgap through voltage scaling (the measure is known to be incorrect for this input)
	5. VCASN
	6. VCASP
	7. VPULSEH
	8. VPULSEL
	9. VRESETP
	10. VRESETD
	11. VCASN2
	12. VCLIP
	13. VTEMP
	14. IRESET
	15. IAUX2
	16. IBIAS
	17. IDB
	18. IREF the value is not correct for this input
	19. ITHR
	20. BANDGAP
	21. TEMPERATURE
*/
	DrBug::Status Preset(const int nInputSelect=0); // set Adac to 64 for selected input (if possible!)
	DrBug::Status Calibrate(const int nInputSelect=0);
	DrBug::Status ManualMeasure(const int nInputSelect, const int nCalInputSelect=0);
	DrBug::Status AutomaticMeasure(const int nCalInputSelect=0);
	DrBug::Status SuperManualMeasure(const int nInputSelect=0);
	DrBug::Status ScanManual(const int nInputSelect, const int nCalInputSelect=0, const int nSplDist=1);
	DrBug::Status ScanSuperManual(const int nInputSelect=0, const int nSplDist=1);
	void AdcEval(); 
	DrBug::Status VerboseIt(); 
	DrBug::Status SaveToTxtFile(const std::string strFileName); 
	DrBug::Status SaveToDatFile(const std::string strFileName); 
protected:
	DrBug::Status UnitCalibrateMeas(const int nInputSelect=0);
	DrBug::Status UnitManualMeas(const int nInputSelect);
	int DoRamp();
	void Print(const string str);
	float GetRealValue(const int nValue, const int nAdcInputSel); ///< convert integer value to float value according to nInput selected
public:
	bool BIsVerbose; // == true to be verbose
	bool BReverse; // == true to perform the ramps in the reverse order (up downto 0)
protected:
	CAdcMonAcq AcqValues		; ///< values acquired
	TReadoutBoardDAQ * PSetupHdl		; ///< Handle to Test Setup
	TAlpide * PAlpideHdl		; ///< Handle to Alpide chip
};

//void CAdcMon_cal          (TReadoutBoardDAQ *ts, TAlpide *myAlpide, const int nInputSelect=0);
//
#endif /* defined(__pALPIDEfs_software__adcmon__) */
