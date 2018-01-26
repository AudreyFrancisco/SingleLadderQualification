#include "scanconfiguration.h"
#include "mainwindow.h"
#include "testselection.h"
#include "ui_mainwindow.h"
#include "ui_scanconfiguration.h"
#include "ui_testselection.h"
#include <QtCore/QCoreApplication>
#include <iomanip>
#include <iostream>
ScanConfiguration::ScanConfiguration(QWidget *parent)
    : QDialog(parent), ui(new Ui::ScanConfiguration) {
  ui->setupUi(this);
  ui->speedyfit->setChecked(false);
  ui->close->hide();
  connect(ui->speedyfit, SIGNAL(clicked(bool)), this->parent(), SLOT(speedycheck(bool)));
  connect(ui->close, SIGNAL(clicked()), this->parent(), SLOT(loaddefaultconfig()));
  connect(ui->load_configuration, SIGNAL(clicked()), this->parent(), SLOT(loadeditedconfig()));
}

ScanConfiguration::~ScanConfiguration() { delete ui; }

void ScanConfiguration::setnumberofmaskstages(int &numberofmaskstages) {
  if (!ui->nmaskstages->toPlainText().isEmpty()) {
    for (int i = 0; i < 513; i++) {

      if (ui->nmaskstages->toPlainText().toInt() == i) {
        numberofmaskstages = ui->nmaskstages->toPlainText().toInt();
        std::cout << "The number of mask stages is : " << numberofmaskstages << std::endl;
      }
    }
  }
}

void ScanConfiguration::setdefaultspeed(bool fit) {
  if (fit) {
    ui->speedyfit->setChecked(fit);
  } else {
    ui->speedyfit->setChecked(false);
  }
}

void ScanConfiguration::setdeaulmaskstages(int ms) {
  QString s = QString::number(ms);
  ui->nmaskstages->setText(s);
}
