#ifndef CALIBRATIONPB_H
#define CALIBRATIONPB_H

#include <QDialog>
#include "iostream"
namespace Ui {
class Calibrationpb;
}

class Calibrationpb : public QDialog
{
    Q_OBJECT

public:
    explicit Calibrationpb(QWidget *parent = 0);
    ~Calibrationpb();

public slots:
    virtual void setresistances(int &analog,int &digital, int &bb);
    virtual void getcalibration(float calv, float cali, float lineres);

private:
    Ui::Calibrationpb *ui;
};

#endif // CALIBRATIONPB_H
