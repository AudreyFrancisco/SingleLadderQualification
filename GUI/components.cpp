#include "components.h"
#include "iostream"
#include "ui_components.h"
Components::Components(QWidget *parent) : QDialog(parent), ui(new Ui::Components)
{
  ui->setupUi(this);
  connect(ui->continue_2, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->quit, SIGNAL(clicked()), this->parent(), SLOT(quittest()));
}

Components::~Components() { delete ui; }

void Components::WriteToLabel(QString name)
{
  QString comment;
  comment = "The component: \n" + name + "\nis not a member of the database. \nHow to you want to "
                                         "proceed? \nIn case you decide to continue \nkeep in mind "
                                         "that the activity \nwill remain open in the database";
  ui->compstatus->setText(comment);
}
