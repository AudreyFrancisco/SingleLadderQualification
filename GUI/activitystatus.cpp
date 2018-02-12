#include "activitystatus.h"
#include "iostream"
#include "ui_activitystatus.h"

ActivityStatus::ActivityStatus(QWidget *parent) : QDialog(parent), ui(new Ui::ActivityStatus)
{
  ui->setupUi(this);
  connect(ui->save, SIGNAL(clicked()), this, SLOT(close()));
}

ActivityStatus::~ActivityStatus() { delete ui; }

void ActivityStatus::getactivitystatus(bool &status)
{

  if (ui->openactiv->isChecked()) {
    ui->closeactiv->setChecked(false);
    status = 1;
  }
  if (ui->closeactiv->isChecked()) {

    ui->openactiv->setChecked(false);
    status = 0;
  }
}

void ActivityStatus::GetComment(QString &comment)
{
  if (!ui->com->toPlainText().isEmpty()) {
    comment = ui->com->toPlainText();
    std::cout << "the comment is : " << comment.toStdString() << std::endl;
  }
}
