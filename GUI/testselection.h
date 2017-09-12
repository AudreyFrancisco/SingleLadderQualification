#ifndef TESTSELECTION_H
#define TESTSELECTION_H
#include<TFifoTest.h>
#include <QDialog>
#include <QPushButton>
#include "dialog.h"

namespace Ui {
class TestSelection;
}

class TestSelection : public QDialog
{
    Q_OBJECT

public:

    explicit TestSelection(QWidget *parent = 0);
    ~TestSelection();
    virtual void SaveSettings(QString &opname, QString &hicid, int &counter, int &lid, int &memberid);
    virtual void connectlocationcombo(std::vector<pair<std::string,int>> floc);
    virtual int GetLocationID(){return locid;}

private:
Ui::TestSelection *ui;
Dialog *missingsettings;
int locid;
int memid;

public slots:
virtual void getlocationcombo(int value);
virtual int GetMemberID();

private slots:
void popupmessage(QString m);


};

#endif // TESTSELECTION_H

