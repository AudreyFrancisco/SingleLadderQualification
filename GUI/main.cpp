#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("ALICE");
  QCoreApplication::setOrganizationDomain("alice.cern");
  QCoreApplication::setApplicationName("Alpide Testing");

  QApplication a(argc, argv);
  MainWindow   w;
  w.show();

  return a.exec();
}
