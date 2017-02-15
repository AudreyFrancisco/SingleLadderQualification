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

public slots:
    void readData();
    void displayError(QLocalSocket::LocalSocketError socketError);

public:
    char *getTheBuffePtr();

private:
    QLocalSocket *Socket;
    char theBuffer[1024];
    RenderArea *theRender;

};

#endif // UNIXSOCKET_H
