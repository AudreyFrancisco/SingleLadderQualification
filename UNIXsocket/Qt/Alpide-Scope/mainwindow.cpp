#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMenuBar>
#include <QStatusBar>


#include "renderarea.h"
#include "unixsocket.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
 //   ui->setupUi(this);

    renderArea = new RenderArea(this);
    theUNIXSocket = new UNIXSocket(renderArea);

    createFrame();
    setWindowTitle(tr("Alpide Scope"));

    renderArea->repaint();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createFrame()
{
    // add status bar message
    showStatus(false,"Alpide Scope ...");

    chipSelector = new QSpinBox;
    chipSelector->setRange(0, 15);
    chipSelector->setSingleStep(1);
    chipSelector->setValue(0);
    connect(chipSelector, SIGNAL(valueChanged(int)), this, SLOT(changeSelectedChip(int)));

    lab1 = new QLabel("Selected chip to view");
  //  lab1->text("Selected chip to view");

    // actions
    connectAction = new QAction(QIcon("images/network-wireless.png"), tr("&Connect"), this);
//    connectAction->setIcon(QIcon("images/network-wireless.png"));
    connectAction->setShortcut(QKeySequence::New);
    connectAction->setStatusTip(tr("Connect to the UNIX socket"));
    connect(connectAction, SIGNAL(triggered()), this, SLOT(connectSocket()));

    exitAction = new QAction(QIcon("images/system-log-out.png"), tr("E&xit"), this);
   // connectAction->setIcon(QIcon("images/system-log-out.png"));
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    aboutAction = new QAction(tr("&About..."), this);
    aboutAction->setStatusTip(tr("Show the About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));


    // Menu
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(connectAction);
    separatorAction = fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    fileMenu = menuBar()->addMenu(tr("&Help"));
    fileMenu->addAction(aboutAction);

    // ToolBar
    fileToolBar = addToolBar(tr("&Actions"));
    fileToolBar->addAction(connectAction);
    fileToolBar->addWidget(lab1);

    fileToolBar->addWidget(chipSelector);
    fileToolBar->addSeparator();
    fileToolBar->addAction(exitAction);
    fileToolBar->setMovable(false);


    // Connections
    connect( theUNIXSocket, SIGNAL(changeState(bool, QString)), this, SLOT(showStatus(bool, QString)) );

}

void MainWindow::connectSocket()
{
    theUNIXSocket->connectServer();
    return;
}

void MainWindow::changeSelectedChip(int chipId)
{
    renderArea->setPaintedChip(chipId);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    //event->ignore();
    return;
}


/************************************ showMessage ************************************/

void  MainWindow::showStatus(bool aState, QString msg )
{
  // display message on main window status bar
  statusBar()->showMessage( msg );
}


void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About ALICE Alpide Scope"),
               tr("<h2>Alpide Scope v.0.1</h2>"
                  "<p>Copyright &copy; 2017 INFN Sez. BARI -ITALY"
                  "<p>Alpide Scope is an application able to display "
                  "Alpide pixel hits map in real time throw the "
                  "UNIX socket connection ... "
                  "auth. A.Franco."));
}
