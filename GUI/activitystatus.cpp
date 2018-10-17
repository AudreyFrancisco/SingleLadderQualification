#include "activitystatus.h"
#include "QListWidgetItem"
#include "QMetaType"
#include "QTextCodec"
#include "THIC.h"
#include "TScanAnalysis.h"
#include "iostream"
#include "ui_activitystatus.h"

ActivityStatus::ActivityStatus(QWidget *parent) : QDialog(parent), ui(new Ui::ActivityStatus)
{

  ui->setupUi(this);
  connect(ui->save, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->scanclassification, &QListWidget::itemClicked, this,
          &ActivityStatus::on_scanclassification_itemClicked);
  ui->label->hide();
  ui->label_2->hide();
  ui->openactiv->hide();
  ui->closeactiv->hide();
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
                                    std::vector<QString>          nameing,
                                    std::vector<TScanResultHic *> cuts)
{
  ui->nameofhic->setText(nameoftheobject);
  ui->oldclassification->setText(oldclass);
  ui->finalclassificaton->setText(finalclass);
  if (finalclass == "RED") ui->finalclassificaton->setStyleSheet("background-color:red;");
  for (unsigned int i = 0; i < nameing.size(); i++) {
    QListWidgetItem *newItem = new QListWidgetItem;
    newItem->setText(nameing.at(i));
    QVariant v;
    v.setValue(cuts.at(i));
    newItem->setData(Qt::UserRole, v);
    ui->scanclassification->addItem(newItem);
  }
}


void ActivityStatus::on_scanclassification_itemClicked(QListWidgetItem *item)
{

  ui->cutdisplay->clear();
  TScanResultHic *hicresult = item->data(Qt::UserRole).value<TScanResultHic *>();
  for (unsigned int i = 0; i < hicresult->GetCuts().size(); i++) {
    QListWidgetItem *newItem = new QListWidgetItem;
    newItem->setText(hicresult->GetCuts().at(i).c_str());
    ui->cutdisplay->addItem(newItem);
  }
}

void ActivityStatus::DisplayStatusOptions()
{
  ui->label->show();
  ui->label_2->show();
  ui->openactiv->show();
  ui->closeactiv->show();
}
