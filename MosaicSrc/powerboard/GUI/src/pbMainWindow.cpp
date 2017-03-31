/*
 * Copyright (C) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/  	 
 *
 * ====================================================
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2017.
 *
 */
#include <QApplication>
#include <QMessageBox> 
#include <QAction>
#include <QMenuBar>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QSplitter>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QGroupBox>
#include <QPushButton>
#include <QTextStream>
#include "mexception.h"
#include "pbMainWindow.h"
#include "optionsDialog.h"
#include "ledRed.xpm"
#include "ledGrey.xpm"

static const char *WINDOW_TITLE = "Power Board Control";


pbMainWindow::pbMainWindow( QWidget* parent, Qt::WFlags fl )
    : QMainWindow( parent, fl )
{
	setWindowTitle(QString(WINDOW_TITLE));
    (void)statusBar();

	// Voltage input string validator
	VbiasValidator = new setValidator(-6.0, 0.0, 2);	// from -6.0 to 0.0 volts 2 decimals
	VsetValidator = new setValidator(1.5, 2.0, 3);	// from 1.5 to 2.0 volts 3 decimals
	IsetValidator = new setValidator(0.0, 2.0, 3);	// from 0 to 2.0 Amp 3 decimals

    // actions
    QAction *fileSaveAction = new QAction("&Save", this );
    fileSaveAction->setStatusTip("Save configuration");

    QAction *fileSaveAsAction = new QAction("S&ave as...", this );
    fileSaveAsAction->setStatusTip("Save configuration with specific name");

    QAction *fileOpenAction = new QAction("&Open...", this );
    fileOpenAction->setStatusTip("Open Configuration");

    QAction *fileExitAction = new QAction("E&xit", this );
    fileExitAction->setStatusTip("Exit application");

    QAction *configureAction = new QAction("&Configure", this );
    configureAction->setStatusTip("Open the configure dialog");

    QAction *storeAction = new QAction("&Store Vset", this );
    storeAction->setStatusTip("Store all voltage settings in the board NVRAM");

	// File Menu
	QMenu *fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction(fileOpenAction);
	fileMenu->addAction(fileSaveAction);
	fileMenu->addAction(fileSaveAsAction);
	fileMenu->addAction(configureAction);
	fileMenu->addSeparator();
	fileMenu->addAction(fileExitAction);

	// Tools Menu
	QMenu *toolsMenu = menuBar()->addMenu("&Tools");
	toolsMenu->addAction(storeAction);

	// Central Widget & Vertical layout
	centralWidgetPtr = new QWidget( this );
    setCentralWidget( centralWidgetPtr );
    QVBoxLayout* verticalLayout = new QVBoxLayout(centralWidget()); 

	verticalLayout->setContentsMargins(0,0,0,0);
	verticalLayout->addWidget(topStatusBar());

	// Channels scroll area
	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	verticalLayout->addWidget(scrollArea);
	QWidget *w = new QWidget(this);
	scrollArea->setWidget(w);
	ChannelSetOnMapper =  new QSignalMapper;
	ChannelSetOffMapper =  new QSignalMapper;
	ChannelVsetMapper =  new QSignalMapper;
	ChannelIsetMapper =  new QSignalMapper;

	QGridLayout* channelLayout = new QGridLayout(w);
	int ch=0;
	for (int r=0; r<4; r++)
		for (int c=0; c<2; c++)
			channelLayout->addWidget(channel(ch++), r, c);


    // signals and slots connections
	connect( fileOpenAction, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
	connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( fileSave() ) );
	connect( fileSaveAsAction, SIGNAL( activated() ), this, SLOT( fileSaveAs() ) );
	connect( fileExitAction, SIGNAL( activated() ), this, SLOT( close() ) );
	connect( configureAction, SIGNAL( activated() ), this, SLOT( configure() ) );
	connect( storeAction, SIGNAL( activated() ), this, SLOT( storeVset() ) );
	connect( ChannelSetOnMapper, SIGNAL(mapped(int)), this, SLOT(channelSetON(int)));
	connect( ChannelSetOffMapper, SIGNAL(mapped(int)), this, SLOT(channelSetOFF(int)));
	connect( ChannelVsetMapper, SIGNAL(mapped(int)), this, SLOT(channelVset(int)));
	connect( ChannelIsetMapper, SIGNAL(mapped(int)), this, SLOT(channelIset(int)));

	statusBar()->showMessage("Ready");

	setFixedWidth(channelLayout->sizeHint().width() + scrollArea->verticalScrollBar()->width());
    resize( QSize(400, 600).expandedTo(minimumSizeHint()) );

	// refresh timer
	refreshTimer = new QTimer( this );
	connect( refreshTimer, SIGNAL(timeout()), this, SLOT(refreshMonitor()));


	board = new PBif();
	pb = board->pb;
	boardAddress=QString("0.0.0.0");
	setOnline(false);
}



