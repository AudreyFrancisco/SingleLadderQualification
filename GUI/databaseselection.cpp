#include "databaseselection.h"
#include "ui_databaseselection.h"

DatabaseSelection::DatabaseSelection(QWidget *parent)
    : QDialog(parent), ui(new Ui::DatabaseSelection)
{
  ui->setupUi(this);
  connect(ui->close, SIGNAL(clicked()), this, SLOT(close()));
}

DatabaseSelection::~DatabaseSelection() { delete ui; }

void DatabaseSelection::setdatabase(bool &database)
{

  if (ui->testdb->isChecked()) {
    ui->productiondb->setChecked(false);
    database = 1;
  }
  if (ui->productiondb->isChecked()) {

    ui->testdb->setChecked(false);
    database = 0;
  }
}
