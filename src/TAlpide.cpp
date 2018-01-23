#include "TAlpide.h"
#include "TReadoutBoard.h"

#include <iostream>
#include <sstream>

using namespace Alpide;

TAlpide::TAlpide(TChipConfig *config)
    : fConfig(config), fChipId(config->GetChipId()), fReadoutBoard(0x0), fADCOffset(-1),
      fADCHalfLSB(false), fADCSign(false) {}

void TAlpide::SetEnable(bool Enable) {
  fReadoutBoard->SetChipEnable(this, Enable);
  fConfig->SetEnable(Enable);
}

int TAlpide::ReadRegister(TRegister address, uint16_t &value) {
  return ReadRegister((uint16_t)address, value);
}

int TAlpide::ReadRegister(uint16_t address, uint16_t &value) {
  int err = fReadoutBoard->ReadChipRegister(address, value, this);
  if (err < 0)
    return err; // readout board should have thrown an exception before

  return err;
}

int TAlpide::WriteRegister(TRegister address, uint16_t value, bool verify) {
  return WriteRegister((uint16_t)address, value, verify);
}

int TAlpide::WriteRegister(uint16_t address, uint16_t value, bool verify) {
  int result = fReadoutBoard->WriteChipRegister(address, value, this);
  if ((!verify) || (result < 0))
    return result;

  uint16_t check;
  result = ReadRegister(address, check);
  if (result < 0)
    return result;
  if (check != value)
    return -1; // raise exception (warning) readback != write value;
  return 0;
}

int TAlpide::ModifyRegisterBits(TRegister address, uint8_t lowBit, uint8_t nBits, uint16_t value,
                                bool verify) {
  if ((lowBit < 0) || (lowBit > 15) || (lowBit + nBits > 15)) {
    return -1; // raise exception illegal limits
  }
  uint16_t registerValue, mask = 0xffff;
  ReadRegister(address, registerValue);
  // std::cout << "Value before modify:0x " << std::hex << registerValue << std::dec << std::endl;
  for (int i = lowBit; i < lowBit + nBits; i++) {
    mask -= (1 << i);
  }

  registerValue &= mask;            // set all bits that are to be overwritten to 0
  value &= (1 << nBits) - 1;        // make sure value fits into nBits
  registerValue |= value << lowBit; // or value into the foreseen spot

  return WriteRegister(address, registerValue, verify);
}