/*
 *  Destroys the object and frees any allocated resources
 */
pbMainWindow::~pbMainWindow()
{
    // no need to delete child widgets, Qt does it all for us
}

void pbMainWindow::setIPaddress(QString add)
{
	boardAddress = add;

	try{
		board->setIPaddress(add.toAscii().data());
	} catch (std::exception& e) {
		qDebug("Error connecting board at address %s", add.toAscii().data());
		setOnline(false);
		QMessageBox::critical ( this, WINDOW_TITLE, 
			"Board comunication error", 
			"OK");
		return;
	}

	try{
		if (!pb->isReady()){
			throw MIPBusErrorWrite("Power board comunication error");			
		}
	} catch (std::exception& e) {
		qDebug("Power Board is not connected or is off");
		setOnline(false);
		QMessageBox::critical ( this, WINDOW_TITLE, 
			"Power Board is not connected or is off", 
			"OK");
		return;
	}


	setOnline(true);
	refreshSettings();
	refreshMonitor();
}

void pbMainWindow::configure()
{
	optionsDialog cfg(this);
	cfg.setAddress(boardAddress);
	if (cfg.exec() == QDialog::Rejected)
		return;

	setIPaddress(cfg.address());	
}


QWidget *pbMainWindow::topStatusBar()
{
	QFontMetrics metrics(QApplication::font());
	QPalette Pal(palette());
	Pal.setColor(QPalette::Background, Qt::black);
	Pal.setColor(QPalette::Foreground, Qt::red);

    QFrame *statusFrame = new QFrame( this );
    statusFrame->setFrameShape( QFrame::StyledPanel );
    statusFrame->setFrameShadow( QFrame::Raised );
    QHBoxLayout *statusFrameLayout = new QHBoxLayout( statusFrame ); 
	statusFrameLayout->setContentsMargins(3,3,3,3);
	statusFrameLayout->setSpacing(6);

	ledRedPixmap = QPixmap(ledRed);
	ledGreyPixmap = QPixmap(ledGrey);

	QLabel *label;
	for (int i=0; i<NUM_TSENSORS; i++){

		label = new QLabel("<h2> T" + QString::number(i+1) + "</h2>", this);
		label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		statusFrameLayout->addWidget(label);

		temperatureLCD[i] = new QLCDNumber( this );
		temperatureLCD[i]->setNumDigits(4);
		temperatureLCD[i]->setAutoFillBackground(true);
		temperatureLCD[i]->setPalette(Pal);
		temperatureLCD[i]->setSegmentStyle(QLCDNumber::Flat);
		temperatureLCD[i]->setSmallDecimalPoint(true);
		temperatureLCD[i]->setFixedSize(QSize(75,30));
		statusFrameLayout->addWidget(temperatureLCD[i]);
	}

	// Vbias
	label = new QLabel(" Vbias", this);
	statusFrameLayout->addWidget(label);	
	VbiasText = new QLineEdit(this);
	VbiasText->setInputMask("-9.99");
	VbiasText->setText("-0.00");
	VbiasText->setFixedWidth(metrics.width("-9.99") + 10);
	VbiasText->setValidator(VbiasValidator);
	statusFrameLayout->addWidget(VbiasText);	
	VbiasON = false;
	connect(VbiasText, SIGNAL(editingFinished()), this, SLOT(VbiasSet())); 
	VbiasLED = new QLabel(NULL);
	VbiasLED->setPixmap(ledGreyPixmap);
	statusFrameLayout->addWidget(VbiasLED);
	statusFrameLayout->addStretch(10);	

	// Global ON/OFF 
	QPushButton *onPushButton = new QPushButton("ALL ON");
	statusFrameLayout->addWidget(onPushButton);
	connect(onPushButton, SIGNAL(clicked()), this, SLOT(allON()));
	connect(onPushButton, SIGNAL(clicked()), this, SLOT(refreshMonitor()));
	QPushButton *offPushButton = new QPushButton("ALL OFF");
	statusFrameLayout->addWidget(offPushButton);
	connect(offPushButton, SIGNAL(clicked()), this, SLOT(allOFF()));
	connect(offPushButton, SIGNAL(clicked()), this, SLOT(refreshMonitor()));

	return statusFrame;
}

