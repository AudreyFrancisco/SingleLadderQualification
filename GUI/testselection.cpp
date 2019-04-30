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

  ui->typetest->addItem(" ", 0);
  // ui->typetest->addItem("OB HIC Qualification Test", OBQualification);
  // ui->typetest->addItem("IB HIC Qualification Test", IBQualification);
  ui->typetest->addItem("MFT HIC FIFO Scan", MFTFifo);
  ui->typetest->addItem("MFT HIC Digital Scan", MFTDigital);
  ui->typetest->addItem("MFT HIC Threshold Scan", MFTThreshold);
  ui->typetest->addItem("MFT HIC Noise Occupancy Scan", MFTNoise);
  ui->typetest->addItem("MFT Eye Measurement", MFTEyeMeasurement);
  ui->typetest->addItem("MFT HIC Qualification", MFTQualification);
  // ui->typetest->addItem("OB HIC Endurance Test", OBEndurance);
  // ui->typetest->addItem("IB HIC Endurance Test", IBEndurance);
  // ui->typetest->addItem("OB HIC Reception Test", OBReception);
  // ui->typetest->addItem("OB HIC Fast Power Test", OBPower);
  // ui->typetest->addItem("OL HS Qualification Test", OBHalfStaveOL);
  // ui->typetest->addItem("IB HIC DCTRL Test", IBDctrl);
  // ui->typetest->addItem("OB Half-Stave Test", OBHalfStaveML);
  // ui->typetest->addItem("OB Stave Test", OBStave);
  // ui->typetest->addItem("IB Stave Test", IBStave);
  ui->databaselocation->hide();

  ui->testbenchlocation->addItem("", 0);
  ui->testbenchlocation->addItem("CERN", cern);
  ui->testbenchlocation->addItem("CEA Saclay", saclay);
  ui->testbenchlocation->addItem("Subatech", subatech);
  ui->testbenchlocation->addItem("IPNL", ipnl);

  ui->Chip_Nb->addItem(" ", 0);
  ui->Chip_Nb->addItem("2", 2);
  ui->Chip_Nb->addItem("3", 3);
  ui->Chip_Nb->addItem("4", 4);
  ui->Chip_Nb->addItem("5", 5);

  missingsettings = 0x0;

  m_testDatabase = testDatabase;

  connect(ui->settings, SIGNAL(clicked()), this->parent(), SLOT(savesettings()));
  connect(ui->close, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->testbenchlocation, SIGNAL(currentIndexChanged(int)), this,
          SLOT(getlocationcombo(int)));
  // connect(ui->databaselocation, SIGNAL(currentIndexChanged(int)), this,
  //         SLOT(printlocation(QString)));
  connect(ui->typetest, SIGNAL(currentIndexChanged(int)), this->parent(),
          SLOT(ConnectTestCombo(int)));
  connect(ui->Chip_Nb, SIGNAL(currentIndexChanged(int)), this->parent(),
          SLOT(ConnectHICSizeCombo(int)));
  connect(ui->disablefour, SIGNAL(toggled(bool)), this->parent(), SLOT(DisableFour(bool)));
  connect(ui->disablefive, SIGNAL(toggled(bool)), this->parent(), SLOT(DisableFive(bool)));
  connect(ui->disablesix, SIGNAL(toggled(bool)), this->parent(), SLOT(DisableSix(bool)));
  connect(ui->disableseven, SIGNAL(toggled(bool)), this->parent(), SLOT(DisableSeven(bool)));
  connect(ui->disableeight, SIGNAL(toggled(bool)), this->parent(), SLOT(DisableEight(bool)));
}

TestSelection::~TestSelection() { delete ui; }

void TestSelection::Init()
{
  QSettings settings;
  ui->operatorstring->setText(settings.value("operator", "").toString());
  ui->ipaddress->setText(settings.value("ipaddress", "").toString());
  int idx = ui->typetest->findText(settings.value("testname", "n/a").toString());
  if (idx >= 0) ui->typetest->setCurrentIndex(idx);
  idx = ui->testbenchlocation->findText(settings.value("testsite", "n/a").toString());
  if (idx >= 0) ui->testbenchlocation->setCurrentIndex(idx);
  ui->id->setText(settings.value("hicnumber", "").toString());
  QString id      = settings.value("hicnumber", "").toString();
  int     idofhic = id.toInt();
  if ((idofhic / 1000) >= 2 && (idofhic / 1000) <= 5) {
    ui->Chip_Nb->setCurrentIndex((idofhic / 1000) - 1);
  }
}

