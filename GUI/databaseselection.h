#ifndef DATABASESELECTION_H
#define DATABASESELECTION_H

#include <QDialog>

namespace Ui {
  class DatabaseSelection;
}

class DatabaseSelection : public QDialog {
  Q_OBJECT

public:
  explicit DatabaseSelection(QWidget *parent = 0);
  ~DatabaseSelection();

public
slots:
  virtual void setdatabase(bool &database);

private:
  Ui::DatabaseSelection *ui;
};

#endif // DATABASESELECTION_H
