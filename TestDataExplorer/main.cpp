#include "QOpenGLWidget"
#include "QSplitter"
#include "QStandardItemModel"
#include "QTreeView"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow   w;
  w.show();

  return a.exec();
}
