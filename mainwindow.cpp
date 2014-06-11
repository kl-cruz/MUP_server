#include "mainwindow.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "iostream"
#include <QMessageBox>

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
    nodesTab->setEnabled(true);
    portBox->setEnabled(false);
    baudRateBox->setEnabled(false);
    fillStatusTab(true);

}

void MainWindow::stopConnection()
{
    nodesTab->setEnabled(false);
    portBox->setEnabled(true);
    baudRateBox->setEnabled(true);
    fillStatusTab(false);
}

void MainWindow::onReadyRead()
{
    if (port->bytesAvailable()) {
        //QString fromPort=QString::fromLatin1(port->readAll());
        readData->append(port->readAll());
        prepareReadData();
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
    }

    if (port->isOpen() && port->queryMode() == QextSerialPort::Polling)
        timer->start();
    else
        timer->stop();
}

void MainWindow::onServerStartStopButtonClicked()
{

    if(tcpServer->isServerStarted())
    {
        tcpServer->serverStop();
        statusBar->showMessage("WYłączono serwer");
        serverStartStopButton->setText("Uruchom serwer");
    } else {
        tcpServer->serverStart();
        statusBar->showMessage("Uruchomiono serwer");
        serverStartStopButton->setText("Wyłącz serwer");
    }

}

void MainWindow::prepareReadData()
{
    //prepare data
    int startFrom=readData->indexOf('!',0);
    if(startFrom==-1)
        return;
    readData->remove(0,startFrom);
    readData->squeeze();

    QString ipAddress;
    QString sensorNumberValue;
    QString sensorValue;
    int howMuch=readData->count();
    int i;
    for(i=0;i<howMuch;++i)
    {
        int newData=readData->indexOf('!',i+1);
        if(newData==-1)
            break;
        char readChar=readData->at(i);
        if(readChar!='!')
            break;
        while(true)
        {

            ++i;
            if(i>=howMuch)
                break;
            readChar=readData->at(i);
            if(readChar!='>')
                ipAddress.append(QChar(readChar));
            else
                break;
        }

        while(true)
        {
            ++i;
            if(i>=howMuch)
                break;
            readChar=readData->at(i);
            if(readChar!=':')
                sensorNumberValue.append(QChar(readChar));
            else
                break;
        }
        while(true)
        {
            ++i;
            if(i>=howMuch)
                break;
            readChar=readData->at(i);
            if(readChar!='<')
                sensorValue.append(QChar(readChar));
            else
                break;
        }
        if(i>=howMuch)
            break;
        //add od update data
        bool isExists=false;
        bool isExistsValue=false;
        int j;
        for(j=0;j<nodes.count();++j)
        {
            if(nodes[j].IP.compare(ipAddress)==0){
                isExists=true;
                break;}
        }
        if(isExists){
            int k;
            for(k=0;k<nodes[j].data.count();++k)
            {
                if(nodes[j].data[k].first==sensorNumberValue){
                    isExistsValue=true;
                    break;}
            }
            if(isExistsValue)
            {
                nodes[j].data[k].second=sensorValue;
            }
            else
            {

                nodes[j].data.append(qMakePair(sensorNumberValue,sensorValue));
            }
            nodes[j].lastMeasure=QDateTime::currentDateTime();
        }
        else{
            nodes.append(nodeData());
            nodes[nodes.count()-1].IP=ipAddress;
            nodes[nodes.count()-1].data.append(qMakePair(sensorNumberValue,sensorValue));
            nodes[nodes.count()-1].lastMeasure=QDateTime::currentDateTime();
        }

    }


    readData->remove(0,i);
    readData->squeeze();
    updateData();

}

void MainWindow::updateData()
{
    //nodesBox->clear();
    for(int i=0;i<nodes.count();++i)
    {
        bool isExists=false;
        for(int j=0;j<nodesBox->count();++j)
        {
            if(nodesBox->itemText(j).compare(nodes[i].IP)==0)
                isExists=true;
        }
        if(!isExists)
            nodesBox->addItem(nodes[i].IP,i);
    }
    nodeChanged(nodesBox->currentIndex());
}

void MainWindow::clearData()
{
    nodes.clear();
    nodesBox->clear();
    QStringList list;
    QStringListModel *model = new QStringListModel;
    model->setStringList(list);
    nodeSensorsInfoListView->setModel(model);
    lastMeasureLabel->setText("");
}

void MainWindow::changedClientHandle()
{
    QString str;

    str.append(QString("%1").arg(tcpServer->clientsValue()));
    serverClientsValueLabel->setText(str);
}

void MainWindow::nodeChanged(int idx)
{
    QStringList list;
    if(idx=-1){
        if(nodesBox->count()>0)
            idx=0;
        else
            return;
    }
    for(int i=0;i<nodes[nodesBox->currentIndex()].data.count();++i)
    {
        list << transtaleDataFromNode(nodes[nodesBox->currentIndex()].data[i]);
    }
    QStringListModel *model = new QStringListModel;
    model->setStringList(list);
    nodeSensorsInfoListView->setModel(model);
    lastMeasureLabel->setText(nodes[nodesBox->currentIndex()].lastMeasure.toString());
}