void TAlpide::DumpConfig(const char *fName, bool writeFile, char *config) {
  uint16_t value;

  if (writeFile) {
    FILE *fp = fopen(fName, "w");
    // DACs
    ReadRegister(0x601, value);
    fprintf(fp, "VRESETP %i\n", value);
    ReadRegister(0x602, value);
    fprintf(fp, "VRESETD %i\n", value);
    ReadRegister(0x603, value);
    fprintf(fp, "VCASP   %i\n", value);
    ReadRegister(0x604, value);
    fprintf(fp, "VCASN   %i\n", value);
    ReadRegister(0x605, value);
    fprintf(fp, "VPULSEH %i\n", value);
    ReadRegister(0x606, value);
    fprintf(fp, "VPULSEL %i\n", value);
    ReadRegister(0x607, value);
    fprintf(fp, "VCASN2  %i\n", value);
    ReadRegister(0x608, value);
    fprintf(fp, "VCLIP   %i\n", value);
    ReadRegister(0x609, value);
    fprintf(fp, "VTEMP   %i\n", value);
    ReadRegister(0x60a, value);
    fprintf(fp, "IAUX2   %i\n", value);
    ReadRegister(0x60b, value);
    fprintf(fp, "IRESET  %i\n", value);
    ReadRegister(0x60c, value);
    fprintf(fp, "IDB     %i\n", value);
    ReadRegister(0x60d, value);
    fprintf(fp, "IBIAS   %i\n", value);
    ReadRegister(0x60e, value);
    fprintf(fp, "ITHR    %i\n", value);

    fprintf(fp, "\n");
    // Mode control register
    ReadRegister(0x1, value);
    fprintf(fp, "MODECONTROL  %i\n", value);

    // FROMU config reg 1: [5]: test pulse mode; [6]: enable test strobe, etc.
    ReadRegister(0x4, value);
    fprintf(fp, "FROMU_CONFIG1  %i\n", value);

    // FROMU config reg 2: strobe duration
    ReadRegister(0x5, value);
    fprintf(fp, "FROMU_CONFIG2  %i\n", value);

    // FROMU pulsing reg 1: delay between pulse and strobe if the feature of automatic strobing is
    // enabled
    ReadRegister(0x7, value);
    fprintf(fp, "FROMU_PULSING1  %i\n", value);

    // FROMU pulsing reg 2: pulse duration
    ReadRegister(0x8, value);
    fprintf(fp, "FROMU_PULSING2  %i\n", value);

    // CMU DMU config reg
    ReadRegister(0x10, value);
    fprintf(fp, "CMUDMU_CONFIG  %i\n", value);

    fclose(fp);
  }

  config[0] = '\0';
  // DACs
  ReadRegister(0x601, value);
  sprintf(config, "VRESETP %i\n", value);
  ReadRegister(0x602, value);
  sprintf(config, "%sVRESETD %i\n", config, value);
  ReadRegister(0x603, value);
  sprintf(config, "%sVCASP   %i\n", config, value);
  ReadRegister(0x604, value);
  sprintf(config, "%sVCASN   %i\n", config, value);
  ReadRegister(0x605, value);
  sprintf(config, "%sVPULSEH %i\n", config, value);
  ReadRegister(0x606, value);
  sprintf(config, "%sVPULSEL %i\n", config, value);
  ReadRegister(0x607, value);
  sprintf(config, "%sVCASN2  %i\n", config, value);
  ReadRegister(0x608, value);
  sprintf(config, "%sVCLIP   %i\n", config, value);
  ReadRegister(0x609, value);
  sprintf(config, "%sVTEMP   %i\n", config, value);
  ReadRegister(0x60a, value);
  sprintf(config, "%sIAUX2   %i\n", config, value);
  ReadRegister(0x60b, value);
  sprintf(config, "%sIRESET  %i\n", config, value);
  ReadRegister(0x60c, value);
  sprintf(config, "%sIDB     %i\n", config, value);
  ReadRegister(0x60d, value);
  sprintf(config, "%sIBIAS   %i\n", config, value);
  ReadRegister(0x60e, value);
  sprintf(config, "%sITHR    %i\n", config, value);

  sprintf(config, "%s\n", config);
  // Mode control register
  ReadRegister(0x1, value);
  sprintf(config, "%sMODECONTROL  %i\n", config, value);

  // FROMU config reg 1: [5]: test pulse mode; [6]: enable test strobe, etc.
  ReadRegister(0x4, value);
  sprintf(config, "%sFROMU_CONFIG1  %i\n", config, value);

  // FROMU config reg 2: strobe duration
  ReadRegister(0x5, value);
  sprintf(config, "%sFROMU_CONFIG2  %i\n", config, value);

  // FROMU pulsing reg 1: delay between pulse and strobe if the feature of automatic strobing is
  // enabled
  ReadRegister(0x7, value);
  sprintf(config, "%sFROMU_PULSING1  %i\n", config, value);

  // FROMU pulsing reg 2: pulse duration
  ReadRegister(0x8, value);
  sprintf(config, "%sFROMU_PULSING2  %i\n", config, value);

  // CMU DMU config reg
  ReadRegister(0x10, value);
  sprintf(config, "%sCMUDMU_CONFIG  %i\n", config, value);
}

