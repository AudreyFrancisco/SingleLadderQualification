#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H





class TBoardConfig {
 private:
  bool fTriggerEnable;
  bool fPulseEnable;
  int  fNTriggers;
  int  fTriggerDelay;
  int  fPulseDelay;
  int  fTriggerSource;


 protected:

 public:
  TBoardConfig(const char *fName = 0, int boardIndex = 0) {};
  
  virtual int GetBoardType() = 0;


};


#endif   /* BOARDCONFIG_H */