QString MainWindow::transtaleDataFromNode(QPair<QString, QString> data)
{
    int indexData=data.first.toInt();
    QString result="";
    switch (indexData){
        case 0:
        result="Nazwa node'a:                                "+data.second;
            break;
    case 1:
        result="Temperatura mikrokontrolera: "+data.second+"\u2103";
        break;
    case 2:
        result="Temperatura 1:                               "+data.second+"\u2103";
        break;
    case 3:
        result="Temperatura 2:                               "+data.second+"\u2103";
        break;
    case 4:
        result="Temperatura 3:                               "+data.second+"\u2103";
        break;
    case 5:
        result="Temperatura 4:                               "+data.second+"\u2103";
        break;
    case 6:
        result="Wilgotność 1:                                  "+data.second+"%";
        break;
    case 7:
        result="Wilgotność 2:                                  "+data.second+"%";
        break;
    }
    return result;
}

void MainWindow::createUI()
{
    /*Create canvas form*/

    if (this->objectName().isEmpty())
        this->setObjectName(QStringLiteral("MainWindow"));
    this->resize(640, 700);
    QString windowTitle="SMIW";
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

    //start tab
    serverTab=new QWidget();
    serverTab->setObjectName(QStringLiteral("serverTab"));
    tabWidget->addTab(serverTab, QString());
    tabWidget->setTabText(tabWidget->indexOf(serverTab), QApplication::translate("MainWindow", "Witaj!", 0));


    serverOpenClosePortLabel=new QLabel("Uruchamianie serwera");

    serverClientsLabel=new QLabel("Podłączonych klientów");
    serverClientsValueLabel=new QLabel("0");

    serverStartStopButton = new QPushButton("Uruchom serwer");

    serverTabLayout=new QFormLayout();
    serverTabLayout->addRow(new QLabel("Połączenie z serwerem"));
    serverTabLayout->addRow(serverOpenClosePortLabel,serverStartStopButton);
    serverTabLayout->addRow(serverClientsLabel,serverClientsValueLabel);
    serverTab->setLayout(serverTabLayout);



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

    openClosePortButton = new QPushButton("Uruchom transmisję");

    optionsLayout=new QFormLayout;
    optionsLayout->addRow(new QLabel("Opcje połączenia RS232"));
    optionsLayout->addRow(portLabel,portBox);
    optionsLayout->addRow(baudRateLabel,baudRateBox);
    optionsLayout->addRow(openClosePortLabel,openClosePortButton);
    optionsTab->setLayout(optionsLayout);


    //nodetab
    nodesTab=new QWidget();
    nodesTab->setObjectName(QStringLiteral("nodesTab"));
    nodesTab->setEnabled(false);
    tabWidget->addTab(nodesTab, QString());
    tabWidget->setTabText(tabWidget->indexOf(nodesTab), QApplication::translate("MainWindow", "nodesTab", 0));

    nodesBox=new QComboBox();
    lastMeasureLabel=new QLabel();

    nodeSensorsInfoListView=new QListView();
    nodeSensorsInfoScrollBar=new QScrollBar(nodeSensorsInfoListView);
    nodeSensorsInfoListView->setVerticalScrollBar(nodeSensorsInfoScrollBar);

    clearMeasureButton=new QPushButton();
    clearMeasureButton->setText("Wyczyść wszystkie odczyty");

    nodesTabLayout=new QFormLayout;
    nodesTabLayout->addRow(new QLabel("Węzeł o IP:"),nodesBox);
    nodesTabLayout->addRow(new QLabel("Ostatni odczyt:"),lastMeasureLabel);
    nodesTabLayout->addRow(new QLabel("Informacje z węzła:"),nodeSensorsInfoListView);
    nodesTabLayout->addRow(new QLabel(""),clearMeasureButton);

    nodesTab->setLayout(nodesTabLayout);

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

    //QMetaObject::connectSlotsByName(this);

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
    connect(serverStartStopButton, SIGNAL(clicked()), SLOT(onServerStartStopButtonClicked()));

    if(port->queryMode()==QextSerialPort::Polling){
        connect(timer, SIGNAL(timeout()), SLOT(onReadyRead()));
    }
    connect(port, SIGNAL(readyRead()), SLOT(onReadyRead()));

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(nodesBox, SIGNAL(currentIndexChanged(int)),SLOT(nodeChanged(int)));
    connect(clearMeasureButton,SIGNAL(clicked()),SLOT(clearData()));

}

void MainWindow::prepareServer()
{
    tcpServer = new server();
    connect(tcpServer, SIGNAL(changedClient()),SLOT(changedClientHandle()));

}