std::string TAlpide::DumpRegisters() {
  std::stringstream dump;

  int chipId = fConfig->GetChipId();

  // Periphery config
  dump << "# Chip ID\tAddress\tValue" << std::endl;
  for (unsigned int reg = (unsigned int)Alpide::REG_MODECONTROL;
       reg <= (unsigned int)Alpide::REG_BUSY_MINWIDTH; ++reg) {
    uint16_t value = 0xDEAD;
    this->ReadRegister((Alpide::TRegister)reg, value);
    dump << chipId << "\t0x" << std::hex << reg << "\t0x" << value << std::dec << std::endl;
  }

  // Pixel config: read-only do nothing

  // DAC and monitoring
  for (unsigned int reg = (unsigned int)Alpide::REG_ANALOGMON;
       reg <= (unsigned int)Alpide::REG_ADC_T2V; ++reg) {
    uint16_t value = 0xDEAD;
    this->ReadRegister((Alpide::TRegister)reg, value);
    dump << chipId << "\t0x" << std::hex << reg << "\t0x" << value << std::dec << std::endl;
  }

  // Test and debug control
  for (unsigned int reg = (unsigned int)Alpide::REG_SEU_ERROR_COUNT;
       reg <= (unsigned int)Alpide::REG_ADC_DEBUG; ++reg) {
    uint16_t value = 0xDEAD;
    this->ReadRegister((Alpide::TRegister)reg, value);
    dump << chipId << "\t0x" << std::hex << reg << "\t0x" << value << std::dec << std::endl;
  }

  return dump.str();
}

// ---------- DAC / ADC section -----------------------

/* ------------------------------------------------------
 * Set the ADC Control Register
 *
 * Parameter  : Mode of ADC measurement [0:Manual 1:Calibrate 2:Auto 3:SupoerManual]
 *              SelectInput the source specification [0:AVSS 1:DVSS 2:AVDD 3:DVDD
 *                                                    4:VBGthVolScal 5:DACMONV 6:DACMONI
 *                                                    7:Bandgap 8:Temperature]
 *              ComparatorCurrent  [0:180uA 1:190uA 2:296uA 3:410uA]
 *              RampSpeed          [0:500ms 1:1us 2:2us 3:4us]
 *
 */
uint16_t TAlpide::SetTheADCCtrlRegister(Alpide::TADCMode Mode, Alpide::TADCInput SelectInput,
                                        Alpide::TADCComparator ComparatorCurrent,
                                        Alpide::TADCRampSpeed RampSpeed) {
  uint16_t Data;
  Data = Mode | (SelectInput << 2) | (ComparatorCurrent << 6) | (fADCSign << 8) | (RampSpeed << 9) |
         (fADCHalfLSB << 11);
  WriteRegister(Alpide::REG_ADC_CONTROL, Data);
  return (Data);
}

/* ------------------------------------------------------
 * Sets the DAC Monitor multiplexer
 *
 * Parameter  : the Index of the DAC register.
 *              the IRef value
 *
 * Note  : Iref =  0:0.25ua 1:0.75uA 2:1.00uA 3:1.25uA
 *
 */
void TAlpide::SetTheDacMonitor(Alpide::TRegister ADac, Alpide::TDACMonIref IRef) {
  int VDAC, IDAC;
  uint16_t Value;
  switch (ADac) {
  case Alpide::REG_VRESETP:
    VDAC = 4;
    IDAC = 0;
    break;
  case Alpide::REG_VRESETD:
    VDAC = 5;
    IDAC = 0;
    break;
  case Alpide::REG_VCASP:
    VDAC = 1;
    IDAC = 0;
    break;
  case Alpide::REG_VCASN:
    VDAC = 0;
    IDAC = 0;
    break;
  case Alpide::REG_VPULSEH:
    VDAC = 2;
    IDAC = 0;
    break;
  case Alpide::REG_VPULSEL:
    VDAC = 3;
    IDAC = 0;
    break;
  case Alpide::REG_VCASN2:
    VDAC = 6;
    IDAC = 0;
    break;
  case Alpide::REG_VCLIP:
    VDAC = 7;
    IDAC = 0;
    break;
  case Alpide::REG_VTEMP:
    VDAC = 8;
    IDAC = 0;
    break;
  case Alpide::REG_IAUX2:
    IDAC = 1;
    VDAC = 0;
    break;
  case Alpide::REG_IRESET:
    IDAC = 0;
    VDAC = 0;
    break;
  case Alpide::REG_IDB:
    IDAC = 3;
    VDAC = 0;
    break;
  case Alpide::REG_IBIAS:
    IDAC = 2;
    VDAC = 0;
    break;
  case Alpide::REG_ITHR:
    IDAC = 5;
    VDAC = 0;
    break;
  default:
    VDAC = 0;
    IDAC = 0;
    break;
  }

  Value = VDAC & 0xf;
  Value |= (IDAC & 0x7) << 4;
  Value |= (IRef & 0x3) << 9;

  WriteRegister(Alpide::REG_ANALOGMON, Value);
  return;
}

