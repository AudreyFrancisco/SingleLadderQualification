#ifndef SCANDATA_H
#define SCANDATA_H
#include "TScanFactory.h"
#include <iostream>
#include <string>
#include <vector>

namespace CommonTools {

  float add(float, float, int);

  struct TVariable {
    std::string displayName;
    std::string hicTestName;
    std::string chipTestName;
    float (*sum)(float, float, int);
  };

  struct TScan {
    TScanType              type;
    std::vector<TVariable> variables;
  };


  extern struct TScan general;

} // namespace CommonTools

class ScanData {

public:
  ScanData();
  ~ScanData();

  void CreateGeneral();

  void CreateGeneralVariables(CommonTools::TScan *gen);


private:
};


#endif
