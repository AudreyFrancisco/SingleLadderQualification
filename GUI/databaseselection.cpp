#include "databaseselection.h"
#include "ui_databaseselection.h"
#include "dialog.h"

DatabaseSelection::DatabaseSelection(QWidget *parent)
    : QDialog(parent), ui(new Ui::DatabaseSelection)
{
  ui->setupUi(this);
  connect(ui->close, SIGNAL(clicked()), this, SLOT(close()));
}

DatabaseSelection::~DatabaseSelection() { delete ui; }

void DatabaseSelection::setdatabase(bool &database)
{

  if (ui->localtest->isChecked()) {
    ui->constructiondb->setChecked(false);
    database = 1;
  }
  if (ui->constructiondb->isChecked()) {
    popup("Construction database not yet available");
    ui->localtest->setChecked(false);
    database = 0;
  }
}

void DatabaseSelection::popup(QString message)
{
  fWindowerr = new Dialog(this);
  fWindowerr->append(message);
  fWindowerr->hidequit();
  fWindowerr->show();
}