/* ------------------------------------------------------
 * Calibrate the internal ADC
 *
 * Returns  : the value of the calculated Bias.
 *
 * Note  : the calibration parameter are stored into the
 *         devoted class members.
 *
 */
int TAlpide::CalibrateADC() {
  uint16_t theVal2, theVal1;

  // Calibration Phase 1
  fADCHalfLSB = false;
  fADCSign = false;
  SetTheADCCtrlRegister(Alpide::MODE_CALIBRATE, Alpide::INP_AVSS, Alpide::COMP_296uA,
                        Alpide::RAMP_1us);
  fReadoutBoard->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
  usleep(4000); // > of 5 milli sec
  ReadRegister(Alpide::REG_ADC_CALIB, theVal1);
  fADCSign = true;
  SetTheADCCtrlRegister(Alpide::MODE_CALIBRATE, Alpide::INP_AVSS, Alpide::COMP_296uA,
                        Alpide::RAMP_1us);
  fReadoutBoard->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
  usleep(4000); // > of 5 milli sec
  ReadRegister(Alpide::REG_ADC_CALIB, theVal2);
  fADCSign = (theVal1 > theVal2) ? false : true;

  // Calibration Phase 2
  fADCHalfLSB = false;
  SetTheADCCtrlRegister(Alpide::MODE_CALIBRATE, Alpide::INP_AVSS, Alpide::COMP_296uA,
                        Alpide::RAMP_1us);
  fReadoutBoard->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
  usleep(4000); // > of 5 milli sec
  ReadRegister(Alpide::REG_ADC_CALIB, theVal1);
  fADCHalfLSB = true;
  SetTheADCCtrlRegister(Alpide::MODE_CALIBRATE, Alpide::INP_AVSS, Alpide::COMP_296uA,
                        Alpide::RAMP_1us);
  fReadoutBoard->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
  usleep(4000); // > of 5 milli sec
  ReadRegister(Alpide::REG_ADC_CALIB, theVal2);
  fADCHalfLSB = (theVal1 > theVal2) ? false : true;

  // Offset Measurement
  fADCOffset = 0;
  unsigned int n_samples = 20;
  for (unsigned int i = 0; i < n_samples; ++i) {
    SetTheADCCtrlRegister(Alpide::MODE_CALIBRATE, Alpide::INP_AVSS, Alpide::COMP_296uA,
                          Alpide::RAMP_1us);
    fReadoutBoard->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
    usleep(4000); // > of 5 milli sec
    ReadRegister(Alpide::REG_ADC_CALIB, theVal1);
    fADCOffset += theVal1;
  }
  fADCOffset =
      static_cast<int>(static_cast<double>(fADCOffset) / static_cast<double>(n_samples) + 0.5);
  return (fADCOffset);
}

/* ------------------------------------------------------
 * Reads the temperature sensor by means of internal ADC
 *
 * Returns  : the value in Celsius degree.
 *
 * Note  : if this was the first measure after the chip
 *         configuration phase, a calibration will be
 *         automatically executed.
 *
 */
float TAlpide::ReadTemperature() {

  uint16_t theResult = 0;
  float theValue;
  if (fADCOffset == -1) { // needs calibration
    CalibrateADC();
  }

  SetTheDacMonitor(
      Alpide::REG_ANALOGMON); // uses the RE_ANALOGMON, in order to disable the monitoring !
  usleep(5000);
  SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_Temperature, Alpide::COMP_296uA,
                        Alpide::RAMP_1us);
  fReadoutBoard->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
  usleep(5000); // Wait for the measurement > of 5 milli sec
  ReadRegister(Alpide::REG_ADC_AVSS, theResult);
  theResult -= (uint16_t)fADCOffset;
  theValue = (((float)theResult) * 0.1281) + 6.8; // first approximation
  return theValue;
}

/* ------------------------------------------------------
 * Reads the analogue supply voltage by means of internal ADC
 *
 * Returns  : the value in Volts.
 *
 * Note  : if this was the first measure after the chip
 *         configuration phase, a calibration will be
 *         automatically executed.
 *
 */
