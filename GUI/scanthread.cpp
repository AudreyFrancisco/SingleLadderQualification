#include "scanthread.h"
#include <iostream>
#include <QDebug>
#include <QtCore>
ScanThread::ScanThread()
{

}
void ScanThread::process(){

  std::cout<<"Iam sad";
  emit resultReady();
  //for (int i=0; i=10000; i++)
  //{

  // std::cout<<"drg"<<std::endl;
  // }    //emit resultReady();
}
