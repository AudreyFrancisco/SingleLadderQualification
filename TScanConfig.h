#ifndef TSCANCONFIG_H
#define TSCANCONFIG_H

class TScanConfig {
 private: 
  int m_chargeStart;
  int m_chargeStop;
  int m_chargeStep;
  int m_nMaskStages;
 protected: 
 public:
  TScanConfig ();
  ~TScanConfig() {};
  int GetChargeStart () {return m_chargeStart;};
  int GetChargeStep  () {return m_chargeStep;};
  int GetChargeStop  () {return m_chargeStop;};
  int GetNMaskStages () {return m_nMaskStages;};
};



#endif
