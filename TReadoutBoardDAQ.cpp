#include <math.h> 

#include "TReadoutBoardDAQ.h"
#include "TAlpide.h"

// constructor
TReadoutBoardDAQ::TReadoutBoardDAQ (libusb_device *ADevice, TBoardConfigDAQ *config) : TUSBBoard (ADevice), TReadoutBoard(config)
{
  fBoardConfigDAQ = config;

  //fLimitDigital  = config->fLimitDigital;
  //fLimitIo       = config->fLimitIo;
  //fLimitAnalogue = config->fLimitAnalogue;

  //fAutoShutdownTime = config->fAutoShutdownTime;
  //fClockEnableTime  = config->fClockEnableTime;
  //fSignalEnableTime = config->fSignalEnableTime;
  //fDrstTime         = config->fDrstTime;

  WriteDelays();

}




//---------------------------------------------------------
// general methods of TReadoutBoard
//---------------------------------------------------------


int TReadoutBoardDAQ::ReadRegister (uint16_t address, uint32_t &value)
{
  unsigned char data_buf[DAQBOARD_WORD_SIZE * 2];
  uint32_t      headerword = 0;
  int           err; 

  err = SendWord ((uint32_t)address +  (1 << (DAQBOARD_REG_ADDR_SIZE + DAQBOARD_MODULE_ADDR_SIZE))); // add 1 bit for read access
  if (err < 0) return -1;
  err = ReceiveData (ENDPOINT_READ_REG, data_buf, DAQBOARD_WORD_SIZE * 2); 

  if (err < 0) return -1;

  value = 0;

  for (int i = 0; i < DAQBOARD_WORD_SIZE; i ++) {
    headerword += (data_buf[i                     ] << (8 * i));   // bytes 0 ... 3 are header
    value      += (data_buf[i + DAQBOARD_WORD_SIZE] << (8 * i));   // bytes 4 ... 7 are data
  }
  return 0;
}


int TReadoutBoardDAQ::WriteRegister (uint16_t address, uint32_t value)
{
  int err;

  err = SendWord((uint32_t)address);

  if (err < 0) return -1;       // add exceptions

  err = SendWord(value);
  if (err < 0) return -1;
  err = ReadAcknowledge();
  if (err < 0) return -1;

  return 0;
}



int TReadoutBoardDAQ::WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId)
{
  int err;
  uint32_t address32 = (uint32_t) address;
  uint32_t chipId32  = (uint32_t) chipId; 
  uint32_t newAddress = (address32 << 16) | (chipId32 << 8) | Alpide::OPCODE_WROP;

  err = WriteRegister (CMU_DATA + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), (uint32_t) value);
  if(err < 0) return -1;
  err = WriteRegister (CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), newAddress);
  if(err < 0) return -1;

  return 0;
}


int TReadoutBoardDAQ::ReadChipRegister (uint16_t address, uint16_t &value, uint8_t chipId) 
{
  int           err; 
  uint32_t      value32; 
  uint32_t      address32  = (uint32_t) address;
  uint32_t      chipId32   = (uint32_t) chipId;
  uint32_t      newAddress = (address32 << 16) | (chipId32 << 8) | Alpide::OPCODE_RDOP;


  err = WriteRegister(CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), newAddress);
  if (err < 0) return -1;
  err = ReadRegister (CMU_DATA + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), value32);
  if (err < 0) return -1;

  value = value32 & 0xffff;
  return 0;
}


int TReadoutBoardDAQ::SendOpCode (uint8_t  OpCode) 
{
  return WriteRegister (CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), (int) OpCode);
}



int TReadoutBoardDAQ::SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay)
{
  return 0;
}


void TReadoutBoardDAQ::SetTriggerSource  (TTriggerSource triggerSource)
{
}


int TReadoutBoardDAQ::Trigger           (int nTriggers)
{
  return 0;
}


int TReadoutBoardDAQ::ReadEventData     (int &NBytes, char *Buffer)
{
  return 0;
}







//---------------------------------------------------------
// methods only for Cagliari DAQ board
//---------------------------------------------------------



// method to send 32 bit words to DAQ board
// FPGA internal registers have 12-bit addres field and 32-bit data payload
int TReadoutBoardDAQ::SendWord (uint32_t value) 
{
    unsigned char data_buf[DAQBOARD_WORD_SIZE];

    for (int i=0; i<DAQBOARD_WORD_SIZE; i++) {
        data_buf[i] = value & 0xff;
        value >>= 8;
    }

    if (SendData (ENDPOINT_WRITE_REG,data_buf,DAQBOARD_WORD_SIZE) != DAQBOARD_WORD_SIZE)
      return -1;
    return 0;
}


