#ifndef DATABASESELECTION_H
#define DATABASESELECTION_H
#include "dialog.h"

#include <QDialog>

namespace Ui {
  class DatabaseSelection;
}

class DatabaseSelection : public QDialog {
  Q_OBJECT

public:
  explicit DatabaseSelection(QWidget *parent = 0);
  ~DatabaseSelection();

public slots:
  virtual void setdatabase(bool &database);
  void         popup(QString message);

private:
  Ui::DatabaseSelection *ui;
  Dialog *               fWindowerr;
};

#endif // DATABASESELECTION_H
