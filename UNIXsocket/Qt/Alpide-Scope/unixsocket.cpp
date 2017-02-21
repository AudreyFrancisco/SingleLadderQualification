#include "unixsocket.h"


UNIXSocket::UNIXSocket(RenderArea *render)
{
    theRender = render;
    Socket = new QLocalSocket();
    isConnected = false;
}

void UNIXSocket::connectServer()
{
    int faultCounter = 12;
    Socket->connectToServer("/tmp/matestream", QIODevice::ReadOnly);
    while( ! Socket->waitForConnected() && faultCounter-- > 0 ) {
        qDebug("Not connected!");
        emit changeState(false, "Try the connection...");
        QThread::sleep(5);
        Socket->connectToServer("/tmp/matestream", QIODevice::ReadOnly);
    }
    connect(Socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));
    connect(Socket, SIGNAL(disconnected()), this, SLOT(disconnectServer()));
    if(faultCounter > 0) {
        connect(Socket, SIGNAL(readyRead()), this, SLOT(readData()));
        isConnected = true;
        emit changeState(isConnected, "Connected !");
    } else {
        isConnected = false;
        emit changeState(isConnected, "Failed to connect !");
    }
    return;
}

void UNIXSocket::readData()
{
    int bytesAvailable = Socket->bytesAvailable();
    if(bytesAvailable > 0) {
        int bytesRead = Socket->read(theBuffer, 1024);
        theRender->fillMatrix(theBuffer, bytesRead);
        emit changeState(isConnected, "Read data.");

    }
}

void UNIXSocket::disconnectServer()
{
    isConnected = false;
    emit changeState(isConnected, "Close the connection");
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
