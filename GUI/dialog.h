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
  void hideignore();
  void hidequit();

private:
  Ui::Dialog *ui;


private slots:
};

#endif // DIALOG_H
