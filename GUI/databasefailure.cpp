#include "databasefailure.h"
#include "ui_databasefailure.h"

Databasefailure::Databasefailure(QWidget *parent) : QDialog(parent), ui(new Ui::Databasefailure)
{
  ui->setupUi(this);
  connect(ui->quit, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->writeagain, SIGNAL(clicked()), this->parent(), SLOT(attachtodatabase()));
}

Databasefailure::~Databasefailure() { delete ui; }


void Databasefailure::assigningproblem(std::vector<QString> errorMessages)
{
  for (unsigned int i = 0; i < errorMessages.size(); i++) {
    ui->listDBErrors->addItem(errorMessages.at(i));
  }
}
