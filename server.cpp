#include "server.h"

server::server()
{
    isStart=0;
    port = 50000;
    tcpServer = new QTcpServer();
    connect(tcpServer, SIGNAL(newConnection()), SLOT(addClient()));
}

bool server::isServerStarted()
{
    return isStart;
}

int server::serverStart()
{
    if (!tcpServer->listen(QHostAddress::Any, port))
       {
            isStart=0;
            return -1;
       }
    isStart=1;
    return 0;
}

void server::serverStop()
{
    tcpServer->close();
    isStart=0;
}

void server::serverQuit()
{

}

int server::clientsValue()
{
    return clientsList.length();
}

void server::addClient()
{
    QTcpSocket *client = tcpServer->nextPendingConnection();
    clientsList.push_back(client);
    qDebug() << "Connected clients:" << clientsList.length();

    /*connect(client, SIGNAL(disconnected()), this, SLOT(removeClient()));*/
    connect(client, SIGNAL(disconnected()), this, SLOT(removeClient()));
    connect(client, SIGNAL(readyRead()), this, SLOT(parseData()));
    emit changedClient();
    /*QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    client->write(data);*/
}

void server::parseData()
{
    QTcpSocket *client = (QTcpSocket*) sender();
    qDebug() << "New request from:" << client->peerAddress();
    qreal dane=random()/100000;
    qDebug() << "sending:" << dane;

    QByteArray data;
       QDataStream out(client);
       out.setVersion(QDataStream::Qt_4_6);
       out<<dane;
       client->write(data);
}

void server::removeClient()
{
    QTcpSocket *client = (QTcpSocket*) sender();
    int index = clientsList.indexOf(client);
    clientsList.removeAt(index);
    qDebug() << "removed client:" << client->peerAddress() ;
    emit changedClient();
}