QLCDNumber *pbMainWindow::newLargeLCD()
{
	QPalette Pal(palette());
	Pal.setColor(QPalette::Background, Qt::black);
	Pal.setColor(QPalette::Foreground, Qt::green);

	QLCDNumber *lcd;
	lcd = new QLCDNumber( this );
	lcd->setNumDigits(4);
	lcd->setAutoFillBackground(true);
	lcd->setPalette(Pal);
	lcd->setSegmentStyle(QLCDNumber::Flat);
	lcd->setSmallDecimalPoint(true);
	lcd->setFixedSize(QSize(100,40));
	return lcd;
}


QWidget *pbMainWindow::channel(int ch)
{
	QLabel *label;
	int col = 0;
	QFontMetrics metrics(QApplication::font());

	QGroupBox *channelGroup = new QGroupBox("Channel " + QString::number(ch+1), this);
	QGridLayout *gbox = new QGridLayout(channelGroup);
	
	// LED
	channelLED[ch] = new QLabel(NULL);
	channelLED[ch]->setPixmap(ledGreyPixmap);
	channelON[ch] = false;
	gbox->addWidget(channelLED[ch], 0, col++, 2, 1);

	// Voltage monitor
	VoltLCD[ch] = newLargeLCD();
	gbox->addWidget(VoltLCD[ch], 0, col++, 2, 1);
	label = new QLabel("<h2>V </h2>", this);
	gbox->addWidget(label, 0, col++, 2, 1);

	// Current monitor
	AmpLCD[ch] = newLargeLCD();
	gbox->addWidget(AmpLCD[ch], 0, col++, 2, 1);
	label = new QLabel("<h2>A </h2>", this);
	gbox->addWidget(label, 0, col++, 2, 1);

	// Voltage & current set
	label = new QLabel("Vset:", this);
	label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	gbox->addWidget(label, 0, col, 1, 1);
	label = new QLabel("Iset:", this);
	label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	gbox->addWidget(label, 1, col++, 1, 1);
	VsetText[ch] = new QLineEdit(this);
	VsetText[ch]->setInputMask("9.999");
	VsetText[ch]->setText("0.000");
	VsetText[ch]->setFixedWidth(metrics.width("9.999") + 10);
	VsetText[ch]->setValidator(VsetValidator);
	gbox->addWidget(VsetText[ch], 0, col, 1, 1);
	IsetText[ch] = new QLineEdit(this);
	IsetText[ch]->setInputMask("9.999");
	IsetText[ch]->setText("2.000");
	IsetText[ch]->setFixedWidth(metrics.width("9.999") + 10);
	IsetText[ch]->setValidator(IsetValidator);
	gbox->addWidget(IsetText[ch], 1, col++, 1, 1);
	
	// ON/OFF pushbutton
	QPushButton *onPushButton = new QPushButton("ON");
	gbox->addWidget(onPushButton, 0, col, 1, 1);
	QPushButton *offPushButton = new QPushButton("OFF");
	gbox->addWidget(offPushButton, 1, col++, 1, 1);
	
	// Signal to mappers
	ChannelSetOnMapper->setMapping(onPushButton, ch);
	connect(onPushButton, SIGNAL(clicked()), ChannelSetOnMapper, SLOT(map()));
	connect(onPushButton, SIGNAL(clicked()), this, SLOT(refreshMonitor()));
	ChannelSetOffMapper->setMapping(offPushButton, ch);
	connect(offPushButton, SIGNAL(clicked()), ChannelSetOffMapper, SLOT(map()));
	connect(offPushButton, SIGNAL(clicked()), this, SLOT(refreshMonitor()));
	ChannelVsetMapper->setMapping(VsetText[ch], ch);
	connect(VsetText[ch], SIGNAL(editingFinished()), ChannelVsetMapper, SLOT(map()));
	connect(VsetText[ch], SIGNAL(editingFinished()), this, SLOT(refreshMonitor()));
	ChannelIsetMapper->setMapping(IsetText[ch], ch);
	connect(IsetText[ch], SIGNAL(editingFinished()), ChannelIsetMapper, SLOT(map()));

	return channelGroup;
}

