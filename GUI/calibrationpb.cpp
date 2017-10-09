#include "calibrationpb.h"
#include "ui_calibrationpb.h"
#include "mainwindow.h"
#include <QtCore/QCoreApplication>
#include <QtCore>
#include <iomanip>

Calibrationpb::Calibrationpb(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Calibrationpb)
{
    ui->setupUi(this);
     connect(ui->cancel,SIGNAL(clicked()),this,SLOT(close()));
     connect(ui->ok,SIGNAL(clicked()),this->parent(),SLOT(writecalibrationfile()));
      connect(ui->calibrate,SIGNAL(clicked()),this->parent(),SLOT(setandgetcalibration()));
}

Calibrationpb::~Calibrationpb()
{
    delete ui;
}


void Calibrationpb::setresistances(int &analog,int &digital, int &bb){
    std::cout<<"seting resistances"<<std::endl;
    if (!ui->acal->toPlainText().isEmpty()){
       analog=ui->acal->toPlainText().toInt();}
    if (!ui->dcal->toPlainText().isEmpty()){
       digital=ui->dcal->toPlainText().toInt();}
    if (!ui->bbcal->toPlainText().isEmpty()){
       bb=ui->bbcal->toPlainText().toInt();}

}


void Calibrationpb::getcalibration(float calv,float cali, float lineres){

ui->Vcalibration->setText(QString::number(calv));
ui->Icalibration->setText(QString::number(cali));
ui->Lineresistances->setText(QString::number(lineres));

std::cout<<"calibration done"<<std::endl;


}
