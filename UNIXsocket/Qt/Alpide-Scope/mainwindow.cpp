#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "renderarea.h"
#include "unixsocket.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    renderArea = new RenderArea(this);
    theUNIXSocket = new UNIXSocket(renderArea);
    setWindowTitle(tr("Alpide Scope"));
    renderArea->repaint();

}

MainWindow::~MainWindow()
{
    delete ui;
}
