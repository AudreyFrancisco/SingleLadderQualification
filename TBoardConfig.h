#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

class TBoardConfig {
 private:
 protected:
 public:
  TBoardConfig(const char *fName = 0, int boardIndex = 0) {};
  virtual int GetBoardType() = 0;
};


#endif   /* BOARDCONFIG_H */
