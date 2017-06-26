#include "testselection.h"
#include "ui_testselection.h"
#include "mainwindow.h"
#include <iostream>
#include <QtCore/QCoreApplication>
#include <iomanip>
#include "TFifoTest.h"
TestSelection::TestSelection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestSelection)
{
    ui->setupUi(this);

    //connect(ui->settings, SIGNAL(clicked()), this->parent(), SLOT(open()));

    connect(ui->typeoftest,SIGNAL(currentIndexChanged(int)),this->parent(),SLOT(connectcombo(int)));



}

TestSelection::~TestSelection()
{
    delete ui;

}


void TestSelection::SaveSettings(QString &opname, int &hicid, int &counter){
    if (ui->operatorstring->toPlainText().isEmpty() || ui->id->toPlainText().isEmpty())
    {
        qDebug()<<"Put your details little shit"<<endl;
        popupmessage("Info missing");
        counter=0;
    }
    else{
        opname = ui->operatorstring->toPlainText();
        hicid=ui->id->toPlainText().toInt();
        counter=1;
        qDebug()<<"The operator name is:"<<opname<<"and the id is: "<<hicid<<endl;
    }
}


void TestSelection::popupmessage(QString m){
  missingsettings=new Dialog(this);
  //need to check this Attribute
 // missingsettings->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlags((windowFlags() & Qt::WindowStaysOnTopHint));
  this->setWindowFlags((windowFlags() & ~Qt::WindowStaysOnTopHint));
  missingsettings->append(m);
  missingsettings->show();

 // missingsettings->activateWindow();
  //missingsettings->raise();
}


