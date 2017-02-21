#ifndef UNIXSOCKET_H
#define UNIXSOCKET_H

#include <QtWidgets>
#include <QLocalSocket>
#include "renderarea.h"

class UNIXSocket : public QObject
{

     Q_OBJECT

public:
    UNIXSocket(RenderArea *render);
    void connectServer();
    void disconnectServer();

public slots:
    void readData();
    void displayError(QLocalSocket::LocalSocketError socketError);

signals:
    void changeState(bool isConnect, QString message);


public:
    char *getTheBuffePtr();

private:
    QLocalSocket *Socket;
    char theBuffer[1024];
    RenderArea *theRender;
    bool isConnected;
};

#endif // UNIXSOCKET_H
