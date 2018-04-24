#include "components.h"
#include "iostream"
#include "ui_components.h"
Components::Components(QWidget *parent) : QDialog(parent), ui(new Ui::Components)
{
  ui->setupUi(this);
  connect(ui->continue_2, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->quit, SIGNAL(clicked()), this->parent(), SLOT(quittest()));
  ui->continue_2->hide();
}

Components::~Components() { delete ui; }

void Components::WriteToLabel(QString name)
{
  QString comment;
  comment = "The component: \n" + name + "\nis not a member of the database. \n Please start a new "
                                         "test with \n a valid component name.";
  ui->compstatus->setText(comment);
}


void Components::WrongPositions()
{

  ui->compstatus->setText("Check the positions \nof the hics in the database");
}
