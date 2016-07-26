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

  // module addresses
  static const int MODULE_CONTROL   = 0x0; 
  static const int MODULE_ADC       = 0x1;
  static const int MODULE_READOUT   = 0x2;
  static const int MODULE_TRIGGER   = 0x3;
  //static const int MODULE_JTAG      = 0x4;
  static const int MODULE_CMU       = 0x4;
  static const int MODULE_RESET     = 0x5;
  //static const int MODULE_IDENT     = 0x6;
  static const int MODULE_ID        = 0x6;
  static const int MODULE_SOFTRESET = 0x7;


  // ADC Module 0x1: Register sub-addresses
  static const int ADC_CONFIG0  = 0x0;
  static const int ADC_CONFIG1  = 0x1;
  static const int ADC_CONFIG2  = 0x2;
  static const int ADC_DATA0    = 0x3; // Read only, previously ADC_READ0
  static const int ADC_DATA1    = 0x4; // Read only, previously ADC_READ1
  static const int ADC_DATA2    = 0x5; // Read only, previously ADC_READ2
  static const int ADC_OVERFLOW = 0x9; // Read only
    
  // READOUT Module 0x2: Register sub-addresses
  static const int READOUT_EVENTBUILDER_CONFIG    = 0x0; // previously   READOUT_CHIP_DATA 
  static const int READOUT_EOR_COMMAND            = 0x1; // previously   READOUT_ENDOFRUN  
  static const int READOUT_EVTID1                 = 0x2; //     
  static const int READOUT_EVTID2                 = 0x3; //     
  static const int READOUT_RESYNC                 = 0x4; //
  static const int READOUT_SLAVE_DATA_EMULATOR    = 0x5; //
  static const int READOUT_TIMESTAMP1             = 0x6; // not existing in manual..
  static const int READOUT_TIMESTAMP2             = 0x7; // not existing in manual..
   
  // TRIGGER Module 0x3: Register sub-addresses
  static const int TRIG_BUSY_DURATION    = 0x0; 
  static const int TRIG_TRIGGER_CONFIG   = 0x1;
  static const int TRIG_START            = 0x2;
  static const int TRIG_STOP             = 0x3;
  static const int TRIG_DELAY            = 0x4;
  static const int TRIG_BUSY_OVERRIDE    = 0x5;
  static const int TRIG_STROBE_COUNT     = 0x6;  // not existing in manual..

  // CMU Module 0x4: Register sub-addresses
  static const int CMU_INSTR        = 0x0; // previously called DAQBOARD_WRITE_INSTR_REG from JTAG?
  static const int CMU_DATA         = 0x1; // previously called DAQBOARD_WRITE_DATA_REG from JTAG?
  static const int CMU_CONFIG       = 0x2; // previously called DAQ_CONFIG_REG
  //// JTAG Module 0x4: Register TODO -> ONLY FOR PALPIDE-1??
  //static const int DAQBOARD_WRITE_INSTR_REG  = 0x0;
  //static const int DAQBOARD_WRITE_DATA_REG   = 0x1;

  // RESET Module 0x5: Register sub-addresses
  static const int RESET_DURATION    = 0x0;
  static const int RESET_DELAYS      = 0x1;
  static const int RESET_DRST        = 0x2;
  static const int RESET_PRST        = 0x3; 
  static const int RESET_PULSE       = 0x4;
  static const int RESET_PULSE_DELAY = 0x5;
  static const int RESET_POR_DISABLE = 0x6;
  
  // IDENTIFICATION Module 0x6: Register sub-addresses
  static const int ID_ADDRESS     = 0x0;
  static const int ID_CHIP        = 0x1;
  static const int ID_FIRMWARE    = 0x2;

  // SOFTRESET Module Register 0x7: Register sub-addresses
  static const int SOFTRESET_DURATION   = 0x0;
  static const int SOFTRESET_COMMAND    = 0x1; // previously SOFTRESET_FPGA_RESET?
  //static const int SOFTRESET_FPGA_RESET = 0x1; // not existing in manual..
  //static const int SOFTRESET_FX3_RESET  = 0x2; // not existing in manual..


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