void TestSelection::SaveSettings(QString &institute, QString &opname, QString &hicid, int &counter,
                                 int &lid, int &memberid, QString &ttwo, QString &tthree,
                                 QString &tfour, QString &tfive, QString &done, QString &dtwo,
                                 QString &dthree, QString &dfour, QString &dfive,
                                 QString &halfstave, QString &stave, QString &address)
{
  // Default value for locid, TODO:Associate a location to a default configuration setup.
  locid = 2;
  if (ui->operatorstring->toPlainText().isEmpty() ||
      /*ui->id->toPlainText().isEmpty() || */ locid == 0 || ui->id->toPlainText().isEmpty() ||
      ui->Chip_Nb->currentIndex() == 0) {
    qDebug() << "Put your details" << endl;
    fCounter = counter = 0;
    QString inf_mess   = "Missing information:\n\n";
    if (ui->operatorstring->toPlainText().isEmpty()) {
      inf_mess.append("\t Operator name\n\n");
    }
    if (ui->ipaddress->toPlainText().isEmpty()) {
      inf_mess.append("\t MOSAIC IP Address\n\n");
    }
    if (ui->id->toPlainText().isEmpty()) {
      inf_mess.append("\t HIC ID Number\n\n");
    }
    if (ui->Chip_Nb->currentIndex() == 0) {
      inf_mess.append("\t Number of chips\n\n");
    }
    popupmessage(inf_mess);
  }
  else {
    /*   if (!ui->t2->toPlainText().isEmpty()) {
          ttwo = ui->t2->toPlainText();
        }
        else {
    */
    ttwo = '\0';
    //    }

    /*    if (!ui->t3->toPlainText().isEmpty()) {
          tthree = ui->t3->toPlainText();
        }
        else {*/
    tthree = '\0';
    //    }

    /*    if (!ui->t4->toPlainText().isEmpty()) {
          tfour = ui->t4->toPlainText();
        }
        else {*/
    tfour = '\0';
    //    }

    /*    if (!ui->t5->toPlainText().isEmpty()) {
          tfive = ui->t5->toPlainText();
        }
        else {*/
    tfive = '\0';
    //    }

    /*    if (!ui->d1->toPlainText().isEmpty()) {
          done = ui->d1->toPlainText();
        }
        else {*/
    done = '\0';
    //    }

    /*    if (!ui->d2->toPlainText().isEmpty()) {
          dtwo = ui->d2->toPlainText();
        }
        else {*/
    dtwo = '\0';
    //    }
    /*    if (!ui->d3->toPlainText().isEmpty()) {
          dthree = ui->d3->toPlainText();
        }
        else {*/
    dthree = '\0';
    //    }

    /*    if (!ui->d4->toPlainText().isEmpty()) {
          dfour = ui->d4->toPlainText();
        }
        else {*/
    dfour = '\0';
    //    }

    /*    if (!ui->d5->toPlainText().isEmpty()) {
          dfive = ui->d5->toPlainText();
        }
        else {*/
    dfive = '\0';
    //    }

    // if (!ui->stavenamefield->toPlainText().isEmpty()) {
    //   stave = ui->stavenamefield->toPlainText();
    // }
    // else {
    //   stave = '\0';
    // }

    opname    = ui->operatorstring->toPlainText();
    hicid     = ui->id->toPlainText();
    halfstave = ui->id->toPlainText();
    address   = ui->ipaddress->toPlainText();
    fCounter = counter = 1;
    lid                = locid;
    institute          = location;
    if (lid != 0) {
      // memberid = GetMemberID();
      memberid = opname.toInt();
    }
    if (memberid == -1) {
      popupmessage("The operator you entered \nis not in the Database");
      if (fCounter == 0) {
        counter = 0;
      }
    }

    QSettings settings;
    settings.setValue("operator", opname);
    settings.setValue("hicnumber", hicid);
    settings.setValue("ipaddress", address);
    settings.setValue("testname", ui->typetest->currentText());
    settings.setValue("testsite", ui->testbenchlocation->currentText());

    qDebug() << "The operator name is: " << opname << "and the hic id is: " << hicid << endl;
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
  ui->databaselocation->clear();
  for (auto const &v : floc) {
    ui->databaselocation->addItem(v.first.c_str(), v.second);
  }
}

void TestSelection::getlocationcombo(int value)
{
  locid = 0;
  if (value > 0) { // first item is empty
    locid    = ui->testbenchlocation->itemData(ui->testbenchlocation->currentIndex()).toInt();
    location = ui->testbenchlocation->currentText();
  }
  printf("locid = %d\n", locid);
}

int TestSelection::GetMemberID()
{
  return 1000;
  /*AlpideDB *DB;
  DB = fmainwindow->GetDB();
  int result;
  result = DbGetMemberId(DB, ui->operatorstring->toPlainText().toStdString());
  return result;*/
}

void TestSelection::ClearLocations() { ui->databaselocation->clear(); }

void TestSelection::adjustendurance()
{

  /*  ui->t2->show();
    ui->t3->show();
    ui->t4->show();
    ui->t5->show();*/
}

void TestSelection::hideendurance()
{

  /*  ui->t2->hide();
    ui->t3->hide();
    ui->t4->hide();
    ui->t5->hide();*/
}

void TestSelection::GetTestTypeName(TTestType &typetest, QString &testname)
{
  int value;
  value    = ui->typetest->itemData(ui->typetest->currentIndex()).toInt();
  testname = ui->typetest->currentText();
  typetest = (TTestType)value;
  std::cout << "the value is: " << value << " the test type is " << typetest
            << " and the string is: " << testname.toStdString().c_str() << std::endl;
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

// void TestSelection::adjuststave()
// {
//
//   ui->stavename->show();
//   ui->stavenamefield->show();
//   ui->hsname->show();
// }
