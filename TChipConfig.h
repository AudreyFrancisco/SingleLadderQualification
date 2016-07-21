#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H


class TChipConfig {
 private: 
  int fChipId;
 protected:
 public:
  TChipConfig   (const char *fName, int chipId);
  int GetChipId () {return fChipId;};
};


#endif   /* CHIPCONFIG_H */
