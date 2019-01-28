#include "mainwindow.h"
#include "DBHelpers.h"
#include "QFrame"
#include "QPushButton"
#include "QSplitter"
#include "QStandardItemModel"
#include "QTableView"
#include "QTextEdit"
#include "QTreeView"
#include "QVBoxLayout"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  QMenu *  menu          = menuBar()->addMenu("&Actions");
  QAction *newtestaction = new QAction("&Search for component", this);
  connect(newtestaction, SIGNAL(triggered()), this, SLOT(OpenWindow()));
  menu->addAction(newtestaction);
  fdb       = new AlpideDB(false);
  fSplitter = new QSplitter(this);
  fModel    = new QStandardItemModel;
  fSplitter->setOrientation(Qt::Horizontal);
  QTreeView * leftSideWidget  = new QTreeView(this);
  QTableView *rightSideWidget = new QTableView(this);
  fSplitter->addWidget(leftSideWidget);
  fSplitter->addWidget(rightSideWidget);
  fParentItem = fModel->invisibleRootItem();


  QStandardItem *toy = NewItem("OBHIC-BR001583");

  if (toy) fParentItem->appendRow(toy);

  leftSideWidget->setModel(fModel);

  setCentralWidget(fSplitter);
}

MainWindow::~MainWindow() { delete ui; }


void MainWindow::OpenWindow() {}


QStandardItem *MainWindow::NewItem(std::string CompName)
{
  ComponentDB *              compDB = new ComponentDB(fdb);
  ComponentDB::componentLong result;
  compDB->Read(CompName, &result);
  if (!compDB) {
    std::cout << "there is no component" << std::endl;
    return fParentItem;
  }
  else {
    int            componentid = result.ID;
    QStandardItem *item        = new QStandardItem(QString::fromStdString(CompName));
    item->setData(componentid);
    std::vector<TChild> children;
    DbGetListOfChildren(fdb, componentid, children);
    for (unsigned int i = 0; i < children.size(); i++) {
      QStandardItem *newitem = NewItem(children.at(i).Name);
      if (newitem) item->appendRow(newitem);
    }

    return item;
  }
}
