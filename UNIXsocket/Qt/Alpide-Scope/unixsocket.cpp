#include "unixsocket.h"


UNIXSocket::UNIXSocket(RenderArea *render)
{
    theRender = render;
    Socket = new QLocalSocket();
    Socket->connectToServer("/tmp/matestream", QIODevice::ReadOnly);
    while( ! Socket->waitForConnected()) {
        qDebug("Not connected!");
        QThread::sleep(5);
        Socket->connectToServer("/tmp/matestream", QIODevice::ReadOnly);
    }
    connect(Socket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(Socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));
}


void UNIXSocket::readData()
{
    int bytesAvailable = Socket->bytesAvailable();
    if(bytesAvailable > 0) {
        int bytesRead = Socket->read(theBuffer, 1024);
        theRender->fillMatrix(theBuffer, bytesRead);
    }
}

void UNIXSocket::displayError(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        break;
    case QLocalSocket::ConnectionRefusedError:
        break;
    case QLocalSocket::PeerClosedError:
        break;
    default:
        break;
    }
}

char * UNIXSocket::getTheBuffePtr()
{
    return( theBuffer );
}
