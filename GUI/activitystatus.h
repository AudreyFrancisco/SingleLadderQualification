#ifndef ACTIVITYSTATUS_H
#define ACTIVITYSTATUS_H

#include <QDialog>

namespace Ui {
class ActivityStatus;
}

class ActivityStatus : public QDialog
{
  Q_OBJECT

public:
  explicit ActivityStatus(QWidget *parent = 0);
  ~ActivityStatus();
public
  slots:
    virtual void getactivitystatus(bool &status);

private:
  Ui::ActivityStatus *ui;
};

#endif // ACTIVITYSTATUS_H
