#include "scanthread.h"
#include <QDebug>
#include <QtCore>
#include <iostream>
ScanThread::ScanThread() {}
void ScanThread::process() {

  std::cout << "Iam sad";
  emit resultReady();
  // for (int i=0; i=10000; i++)
  //{

  // std::cout<<"drg"<<std::endl;
  // }    //emit resultReady();
}
