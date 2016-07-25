#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H


class TChipConfig {
 private: 
  int fChipId;
 protected:
 public:
  TChipConfig   (int chipId, const char *fName = 0);
  int GetChipId () {return fChipId;};
};


#endif   /* CHIPCONFIG_H */
