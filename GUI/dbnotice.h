#ifndef DBNOTICE_H
#define DBNOTICE_H

#include <QDialog>

namespace Ui {
  class DBnotice;
}

class DBnotice : public QDialog {
  Q_OBJECT

public:
  explicit DBnotice(QWidget *parent = 0);
  ~DBnotice();
public slots:
  virtual void adjustingtemplate();

private:
  Ui::DBnotice *ui;
};

#endif // DBNOTICE_H
