#ifndef SCANDATA_H
#define SCANDATA_H
#include <iostream>
#include <string>

class ScanData {


public:
  ScanData();
  ~ScanData();


  struct TVariable {
    std::string displayName;
    std::string hicTestName;
    std::string chipTestName;
    float (ScanData::*sum)(float, float, int);
  };

  float add(float, float, int);


private:
};

#endif
