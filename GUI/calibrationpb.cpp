#include "calibrationpb.h"
#include "mainwindow.h"
#include "ui_calibrationpb.h"
#include <QtCore/QCoreApplication>
#include <QtCore>
#include <iomanip>

Calibrationpb::Calibrationpb(QWidget *parent) : QDialog(parent), ui(new Ui::Calibrationpb)
{
  ui->setupUi(this);
  connect(ui->cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->ok, SIGNAL(clicked()), this->parent(), SLOT(writecalibrationfile()));
  connect(ui->calibrate, SIGNAL(clicked()), this->parent(), SLOT(setandgetcalibration()));
  ui->agndcal->hide();
  ui->ground2->hide();
  ui->ground1->setText("Ground");
}

Calibrationpb::~Calibrationpb() { delete ui; }

void Calibrationpb::setresistances(float &analog, float &digital, float &bb, float &agnd)
{
  std::cout << "seting resistances" << std::endl;
  if (!ui->acal->toPlainText().isEmpty()) {
    analog = ui->acal->toPlainText().toFloat();
  }
  if (!ui->dcal->toPlainText().isEmpty()) {
    digital = ui->dcal->toPlainText().toFloat();
  }
  if (!ui->dgndcal->toPlainText().isEmpty()) {
    bb = ui->dgndcal->toPlainText().toFloat();
  }
  if (!ui->agndcal->toPlainText().isEmpty()) {
    agnd = ui->agndcal->toPlainText().toFloat();
  }
}

void Calibrationpb::getcalibration(float savdd, float iavdd, float sdvdd, float idvdd,
                                   float offsetia, float offsetid)
{

  ui->savdd->setText(QString::number(savdd));
  ui->iavdd->setText(QString::number(iavdd));
  ui->sdvdd->setText(QString::number(sdvdd));
  ui->idvdd->setText(QString::number(idvdd));
  ui->offsetia->setText(QString::number(offsetia));
  ui->offsetid->setText(QString::number(offsetid));
  std::cout << "calibration done" << std::endl;
}


void Calibrationpb::enableanaloguegnd()
{
  ui->ground1->setText("Digital Ground");
  ui->agndcal->show();
  ui->ground2->show();
}