float TAlpide::ReadAnalogueVoltage() {

  if (fADCOffset == -1) { // needs calibration
    CalibrateADC();
  }

  float AVDD_direct = 0.;
  float AVDD_VTEMP = 0.;

  bool AVDD_saturated = false;

  uint16_t theResult = 0;
  const unsigned int nSamples = 30;

  // AVDD direct
  for (unsigned int repetition = 0; repetition < nSamples; ++repetition) {
    SetTheDacMonitor(Alpide::REG_ANALOGMON);
    SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_AVDD, Alpide::COMP_296uA,
                          Alpide::RAMP_1us);
    usleep(5000);
    GetReadoutBoard()->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
    usleep(5000);
    ReadRegister(Alpide::REG_ADC_AVSS, theResult);
    if (theResult == 1055)
      AVDD_saturated = true;
    AVDD_direct +=
        2. * ((float)theResult - (float)(GetADCOffset())) * 0.823e-3; // first approximation
  }
  AVDD_direct /= nSamples;

  // AVDD via VTEMP @ 200
  WriteRegister(Alpide::REG_VTEMP, 200);
  usleep(5000);

  for (unsigned int repetition = 0; repetition < nSamples; ++repetition) {
    // calculated AVDD based on VTEMP
    AVDD_VTEMP += ReadDACVoltage(Alpide::REG_VTEMP) / 0.772 + 0.023;
  }
  AVDD_VTEMP /= nSamples;

  if (AVDD_direct < 1.7 && !AVDD_saturated)
    return AVDD_direct;
  else
    return AVDD_VTEMP;
}

/* ------------------------------------------------------
 * Reads the output voltage of one DAC by means of internal ADC
 *
 * Parameter : the Index that define the DAC register
 *
 * Returns  : the value in Volts.
 *
 * Note  : if this was the first measure after the chip
 *         configuration phase, a calibration will be
 *         automatically executed.
 *
 *   13/6/17 - Returns negative values (A.Franco)
 *
 */
float TAlpide::ReadDACVoltage(Alpide::TRegister ADac) {

  uint16_t theRowValue = 0;
  int theResult = 0;
  float theValue = 0.0;
  if (fADCOffset == -1) { // needs calibration
    CalibrateADC();
  }

  SetTheDacMonitor(ADac);
  usleep(5000);
  SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_DACMONV, Alpide::COMP_296uA,
                        Alpide::RAMP_1us);
  fReadoutBoard->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
  usleep(5000); // Wait for the measurement > of 5 milli sec
  ReadRegister(Alpide::REG_ADC_AVSS, theRowValue);
  theResult = ((int)theRowValue) - fADCOffset;
  theValue = (((float)theResult) * 0.001644); // V scale first approximation
  return (theValue);
}

/* ------------------------------------------------------
 * Reads the output current of one DAC by means of internal ADC
 *
 * Parameter : the Index that define the DAC register
 *
 * Returns  : the value in Micro Ampere.
 *
 * Note  : if this was the first measure after the chip
 *         configuration phase, a calibration will be
 *         automatically executed.
 *
 *   13/6/17 - Returns negative values (A.Franco)
 *
 */
float TAlpide::ReadDACCurrent(Alpide::TRegister ADac) {

  uint16_t theRowValue = 0;
  int theResult = 0;
  float theValue = 0.0;
  if (fADCOffset == -1) { // needs calibration
    CalibrateADC();
  }

  SetTheDacMonitor(ADac);
  usleep(5000);
  SetTheADCCtrlRegister(Alpide::MODE_MANUAL, Alpide::INP_DACMONI, Alpide::COMP_296uA,
                        Alpide::RAMP_1us);

  fReadoutBoard->SendCommand(Alpide::COMMAND_ADCMEASURE, this);
  usleep(5000); // Wait for the measurement > of 5 milli sec
  ReadRegister(Alpide::REG_ADC_AVSS, theRowValue);
  theResult = ((int)theRowValue) - fADCOffset;
  theValue = (((float)theResult) * 0.164); // uA scale   first approximation
  return (theValue);
}
