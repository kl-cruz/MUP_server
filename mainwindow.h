#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QStatusBar>
#include <QGridLayout>
#include <QPushButton>
#include <QListView>
#include <QLabel>
#include <QGroupBox>
#include <QTextEdit>
#include <QScrollBar>
#include <QFormLayout>
#include <QTimer>
#include <QTime>
#include <QComboBox>
#include <QListView>
#include <QByteArray>
#include <QVector>
#include <QStringListModel>
#include "server.h"

class QextSerialPort;
class QextSerialEnumerator;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
public slots:
    void onReadyRead();
    void fillControls();
    void fillStatusTab(bool status);
    void startConnection();
    void stopConnection();

    void onPortNameChanged(const QString &name);
    void onBaudRateChanged(int idx);
    void onOpenCloseButtonClicked();
    void onServerStartStopButtonClicked();

    void prepareReadData();
    void updateData();
    void clearData();
    void changedClientHandle();

    void nodeChanged(int idx);
    QString transtaleDataFromNode(QPair<QString,QString> data);


    void onPortAddedOrRemoved();
private slots:

private:
    void createUI();
    void prepareServer();


    server *tcpServer;

    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QTimer *elapsedTimeTimer;

    QStatusBar *statusBar;

    //starttab
    QWidget *serverTab;
    QLabel *serverClientsLabel;
    QLabel *serverClientsValueLabel;

    QLabel *serverOpenClosePortLabel;
    QPushButton *serverStartStopButton;
    QFormLayout *serverTabLayout;

    //nodestab
    QWidget *nodesTab;
    QComboBox *nodesBox;
    QListView *nodeSensorsInfoListView;
    QScrollBar *nodeSensorsInfoScrollBar;
    QFormLayout* nodesTabLayout;
    QPushButton *clearMeasureButton;
    QLabel *lastMeasureLabel;



    void teachConsoleAddText(QString text);
    void teachConsoleAddOnlyPlainText(QString text);

    //OptionsTab
    QWidget *optionsTab;
    QLabel *portLabel;
    QComboBox *portBox;
    QLabel *baudRateLabel;
    QComboBox *baudRateBox;
    QLabel *openClosePortLabel;
    QPushButton *openClosePortButton;
    QFormLayout *optionsLayout;

    //Port info Tab
    QWidget *portinfoTab;
    QLabel *portEnabledLabel;
    QLabel *portEnabledValue;
    QLabel *setPortLabel;
    QLabel *setPortValue;
    QLabel *setBaudRateLabel;
    QLabel *setBaudRateValue;

    QFormLayout *portinfoLayout;

    //Port
    QTimer *timer;
    QTimer *lineStatusTimer;
    QextSerialPort *port;
    QextSerialEnumerator *enumerator;

    //data
    class nodeData{
    public:
        QString IP;
        QVector< QPair <QString,QString> > data;
        QDateTime lastMeasure;
    };

    QList<nodeData> nodes;
    QByteArray *readData;
};

#endif // MAINWINDOW_H
