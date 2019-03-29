#include "mainwindow.h"
#include "DBHelpers.h"
#include "QFrame"
#include "QList"
#include "QPushButton"
#include "QSplitter"
#include "QStandardItemModel"
#include "QTableView"
#include "QTextEdit"
#include "QTreeView"
#include "QVBoxLayout"
#include "ScanData.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <string>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  QMenu *  menu          = menuBar()->addMenu("&Actions");
  QAction *newtestaction = new QAction("&Search for component", this);
  connect(newtestaction, SIGNAL(triggered()), this, SLOT(OpenWindow()));
  menu->addAction(newtestaction);
  fdb         = new AlpideDB(false);
  fSplitter   = new QSplitter(this);
  fModel      = new QStandardItemModel;
  fModelTable = new QStandardItemModel;

  fSplitter->setOrientation(Qt::Horizontal);
  QTreeView * leftSideWidget  = new QTreeView(this);
  QTableView *rightSideWidget = new QTableView(this);
  fSplitter->addWidget(leftSideWidget);
  fSplitter->addWidget(rightSideWidget);
  fParentItem         = fModel->invisibleRootItem();
  QHeaderView *header = rightSideWidget->horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);

  QStandardItem *toy = NewItem("D-OL-Stave-003");
  //  QStandardItem *toy = NewItem("OBHIC-BR001583");

  if (toy) fParentItem->appendRow(toy);


  fModelTable->setHorizontalHeaderItem(0, new QStandardItem("Test Type"));
  fModelTable->setHorizontalHeaderItem(1, new QStandardItem("Data"));


  leftSideWidget->setModel(fModel);
  rightSideWidget->setModel(fModelTable);
  setCentralWidget(fSplitter);

  connect(leftSideWidget, SIGNAL(clicked(const QModelIndex &)), this,
          SLOT(onTreeClicked(const QModelIndex &)));
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
    std::cout << "component " << CompName << ", the component id is " << componentid << std::endl;
    DbGetListOfChildren(fdb, componentid, children);
    for (unsigned int i = 0; i < children.size(); i++) {
      QStandardItem *newitem = NewItem(children.at(i).Name);
      if (newitem) item->appendRow(newitem);
    }

    return item;
  }
}


void MainWindow::onTreeClicked(const QModelIndex &index)
{
  char Date[30];
  if (index.isValid()) {
    QStandardItem *item = fModel->itemFromIndex(index);
    int            compId;
    compId = item->data().toInt();
    vector<ComponentDB::compActivity> tests;
    DbGetAllTests(fdb, compId, tests, STDigital, true);
    fModelTable->removeRows(0, fModelTable->rowCount()); // clear the table in order to update it
    fModelTable->removeRows(1, fModelTable->rowCount());
    for (unsigned int d = 0; d < tests.size(); d++) {
      ComponentDB::compActivity act;
      act = tests.at(d);

      strftime(Date, 30, "%b %d, %Y", localtime(&act.EndDate));
      std::string            ts           = string(Date);
      QStandardItem *        testitemtype = new QStandardItem(QString::fromStdString(act.Typename));
      QStandardItem *        testitemdata = new QStandardItem(QString::fromStdString(ts));
      QList<QStandardItem *> items;
      items.push_back(testitemtype);
      items.push_back(testitemdata);
      fModelTable->appendRow(items);
    }
    std::cout << " the data number is" << compId << std::endl;
  }
}
