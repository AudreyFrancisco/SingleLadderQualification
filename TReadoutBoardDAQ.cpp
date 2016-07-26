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
  uint32_t command [4];
  uint32_t address32 = (uint32_t) address;
  uint32_t chipId32  = (uint32_t) chipId; 
  uint32_t newAddress = (address32 << 16) | (chipId32 << 8) | Alpide::OPCODE_WROP;

  command[0] = CMU_DATA   + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE);
  command[1] = (uint32_t) value;
  command[2] = CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE);
  command[3] = (uint32_t) newAddress;

  SendWord (command[0]);
  SendWord (command[1]);
  err = ReadAcknowledge ();
  if(err < 0) return -1;

  SendWord (command[2]);
  SendWord (command[3]);
  err = ReadAcknowledge();
  if(err < 0) return -1;

  return 0;
}


int TReadoutBoardDAQ::ReadChipRegister (uint16_t address, uint16_t &value, uint8_t chipId) 
{
  int           err; 
  unsigned char data_buf[DAQBOARD_WORD_SIZE * 2];
  uint32_t      command [2];
  uint32_t      headerword = 0;
  uint32_t      address32  = (uint32_t) address;
  uint32_t      chipId32   = (uint32_t) chipId;
  uint32_t      newAddress = (address32 << 16) | (chipId32 << 8) | Alpide::OPCODE_RDOP;

  command[0] = CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE);
  command[1] = newAddress;
  SendWord (command[0]);
  SendWord (command[1]);
  err = ReadAcknowledge ();

  if (err < 0) return -1;

  SendWord (0x00001401);       // read data word back from FPGA

  err = ReceiveData (ENDPOINT_READ_REG, data_buf,DAQBOARD_WORD_SIZE * 2);
  if (err < 0) return -1;

  value = 0;

  for (int i = 0; i < DAQBOARD_WORD_SIZE; i ++) {
    headerword += (data_buf[i                     ] << (8 * i));   // bytes 0 ... 3 are header
    if (i < 2) 
      value += (data_buf[i + DAQBOARD_WORD_SIZE] << (8 * i));   // bytes 4 ... 7 are data, but value is 16 bit only
  }
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

void TReadoutBoardDAQ::WriteDelays () 
{
  uint32_t delays = ((fBoardConfigDAQ->GetDrstTime()         & 0xff) << 24) 
                  | ((fBoardConfigDAQ->GetSignalEnableTime() & 0xff) << 16) 
                  | ((fBoardConfigDAQ->GetClockEnableTime()  & 0xff) << 8) 
                  | ( fBoardConfigDAQ->GetAutoShutdownTime() & 0xff);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DELAYS, delays);  
}


float TReadoutBoardDAQ::ADCToTemperature (int AValue) {
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

bool TReadoutBoardDAQ::PowerOn (int &AOverflow) {
  //int   delayDrst    = 13;
  //int   delaySig     = 12;
  //int   delayClk     = 12;
  //int   delayShtdn   = 10;
 
  // set current limits with voltages off
  WriteCurrentLimits(false, true); 
  // switch on voltages
  WriteCurrentLimits(true, true);

  return ReadLDOStatus(AOverflow);
}



//****************************************************************************
// methods module by module
//****************************************************************************

// ADC Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteCurrentLimits (bool LDOOn, bool Autoshutdown) {
  int limitDigital = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitDigital());
  int limitIo      = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitIo());
  int limitAnalog  = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitAnalogue());

  

  uint32_t config0 = (((int) limitDigital) & 0xfff) | ((((int) limitIo) & 0xfff) << 12);
  config0 |= ((Autoshutdown?1:0) << 24);
  config0 |= ((LDOOn       ?1:0) << 25);   
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG0, config0);
  uint32_t config1 = ((int) limitAnalog) & 0xfff;
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG1, config1);
}


bool TReadoutBoardDAQ::ReadLDOStatus(int &AOverflow) {
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


float TReadoutBoardDAQ::ReadAnalogI() {
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToCurrent(Value);
}

float TReadoutBoardDAQ::ReadDigitalI() {
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToCurrent(Value);
}

float TReadoutBoardDAQ::ReadIoI() {
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  int Value = (ReadValue) & 0xfff;

  return ADCToCurrent(Value);
}

float TReadoutBoardDAQ::ReadMonV() {
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToCurrent(Value);
}

float TReadoutBoardDAQ::ReadTemperature() {
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
    //printf("NTC ADC: 0x%08X\n",Reading);
  int Value = (ReadValue) & 0xfff;

  return ADCToTemperature (Value);
}













