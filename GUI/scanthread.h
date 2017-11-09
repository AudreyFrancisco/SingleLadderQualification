#ifndef SCANTHREAD_H
#define SCANTHREAD_H
#include <QThread>
#include <QtCore>
class ScanThread:public QObject
{
  Q_OBJECT



 signals:
  void resultReady();

  public slots:
    void process();
 public:
    ScanThread();
    //void run();

};

#endif // SCANTHREAD_H
