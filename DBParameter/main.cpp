#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  ITSDB        w;
  w.show();

  return a.exec();
}
