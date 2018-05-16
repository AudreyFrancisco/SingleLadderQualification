#ifndef ACTIVITYSTATUS_H
#define ACTIVITYSTATUS_H

#include "TScanAnalysis.h"
#include <QDialog>
#include <QListWidgetItem>
namespace Ui {
  class ActivityStatus;
}

class ActivityStatus : public QDialog {
  Q_OBJECT

public:
  explicit ActivityStatus(QWidget *parent = 0);
  ~ActivityStatus();


public slots:
  virtual void getactivitystatus(bool &status);
  virtual void GetComment(QString &comment);
  virtual void ClearWindow();
  virtual void PopulateWindow(QString nameoftheobject, QString oldclass, QString finalclass,
                              std::vector<QString> nameing, std::vector<TScanResultHic *> cuts);
  // virtual void PopulateWindow(QString nameoftheobject, QString oldclass, QString finalclass,
  // std::vector<QString> nameing);
  void on_scanclassification_itemClicked(QListWidgetItem *item);
  virtual void DisplayStatusOptions();

private:
  Ui::ActivityStatus *ui;
};

#endif // ACTIVITYSTATUS_H
