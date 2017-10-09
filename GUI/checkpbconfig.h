#ifndef CHECKPBCONFIG_H
#define CHECKPBCONFIG_H

#include <QDialog>
#include "calibrationpb.h"

namespace Ui {
class checkpbconfig;
}

class checkpbconfig : public QDialog
{
    Q_OBJECT

public:
    explicit checkpbconfig(QWidget *parent = 0);
    ~checkpbconfig();

public slots:
   // void opencalibration();

private:
    Ui::checkpbconfig *ui;
};

#endif // CHECKPBCONFIG_H
