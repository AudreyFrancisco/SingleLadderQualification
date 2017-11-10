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
  //  ui->settings->hide();
  ui->t2->hide();
  ui->t3->hide();
  ui->t4->hide();
  ui->t5->hide();
  ui->d1->hide();
  ui->d2->hide();
  ui->d3->hide();
  ui->d4->hide();
  ui->d5->hide();

  connect(ui->settings, SIGNAL(clicked()), this->parent(), SLOT(savesettings()));

  connect(ui->typeoftest,SIGNAL(currentIndexChanged(int)),this->parent(),SLOT(connectcombo(int)));
  connect(ui->close,SIGNAL(clicked()),this,SLOT(close()));
  connect(ui->databaselocation,SIGNAL(currentIndexChanged(int)),this->parent(),SLOT(connectlocationcombo(int)));
  connect(ui->databaselocation,SIGNAL(currentIndexChanged(int)),this,SLOT(getlocationcombo(int)));

}

TestSelection::~TestSelection()
{
  delete ui;

}


void TestSelection::SaveSettings(QString &opname, QString &hicid, int &counter, int &lid, int &memberid, QString &ttwo, QString &tthree, QString &tfour, QString &tfive, QString &done, QString &dtwo, QString dthree, QString dfour, QString dfive){
  if (ui->operatorstring->toPlainText().isEmpty() || ui->id->toPlainText().isEmpty() || locid== 0)
    {
      qDebug()<<"Put your details little shit"<<endl;
      popupmessage("Info missing");
      counter=0;

    }
  else{
    if (!ui->t2->toPlainText().isEmpty()){
      ttwo=ui->t2->toPlainText();
    }
    else{ttwo='\0';}

    if (!ui->t3->toPlainText().isEmpty()){
      tthree=ui->t3->toPlainText();
    }
    else{tthree='\0';}

    if (!ui->t4->toPlainText().isEmpty()){
      tfour=ui->t4->toPlainText();
    }
    else{tfour='\0';}

    if (!ui->t5->toPlainText().isEmpty()){
      tfive=ui->t5->toPlainText();
    }
    else{tfive='\0';}

    if (!ui->d1->toPlainText().isEmpty()){
      done=ui->d1->toPlainText();
    }
    else{done='\0';}

    if (!ui->d2->toPlainText().isEmpty()){
      dtwo=ui->d2->toPlainText();
    }
    else{dtwo='\0';}
    if (!ui->d3->toPlainText().isEmpty()){
      dthree=ui->d3->toPlainText();
    }
    else{dthree='\0';}

    if (!ui->d4->toPlainText().isEmpty()){
      dfour=ui->d4->toPlainText();
    }
    else{dfour='\0';}

    if (!ui->d5->toPlainText().isEmpty()){
      dfive=ui->d5->toPlainText();
    }
    else{dfive='\0';}
    opname = ui->operatorstring->toPlainText();
    //hicid=ui->id->toPlainText().toInt();
    hicid=ui->id->toPlainText();
    counter=1;
    lid=locid;
    GetMemberID();
    memberid=memid;
    qDebug()<<"The operator name is:"<<opname<<"and the hic id is: "<<hicid<<endl;
  }
}


void TestSelection::popupmessage(QString m){
  missingsettings=new Dialog(this);
  //need to check this Attribute
  // missingsettings->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlags((windowFlags() & Qt::WindowStaysOnTopHint));
  this->setWindowFlags((windowFlags() & ~Qt::WindowStaysOnTopHint));
  missingsettings->append(m);
  //  missingsettings->show();
  missingsettings->exec();

  // missingsettings->activateWindow();
  //missingsettings->raise();
}



void TestSelection::connectlocationcombo(std::vector<pair<std::string,int>> floc){

  for( auto const& v : floc ) {
    ui->databaselocation->addItem(v.first.c_str(),v.second);
  }
}

void TestSelection::getlocationcombo(int value){
  locid=0;
  switch(value){
  case 0:{
    break;}
  case 1:{
    locid=ui->databaselocation->itemData(ui->databaselocation->currentIndex()).toInt();
    break;}
  case 2:{
    locid=ui->databaselocation->itemData(ui->databaselocation->currentIndex()).toInt();
    break;}
  case 3:
    {locid=ui->databaselocation->itemData(ui->databaselocation->currentIndex()).toInt();
      break;
    }
  case 4:
    {locid=ui->databaselocation->itemData(ui->databaselocation->currentIndex()).toInt();
      break;
    }
  case 5:
    {locid=ui->databaselocation->itemData(ui->databaselocation->currentIndex()).toInt();
      break;
    }
  case 6:
    {locid=ui->databaselocation->itemData(ui->databaselocation->currentIndex()).toInt();
      break;
    }
  case 7:
    {locid=ui->databaselocation->itemData(ui->databaselocation->currentIndex()).toInt();
      break;
    }
  }}


int TestSelection::GetMemberID(){
  AlpideDB *myDB=new AlpideDB();
  ProjectDB *myproject=new ProjectDB(myDB);
  MemberDB *mymember= new MemberDB(myDB);
  std::vector<MemberDB::member> memberlist;
  mymember->GetList(21,&memberlist);
  for(unsigned int i=0; i<memberlist.size(); i++){
    if (strcmp(ui->operatorstring->toPlainText().toLatin1(),memberlist.at(i).FullName.c_str())==0){
      std::cout<<"Member "<<memberlist.at(i).FullName<<"  "<<memberlist.at(i).PersonalID<<std::endl;
      memid=memberlist.at(i).PersonalID;
    }

  }
  delete myDB;
  delete myproject;
  delete mymember;

  return 0;
}

void TestSelection::ClearLocations(){
  ui->databaselocation->clear();
}


void TestSelection::adjustendurance(){

  ui->t2->show();
  ui->t3->show();
  ui->t4->show();
  ui->t5->show();
  ui->d1->show();
  ui->d2->show();
  ui->d3->show();
  ui->d4->show();

  ui->d5->show();

}


void TestSelection::hideendurance(){

  ui->t2->hide();
  ui->t3->hide();
  ui->t4->hide();
  ui->t5->hide();
  ui->d1->hide();
  ui->d2->hide();
  ui->d3->hide();
  ui->d4->hide();

  ui->d5->hide();

}
