#ifndef THIC_CONFIG_H
#define THIC_CONFIG_H

#include <map>
#include <string>
#include <vector>

class TConfig;

class THicConfig {
private:
  int fDisableSource;

protected:
  std::map<std::string, int *> fSettings;
  TConfig *                    fConfig;
  int                          fModId;
  int                          fEnabled;

public:
  THicConfig(TConfig *config, int modId);
  virtual void InitParamMap();
  bool         SetParamValue(std::string Name, std::string Value);
  bool         SetParamValue(std::string Name, int Value);
  int          GetParamValue(std::string Name);
  bool         IsParameter(std::string Name) { return (fSettings.count(Name) > 0); };
  int          GetModId() { return fModId; };
  bool         IsEnabled() { return (fEnabled != 0); };
  void         SetEnable(bool Enabled) { fEnabled = Enabled ? 1 : 0; };
  int          GetDisableSource() { return fDisableSource; };
  void         SetDisableSource(int disableSource) { fDisableSource = disableSource; };
};

class THicConfigOB : public THicConfig {
private:
  int fEnabledA8;
  int fEnabledB0;
  int fHSPosById;
  int fPoweredCombo;

protected:
public:
  THicConfigOB(TConfig *config, int modId);
  void InitParamMap();
  bool IsEnabledA8() { return (IsEnabled() && fEnabledA8); };
  bool IsEnabledB0() { return (IsEnabled() && fEnabledB0); };
};

class THicConfigIB : public THicConfig {
};

#endif
