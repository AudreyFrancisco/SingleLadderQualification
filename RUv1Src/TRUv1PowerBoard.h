#ifndef TRUV1POWERBOARD_H
#define TRUV1POWERBOARD_H

#include <cstdint>
#include <iostream>
#include <vector>

#include "TRUv1WishboneModule.h"

class TRUv1PowerBoard : public TRUv1WishboneModule {
public:
  static const uint16_t InternalRegister         = 0x0;
  static const uint16_t TempThreshConfigAddress  = 0x1;
  static const uint16_t ThresCurrAddress_0       = 0x2;
  static const uint16_t ThresCurrAddress_1       = 0x3;
  static const uint16_t ThresCurrAddress_2       = 0x4;
  static const uint16_t ThresCurrAddress_3       = 0x5;
  static const uint16_t PotPowerAddress_0        = 0x6;
  static const uint16_t PotPowerAddress_1        = 0x7;
  static const uint16_t PotPowerAddress_2        = 0x8;
  static const uint16_t PotPowerAddress_3        = 0x9;
  static const uint16_t PotBiasAddress           = 0xA;
  static const uint16_t ADCAddressSetup_0        = 0xB;
  static const uint16_t ADCAddressSetup_1        = 0xC;
  static const uint16_t ADCAddressSetup_2        = 0xD;
  static const uint16_t ADCAddressSetup_3        = 0xE;
  static const uint16_t ADCBiasAddressSetup      = 0xF;
  static const uint16_t ADCAddress_0             = 0x10;
  static const uint16_t ADCAddress_1             = 0x11;
  static const uint16_t ADCAddress_2             = 0x12;
  static const uint16_t ADCAddress_3             = 0x13;
  static const uint16_t ADCBiasAddress           = 0x14;
  static const uint16_t TempThreshRdAddress      = 0x15;
  static const uint16_t IOExpanderBiasAddress    = 0x16;
  static const uint16_t IOExpanderPowerAddress_0 = 0x20;
  static const uint16_t IOExpanderPowerAddress_1 = 0x21;

  TRUv1PowerBoard(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);

  void                  ConfigureBiasADC(bool commit = true);
  std::vector<uint16_t> ReadBiasADC();
  uint16_t              ReadBiasADCChannel(int channel);
  void                  ConfigurePowerADC(bool commit = true);
  std::vector<uint16_t> ReadPowerADC();
  uint16_t              ReadPowerADCChannel(int channel);
  void                  RaiseThresholdsToMax(bool commit = true);
  void                  LowerThresholdsToMin(bool commit = true);
  void                  SetThreshold(int channel, uint16_t value, bool commit = true);
  void                  SetThresholdWithMask(uint16_t mask, uint16_t value, bool commit = true);
  void                  SetThresholdAll(uint16_t value, bool commit = true);
  void                  SetBiasVoltage(uint16_t voltageCode, bool commit = true);
  void                  SetPowerVoltage(int channel, uint16_t voltageCode, bool commit);
  void                  SetPowerVoltageAll(uint16_t voltageCode, bool commit = true);
  void                  EnablePowerAll(bool commit = true);
  void                  EnablePower(int channel, bool commit = true);
  void                  EnablePowerWithMask(uint16_t mask, bool commit = true);
  void                  DisablePowerAll(bool commit = true);
  void                  EnableBiasAll(bool commit = true);
  void                  EnableBiasWithMask(uint16_t mask, bool commit = true);
  void                  DisableBiasAll(bool commit = true);
  void                  ConfigureRTD(int sensorID, bool commit = true);
  double                ReadRTD(int sensorID);
  void                  DumpConfig();
};

#endif // TRUV1POWERBOARD_H
