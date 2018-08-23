#include "TRUv1PowerBoard.h"

#include <cassert>
#include <iostream>
#include <vector>

#include "TReadoutBoardRUv1.h"

TRUv1PowerBoard::TRUv1PowerBoard(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1PowerBoard::ConfigureBiasADC(bool commit)
{
  uint16_t slaveAddress = TRUv1PowerBoard::ADCBiasAddressSetup;
  uint16_t byte2        = 0x0B;
  uint16_t byte3        = 0x02;
  uint16_t data         = (byte2 << 8) | byte3;
  Write(slaveAddress, data, false);
  byte2 = 0x00;
  byte3 = 0x01;
  data  = (byte2 << 8) | byte3;
  Write(slaveAddress, data, commit);
}

std::vector<uint16_t> TRUv1PowerBoard::ReadBiasADC()
{

  std::vector<uint16_t> adcData;
  uint16_t              slaveAddress = TRUv1PowerBoard::ADCBiasAddress;
  for (int channel = 0; channel < 2; channel++) {
    uint16_t byte3 = 0x20 | channel;
    Write(slaveAddress, byte3, true);
    uint16_t readValue = Read(slaveAddress);
    adcData.push_back(readValue >> 4);
  }

  return adcData;
}

uint16_t TRUv1PowerBoard::ReadBiasADCChannel(int channel)
{

  assert((channel >= 0) && (channel < 3));

  uint16_t byte3        = 0x20 | channel;
  uint16_t slaveAddress = TRUv1PowerBoard::ADCBiasAddress;
  Write(slaveAddress, byte3, true);
  uint16_t ADCValue = Read(slaveAddress);
  uint16_t ADCData  = ADCValue >> 4;

  return ADCData;
}

void TRUv1PowerBoard::ConfigurePowerADC(bool commit)
{
  // The four registers dedicated to ADC Address setup
  uint16_t ADCAddressSetup[4] = {0x0B, 0x0C, 0x0D, 0x0E};
  for (int adcAdd = 0; adcAdd < 4; adcAdd++) {
    uint16_t byte2 = 0x0B;
    uint16_t byte3 = 0x02;
    uint16_t data  = (byte2 << 8) | byte3;
    Write(ADCAddressSetup[adcAdd], data, false);
    byte2 = 0x00;
    byte3 = 0x01;
    data  = (byte2 << 8) | byte3;
    Write(ADCAddressSetup[adcAdd], data, commit);
  }
}

std::vector<uint16_t> TRUv1PowerBoard::ReadPowerADC()
{

  std::vector<uint16_t> adcData;
  uint16_t              ADCAddress[4] = {0x10, 0x11, 0x12, 0x13};
  for (int add = 0; add < 4; add++) {
    for (int channel = 0; channel < 8; channel++) {
      uint16_t byte3 = 0x20 | channel;
      Write(ADCAddress[add], byte3, true);
      uint16_t readValue = Read(ADCAddress[add]);
      adcData.push_back(readValue >> 4);
    }
  }

  return adcData;
}

uint16_t TRUv1PowerBoard::ReadPowerADCChannel(int channel)
{

  assert((channel >= 0) && (channel < 32));

  uint16_t ADCAddress[4] = {0x10, 0x11, 0x12, 0x13};

  uint16_t channel_msb = channel >> 3;
  uint16_t channel_lsb = channel & 0x7;

  uint16_t byte3        = 0x20 | channel_lsb;
  uint16_t slaveAddress = ADCAddress[channel_msb];
  Write(slaveAddress, byte3, true);
  uint16_t ADCValue = Read(slaveAddress);
  uint16_t ADCData  = ADCValue >> 4;

  return ADCData;
}


void TRUv1PowerBoard::SetBiasVoltage(uint16_t voltageCode, bool commit)
{
  assert((voltageCode | 0xFF) == 0xFF);
  uint16_t byte2 = 0x11;
  uint16_t byte3 = voltageCode;
  uint16_t data  = (byte2 << 8) | byte3;
  Write(TRUv1PowerBoard::PotBiasAddress, data, commit);
}

void TRUv1PowerBoard::SetPowerVoltage(int channel, uint16_t voltageCode, bool commit)
{
  assert((voltageCode | 0xFF) == 0xFF);
  assert((channel < 16) && (channel >= 0));

  uint16_t potPower[4] = {0x06, 0x07, 0x08, 0x09};

  uint16_t channel_msb = channel >> 2;
  uint16_t channel_lsb = channel & 0x3;

  uint16_t byte2 = channel_lsb;
  uint16_t byte3 = voltageCode;
  uint16_t data  = (byte2 << 8) | byte3;
  Write(potPower[(int)channel_msb], data, commit);
}

void TRUv1PowerBoard::RaiseThresholdsToMax(bool commit)
{

  uint16_t threshCurrAdd[4] = {0x02, 0x03, 0x04, 0x05};
  uint16_t byte1            = 0x3F;
  uint16_t byte2            = 0xFF;
  uint16_t byte3            = 0xFF;

  Write(0, byte1, false);

  uint16_t data = (byte2 << 8) | byte3;

  for (int add = 0; add < 4; add++) {
    Write(threshCurrAdd[add], data, commit);
  }
}

void TRUv1PowerBoard::LowerThresholdsToMin(bool commit)
{

  uint16_t threshCurrAdd[4] = {0x02, 0x03, 0x04, 0x05};
  uint16_t byte1            = 0x3F;
  uint16_t byte2            = 0x00;
  uint16_t byte3            = 0x00;

  Write(0, byte1, false);

  uint16_t data = (byte2 << 8) | byte3;

  for (int add = 0; add < 4; add++) {
    Write(threshCurrAdd[add], data, commit);
  }
}

void TRUv1PowerBoard::SetThreshold(int channel, uint16_t value, bool commit)
{

  assert((channel < 16) && (channel >= 0));
  assert((value | 0xFFF) == 0xFFF);

  uint16_t threshCurrAdd[4] = {0x02, 0x03, 0x04, 0x05};

  uint16_t channel_msb = channel >> 2;
  uint16_t channel_lsb = channel & 0x3;

  uint16_t byte1 = (0x3 << 4) | channel_lsb;
  uint16_t byte2 = (value >> 4) & 0xFF;
  uint16_t byte3 = (value & 0xf) << 4;

  Write(0, byte1, false);

  uint16_t data = (byte2 << 8) | byte3;
  Write(threshCurrAdd[channel_msb], data, commit);
}

void TRUv1PowerBoard::SetThresholdWithMask(uint16_t mask, uint16_t value, bool commit)
{

  // This is equivalent to calling SetThreshold on each "high" channel bit in the mask
  assert((value | 0xFFF) == 0xFFF);
  assert((mask | 0xFFFF) == 0xFFFF);

  for (int channel = 0; channel < 16; channel++) {
    if ((mask >> channel) & 1) {
      SetThreshold(channel, value, commit);
    }
  }
}

void TRUv1PowerBoard::SetThresholdAll(uint16_t value, bool commit)
{

  assert((value | 0xFFF) == 0xFFF);

  uint16_t threshCurrAdd[4] = {0x02, 0x03, 0x04, 0x05};

  uint16_t byte1 = 0x3F;
  uint16_t byte2 = (value >> 4) & 0xFF;
  uint16_t byte3 = (value & 0xf) << 4;

  Write(0, byte1, false);

  uint16_t data = (byte2 << 8) | byte3;
  for (int add = 0; add < 4; add++) {
    Write(threshCurrAdd[add], data, commit);
  }
}

void TRUv1PowerBoard::SetPowerVoltageAll(uint16_t voltageCode, bool commit)
{

  for (int channel = 0; channel < 16; channel++) {
    SetPowerVoltage(channel, voltageCode, commit);
  }
}

void TRUv1PowerBoard::EnablePowerAll(bool commit)
{
  uint16_t byte3 = 0xFF;
  Write(TRUv1PowerBoard::IOExpanderPowerAddress_0, byte3, false);
  Write(TRUv1PowerBoard::IOExpanderPowerAddress_1, byte3, commit);
}

void TRUv1PowerBoard::EnablePower(int channel, bool commit)
{
  assert((channel < 16) && (channel >= 0));

  uint16_t expPow[2] = {TRUv1PowerBoard::IOExpanderPowerAddress_0,
                        TRUv1PowerBoard::IOExpanderPowerAddress_1};

  uint16_t channel_msb = channel >> 3;
  uint16_t channel_lsb = channel & 0x7;
  uint16_t byte3       = 0x01 << channel_lsb;
  Write(expPow[(int)channel_msb], byte3, commit);
}

void TRUv1PowerBoard::EnablePowerWithMask(uint16_t mask, bool commit)
{

  assert((mask | 0xFFFF) == 0xFFFF);
  uint16_t byte3 = mask & 0xFF;
  Write(TRUv1PowerBoard::IOExpanderPowerAddress_0, byte3, false);
  byte3 = (mask >> 8) & 0xFF;
  Write(TRUv1PowerBoard::IOExpanderPowerAddress_1, byte3, commit);
}

void TRUv1PowerBoard::DisablePowerAll(bool commit)
{

  uint16_t byte3 = 0x00;
  Write(TRUv1PowerBoard::IOExpanderPowerAddress_0, byte3, false);
  Write(TRUv1PowerBoard::IOExpanderPowerAddress_1, byte3, commit);
}


void TRUv1PowerBoard::EnableBiasAll(bool commit)
{

  uint16_t byte3 = 0x00;
  Write(TRUv1PowerBoard::IOExpanderBiasAddress, byte3, commit);
}

void TRUv1PowerBoard::EnableBiasWithMask(uint16_t mask, bool commit)
{

  assert((mask | 0xFFFF) == 0xFFFF);

  uint16_t byte3 = 0xFF ^ mask;
  Write(TRUv1PowerBoard::IOExpanderBiasAddress, byte3, commit);
}

void TRUv1PowerBoard::DisableBiasAll(bool commit)
{

  uint16_t byte3 = 0xFF;
  Write(TRUv1PowerBoard::IOExpanderBiasAddress, byte3, commit);
}

void TRUv1PowerBoard::ConfigureRTD(int sensorID, bool commit)
{

  uint16_t slaveAddress = TRUv1PowerBoard::TempThreshConfigAddress;

  uint16_t byte1 = 0x1 << (sensorID - 1);
  uint16_t byte2 = 0x80;
  uint16_t byte3 = 0xC2;
  uint16_t data  = (byte2 << 8) | byte3;

  Write(0, byte1, false);
  Write(slaveAddress, data, commit);
}

double TRUv1PowerBoard::ReadRTD(int sensorID)
{

  assert((sensorID >= 1) && (sensorID <= 3));
  uint16_t slaveAddress = TRUv1PowerBoard::TempThreshConfigAddress;

  uint16_t byte1 = 0x1 << (sensorID - 1);
  uint16_t byte2 = 0x1;
  uint16_t byte3 = 0xff;
  uint16_t data  = (byte2 << 8) | byte3;

  Write(0, byte1, false);

  Write(slaveAddress, data, true);
  uint16_t res_one = Read(slaveAddress);

  byte2 = 0x2;
  data  = (byte2 << 8) | byte3;

  Write(slaveAddress, data, true);
  uint16_t res_two = Read(slaveAddress);

  double resistanceValue = ((res_one & 0xFF) << 7) | ((res_two & 0xFF) >> 1);
  double tempValue       = (resistanceValue - 8192.) / 31.54;

  return tempValue;
}


void TRUv1PowerBoard::DumpConfig()
{
  std::cout << "....TRUV1POWERBOARD MODULE CONFIG....\n";
  for (int i = 0; i < 25; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}
