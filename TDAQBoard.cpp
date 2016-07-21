#include "TDAQBoard.h"
#include "TAlpide.h"


TDAQBoard::TDAQBoard (libusb_device *ADevice, TConfig *config) : TUSBBoard (ADevice), TReadoutBoard(config)
{

}


int TDAQBoard::SendWord (uint32_t value) 
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


int TDAQBoard::ReadAcknowledge() 
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


int TDAQBoard::ReadRegister (uint32_t address, uint32_t &value)
{
  unsigned char data_buf[DAQBOARD_WORD_SIZE * 2];
  uint32_t      headerword = 0;
  int           err; 

  err = SendWord (address +  (1 << (DAQBOARD_ADDR_REG_SIZE + DAQBOARD_ADDR_MODULE_SIZE))); // add 1 bit for read access
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


int TDAQBoard::WriteRegister (uint32_t address, uint32_t value)
{
  int err;
  err = SendWord(address);
  if (!err) return -1;       // add exceptions
  err = SendWord(value);
  if (!err) return -1;
  err = ReadAcknowledge();
  if (!err) return -1;

  return 0;
}


int TDAQBoard::WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId)
{
  int err;
  uint32_t command [4];
  uint32_t address32 = (uint32_t) address;
  uint32_t chipId32  = (uint32_t) chipId; 
  uint32_t newAddress = (address32 << 16) | (chipId32 << 8) | TAlpide::OPCODE_WROP;

  command[0] = DAQBOARD_WRITE_DATA_REG + (MODULE_JTAG << DAQBOARD_ADDR_REG_SIZE);
  command[1] = (uint32_t) value;
  command[2] = DAQBOARD_WRITE_INSTR_REG + (MODULE_JTAG << DAQBOARD_ADDR_REG_SIZE);
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


int TDAQBoard::ReadChipRegister (uint16_t address, uint16_t &value, uint8_t chipId) 
{
  int           err; 
  unsigned char data_buf[DAQBOARD_WORD_SIZE * 2];
  uint32_t      command [2];
  uint32_t      headerword = 0;
  uint32_t      address32  = (uint32_t) address;
  uint32_t      chipId32   = (uint32_t) chipId;
  uint32_t      newAddress = (address32 << 16) | (chipId32 << 8) | TAlpide::OPCODE_RDOP;

  command[0] = DAQBOARD_WRITE_INSTR_REG + (MODULE_JTAG << DAQBOARD_ADDR_REG_SIZE);
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


int TDAQBoard::SendOpCode (uint8_t  OpCode) 
{
  return WriteRegister (DAQBOARD_WRITE_INSTR_REG + (MODULE_JTAG << DAQBOARD_ADDR_REG_SIZE), (int) OpCode);
}



int TDAQBoard::SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay)
{
  return 0;
}


void TDAQBoard::SetTriggerSource  (TTriggerSource triggerSource)
{
}


int TDAQBoard::Trigger           (int nTriggers)
{
  return 0;
}


int TDAQBoard::ReadEventData     (int &NBytes, char *Buffer)
{
  return 0;
}
