#ifndef TESTSELECTION_H
#define TESTSELECTION_H
#include "dialog.h"
#include "mainwindow.h"
#include <QDialog>
#include <QPushButton>
#include <TFifoTest.h>
namespace Ui {
  class TestSelection;
}

class TestSelection : public QDialog {
  Q_OBJECT

public:
  explicit TestSelection(QWidget *parent = 0, bool testDatabase = true);
  ~TestSelection();
  virtual void SaveSettings(QString &institute, QString &opname, QString &hicid, int &counter,
                            int &lid, int &memberid, QString &ttwo, QString &tthree, QString &tfour,
                            QString &tfive, QString &done, QString &dtwo, QString &dthree,
                            QString &dfour, QString &dfive, QString &halfstave, QString &stave);
  virtual int  GetLocationID() { return locid; }
  virtual void ClearLocations();
  virtual void Init();

private:
  Ui::TestSelection *ui;
  Dialog *           missingsettings;
  int                locid = 0;
  int                memid;
  bool               m_testDatabase;
  QString            location;
  QString            toptwo, topthree, topfour, topfive;
  QString            downone, downtwo, downthree, downfour, downfive;
  MainWindow *       fmainwindow;
  int                fCounter;

public slots:
  virtual void getlocationcombo(int value);
  virtual int  GetMemberID();
  virtual void adjustendurance();
  virtual void hideendurance();
  virtual void GetTestTypeName(TTestType &typetest, QString &testname);
  virtual void nextstep();
  virtual void getwindow();
  virtual int  getcounter();
  virtual void connectlocationcombo(std::vector<std::pair<std::string, int>> floc);
  virtual void adjuststave();


private slots:
  void popupmessage(QString m);
};

#endif // TESTSELECTION_H
