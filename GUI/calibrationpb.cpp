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


void Calibrationpb::getcalibration(float savdd,float iavdd, float sdvdd,float idvdd,float offsetia,float offsetid){

ui->savdd->setText(QString::number(savdd));
ui->iavdd->setText(QString::number(iavdd));
ui->sdvdd->setText(QString::number(sdvdd));
ui->idvdd->setText(QString::number(idvdd));
ui->offsetia->setText(QString::number(offsetia));
ui->offsetid->setText(QString::number(offsetid));
std::cout<<"calibration done"<<std::endl;


}


void Calibrationpb::setpowerunit(int &unit){

    if(ui->top->isChecked()){
        ui->bottom->setChecked(false);
        unit=1;
    }
     if(ui->bottom->isChecked()){

         ui->top->setChecked(false);
         unit=0;
     }

}
