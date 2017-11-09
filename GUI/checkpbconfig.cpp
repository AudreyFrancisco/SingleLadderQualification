#include "checkpbconfig.h"
#include "ui_checkpbconfig.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QtCore/QCoreApplication>
#include <iomanip>

checkpbconfig::checkpbconfig(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::checkpbconfig)
{
  ui->setupUi(this);
  connect(ui->no,SIGNAL(clicked()),this,SLOT(close()));
  connect(ui->yes,SIGNAL(clicked()),this->parent(),SLOT(opencalibration()));

}

checkpbconfig::~checkpbconfig()
{
  delete ui;
}

