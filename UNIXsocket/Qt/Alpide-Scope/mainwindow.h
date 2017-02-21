#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSpinBox>
#include <QMainWindow>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class RenderArea;
class UNIXSocket;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
   void showStatus( bool, QString );        // show message on status bar
   void connectSocket();
   void changeSelectedChip(int);
    void showAbout();

private:
    Ui::MainWindow *ui;
    void createFrame();
    void closeEvent(QCloseEvent *event);

public:
    RenderArea *renderArea;
    UNIXSocket *theUNIXSocket;


private:
    QAction *connectAction;
    QAction *exitAction;
    QAction *aboutAction;
    QAction *separatorAction;

    QMenu *fileMenu;
    QToolBar *fileToolBar;

    QSpinBox *chipSelector;
    QLabel *lab1;
};

#endif // MAINWINDOW_H
