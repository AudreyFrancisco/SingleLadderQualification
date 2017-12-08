#ifndef TESTSELECTION_H
#define TESTSELECTION_H
#include<TFifoTest.h>
#include <QDialog>
#include <QPushButton>
#include "dialog.h"

namespace Ui {
  class TestSelection;
}

class TestSelection : public QDialog
{
  Q_OBJECT

 public:

  explicit TestSelection(QWidget *parent = 0);
  ~TestSelection();
  virtual void SaveSettings(QString &institute, QString &opname, QString &hicid, int &counter, int &lid, int &memberid, QString &ttwo, QString &tthree, QString &tfour, QString &tfive, QString &done, QString &dtwo, QString &dthree, QString &dfour, QString &dfive);
  virtual void connectlocationcombo(std::vector<std::pair<std::string,int>> floc);
  virtual int GetLocationID(){return locid;}
  virtual void ClearLocations();

 private:
  Ui::TestSelection *ui;
  Dialog *missingsettings;
  int locid;
  int memid;
  QString location;
  QString toptwo, topthree, topfour, topfive;
  QString downone, downtwo,downthree, downfour,downfive;

  public slots:
    virtual void getlocationcombo(int value);
  virtual int GetMemberID();
  virtual void adjustendurance();
  virtual void hideendurance();

  private slots:
    void popupmessage(QString m);
};

#endif // TESTSELECTION_H

