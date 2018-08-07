#include "debugwindow.h"
#include "ui_debugwindow.h"

#include "mainwindow.h"
#include <QFileDialog>
#include <map>
#include <vector>

DebugWindow::DebugWindow(QWidget *parent) : QDialog(parent), ui(new Ui::DebugWindow)
{
  ui->setupUi(this);
  ui->scanSelector->addItem("Load config file first", -1);

  connect(ui->startButton, SIGNAL(clicked()), this, SLOT(passSelectedScan()));
  connect(ui->configLoadButton, SIGNAL(clicked()), this, SLOT(selectAndLoadConfig()));
  connect(this, SIGNAL(selectScan(TScanType)), this->parent(), SLOT(doDebugScan(TScanType)));
  connect(this, SIGNAL(passConfigFile(QByteArray)), this->parent(),
          SLOT(loadConfigFile(QByteArray)));
  connect(this->parent(), SIGNAL(deviceLoaded(TDeviceType)), this,
          SLOT(displayDeviceAndScans(TDeviceType)));
}

DebugWindow::~DebugWindow() { delete ui; }

void DebugWindow::selectAndLoadConfig()
{
  QString configFilename = QFileDialog::getOpenFileName(this, "Select config file",
                                                        QDir::currentPath(), "All files (*.*)");
  if (!configFilename.isNull()) {
    ui->configFileDisplay->setPlainText(configFilename);
    emit passConfigFile(configFilename.toLatin1());
  }
}

void DebugWindow::displayDeviceAndScans(TDeviceType deviceType)
{
  ui->deviceDisplay->setText("Device type selected: " + DeviceNameMap.at(deviceType));
  ui->scanSelector->clear();
  switch (deviceType) {
  case TYPE_POWER:
    addFastPowerScan();
    break;
  case TYPE_OBHIC:
  case TYPE_HALFSTAVE:
  case TYPE_MLHALFSTAVE:
  case TYPE_MLSTAVE:
  case TYPE_HALFSTAVERU:
    addStandardScans();
    addLocalBusScan();
    break;
  default:
    addStandardScans();
    break;
  }
}

void DebugWindow::addStandardScans()
{
  for (std::pair<TScanType, QString> scanIterator : ScanNameMap) {
    if (scanIterator.first == STLocalBus) {
      continue;
    }
    else {
      ui->scanSelector->addItem(scanIterator.second, scanIterator.first);
    }
  }
}

void DebugWindow::addFastPowerScan()
{
  ui->scanSelector->addItem(ScanNameMap.at(STFastPowerTest), STFastPowerTest);
}

void DebugWindow::addLocalBusScan()
{
  ui->scanSelector->addItem(ScanNameMap.at(STLocalBus), STLocalBus);
}

void DebugWindow::passSelectedScan()
{
  int selectedScan = ui->scanSelector->currentData().toInt();
  if (selectedScan == -1) {
    std::cout << "Select a scan" << std::endl;
  }
  else {
    emit selectScan((TScanType)selectedScan);
  }
}
