#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
  class Dialog;
}

class Dialog : public QDialog {
  Q_OBJECT

public:
  explicit Dialog(QWidget *parent = 0);
  ~Dialog();
  void append(QString error);

private:
  Ui::Dialog *ui;

private slots:

  // public slots:
  // void append(QString error);
};

#endif // DIALOG_H
