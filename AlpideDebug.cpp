#include "AlpideDebug.h"
#include "TAlpide.h"


bool AlpideDebug::ReadStream (TAlpide *chip, TRegister reg, uint16_t *stream, int len, uint16_t Header) {
  for (int i = 0; i < len; i++) {
    chip->ReadRegister(reg, stream[i]);
  }
  if (stream[0] != Header) return false;
  return true;  
}


bool AlpideDebug::GetBMUDebugStream (TAlpide *chip, TBMUDebugStream &stream) {
  uint16_t streamData[2];

  if (! ReadStream (chip, REG_BMU_DEBUG, streamData, 2, 0xDEBB)) return false;

  return true;
}


bool AlpideDebug::GetDMUDebugStream (TAlpide *chip, TDMUDebugStream &stream) {
  uint16_t streamData[4];

  if (! ReadStream (chip, REG_DMU_DEBUG, streamData, 4, 0xDEBD)) return false;

  return true;
}


bool AlpideDebug::GetTRUDebugStream (TAlpide *chip, TTRUDebugStream &stream) {
  uint16_t streamData[5];

  if (! ReadStream (chip, REG_TRU_DEBUG, streamData, 5, 0xDEB7)) return false;

  return true;
}


bool AlpideDebug::GetRRUDebugSteam (TAlpide *chip, TRRUDebugStream &stream) {
  uint16_t streamData[65];

  if (! ReadStream (chip, REG_RRU_DEBUG, streamData, 65, 0xDEB8)) return false;

  return true;
}


bool AlpideDebug::GetFromuDebugStream (TAlpide *chip, TFromuDebugStream &stream) {
  uint16_t streamData[8];

  if (! ReadStream (chip, REG_FROMU_DEBUG, streamData, 8, 0xDEBF)) return false;

  return true;
}


bool AlpideDebug::GetADCDebugStream (TAlpide *chip, TADCDebugStream &stream) {
  uint16_t streamData[4];

  if (! ReadStream (chip, REG_BMU_DEBUG, streamData, 4, 0xDEBA)) return false;

  return true;
}
