#ifndef DATABASEFAILURE_H
#define DATABASEFAILURE_H

#include <QDialog>

namespace Ui {
  class Databasefailure;
}

class Databasefailure : public QDialog {
  Q_OBJECT

public:
  explicit Databasefailure(QWidget *parent = 0);
  ~Databasefailure();

private:
  Ui::Databasefailure *ui;
};

#endif // DATABASEFAILURE_H
