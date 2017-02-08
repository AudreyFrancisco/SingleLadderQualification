#include "TAlpide.h"
#include <iostream>

using namespace Alpide;

TAlpide::TAlpide (TChipConfig *config)
  : fConfig(config)
  , fChipId(config->GetChipId())
  , fReadoutBoard(0x0)
  , fADCBias(-1)
  , fADCHalfLSB(false)
  , fADCSign(false)
{

}


void TAlpide::SetEnable (bool Enable) {
  fReadoutBoard->SetChipEnable (fChipId, Enable);
  fConfig      ->SetEnable     (Enable);
}


int TAlpide::ReadRegister (TRegister address, uint16_t &value) {
  return ReadRegister ((uint16_t) address, value);
}


int TAlpide::ReadRegister (uint16_t address, uint16_t &value) {
  int err = fReadoutBoard->ReadChipRegister(address, value, fChipId);
  if (err < 0) return err;  // readout board should have thrown an exception before

  return err;

}


int TAlpide::WriteRegister (TRegister address, uint16_t value, bool verify) {
  return WriteRegister ((uint16_t) address, value, verify);
}


int TAlpide::WriteRegister (uint16_t address, uint16_t value, bool verify) {
  int result = fReadoutBoard->WriteChipRegister(address, value, fChipId);
  if ((!verify) || (result < 0)) return result;

  uint16_t check;
  result = ReadRegister (address, check);
  if (result < 0) return result;
  if (check != value) return -1;      // raise exception (warning) readback != write value;
  return 0;  
}


int TAlpide::ModifyRegisterBits (TRegister address, uint8_t lowBit, uint8_t nBits, uint16_t value, bool verify) {
  if ((lowBit < 0) || (lowBit > 15) || (lowBit + nBits > 15)) {
    return -1;    // raise exception illegal limits
  }
  uint16_t registerValue, mask = 0xffff; 
  ReadRegister(address, registerValue); 
  
  for (int i = lowBit; i < lowBit + nBits; i++) {
    mask -= 1 << i;
  }

  registerValue &= mask;                  // set all bits that are to be overwritten to 0
  value         &= (1 << nBits) -1;       // make sure value fits into nBits
  registerValue |= value << nBits;        // or value into the foreseen spot

  return WriteRegister (address, registerValue, verify);

}


void TAlpide::DumpConfig (const char *fName, bool writeFile, char *config) {
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

    // FROMU pulsing reg 1: delay between pulse and strobe if the feature of automatic strobing is enabled
    ReadRegister(0x7, value);
    fprintf(fp, "FROMU_PULSING1  %i\n", value);

    // FROMU pulsing reg 2: pulse duration
    ReadRegister(0x8, value);
    fprintf(fp, "FROMU_PULSING2  %i\n", value);

    // CMU DMU config reg
    ReadRegister(0x10, value);
    fprintf(fp, "CMUDMU_CONFIG  %i\n", value);

    fclose (fp);
  }

  sprintf(config, "");
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

  // FROMU pulsing reg 1: delay between pulse and strobe if the feature of automatic strobing is enabled
  ReadRegister(0x7, value);
  sprintf(config, "%sFROMU_PULSING1  %i\n", config, value);

  // FROMU pulsing reg 2: pulse duration
  ReadRegister(0x8, value);
  sprintf(config, "%sFROMU_PULSING2  %i\n", config, value);

  // CMU DMU config reg
  ReadRegister(0x10, value);
  sprintf(config, "%sCMUDMU_CONFIG  %i\n", config, value);

}
