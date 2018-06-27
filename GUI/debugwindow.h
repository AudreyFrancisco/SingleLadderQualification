#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include "mainwindow.h"
#include <map>
#include <QDialog>
#include <QString>
#include <vector>

namespace Ui {
class DebugWindow;
}

class DebugWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DebugWindow(QWidget *parent = 0);
    ~DebugWindow();

signals:
	void selectScan(TScanType scanType);

private:
    Ui::DebugWindow *ui;
    std::map<TScanType, QString> ScanName;
    void addModuleScans();
    void displayScans(std::vector<TScanType> scansForSetup);
    void buildScanNameMap();

private slots:
	void getScans(int nSetup);
    void passSelectedScan();
};



#endif // DEBUGWINDOW_H
