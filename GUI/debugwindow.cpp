#include "debugwindow.h"
#include "ui_debugwindow.h"

#include "mainwindow.h"
#include <map>
#include <vector>

DebugWindow::DebugWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugWindow)
{
	buildScanNameMap();

    ui->setupUi(this);
    ui->setupSelector->addItem("", 0);
    ui->setupSelector->addItem("HIC module", 1);
    ui->scanSelector->addItem("Select setup first", -1);

    connect(ui->setupSelector, SIGNAL(currentIndexChanged(int)),
    	this, SLOT(getScans(int)));
    connect(ui->startButton, SIGNAL(clicked()),
    	this, SLOT(passSelectedScan()));
    connect(this, SIGNAL(selectScan(TScanType)),
    	this->parent(), SLOT(doDebugScan(TScanType)));
}

DebugWindow::~DebugWindow()
{
    delete ui;
}

void DebugWindow::getScans(int nSetup)
{
	ui->scanSelector->clear();
	switch(nSetup)
	{
		case 0: ui->scanSelector->addItem("Select setup first", -1);
				break;
		case 1:	addModuleScans();
				break;
	}
		
}

void DebugWindow::passSelectedScan()
{
	int selectedScan = ui->scanSelector->currentData().toInt();
	if (selectedScan == -1) {
		std::cout << "Select a scan" << std::endl;
	} else {
		emit selectScan((TScanType) selectedScan);
	}
}

void DebugWindow::addModuleScans()
{
	std::vector<TScanType> scansForSetup;
	scansForSetup.push_back(STPower);

	displayScans(scansForSetup);
}

void DebugWindow::displayScans(const std::vector<TScanType> scansForSetup)
{
	ui->scanSelector->addItem("", -1);
	for(TScanType scanType : scansForSetup)
	{
		ui->scanSelector->addItem(ScanName[scanType], scanType);
	}

}

void DebugWindow::buildScanNameMap()
{
	ScanName[STPower] = "Power test";
}
