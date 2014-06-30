#include "server.h"


qreal server::getData() const
{
    return data;
}

void server::setData(const qreal &value)
{
    data = value;
}
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
    char clientID=(clientsList.indexOf(client))+49;
    emit changedClient(clientID);
}

void server::parseData()
{
    QTcpSocket *client = (QTcpSocket*) sender();
    if(clientsList.indexOf(client)>4){
        client->close();
        client->disconnect();
        return;
    }
    QByteArray clientData=client->readLine();

    qDebug() << "New data:" << clientData;
    if(clientData==QByteArray("ok")){
        char clientID=(clientsList.indexOf(client))+49;
        emit readDataByClient(clientID);
        qDebug() << "client:" << clientID;
        return;
    }
    qDebug() << "New request from:" << client->peerAddress();
    qDebug() << "sending:" << data;

    QByteArray data_to_send;
    QDataStream out(&data_to_send, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);

    out<<data;
    client->write(data_to_send);
}

void server::removeClient()
{
    QTcpSocket *client = (QTcpSocket*) sender();
    int index = clientsList.indexOf(client);
    clientsList.removeAt(index);
    qDebug() << "removed client:" << client->peerAddress() ;
    char clientID=(clientsList.indexOf(client))+49+6;
    emit changedClient(clientID);
}
