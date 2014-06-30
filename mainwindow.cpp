#include "mainwindow.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "iostream"
#include <QMessageBox>
#include <qthread.h>

#include "src/qextserialport.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    prepareServer();
    createUI();
}

MainWindow::~MainWindow()
{

}

void MainWindow::fillControls()
{

    baudRateBox->addItem("19200",BAUD19200);
    baudRateBox->addItem("38400",BAUD38400);
    baudRateBox->addItem("57600",BAUD57600);
    baudRateBox->addItem("115200",BAUD115200);
    baudRateBox->setCurrentIndex(2);

     foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
     {

         portBox->addItem(info.portName);
     }
     //make sure user can input their own port name!
     portBox->setEditable(true);

     fillStatusTab(false);
}

void MainWindow::fillStatusTab(bool status)
{
    if(status)
    {
        portEnabledValue->setText("aktywny");
        portEnabledValue->setStyleSheet("background-color:green;color:white");
        setPortValue->setText(portBox->currentText());
        setBaudRateValue->setText(QString("%1").arg(baudRateBox->itemData(baudRateBox->currentIndex()).toInt()));
    }
    else
    {
        portEnabledValue->setText("nieaktywny");
        portEnabledValue->setStyleSheet("background-color:red;color:black");
        setPortValue->setText("");
        setBaudRateValue->setText("");
    }
}

void MainWindow::startConnection()
{
    portBox->setEnabled(false);
    baudRateBox->setEnabled(false);
    fillStatusTab(true);

}

void MainWindow::stopConnection()
{
    portBox->setEnabled(true);
    baudRateBox->setEnabled(true);
    fillStatusTab(false);
}

void MainWindow::onReadyRead()
{
    if (port->bytesAvailable()) {
        fromPort+=QString::fromLatin1(port->readAll());
    }
    QString test=fromPort.simplified();

    QRegExp rx(".*rh=\\[([-+]?\\d+[.]?\\d+)\\]");
    int pos = 0;

    while ((pos = rx.indexIn(test, pos)) != -1) {
        fromPort="";
        if(tcpServer){
            tcpServer->setData(rx.cap(1).toDouble());
            devValueValueLabel->setText(rx.cap(1));
        }
        pos += rx.matchedLength();
    }
}

void MainWindow::onPortAddedOrRemoved()
{
    QString current = portBox->currentText();

    portBox->blockSignals(true);
    portBox->clear();
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        portBox->addItem(info.portName);

    portBox->setCurrentIndex(portBox->findText(current));

    portBox->blockSignals(false);
}


void MainWindow::onPortNameChanged(const QString &name)
{
    if (port->isOpen()) {
        port->close();
    }
}

void MainWindow::onBaudRateChanged(int idx)
{
    port->setBaudRate((BaudRateType)baudRateBox->itemData(idx).toInt());
}



void MainWindow::onOpenCloseButtonClicked()
{
    if (!port->isOpen()) {
        port->setPortName(portBox->currentText());
        port->open(QIODevice::ReadWrite);
        if (port->isOpen()) {
        statusBar->showMessage("Port jest otwarty");
        openClosePortButton->setText("Zakończ transmisję");
        tabWidget->setCurrentIndex(2);
        startConnection();
        if(tcpServer->isServerStarted())
        {
            tcpServer->serverStop();
            statusBar->showMessage("Wyłączono serwer");
        } else {
            tcpServer->serverStart();
            statusBar->showMessage("Uruchomiono serwer");
            tcpServer->setData(2.534);
        }
        }
        else
        {
            QMessageBox::information(NULL, "Ostrzeżenie!", "Port jest nieaktywny!");
        }
    }
    else {
        lineStatusTimer->stop();
        port->close();
        statusBar->showMessage("Port jest zamknięty");
        openClosePortButton->setText("Uruchom transmisję");
        stopConnection();
        tcpServer->serverStop();
        statusBar->showMessage("Wyłączono serwer");
    }

    if (port->isOpen() && port->queryMode() == QextSerialPort::Polling)
        timer->start();
    else
        timer->stop();
}


void MainWindow::changedClientHandle(char client)
{
    QString str;

    str.append(QString("%1").arg(tcpServer->clientsValue()));
    serverClientsValueLabel->setText(str);
    qDebug() << "client changed Handle:" << client;
    char clear=49+6;
    port->write(&clear,1);
    clear++;
    port->write(&clear,1);
    clear++;
    port->write(&clear,1);
    clear++;
    port->write(&clear,1);
    clear++;
    port->write(&clear,1);
    port->write(&client,1);

}

