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
    virtual void SaveSettings(QString &opname, int &hicid, int &counter);


private:
Ui::TestSelection *ui;
Dialog *missingsettings;



private slots:
void popupmessage(QString m);


};

#endif // TESTSELECTION_H
