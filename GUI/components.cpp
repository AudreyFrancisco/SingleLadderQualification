#include "components.h"
#include "iostream"
#include "ui_components.h"
Components::Components(QWidget *parent) : QDialog(parent), ui(new Ui::Components)
{
  ui->setupUi(this);
  connect(ui->quit, SIGNAL(clicked()), this->parent(), SLOT(quittest()));
  connect(ui->continuetest, SIGNAL(clicked()), this->parent(), SLOT(continuetest()));
}

Components::~Components() { delete ui; }

void Components::WriteToLabel(QString name)
{
  QString comment;
  comment =
      "The component: \n" + name +
      "\nis not a member of the database. \nYou will not be able to write \n to the database.";
  ui->compstatus->setText(comment);
}


void Components::WrongPositions()
{

  ui->compstatus->setText("Check the positions \nof the hics in the database");
}
