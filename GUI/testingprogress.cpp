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
  connect(ui->continuetest, SIGNAL(clicked()), this->parent(), SLOT(continuescans()));
  connect(ui->stoptest, SIGNAL(clicked()), this->parent(), SLOT(stopscans()));
  connect(ui->retryscan, SIGNAL(clicked()), this->parent(), SLOT(retryfailedscan()));
  //   connect(ui->testingprogress,)
}

Testingprogress::~Testingprogress() { delete ui; }

void Testingprogress::setnotification(QString notification)
{
  ui->testingprogress2->setText(notification);
}

void Testingprogress::stopaddingscans() { ui->retryscan->hide(); }
