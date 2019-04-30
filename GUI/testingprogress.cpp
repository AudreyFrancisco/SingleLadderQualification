#include "testingprogress.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_testingprogress.h"
#include <QtCore/QCoreApplication>
#include <iomanip>
#include <iostream>
Testingprogress::Testingprogress(QWidget *parent) : QDialog(parent), ui(new Ui::Testingprogress)
{
  ui->setupUi(this);
  connect(ui->stoptest, SIGNAL(clicked()), this->parent(), SLOT(stopscans()));
  connect(ui->retryscan, SIGNAL(clicked()), this->parent(), SLOT(retryfailedscan()));
  connect(ui->abortcurrentscan, SIGNAL(clicked()), this->parent(), SLOT(abortscan()));
  //   connect(ui->testingprogress,)
}

Testingprogress::~Testingprogress() { delete ui; }

void Testingprogress::setnotification(QString notification, QString exceptiondescription)
{
  ui->testingprogress2->setText(notification);
  ui->exceptiontype->setText(exceptiondescription);
}

void Testingprogress::stopaddingscans() { ui->retryscan->hide(); }
