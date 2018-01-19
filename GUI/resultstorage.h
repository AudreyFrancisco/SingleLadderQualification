#ifndef RESULTSTORAGE_H
#define RESULTSTORAGE_H

#include <QDialog>

namespace Ui {
class resultstorage;
}

class resultstorage : public QDialog
{
  Q_OBJECT

public:
  explicit resultstorage(QWidget *parent = 0);
  ~resultstorage();

private:
  Ui::resultstorage *ui;
};

#endif // RESULTSTORAGE_H
