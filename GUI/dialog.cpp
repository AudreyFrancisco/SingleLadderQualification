/****************************************************************************
 ** Form implementation generated from reading ui file 'dialog.ui'
 **
 ** Created by User Interface Compiler
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#include "dialog.h"

#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::Dialog)
{
  ui->setupUi(this);
  connect(ui->quitex,SIGNAL(clicked()),this,SLOT(close()));

}

Dialog::~Dialog()
{
  delete ui;
}

void Dialog::append(QString error){
  ui->labelex->setText(error);
}
