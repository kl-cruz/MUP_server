#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>

class server : public QObject
{
    Q_OBJECT
private:
    int port;
    QList<QTcpSocket *> clientsList;
    QTcpServer *tcpServer;
    bool isStart;
public:
    server();
    bool isServerStarted();
    int serverStart();
    void serverStop();
    void serverQuit();
    int clientsValue();
public slots:
    void addClient();
    void parseData();
    void removeClient();
signals:
    void changedClient();
};

#endif // SERVER_H
