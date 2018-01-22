#include "dbnotice.h"
#include "ui_dbnotice.h"

DBnotice::DBnotice(QWidget *parent) : QDialog(parent), ui(new Ui::DBnotice) {
  ui->setupUi(this);
  connect(ui->write, SIGNAL(clicked()), this->parent(), SLOT(finalwrite()));
  connect(ui->nowrite, SIGNAL(clicked()), this, SLOT(close()));
}

DBnotice::~DBnotice() { delete ui; }
