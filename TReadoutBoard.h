#ifndef READOUTBOARD_H
#define READOUTBOARD_H

#include <stdint.h>
#include <vector>

typedef enum {trigInt, trigExt} TTriggerSource;   // move these to TBoardConfig?
typedef enum {boardDAQ, boardMOSAIC} TBoardType;

#include "TBoardConfig.h"

typedef struct {
  int  chipId;
  int  controlInterface;
  int  receiver;
  bool enabled; 
} TChipPos;

class TBoardConfig;
 
//************************************************************
// abstract base class for all readout boards
//************************************************************

class TReadoutBoard {
 private:
  int  fNChips;                              // probably obsolete, use fChipPositions.size() instead
  
 protected:
  std::vector <TChipPos> fChipPositions;  // Antonio : change in protected to access from derived class
  TBoardConfig *fBoardConfig;

  virtual int WriteChipRegister   (uint16_t Address, uint16_t Value, uint8_t chipId = 0)  = 0;
  int         GetControlInterface (uint8_t chipId);
  int         GetReceiver         (uint8_t chipId);
  int         GetChipById         (uint8_t chipId);
  friend class TAlpide;     // could be reduced to the relevant methods ReadRegister, WriteRegister
 public:
  //TReadoutBoard  (TBoardConfig *config) {};
  TReadoutBoard  (TBoardConfig *config);
  ~TReadoutBoard () {};

  int          AddChip           (uint8_t chipId, int controlInterface, int receiver);
  void         SetChipEnable     (uint8_t chipId, bool Enable);

  void         SetControlInterface (uint8_t chipId, int controlInterface);
  void         SetReceiver         (uint8_t chipId, int receiver);

  TBoardConfig *GetConfig        () {return fBoardConfig;};

  virtual int  ReadRegister      (uint16_t Address, uint32_t &Value) = 0;
  virtual int  WriteRegister     (uint16_t Address, uint32_t Value)  = 0;

  virtual int  ReadChipRegister  (uint16_t Address, uint16_t &Value, uint8_t chipID = 0) = 0;

  // sends op code to all control interfaces
  virtual int  SendOpCode        (uint16_t  OpCode) = 0;
  // sends op code to control interface belonging to chip chipId
  virtual int  SendOpCode        (uint16_t  OpCode, uint8_t chipId) = 0;

  virtual int  SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay) = 0;
  virtual void SetTriggerSource  (TTriggerSource triggerSource) = 0;
  virtual int  Trigger           (int nTriggers) = 0;
  virtual int  ReadEventData     (int &NBytes, unsigned char *Buffer) = 0; // TODO: max buffer size not needed??

};


#endif  /* READOUTBOARD_H */