/*
	SLOTS
*/
void pbMainWindow::channelSetON(int ch)
{
	channelIset(ch);
	try{
		pb->onVout(ch);
	} catch (std::exception& e) {	
		comErrorExit(e);
	}
}

void pbMainWindow::channelSetOFF(int ch)
{
	try{
		pb->setIth(ch, 0.0);
	} catch (std::exception& e) {	
		comErrorExit(e);
	}
}

void pbMainWindow::channelVset(int ch)
{
	QString s;
	float V = VsetText[ch]->text().toFloat();
	
	if (V<0)
		V = 0;
	if (V>2.0)
		V = 2.0;
	s.setNum(V, 'F', 3);
 
	try{
		pb->setVout(ch, V);
	} catch (std::exception& e) {	
		comErrorExit(e);
	}

	refreshSettings();
}

void pbMainWindow::storeVset()
{
	try{
		pb->storeAllVout();
	} catch (std::exception& e) {	
		comErrorExit(e);
	}
}

void pbMainWindow::channelIset(int ch)
{
	QString s;
	float I = IsetText[ch]->text().toFloat();
	
	if (I<0)
		I = 0;
	if (I>2.0)
		I = 2.0;
	s.setNum(I, 'F', 3);
	IsetText[ch]->setText(s);
 
	try{
		pb->setIth(ch, I);
	} catch (std::exception& e) {	
		comErrorExit(e);
	}

}

void pbMainWindow::VbiasSet()
{
	float V = VbiasText->text().toFloat();

	try{
		pb->setVbias(V);
	} catch (std::exception& e) {	
		comErrorExit(e);
	}

	enVbias(V!=0.0);
}

void pbMainWindow::enVbias(bool en)
{
	VbiasON = en;
	VbiasLED->setPixmap(en ? ledRedPixmap : ledGreyPixmap);

	try{
		pb->enVbias(VbiasON);
	} catch (std::exception& e) {	
		comErrorExit(e);
	}
}

void pbMainWindow::refreshMonitor()
{
	QString s;
	powerboard::pbstate pbStat;

	try{
		pb->getState(&pbStat, powerboard::GetMonitor);
	} catch (std::exception& e) {	
		comErrorExit(e);
	}
	
	// Temperature sensors
	for (int i=0; i<NUM_TSENSORS; i++){
		float t = pbStat.T[i];
		if (t<-9.99 || t>99.99)
			s = "-----";
		else
			s.setNum(t, 'F', 2);
		temperatureLCD[i]->display(s);
	}

	// Channel ON
	for (int i=0; i<NUM_CHANNELS; i++){
		if (pbStat.chOn & 1<<i){
			channelON[i] = false;
			channelLED[i]->setPixmap(ledGreyPixmap);
		} else {
			channelON[i] = true;
			channelLED[i]->setPixmap(ledRedPixmap);
		}
	}

	// Output voltage
	for (int i=0; i<NUM_CHANNELS; i++){
		float v = pbStat.Vmon[i];
		if (v<0 || v>9.99)
			s = "-----";
		else
			s.setNum(v, 'F', 3);
		VoltLCD[i]->display(s);
	}

	// Output currents
	for (int i=0; i<NUM_CHANNELS; i++){
		float I = pbStat.Imon[i];
		if (I<0 || I>9.99)
			s = "-----";
		else
			s.setNum(I, 'F', 3);
		AmpLCD[i]->display(s);
	}
}

