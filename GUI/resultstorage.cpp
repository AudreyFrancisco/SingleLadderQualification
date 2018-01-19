#include "resultstorage.h"
#include "ui_resultstorage.h"

resultstorage::resultstorage(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::resultstorage)
{
  ui->setupUi(this);
  connect(ui->nowriting,SIGNAL(clicked()),this->parent(),SLOT(ContinueWithoutWriting()));
  connect(ui->writeopen,SIGNAL(clicked()),this->parent(),SLOT(attachtodatabase()));
}

resultstorage::~resultstorage()
{
  delete ui;
}
