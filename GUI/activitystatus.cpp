#include "activitystatus.h"
#include "QListWidgetItem"
#include "QMetaType"
#include "TScanAnalysis.h"
#include "iostream"
#include "ui_activitystatus.h"

ActivityStatus::ActivityStatus(QWidget *parent) : QDialog(parent), ui(new Ui::ActivityStatus)
{
  ui->setupUi(this);
  connect(ui->save, SIGNAL(clicked()), this, SLOT(close()));
}

Q_DECLARE_METATYPE(TScanResultHic *)


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


void ActivityStatus::ClearWindow()
{

  ui->nameofhic->setText("---");
  ui->finalclassificaton->setText("---");
  ui->oldclassification->setText("---");
  ui->cutdisplay->clear();
  ui->scanclassification->clear();
}

void ActivityStatus::PopulateWindow(QString nameoftheobject, QString oldclass, QString finalclass,
                                    std::vector<QString> nameing)
{


  ui->nameofhic->setText(nameoftheobject);
  ui->oldclassification->setText(oldclass);
  ui->finalclassificaton->setText(finalclass);
  for (unsigned int i = 0; i < nameing.size(); i++) {
    QListWidgetItem *newItem = new QListWidgetItem;
    newItem->setText(nameing.at(i));
    // QVariant v;
    // v.setValue(cuts.at(i));
    // newItem->setData(Qt::UserRole,v);
    ui->scanclassification->addItem(newItem);
  }
}