/*
	read back settings from power board
*/
void pbMainWindow::refreshSettings()
{
	QString s;
	powerboard::pbstate pbStat;

	try{
		pb->getState(&pbStat, powerboard::GetSettings);
	} catch (std::exception& e) {	
		comErrorExit(e);
	}

	// Output voltage
	for (int i=0; i<NUM_CHANNELS; i++){
		float v = pbStat.Vout[i];
		s.setNum(v, 'f', 3);
		VsetText[i]->setText(s);
	}
}

/*
	Switch ON all channels
*/
void pbMainWindow::allON()
{	
	for (int i=0; i<NUM_CHANNELS; i++)
		channelIset(i);

	try{
		pb->onAllVout();
	} catch (std::exception& e) {	
		comErrorExit(e);
	}
	VbiasSet();	
}

/*
	Switch OFF all channels
*/
void pbMainWindow::allOFF()
{	
	enVbias(false);
	for (int i=0; i<NUM_CHANNELS; i++)
		channelSetOFF(i);
}

/*
	Confiuration load/save
*/
void pbMainWindow::fileSave()
{
	if (cfgFilename.isEmpty()){
		fileSaveAs();
		return;
	}
	saveConfiguration();	
}

void pbMainWindow::fileSaveAs()
{
	QString filename;

	filename = QFileDialog::getSaveFileName(this, "Save Configuration", ".", "Config file (*.cfg)");
	if (filename.isEmpty())
		return;

	if (filename.right(4)!=".cfg")
		filename.append(".cfg");

	cfgFilename = filename;
	saveConfiguration();	
}

void pbMainWindow::saveConfiguration()
{
	qDebug("Saving configuration into file %s", cfgFilename.toAscii().data());

	QFile of(cfgFilename);
	if (!of.open(QFile::WriteOnly | QFile::Truncate)){
		QMessageBox::critical ( this, WINDOW_TITLE, 
					"Error opening file<br>", 
					QMessageBox::Ok, Qt::NoButton,
					Qt::NoButton );
		return;
	}

	// Create XML config document
	QTextStream stream( &of );
	QDomDocument doc( "CFG" );
	QDomElement root = doc.createElement( "PowerBoardSetup" );
	doc.appendChild( root );

	QDomElement mosaic = doc.createElement( "MOSAIC" );;
	mosaic.setAttribute ( "IP", boardAddress );
	root.appendChild( mosaic );

	QDomElement bias = doc.createElement( "Bias" );;
	bias.setAttribute ( "Vbias", VbiasText->text() );
	bias.setAttribute ( "ON", VbiasON );
	root.appendChild( bias );

	for (int ch=0; ch<NUM_CHANNELS; ch++){
		QString chN;
		chN.setNum(ch);
		QDomElement channel = doc.createElement( "Channel_" + chN );

		channel.setAttribute ( "Vset", VsetText[ch]->text() );
		channel.setAttribute ( "Iset", IsetText[ch]->text() );
		channel.setAttribute ( "ON",   channelON[ch] );
		root.appendChild( channel );
	}

	// Save the document
	QString xml = doc.toString();
	stream << xml;
	of.close();
}



