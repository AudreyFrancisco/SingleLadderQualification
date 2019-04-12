#include "ScanData.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QCoreApplication>
#include <iomanip>
#include <iostream>

struct CommonTools::TScan general;

float CommonTools::add(float a, float b, int n = 1) { return a + b; }

ScanData::ScanData()
{

  CommonTools::TVariable numWorkingChips;
  numWorkingChips.displayName = "Number of Working Chips";
  numWorkingChips.hicTestName = "Number of Working Chips";
  numWorkingChips.sum         = &CommonTools::add;

  CreateGeneral();
}

ScanData::~ScanData() {}


void ScanData::CreateGeneral()
{

  general.type = STPower;
  CreateGeneralVariables(&general);
}

void ScanData::CreateGeneralVariables(CommonTools::TScan *gen)
{

  CommonTools::TVariable myvar;
  gen->variables.push_back(myvar);
}