int TReadoutBoardDAQ::ReadAcknowledge() 
{ 
  unsigned char data_buf[2 * DAQBOARD_WORD_SIZE];
  uint32_t      headerword = 0, 
                dataword = 0;
  int           err;

  err=ReceiveData(ENDPOINT_READ_REG, data_buf, 2 * DAQBOARD_WORD_SIZE);

  if (err < 0) return -1;

  for (int i = 0; i < DAQBOARD_WORD_SIZE; i ++) {
    headerword += (data_buf[i                     ] << (8 * i));   // bytes 0 ... 3 are header
    dataword   += (data_buf[i + DAQBOARD_WORD_SIZE] << (8 * i));   // bytes 4 ... 7 are data
  }

  return 0;  
}



int TReadoutBoardDAQ::CurrentToADC (int current)
{
  float Result = (float) current / 100. * 4096. / 3.3;
  //std::cout << "Current to ADC, Result = " << Result << std::endl;
  return (int) Result;
}


float TReadoutBoardDAQ::ADCToCurrent (int value)
{
    float Result = (float) value * 3.3 / 4096.;   // reference voltage 3.3 V, full range 4096
    Result /= 0.1;    // 0.1 Ohm resistor
    Result *= 10;     // / 100 (gain) * 1000 (conversion to mA);
    return Result;
}


float TReadoutBoardDAQ::ADCToTemperature (int AValue) 
{
    float    Temperature, R;
    float    AVDD = 1.8;
    float    R2   = 5100;
    float    B    = 3900;
    float    T0   = 273.15 + 25;
    float    R0   = 10000;

    float Voltage = (float) AValue;
    Voltage       *= 3.3;
    Voltage       /= (1.8 * 4096);

    R           = (AVDD/Voltage) * R2 - R2;   // Voltage divider between NTC and R2
    Temperature = B / (log (R/R0) + B/T0);

    return Temperature;
}

bool TReadoutBoardDAQ::PowerOn (int &AOverflow) 
{
  
  // set current limits with voltages off
  WriteCurrentLimits(false, true); 
  // switch on voltages
  WriteCurrentLimits(true, true);

  return ReadLDOStatus(AOverflow);
}


void TReadoutBoardDAQ::PowerOff () 
{
  // registers set in sequence similar to old software.. 

  fBoardConfigDAQ->SetDataPortSelect(0); // select no dataport
  WriteReadoutModuleConfigRegisters();

  fBoardConfigDAQ->SetDrstTime(0);          // TODO necessary? 
  fBoardConfigDAQ->SetClockEnableTime(0);   // TODO necessary? 
  fBoardConfigDAQ->SetSignalEnableTime(0);  // TODO necessary? 
  fBoardConfigDAQ->SetAutoShutdownTime(1);  // TODO necessary? 
  //WriteDelays();
  WriteResetModuleConfigRegisters();
    
  fBoardConfigDAQ->SetAutoShutdownEnable(1);
  fBoardConfigDAQ->SetLDOEnable(0);
  WriteADCModuleConfigRegisters();

}



//---------------------------------------------------------
// methods module by module
//---------------------------------------------------------

// ADC Module
//----------------------------------------------------------------------------


void TReadoutBoardDAQ::WriteADCModuleConfigRegisters() 
{
  int limitDigital = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitDigital());
  int limitIo      = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitIo());
  int limitAnalogue= CurrentToADC (fBoardConfigDAQ->GetCurrentLimitAnalogue());

  // ADC config reg 0
  uint32_t config0 = 0;
  config0 |= ( limitDigital                              & 0xfff);
  config0 |= ((limitIo                                   & 0xfff) << 12);
  config0 |= ((fBoardConfigDAQ->GetAutoShutdownEnable() ?1:0)     << 24);
  config0 |= ((fBoardConfigDAQ->GetLDOEnable()          ?1:0)     << 25);   
  config0 |= ((fBoardConfigDAQ->GetADCEnable()          ?1:0)     << 26);   
  config0 |= ((fBoardConfigDAQ->GetADCSelfStop()        ?1:0)     << 27);   
  config0 |= ((fBoardConfigDAQ->GetDisableTstmpReset()  ?1:0)     << 28);   
  config0 |= ((fBoardConfigDAQ->GetPktBasedROEnableADC()?1:0)     << 29);   
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG0, config0);

  // ADC config reg 1
  uint32_t config1 = 0;
  config1 |= ( limitAnalogue                              & 0xfff);
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG1, config1);

  // ADC config reg 2
  uint32_t config2 = 0;
  config2 |= ( fBoardConfigDAQ->GetAutoShutOffDelay()     & 0xfffff);
  config2 |= ( fBoardConfigDAQ->GetADCDownSamplingFact()  & 0xfff  << 20);
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG2, config2);

}


