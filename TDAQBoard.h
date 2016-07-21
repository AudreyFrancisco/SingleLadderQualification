//
//  TDaqboard.h
//

#ifndef DAQBOARD_H
#define DAQBOARD_H

#include "USB.h"
#include "TReadoutBoard.h"

class TConfig; 



class TDAQBoard : public TUSBBoard, public TReadoutBoard {
 private: 
  static const int NEndpoints = 4;
  static const int ENDPOINT_WRITE_REG =0;
  static const int ENDPOINT_READ_REG  =1;
  static const int ENDPOINT_READ_ADC  =2;
  static const int ENDPOINT_READ_DATA =3;

  static const int DAQBOARD_WORD_SIZE        = 4;
  static const int DAQBOARD_ADDR_REG_SIZE    = 8;
  static const int DAQBOARD_ADDR_MODULE_SIZE = 4;

  static const int DAQBOARD_WRITE_INSTR_REG  = 0x0;
  static const int DAQBOARD_WRITE_DATA_REG   = 0x1;


  static const int MODULE_FPGA      = 0x0;
  static const int MODULE_ADC       = 0x1;
  static const int MODULE_READOUT   = 0x2;
  static const int MODULE_TRIGGER   = 0x3;
  static const int MODULE_JTAG      = 0x4;
  static const int MODULE_RESET     = 0x5;
  static const int MODULE_IDENT     = 0x6;
  static const int MODULE_SOFTRESET = 0x7;

  int SendWord          (uint32_t value);
  int ReadAcknowledge   ();
  int WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId = 0);
 protected: 
 public: 
  TDAQBoard(libusb_device *ADevice, TConfig *config);


  int  ReadRegister      (uint32_t Address, uint32_t &Value);
  int  WriteRegister     (uint32_t Address, uint32_t Value);

  int  ReadChipRegister  (uint16_t Address, uint16_t &Value, uint8_t chipID = 0);

  int  SendOpCode        (uint8_t  OpCode) ;

  int  SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay);
  void SetTriggerSource  (TTriggerSource triggerSource);
  int  Trigger           (int nTriggers);
  int  ReadEventData     (int &NBytes, char *Buffer);


};



#endif    /* DAQBOARD_H */
