#include "ScanData.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QCoreApplication>
#include <iomanip>
#include <iostream>


ScanData::ScanData()
{

  ScanData::TVariable numWorkingChips;
  numWorkingChips.displayName = "Number of Working Chips";
  numWorkingChips.hicTestName = "Number of Working Chips";
  numWorkingChips.sum         = &ScanData::add;
}

ScanData::~ScanData() {}


float ScanData::add(float a, float b, int n = 1) { return a + b; }