void TReadoutBoardDAQ::WriteCurrentLimits (bool ALDOEnable, bool AAutoshutdown) 
{
  int limitDigital = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitDigital());
  int limitIo      = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitIo());
  int limitAnalog  = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitAnalogue());
    
  fBoardConfigDAQ->SetAutoShutdownEnable(AAutoshutdown);  // keep track of settings in BoardConfig..
  fBoardConfigDAQ->SetLDOEnable(ALDOEnable);              // keep track of settings in BoardConfig..

  uint32_t config0 = (((int) limitDigital) & 0xfff) | ((((int) limitIo) & 0xfff) << 12);
  config0 |= ((AAutoshutdown?1:0) << 24);
  config0 |= ((ALDOEnable       ?1:0) << 25);   
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG0, config0);
  uint32_t config1 = ((int) limitAnalog) & 0xfff;
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG1, config1);
}


bool TReadoutBoardDAQ::ReadLDOStatus(int &AOverflow) 
{
  uint32_t ReadValue;
  bool     err, reg0, reg1, reg2;

  err  = ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  reg0 = ((ReadValue & 0x1000000) != 0); // LDO off if bit==0, on if bit==1
  err  = ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  reg1 = ((ReadValue & 0x1000000) != 0);
  err  = ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  reg2 = ((ReadValue & 0x1000000) != 0);
  
  err = ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_OVERFLOW, ReadValue);
  AOverflow = (int) ReadValue;

  if (! (reg0 & reg1 & reg2)) 
    std::cout << "GetLDOStatus, LDO status = " << reg0 << ", " << reg1 << ", " << reg2 << std::endl;

  return ( reg0 & reg1 & reg2);
}


void TReadoutBoardDAQ::DecodeOverflow  (int AOverflow) {
    if (AOverflow & 0x1) {
        std::cout << "Overflow in digital current" << std::endl;
    }
    if (AOverflow & 0x2) {
        std::cout << "Overflow in digital I/O current" << std::endl;
    }
    if (AOverflow & 0x4) {
        std::cout << "Overflow in analogue current" << std::endl;
    }
}


float TReadoutBoardDAQ::ReadAnalogI() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToCurrent(Value);
}

float TReadoutBoardDAQ::ReadDigitalI() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToCurrent(Value);
}

float TReadoutBoardDAQ::ReadIoI() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  int Value = (ReadValue) & 0xfff;

  return ADCToCurrent(Value);
}

float TReadoutBoardDAQ::ReadMonV() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToCurrent(Value);
}

float TReadoutBoardDAQ::ReadTemperature() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
    //printf("NTC ADC: 0x%08X\n",Reading);
  int Value = (ReadValue) & 0xfff;

  return ADCToTemperature (Value);
}