void pbMainWindow::fileOpen(char *fname)
{
	QString filename;

	if (fname && strlen(fname)>0){
		filename = fname;
	} else {
		filename = QFileDialog::getOpenFileName(this, "Load Configuration", ".", "Config file (*.cfg)");
		if (filename.isEmpty())
			return;

		if (filename.right(4)!=".cfg")
			filename.append(".cfg");
	}
	
	qDebug("Reading configuration from file %s", filename.toAscii().data());

	// Open file
	QFile ifile(filename);
	if (!ifile.open(QIODevice::ReadOnly)){
		QMessageBox::critical ( this, WINDOW_TITLE, 
					"Error opening file<br>", 
					QMessageBox::Ok, Qt::NoButton,
					Qt::NoButton );
		return;
	}

	// read CFG file
	QDomDocument doc;
	if ( !doc.setContent( &ifile ) ){ 
		QMessageBox::critical ( this, WINDOW_TITLE, 
					"Error reading file<br>", 
					QMessageBox::Ok, Qt::NoButton,
					Qt::NoButton );
		ifile.close();
		return;
	}
	ifile.close();

	// parse file
	QDomElement root = doc.documentElement();
	if (root.tagName()!= "PowerBoardSetup"){
		QMessageBox::critical ( this, WINDOW_TITLE, 
					"Root tag != PowerBoardSetup<br>", 
					QMessageBox::Ok, Qt::NoButton,
					Qt::NoButton );
		return;
	}

	// Read MOSAIC board address
	if (!XMLreadMOSAIC(root))
		goto cfgReadError;

	// Read Vbias settings
	if (!XMLreadBias(root))
		goto cfgReadError;

	// read channel settings
	for (int i=0; i<NUM_CHANNELS; i++){
		if (!XMLreadChannel(root, i))
			goto cfgReadError;
	}

	cfgFilename = filename;
	refreshMonitor();
	return;

cfgReadError:
	QMessageBox::critical ( this, WINDOW_TITLE, 
			"Error reading configuration<br>", 
			QMessageBox::Ok, QMessageBox::NoButton,
			QMessageBox::NoButton );
	return;
}

bool pbMainWindow::XMLreadChannel(QDomElement &root, int n)
{
	QString chN, val;
	chN.setNum(n);
	chN = "Channel_" + chN;
	
	QDomElement channel = XMLgetTag(root, chN);
	if (channel.isNull())
		return FALSE;

	val = channel.attribute ( "Vset", "0.000" );
	VsetText[n]->setText(val); 
	channelVset(n);
	val = channel.attribute ( "Iset", "0.000" ); 
	IsetText[n]->setText(val); 
	channelIset(n);
	val = channel.attribute ( "ON", "0" ); 
	if (val=="1")
		channelSetON(n);
	else
		channelSetOFF(n);

	return true;
}

bool pbMainWindow::XMLreadBias(QDomElement &root)
{
	QString val;

	// Read Vbias settings
	QDomElement bias = XMLgetTag(root, "Bias");
	if (bias.isNull())
		return false;

	val = bias.attribute ( "Vbias", "-0.00" );
	VbiasText->setText(val);
	VbiasSet();
	val = bias.attribute ( "ON", "0" ); 
	enVbias(val=="1");
	return true;
}

bool pbMainWindow::XMLreadMOSAIC(QDomElement &root)
{
	QString val;

	// Read Vbias settings
	QDomElement mosaic = XMLgetTag(root, "MOSAIC");
	if (mosaic.isNull())
		return false;

	val = mosaic.attribute ( "IP", "0.0.0.0" );
	setIPaddress(val);
	return true;
}



QDomElement pbMainWindow::XMLgetTag(QDomElement &e, QString tagName)
{
	QDomElement ret;

	// enter in TAG
	QDomNodeList nodeTag = e.elementsByTagName ( tagName );
	if (nodeTag.count() ==0){
		qDebug("tag %s not found", (char*) tagName.data());
		return ret;
	}

	QDomNode d;
	d = nodeTag.item(0);
	if (d.isNull()){
		qDebug("tag %s is void", (char*) tagName.data());
		return ret;
	}

	ret = d.toElement(); // try to convert the node to an element.
	if( ret.isNull() ) {
		qDebug("tag element %s is void", (char*) tagName.data());
		return ret;
	}

	return ret;
}

void pbMainWindow::setOnline(bool online)
{
	boardIsOnline = online;
	centralWidgetPtr->setEnabled(online);
	if (online)
		refreshTimer->start(1000);
	else
		refreshTimer->stop();
}


void pbMainWindow::comErrorExit(std::exception& e)
{
	qDebug(e.what());

	setOnline(false);
	QMessageBox::critical ( this, WINDOW_TITLE, 
		"Board comunication error", 
		"Exit");
	exit(1);
}