void MainWindow::readClientHandle(char client)
{
    qDebug() << "client change:" << client;
    client+=5;
    port->write(&client,1);
    QThread::msleep ( 100 ) ;
    client-=5;
    port->write(&client,1);
}
void MainWindow::createUI()
{
    /*Create canvas form*/

    if (this->objectName().isEmpty())
        this->setObjectName(QStringLiteral("MainWindow"));
    this->resize(640, 700);
    QString windowTitle="MUP Server";
    this->setWindowTitle(QApplication::translate("MainWindow", windowTitle.toStdString().c_str(), 0));

    //end

    //DynamicData

    readData=new QByteArray();
    //

    centralWidget = new QWidget(this);
    centralWidget->setObjectName(QStringLiteral("centralWidget"));

    gridLayout = new QGridLayout(centralWidget);
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));


    tabWidget = new QTabWidget(centralWidget);
    tabWidget->setObjectName(QStringLiteral("tabWidget"));


    //options tab
    optionsTab=new QWidget();
    optionsTab->setObjectName(QStringLiteral("optionsTab"));
    tabWidget->addTab(optionsTab, QString());
    tabWidget->setTabText(tabWidget->indexOf(optionsTab), QApplication::translate("MainWindow", "Opcje", 0));

    baudRateLabel=new QLabel("Baudrate połączenia");
    portLabel=new QLabel("Port podłączonego urządzenia");

    openClosePortLabel=new QLabel("Uruchamianie portu");



    baudRateBox= new QComboBox();

    portBox= new QComboBox();

    serverClientsLabel=new QLabel("Podłączonych klientów");
    serverClientsValueLabel=new QLabel("0");

    devValueLabel=new QLabel("Obecny odczyt z czujnika");
    devValueValueLabel=new QLabel("0.00");

    openClosePortButton = new QPushButton("Uruchom serwer");

    optionsLayout=new QFormLayout;
    optionsLayout->addRow(new QLabel("Opcje połączenia RS232"));
    optionsLayout->addRow(portLabel,portBox);
    optionsLayout->addRow(baudRateLabel,baudRateBox);
    optionsLayout->addRow(serverClientsLabel,serverClientsValueLabel);
    optionsLayout->addRow(devValueLabel,devValueValueLabel);
    optionsLayout->addRow(openClosePortLabel,openClosePortButton);

    optionsTab->setLayout(optionsLayout);

    //portinfo
    portinfoTab=new QWidget();
    portinfoTab->setObjectName(QStringLiteral("portinfoTab"));
    tabWidget->addTab(portinfoTab, QString());
    tabWidget->setTabText(tabWidget->indexOf(portinfoTab), QApplication::translate("MainWindow", "Stan obecnego połączenia", 0));


    portEnabledLabel=new QLabel("Stan połączenia");
    portEnabledValue=new QLabel("nieaktywne");
    setBaudRateLabel=new QLabel("Baudrate połączenia");
    setPortLabel=new QLabel("Port podłączonego urządzenia");


    setBaudRateValue=new QLabel("wartość");
    setPortValue=new QLabel("wartość");



    portinfoLayout=new QFormLayout;
    portinfoLayout->addRow(new QLabel("Stan uruchomionego połączenia"));
    portinfoLayout->addRow(portEnabledLabel,portEnabledValue);
    portinfoLayout->addRow(setPortLabel,setPortValue);
    portinfoLayout->addRow(setBaudRateLabel,setBaudRateValue);

    portinfoTab->setLayout(portinfoLayout);


    gridLayout->addWidget(tabWidget, 0, 0, 1, 1);

    this->setCentralWidget(centralWidget);
    statusBar = new QStatusBar(this);
    statusBar->setObjectName(QStringLiteral("statusBar"));
    this->setStatusBar(statusBar);
    statusBar->showMessage("Program gotowy do pracy");

    fillControls();

    timer = new QTimer(this);
    timer->setInterval(5);
    //! [1]
    PortSettings settings = {BAUD57600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 20};
    port = new QextSerialPort(portBox->currentText(), settings, QextSerialPort::EventDriven);//QextSerialPort::Polling);
    lineStatusTimer=new QTimer(this);
    lineStatusTimer->setInterval(100);
    //! [1]

    enumerator = new QextSerialEnumerator(this);
    enumerator->setUpNotifications();

    connect(baudRateBox, SIGNAL(currentIndexChanged(int)), SLOT(onBaudRateChanged(int)));
    connect(portBox, SIGNAL(editTextChanged(QString)), SLOT(onPortNameChanged(QString)));
    connect(openClosePortButton, SIGNAL(clicked()), SLOT(onOpenCloseButtonClicked()));

    if(port->queryMode()==QextSerialPort::Polling){
        connect(timer, SIGNAL(timeout()), SLOT(onReadyRead()));
    }
    connect(port, SIGNAL(readyRead()), SLOT(onReadyRead()));

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

}

void MainWindow::prepareServer()
{
    tcpServer = new server();
    connect(tcpServer, SIGNAL(changedClient(char)),SLOT(changedClientHandle(char)));
    connect(tcpServer, SIGNAL(readDataByClient(char)),SLOT(readClientHandle(char)));


}