// READOUT Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteReadoutModuleConfigRegisters() 
{

  // Event builder config reg 0
  uint32_t config = 0;
  config |= ( fBoardConfigDAQ->GetMaxDiffTriggers()       & 0xf);
  config |= ((fBoardConfigDAQ->GetSamplingEdgeSelect()?1:0)       << 4);
  config |= ((fBoardConfigDAQ->GetPktBasedROEnable()?  1:0)       << 5);
  config |= ((fBoardConfigDAQ->GetDDREnable()?         1:0)       << 6);
  config |= ((fBoardConfigDAQ->GetDataPortSelect()        & 0x3)  << 7);
  config |= ((fBoardConfigDAQ->GetFPGAEmulationMode()     & 0x3)  << 9);

  WriteRegister ((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_EVENTBUILDER_CONFIG, config);

}

bool TReadoutBoardDAQ::ResyncSerialPort ()
{
    return WriteRegister((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_RESYNC, 0x0);
}

bool TReadoutBoardDAQ::WriteSlaveDataEmulatorReg(uint32_t AWord) {
    AWord &= 0xffffffff;
    return WriteRegister((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_SLAVE_DATA_EMULATOR, AWord);
}


// TRIGGER Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteTriggerModuleConfigRegisters() 
{

  //  busy config reg
  uint32_t config0 = 0;
  config0 |= fBoardConfigDAQ->GetBusyDuration();
  WriteRegister ((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_DURATION, config0);

  // trigger conif reg
  uint32_t config1 = 0;
  config1 |= ( fBoardConfigDAQ->GetNTriggers()          & 0xffff);
  config1 |= ((fBoardConfigDAQ->GetTriggerMode()        & 0x7)   << 16);
  config1 |= ((fBoardConfigDAQ->GetStrobeDuration()     & 0xff)  << 19);
  WriteRegister ((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_TRIGGER_CONFIG, config1);

  //  strobe delay config reg
  uint32_t config2 = 0;
  config2 |= fBoardConfigDAQ->GetStrobeDelay();
  WriteRegister ((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_DELAY, config2);

  //  busy override config reg
  uint32_t config3 = 0;
  config3 |= (fBoardConfigDAQ->GetBusyOverride()?1:0);
  WriteRegister ((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_OVERRIDE, config3);

}



bool TReadoutBoardDAQ::StartTrigger()
{
    return WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_START, 13);
}


bool TReadoutBoardDAQ::StopTrigger ()
{
    return WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_STOP, 13);
}


bool TReadoutBoardDAQ::WriteBusyOverrideReg(bool ABusyOverride)
{
    fBoardConfigDAQ->SetBusyOverride(ABusyOverride);
    bool err;
    err = WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_OVERRIDE, ABusyOverride);
    if (!err) return false;

    return err;
}



// CMU Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteCMUModuleConfigRegisters () 
{

  //  CMU config reg
  uint32_t config = 0;
  config |= ( fBoardConfigDAQ->GetManchesterDisable()     ?1:0);
  config |= ((fBoardConfigDAQ->GetSamplingEdgeSelectCMU() ?1:0)       << 1);
  config |= ((fBoardConfigDAQ->GetInvertCMUBus()          ?1:0)       << 2);
  config |= ((fBoardConfigDAQ->GetChipMaster()            ?1:0)       << 3);
  WriteRegister ((MODULE_CMU << DAQBOARD_REG_ADDR_SIZE) + CMU_CONFIG, config);
}



// RESET Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteResetModuleConfigRegisters () 
{
  //  PULSE DRST PRST duration reg
  uint32_t config0 = 0;
  config0 |= ( fBoardConfigDAQ->GetPRSTDuration()        & 0xff );
  config0 |= ((fBoardConfigDAQ->GetDRSTDuration()        & 0xff)       << 8);
  config0 |= ((fBoardConfigDAQ->GetPULSEDuration()       & 0xffff)     << 16);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DURATION, config0);

  // power up sequencer delay register
  uint32_t config1 = ((fBoardConfigDAQ->GetDrstTime()        & 0xff) << 24) 
                  | ((fBoardConfigDAQ->GetSignalEnableTime() & 0xff) << 16) 
                  | ((fBoardConfigDAQ->GetClockEnableTime()  & 0xff) << 8) 
                  | ( fBoardConfigDAQ->GetAutoShutdownTime() & 0xff);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DELAYS, config1);  

  // PULSE STROBE delay sequence reg
  uint32_t config2 = 0;
  config2 |= ( fBoardConfigDAQ->GetPulseDelay()          & 0xffff );
  config2 |= ((fBoardConfigDAQ->GetStrobePulseSeq()      & 0x3)     << 16);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_PULSE_DELAY, config2);

  // Power On Reset disable reg
  uint32_t config3 = 0;
  config3 |= ( fBoardConfigDAQ->GetPORDisable()     ?1:0);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_POR_DISABLE, config3);
}


void TReadoutBoardDAQ::WriteDelays () 
{
  uint32_t delays = ((fBoardConfigDAQ->GetDrstTime()         & 0xff) << 24) 
                  | ((fBoardConfigDAQ->GetSignalEnableTime() & 0xff) << 16) 
                  | ((fBoardConfigDAQ->GetClockEnableTime()  & 0xff) << 8) 
                  | ( fBoardConfigDAQ->GetAutoShutdownTime() & 0xff);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DELAYS, delays);  
}



// ID Module
//----------------------------------------------------------------------------

int TReadoutBoardDAQ::ReadBoardAddress() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_ADDRESS, ReadValue);
  int BoardAddress = ReadValue & 0xff;
  
  return BoardAddress;

}


uint32_t TReadoutBoardDAQ::ReadFirmwareVersion() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, ReadValue);

  return ReadValue;
}


uint32_t TReadoutBoardDAQ::ReadFirmwareDate() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, ReadValue);

  return (ReadValue & 0xffffff);
}


int TReadoutBoardDAQ::ReadFirmwareChipVersion() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, ReadValue);

  return ((ReadValue & 0xf0000000) >> 28);
}





// SOFTRESET Module
//----------------------------------------------------------------------------

bool TReadoutBoardDAQ::ResetBoardFPGA (int ADuration)
{
    fBoardConfigDAQ->SetSoftResetDuration(ADuration); // keep track of latest config in TBoardConfigDAQ
    bool err;
    err = WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_DURATION, ADuration);
    if (!err) return false;
    return WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_FPGA_RESET, 13);
}


bool TReadoutBoardDAQ::ResetBoardFX3 (int ADuration)
{
    fBoardConfigDAQ->SetSoftResetDuration(ADuration); // keep track of latest config in TBoardConfigDAQ
    bool err;
    err = WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_DURATION, ADuration);
    if (!err) return false;
    return WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_FX3_RESET, 13);
}








