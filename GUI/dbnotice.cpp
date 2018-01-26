#include "dbnotice.h"
#include "ui_dbnotice.h"

DBnotice::DBnotice(QWidget *parent) : QDialog(parent), ui(new Ui::DBnotice) {
  ui->setupUi(this);
  connect(ui->write, SIGNAL(clicked()), this->parent(), SLOT(finalwrite()));
  connect(ui->nowrite, SIGNAL(clicked()), this, SLOT(close()));
}

DBnotice::~DBnotice() { delete ui; }

void DBnotice::adjustingtemplate() {
  ui->label->setText(
      "The files of your test \n were not written to the database\n how do you proceed?");
  ui->nowrite->setText("Quit GUI");
  ui->write->setText("Write to Database and Quit");
}
