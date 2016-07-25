//
//  TDaqboard.h
//

#ifndef READOUTBOARDDAQ_H
#define READOUTBOARDDAQ_H

#include "USB.h"
#include "TReadoutBoard.h"
#include "TConfig.h"
#include "TBoardConfigDAQ.h"


//enum TTriggerSource {TRIG_INT, TRIG_EXT};


//************************************************************
// TReadOutBoardDAQ: implementationn for Cagliari DAQboard 
//************************************************************

class TReadoutBoardDAQ : public TUSBBoard, public TReadoutBoard {
 private: 
  TBoardConfigDAQ *fBoardConfigDAQ;

  static const int NEndpoints = 4;
  static const int ENDPOINT_WRITE_REG =0;
  static const int ENDPOINT_READ_REG  =1;
  static const int ENDPOINT_READ_ADC  =2;
  static const int ENDPOINT_READ_DATA =3;

  // instruction words
  static const int DAQBOARD_WORD_SIZE        = 4;  // communication to DAQboard based on 32-bit words, 4 bytes
  static const int DAQBOARD_REG_ADDR_SIZE    = 8;  // sub(reg)-address size = 12-bit
  static const int DAQBOARD_MODULE_ADDR_SIZE = 4;  // module-address size   =  4-bit

  // CMU Module?
  //// JTAG Module: Register 
  static const int DAQBOARD_WRITE_INSTR_REG  = 0x0;
  static const int DAQBOARD_WRITE_DATA_REG   = 0x1;

  // module addresses
  static const int MODULE_FPGA      = 0x0;  // Controller module
  static const int MODULE_ADC       = 0x1;
  static const int MODULE_READOUT   = 0x2;
  static const int MODULE_TRIGGER   = 0x3;
  static const int MODULE_JTAG      = 0x4;
  //static const int MODULE_CMU       = 0x4;
  static const int MODULE_RESET     = 0x5;
  static const int MODULE_IDENT     = 0x6;
  //static const int MODULE_ID        = 0x6;
  static const int MODULE_SOFTRESET = 0x7;

  // ADC Module: Register
  static const int ADC_CONFIG0  = 0x0;
  static const int ADC_CONFIG1  = 0x1;
  static const int ADC_CONFIG2  = 0x2;
  static const int ADC_READ0    = 0x3; // Read only
  static const int ADC_READ1    = 0x4; // Read only
  static const int ADC_READ2    = 0x5; // Read only
  static const int ADC_OVERFLOW = 0x9; // Read only

  // RESET Module: Register
  static const int RESET_DURATION    = 0x0;
  static const int RESET_DELAYS      = 0x1;
  static const int RESET_DRST        = 0x2;
  static const int RESET_PRST        = 0x3;
  static const int RESET_PULSE       = 0x4;
  static const int RESET_PULSE_DELAY = 0x5;

  //int fLimitDigital;
  //int fLimitIo;
  //int fLimitAnalogue;

  //int fAutoShutdownTime;
  //int fClockEnableTime;
  //int fSignalEnableTime;
  //int fDrstTime;


  int SendWord          (uint32_t value);
  int ReadAcknowledge   ();

  int WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId = 0);
  int  ReadChipRegister  (uint16_t Address, uint16_t &Value, uint8_t chipID = 0);

 protected: 
 public: 
  TReadoutBoardDAQ(libusb_device *ADevice, TBoardConfigDAQ *config);


  int  ReadRegister      (uint16_t Address, uint32_t &Value);
  int  WriteRegister     (uint16_t Address, uint32_t Value);

  int  SendOpCode        (uint8_t  OpCode) ;

  int  SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay);
  void SetTriggerSource  (TTriggerSource triggerSource);
  int  Trigger           (int nTriggers);
  int  ReadEventData     (int &NBytes, char *Buffer);
  bool PowerOn           (int &AOverflow);

  // this should probably be moved elsewhere and get the config structure as parameter
  int  SetADCConfig      ();
  bool GetLDOStatus(int &AOverflow);
  float ReadAnalogI  ();
  float ReadDigitalI ();
  int   CurrentToADC (int current);
  float ADCToCurrent (int value);
  void  WriteDelays    ();
  void  WriteLimits    (bool LDOOn, bool Autoshutdown);
};

//************************************************************



#endif    /* READOUTBOARDDAQ_H */
