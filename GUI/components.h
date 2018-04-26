#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <QDialog>

namespace Ui {
  class Components;
}

class Components : public QDialog {
  Q_OBJECT

public:
  explicit Components(QWidget *parent = 0);
  ~Components();
public slots:
  void WriteToLabel(QString name);
  void WrongPositions();

private:
  Ui::Components *ui;
};

#endif // COMPONENTS_H
