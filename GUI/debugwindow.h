#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include "TConfig.h"
#include "mainwindow.h"
#include <QDialog>
#include <QString>
#include <map>
#include <vector>

namespace Ui {
  class DebugWindow;
}

class DebugWindow : public QDialog {
  Q_OBJECT

public:
  explicit DebugWindow(QWidget *parent = 0);
  ~DebugWindow();

public slots:
  void displayDeviceAndScans(TDeviceType deviceType);

signals:
  void selectScan(TScanType scanType);
  void passConfigFile(QByteArray configFilename);

private:
  Ui::DebugWindow *ui;
  void             addStandardScans();
  void             addFastPowerScan();
  void             addLocalBusScan();

  const std::map<TScanType, QString> ScanNameMap = {
      {STPower, "Power"},     {STFifo, "Fifo"},           {STLocalBus, "LocalBusTest"},
      {STDigital, "Digital"}, {STDigitalWF, "DigitalWF"}, {STThreshold, "Threshold"},
      {STVCASN, "VCASN"},     {STITHR, "ITHR"},           {STNoise, "Noise"},
      {STReadout, "Readout"}, {STEndurance, "Endurance"}, {STFastPowerTest, "FastPowerTest"},
      {STDctrl, "Dctrl"},     {STEyeScan, "EyeScan"}};

  const std::map<TDeviceType, QString> DeviceNameMap = {{TYPE_CHIP, "CHIP"},
                                                        {TYPE_TELESCOPE, "TELESCOPE"},
                                                        {TYPE_OBHIC, "OBHIC"},
                                                        {TYPE_IBHIC, "IBHIC"},
                                                        {TYPE_CHIP_MOSAIC, "CHIP_MOSAIC"},
                                                        {TYPE_HALFSTAVE, "HALFSTAVE"},
                                                        {TYPE_HALFSTAVERU, "HALFSTAVERU"},
                                                        {TYPE_MLHALFSTAVE, "MLHALFSTAVE"},
                                                        {TYPE_MLSTAVE, "MLSTAVE"},
                                                        {TYPE_IBHICRU, "IBHICRU"},
                                                        {TYPE_ENDURANCE, "ENDURANCE"},
                                                        {TYPE_POWER, "POWER"},
                                                        {TYPE_UNKNOWN, "UNKNOWN"}};

private slots:
  void passSelectedScan();
  void selectAndLoadConfig();
};

#endif // DEBUGWINDOW_H
