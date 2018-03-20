#include "testselection.h"
#include "DBHelpers.h"
#include "TFifoTest.h"
#include "mainwindow.h"
#include "ui_testselection.h"
#include <QtCore/QCoreApplication>
#include <iomanip>
#include <iostream>
TestSelection::TestSelection(QWidget *parent, bool testDatabase)
    : QDialog(parent), ui(new Ui::TestSelection)
{
  ui->setupUi(this);
  fmainwindow = qobject_cast<MainWindow *>(parent);
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
  ui->typetest->addItem(" ", 0);
  ui->typetest->addItem("OB HIC Qualification Test", OBQualification);
  ui->typetest->addItem("IB HIC Qualification Test", IBQualification);
  ui->typetest->addItem("OB HIC Endurance Test", OBEndurance);
  ui->typetest->addItem("IB HIC Endurance Test", IBEndurance);
  ui->typetest->addItem("OB HIC Reception Test", OBReception);
  ui->typetest->addItem("OB HIC Fast Power Test", OBPower);
  ui->typetest->addItem("OL-HS-Upper assembly", OBHalfStaveOL);
  ui->typetest->addItem("IB DCTRL Test", IBDctrl);
  // ui->typetest->addItem("OB Half-Stave Test", OBHalfStaveML);
  // ui->typetest->addItem("OB Stave Test", OBStave);
  // ui->typetest->addItem("IB Stave Test", IBStave);
  ui->typeoftest->hide();
  missingsettings = 0x0;

  m_testDatabase = testDatabase;

  connect(ui->settings, SIGNAL(clicked()), this->parent(), SLOT(savesettings()));
  connect(ui->close, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->databaselocation, SIGNAL(currentIndexChanged(int)), this,
          SLOT(getlocationcombo(int)));
  connect(ui->typetest, SIGNAL(currentIndexChanged(int)), this->parent(),
          SLOT(ConnectTestCombo(int)));
}

TestSelection::~TestSelection() { delete ui; }

void TestSelection::SaveSettings(QString &institute, QString &opname, QString &hicid, int &counter,
                                 int &lid, int &memberid, QString &ttwo, QString &tthree,
                                 QString &tfour, QString &tfive, QString &done, QString &dtwo,
                                 QString &dthree, QString &dfour, QString &dfive)
{
  if (ui->operatorstring->toPlainText().isEmpty() ||
      /*ui->id->toPlainText().isEmpty() || */ locid == 0) {
    qDebug() << "Put your details" << endl;
    fCounter = counter = 0;
    popupmessage("Info missing");
  }
  else {
    if (!ui->t2->toPlainText().isEmpty()) {
      ttwo = ui->t2->toPlainText();
    }
    else {
      ttwo = '\0';
    }

    if (!ui->t3->toPlainText().isEmpty()) {
      tthree = ui->t3->toPlainText();
    }
    else {
      tthree = '\0';
    }

    if (!ui->t4->toPlainText().isEmpty()) {
      tfour = ui->t4->toPlainText();
    }
    else {
      tfour = '\0';
    }

    if (!ui->t5->toPlainText().isEmpty()) {
      tfive = ui->t5->toPlainText();
    }
    else {
      tfive = '\0';
    }

    if (!ui->d1->toPlainText().isEmpty()) {
      done = ui->d1->toPlainText();
    }
    else {
      done = '\0';
    }

    if (!ui->d2->toPlainText().isEmpty()) {
      dtwo = ui->d2->toPlainText();
    }
    else {
      dtwo = '\0';
    }
    if (!ui->d3->toPlainText().isEmpty()) {
      dthree = ui->d3->toPlainText();
    }
    else {
      dthree = '\0';
    }

    if (!ui->d4->toPlainText().isEmpty()) {
      dfour = ui->d4->toPlainText();
    }
    else {
      dfour = '\0';
    }

    if (!ui->d5->toPlainText().isEmpty()) {
      dfive = ui->d5->toPlainText();
    }
    else {
      dfive = '\0';
    }

    opname   = ui->operatorstring->toPlainText();
    hicid    = ui->id->toPlainText();
    fCounter = counter = 1;
    lid                = locid;
    institute          = location;
    memberid           = GetMemberID();
    if (memberid == -1) {
      popupmessage("The operator you entered \nis not in the Database");
      if (fCounter == 0) {
        counter = 0;
      }
    }

    qDebug() << "The operator name is:" << opname << "and the hic id is: " << hicid << endl;
  }
}

void TestSelection::popupmessage(QString m)
{
  missingsettings = new Dialog(this);
  // need to check this Attribute
  // missingsettings->setAttribute(Qt::WA_DeleteOnClose);
  // this->setWindowFlags((windowFlags() & Qt::WindowStaysOnTopHint));
  // this->setWindowFlags((windowFlags() & ~Qt::WindowStaysOnTopHint));
  missingsettings->append(m);
  if (fCounter == 0) {
    missingsettings->hideignore();
  }
  //  missingsettings->show();
  missingsettings->exec();

  // missingsettings->activateWindow();
  // missingsettings->raise();
}

void TestSelection::connectlocationcombo(std::vector<pair<std::string, int>> floc)
{

  for (auto const &v : floc) {
    ui->databaselocation->addItem(v.first.c_str(), v.second);
  }
}

void TestSelection::getlocationcombo(int value)
{
  locid = 0;
  if (value > 0) { // first item is empty
    locid    = ui->databaselocation->itemData(ui->databaselocation->currentIndex()).toInt();
    location = ui->databaselocation->currentText();
  }
}

int TestSelection::GetMemberID()
{
  AlpideDB *DB;
  DB = fmainwindow->GetDB();
  int result;
  result = DbGetMemberId(DB, ui->operatorstring->toPlainText().toStdString());
  return result;
}

void TestSelection::ClearLocations() { ui->databaselocation->clear(); }

void TestSelection::adjustendurance()
{

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

void TestSelection::hideendurance()
{

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

void TestSelection::GetTestTypeName(TTestType &typetest, QString &testname)
{
  int value;
  value    = ui->typetest->itemData(ui->typetest->currentIndex()).toInt();
  testname = ui->typetest->currentText();
  typetest = (TTestType)value;
  std::cout << "the value is: " << value << " the test type is " << typetest
            << "and the string is: " << testname.toStdString().c_str() << std::endl;
}

void TestSelection::nextstep()
{
  if (missingsettings) {
    missingsettings->close();
    fCounter = 0;
  }
}

void TestSelection::getwindow()
{
  if (!missingsettings) {
    missingsettings = new Dialog(this);
  }
  missingsettings->append("The activity cannot be found \nin the Database \nCheck your database "
                          "connection\nMaybe you need to renew \nyour ticket\nOr there is problem "
                          "in the db.");
  missingsettings->hideignore();
  missingsettings->exec();
}

int TestSelection::getcounter() { return fCounter; }
