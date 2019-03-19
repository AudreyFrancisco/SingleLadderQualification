#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "DBHelpers.h"
#include "QFrame"
#include "QPushButton"
#include "QSplitter"
#include "QStandardItemModel"
#include "QTableView"
#include "QTextEdit"
#include "QTreeView"
#include "QVBoxLayout"
#include <QMainWindow>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  QStandardItem *     NewItem(std::string CompName);
  QSplitter *         fSplitter;
  QStandardItemModel *fModel;
  QStandardItemModel *fModelTable;
  AlpideDB *          fdb;
  QStandardItem *     fParentItem = 0;


private:
  Ui::MainWindow *ui;


private slots:
  void OpenWindow();

public slots:
  void onTreeClicked(const QModelIndex &index);
};

#endif // MAINWINDOW_H
