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
    qreal data;
public:
    server();
    bool isServerStarted();
    int serverStart();
    void serverStop();
    void serverQuit();
    int clientsValue();
    qreal getData() const;
    void setData(const qreal &value);

public slots:
    void addClient();
    void parseData();
    void removeClient();
signals:
    void changedClient(char client);
    void readDataByClient(char client);
};

#endif // SERVER_H
