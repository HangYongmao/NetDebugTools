#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置标题
    MainWindow::setWindowTitle("网络调试助手");
    // 设置标题的图标
    MainWindow::setWindowIcon(QIcon(":/images/disconnect.png"));

    // Server 客户端列表
    InitClientListTableWidgetUI();
    // UDP 客户端列表
    InitUDPClientTableWidget();

    clientSocket = new QTcpSocket();

    tcpServer    = new QTcpServer();

    // 连接信号槽
    // Client
    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::client_Socket_Read_Data);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MainWindow::client_Socket_DisConnected);

    typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
    connect(clientSocket, static_cast<QAbstractSocketErrorSignal>(&QTcpSocket::error), this, &MainWindow::client_Socket_Error);

    // Server
    connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::server_New_connect);

    // UDP
    udpSocket = new QUdpSocket();

    QString verticalBarStyleSheet="QScrollBar{background:transparent; width: 10px;}"
                                  "QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
                                  "QScrollBar::handle:hover{background:gray;}"
                                  "QScrollBar::sub-line{background:transparent;}"
                                  "QScrollBar::add-line{background:transparent;}";
    ui->tableWidget->verticalScrollBar()->setStyleSheet(verticalBarStyleSheet);
    ui->tableWidget_UDP->verticalScrollBar()->setStyleSheet(verticalBarStyleSheet);

    ui->textEdit_Client_Receive->verticalScrollBar()->setStyleSheet(verticalBarStyleSheet);
    ui->textEdit_Client_Send->verticalScrollBar()->setStyleSheet(verticalBarStyleSheet);
    ui->textEdit_Server_Receive->verticalScrollBar()->setStyleSheet(verticalBarStyleSheet);
    ui->textEdit_Server_Send->verticalScrollBar()->setStyleSheet(verticalBarStyleSheet);
    ui->textEdit_UDP_Receive->verticalScrollBar()->setStyleSheet(verticalBarStyleSheet);
    ui->textEdit_UDP_Send->verticalScrollBar()->setStyleSheet(verticalBarStyleSheet);
}

MainWindow::~MainWindow()
{
    tcpServer->close();
    tcpServer->deleteLater();
    delete this->clientSocket;
    server_Close_All_Client();
    delete ui;
}

// Client 连接/断开
void MainWindow::on_pushButton_Clien_Connect_clicked()
{
    if (ui->pushButton_Clien_Connect->text() == "连接")
    {
        // 获取IP地址
        QString IP = ui->comboBox_Client_IP->currentText();

        // 获取端口号
        int Port = ui->comboBox_Client_Port->currentText().toInt();

        // 取消已有的连接
        clientSocket->abort();

        // 连接服务器
        clientSocket->connectToHost(IP, Port);

        // 等待连接成功
        if (!clientSocket->waitForConnected(3000))
        {
            qDebug() << "Connection Failed.";
            return;
        }
        qDebug() << "Connection Successfully.";

        ui->pushButton_Client_Send->setEnabled(true);
        ui->pushButton_Clien_Connect->setText("断开");
        ui->comboBox_Client_IP->setEnabled(false);
        ui->comboBox_Client_Port->setEnabled(false);

        // 设置标题的图标
        MainWindow::setWindowIcon(QIcon(":/images/connect.png"));
    }
    else
    {
        // 断开连接
        clientSocket->disconnectFromHost();

        ui->pushButton_Clien_Connect->setText("连接");
        ui->pushButton_Client_Send->setEnabled(false);
        ui->comboBox_Client_IP->setEnabled(true);
        ui->comboBox_Client_Port->setEnabled(true);

        // 设置标题的图标
        MainWindow::setWindowIcon(QIcon(":/images/disconnect.png"));
    }
}

// Client 发送数据
void MainWindow::on_pushButton_Client_Send_clicked()
{
    clientSocket->write(ui->textEdit_Client_Send->toPlainText().toLatin1());
    clientSocket->flush();
}

// Client 接收数据
void MainWindow::client_Socket_Read_Data()
{
    QByteArray buffer;
    buffer = clientSocket->readAll();
    if (buffer.isEmpty())
        return;

    if (ui->checkBox_Client_Receive_Time->isChecked())
    {
        if (ui->textEdit_Client_Receive->toPlainText().isEmpty())
            ui->textEdit_Client_Receive->append(QString("[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));
        else
            ui->textEdit_Client_Receive->append(QString("\r\n[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));
    }
    if (ui->radioButton_Client_Receive_A->isChecked())
        ui->textEdit_Client_Receive->append(buffer);
    else
        ui->textEdit_Client_Receive->append(QByteArrayToHex(buffer));
}

// Client 断开Socket
void MainWindow::client_Socket_DisConnected()
{
    ui->pushButton_Client_Send->setEnabled(false);
    ui->pushButton_Clien_Connect->setText("连接");
    ui->comboBox_Client_IP->setEnabled(true);
    ui->comboBox_Client_Port->setEnabled(true);

    // 设置标题的图标
    MainWindow::setWindowIcon(QIcon(":/images/disconnect.png"));
    qDebug() << "Client DisConnected.";
}

// Client 错误信息
void MainWindow::client_Socket_Error(QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError;
}

// Server 打开/关闭
void MainWindow::on_pushButton_Server_Open_clicked()
{
    if (ui->pushButton_Server_Open->text() == "打开")
    {
        int Port = ui->comboBox_Server_Port->currentText().toInt();

        // 监听指定端口
        if (!tcpServer->listen(QHostAddress::Any, Port))
        {
            qDebug() << tcpServer->errorString();
            return;
        }

        ui->pushButton_Server_Open->setText("关闭");
        qDebug() << "Listen Successfully.";
    }
    else
    {
        // 断开所有客户端
        server_Close_All_Client();

        // 关闭监听
        tcpServer->close();
        ui->pushButton_Server_Open->setText("打开");
        ui->pushButton_Server_Send->setEnabled(false);
    }
}

// Server 发送数据
void MainWindow::on_pushButton_Server_Send_clicked()
{
    int clientCount=0;
    for (int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        if (ui->tableWidget->item(i, 0) != NULL)
        {
            if (ui->tableWidget->item(i, 0)->checkState() == Qt::Checked)
            {
                tcpClientSocketList.at(i)->write(ui->textEdit_Server_Send->toPlainText().toLatin1());
                clientCount++;
            }
        }
    }
    if (clientCount == 0)
        qDebug() << "No Client Select.";
}

// Server 创建新连接
void MainWindow::server_New_connect()
{
    // 获取客户端连接
    QTcpSocket *serverSocket = tcpServer->nextPendingConnection();
    tcpClientSocketList.append(serverSocket);
    InsertClientIntoTableWidget(serverSocket->peerAddress().toString().mid(serverSocket->peerAddress().toString().indexOf(QRegExp("\\d+"))), \
                                serverSocket->peerPort());
//    qDebug() << serverSocket->peerAddress().toString().mid(serverSocket->peerAddress().toString().indexOf(QRegExp("\\d+")));
//    qDebug() << serverSocket->peerPort();

    // 连接QTcpSocket的信号槽
    connect(serverSocket, &QTcpSocket::readyRead, this, &MainWindow::server_Read_Data);
    connect(serverSocket, &QTcpSocket::disconnected, this, &MainWindow::server_Disconnected);

    // 发送按键使能
    ui->pushButton_Server_Send->setEnabled(true);
    qDebug() << "A New Client Connect." << serverSocket->peerAddress().toString().mid(serverSocket->peerAddress().toString().indexOf(QRegExp("\\d+")))\
             << ":" << serverSocket->peerPort();
}

// Server 接收数据
void MainWindow::server_Read_Data()
{
    QByteArray buffer;
    QTcpSocket * clientSocket = qobject_cast<QTcpSocket *>(sender());
    buffer = clientSocket->readAll();
    if (buffer.isEmpty())
        return;

    // 在开头添加空行
    if (ui->checkBox_Server_Receive_Time->isChecked() || ui->checkBox_Server_For_Client->isChecked())
    {
        if (!ui->textEdit_Server_Receive->toPlainText().isEmpty())
            ui->textEdit_Server_Receive->append("");
    }

    // 显示接收时间
    if (ui->checkBox_Server_Receive_Time->isChecked())
    {
        ui->textEdit_Server_Receive->append(QString("[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));
    }


    // 显示客户端来源
    if (ui->checkBox_Server_For_Client->isChecked())
    {
        QString IP = clientSocket->peerAddress().toString().mid(clientSocket->peerAddress().toString().indexOf(QRegExp("\\d+")));
        int Port = clientSocket->peerPort();
        ui->textEdit_Server_Receive->append(QString("[From %1:%2]").arg(IP).arg(Port));
    }

    // 显示Hex
    if (ui->radioButton_Server_Receive_A->isChecked())
        ui->textEdit_Server_Receive->append(buffer);
    else
        ui->textEdit_Server_Receive->append(QByteArrayToHex(buffer));
}

// Server 断开Socket
void MainWindow::server_Disconnected()
{
    QTcpSocket * disConnectSocket = qobject_cast<QTcpSocket *>(sender());

    qDebug() << "Server Disconnected." << disConnectSocket->peerAddress().toString().mid(disConnectSocket->peerAddress().toString().indexOf(QRegExp("\\d+")))\
             << ":" << disConnectSocket->peerPort();
    RemoveClientRow(disConnectSocket->peerAddress().toString().mid(disConnectSocket->peerAddress().toString().indexOf(QRegExp("\\d+"))), \
                    disConnectSocket->peerPort());

    if (tcpClientSocketList.count() == 0)
        ui->pushButton_Server_Send->setEnabled(false);
}

// QByteArray转Hex 含空格
QByteArray MainWindow::QByteArrayToHex(QByteArray byteArray)
{
    QByteArray buffer;
    const char HEX[] = "0123456789ABCDEF";

    for (int i=0; i<byteArray.size()-1; i++)
    {
        buffer.append(HEX[byteArray.at(i)/16]);
        buffer.append(HEX[byteArray.at(i)%16]);
        buffer.append(" ");
    }
    buffer.append(HEX[byteArray.at(byteArray.size()-1)/16]);
    buffer.append(HEX[byteArray.at(byteArray.size()-1)%16]);
    return buffer;
}

// Server 客户端列表
void MainWindow::InitClientListTableWidgetUI()
{
    // 设置列数, 行数动态增加
    ui->tableWidget->setColumnCount(2);

    // 创建表头
    QStringList header;
    header << "IP" << "Port";
    ui->tableWidget->setHorizontalHeaderLabels(header);

    // 列宽
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width()-ui->tableWidget->width()/3);
    ui->tableWidget->setColumnWidth(1, ui->tableWidget->width()/3);

    // 表格禁止编辑
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 隐藏行表头
    ui->tableWidget->verticalHeader()->setVisible(false);

    // 隔行换色
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setStyleSheet("alternate-background-color: rgb(240, 240, 240);");

    // 列宽禁止拖动
    // ui->tableWidget->horizontalHeader()->setDisabled(true);

    // 行高禁止拖动
    ui->tableWidget->verticalHeader()->setDisabled(true);

    // 设置选中模式为选中行
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 设置选中单个
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // 点击表时不对表头行光亮（获取焦点）
    ui->tableWidget->horizontalHeader()->setHighlightSections(false);
}

// 控制TableWidget的列宽
void MainWindow::paintEvent(QPaintEvent *event)
{
    // 2:1
    int width=0;
    if (event != NULL)
    {
        if (ui->tableWidget->verticalScrollBar()->isVisible())
        {
            width = ui->tableWidget->width()-ui->tableWidget->verticalScrollBar()->width();
        }
        else
        {
            width = ui->tableWidget->width();
        }
        ui->tableWidget->setColumnWidth(0, width-width/3);
        ui->tableWidget->setColumnWidth(1, width/3);

        if (ui->tableWidget_UDP->verticalScrollBar()->isVisible())
        {
            width = ui->tableWidget_UDP->width()-ui->tableWidget_UDP->verticalScrollBar()->width();
        }
        else
        {
            width = ui->tableWidget_UDP->width();
        }

        ui->tableWidget_UDP->setColumnWidth(0, width-width/3);
        ui->tableWidget_UDP->setColumnWidth(1, width/3);
    }
}

// Server 客户端列表中添加数据
void MainWindow::InsertClientIntoTableWidget(QString IP, quint16 Port)
{
    // 插入一行
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setRowHeight(row, 20);

    QTableWidgetItem *check = new QTableWidgetItem();
    check->setCheckState(Qt::Unchecked);
    ui->tableWidget->setItem(row, 0, check); //插入复选框
    check->setText(IP);

    QTableWidgetItem *itemPort = new QTableWidgetItem(QString::number(Port));
    itemPort->setTextAlignment(Qt::AlignLeft);
    ui->tableWidget->setItem(row, 1, itemPort);
}

// Server 删除某个客户端连接
void MainWindow::RemoveClientRow(QString IP, int Port)
{
    for (int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        if ((ui->tableWidget->item(i, 0) != NULL) && (ui->tableWidget->item(i, 1) != NULL))
        {
            if ((ui->tableWidget->item(i, 0)->text() == IP ) && (QString::number(Port) == ui->tableWidget->item(i, 1)->text()))
            {
                ui->tableWidget->removeRow(i);
                tcpClientSocketList.removeAt(i);
                break;
            }
        }
    }
}

// 断开所有客户端
void MainWindow::server_Close_All_Client()
{
    foreach (QTcpSocket *clientSocket, tcpClientSocketList) {
        clientSocket->abort();
    }
}

// UDP 客户端列表
void MainWindow::InitUDPClientTableWidget()
{
    // 设置列数, 行数动态增加
    ui->tableWidget_UDP->setColumnCount(2);

    // 创建表头
    QStringList header;
    header << "IP" << "Port";
    ui->tableWidget_UDP->setHorizontalHeaderLabels(header);

    // 列宽
    ui->tableWidget_UDP->setColumnWidth(0, ui->tableWidget_UDP->width()-ui->tableWidget_UDP->width()/3);
    ui->tableWidget_UDP->setColumnWidth(1, ui->tableWidget_UDP->width()/3);

    // 表格禁止编辑
    ui->tableWidget_UDP->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 隐藏行表头
    ui->tableWidget_UDP->verticalHeader()->setVisible(false);

    // 隔行换色
    ui->tableWidget_UDP->setAlternatingRowColors(true);
    ui->tableWidget_UDP->setStyleSheet("alternate-background-color: rgb(240, 240, 240);");

    // 列宽禁止拖动
    // ui->tableWidget_UDP->horizontalHeader()->setDisabled(true);

    // 行高禁止拖动
    ui->tableWidget_UDP->verticalHeader()->setDisabled(true);

    // 设置选中模式为选中行
    ui->tableWidget_UDP->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 设置选中单个
    ui->tableWidget_UDP->setSelectionMode(QAbstractItemView::SingleSelection);

    // 点击表时不对表头行光亮（获取焦点）
    ui->tableWidget->horizontalHeader()->setHighlightSections(false);
}

// UDP 客户端列表中添加数据
void MainWindow::InsertUDPClientUI(QString IP, quint16 Port)
{
    if (IP.isEmpty())
        return;
    foreach (UDPClient udpClient, UDPClientList) {
        if ((IP == udpClient.IP.toString()) && (Port == udpClient.Port))
            return;
    }

    UDPClient udpClient = {QHostAddress(IP), Port};
    UDPClientList.append(udpClient);

    // 插入一行
    int row = ui->tableWidget_UDP->rowCount();
    ui->tableWidget_UDP->insertRow(row);
    ui->tableWidget_UDP->setRowHeight(row, 20);

    QTableWidgetItem *check = new QTableWidgetItem();
    check->setCheckState(Qt::Unchecked);
    ui->tableWidget_UDP->setItem(row, 0, check); //插入复选框
    check->setText(IP);

    QTableWidgetItem *itemPort = new QTableWidgetItem(QString::number(Port));
    itemPort->setTextAlignment(Qt::AlignLeft);
    ui->tableWidget_UDP->setItem(row, 1, itemPort);
}

// UDP 打开/关闭
void MainWindow::on_pushButton_UDP_Open_clicked()
{
    if (ui->pushButton_UDP_Open->text() == "打开")
    {
        if (udpSocket->bind(QHostAddress(ui->comboBox_UDP_IP->currentText()), \
                            ui->comboBox_UDP_Port->currentText().toShort(), QUdpSocket::ShareAddress) == false)
            return;
        connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::udp_Socket_Read_Data);
        typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
        connect(udpSocket, static_cast<QAbstractSocketErrorSignal>(&QUdpSocket::error), this, &MainWindow::udp_Socket_Error);

        ui->pushButton_UDP_Send->setEnabled(true);
        ui->pushButton_UDP_Open->setText("关闭");
    }
    else
    {
        ui->pushButton_UDP_Send->setEnabled(false);
        ui->pushButton_UDP_Open->setText("打开");

        // 解除connect
        disconnect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::udp_Socket_Read_Data);
        typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
        disconnect(udpSocket, static_cast<QAbstractSocketErrorSignal>(&QUdpSocket::error), this, &MainWindow::udp_Socket_Error);
        udpSocket->close();
    }

}

// UDP 发送数据
void MainWindow::on_pushButton_UDP_Send_clicked()
{
    if (ui->textEdit_UDP_Send->toPlainText().isEmpty())
        return;
    int clientCount=0;
    for (int i=0; i<ui->tableWidget_UDP->rowCount(); i++)
    {
        if (ui->tableWidget_UDP->item(i, 0) != NULL)
        {
            if (ui->tableWidget_UDP->item(i, 0)->checkState() == Qt::Checked)
            {
                udpSocket->writeDatagram(ui->textEdit_UDP_Send->toPlainText().toLatin1(), UDPClientList.at(i).IP, UDPClientList.at(i).Port);
                clientCount++;
            }
        }
    }
    if (clientCount == 0)
        qDebug() << "No Client Select.";
}

// UDP 接收数据
void MainWindow::udp_Socket_Read_Data()
{
    QByteArray ReceiveData;
    QHostAddress clientAddress;
    quint16 clientPort;
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray buffer;
        buffer.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(buffer.data(), buffer.size(), &clientAddress, &clientPort);
        ReceiveData.append(buffer);
    }

    //qDebug() << clientAddress.toString() << clientPort;
    InsertUDPClientUI(clientAddress.toString(), clientPort);

    // 在开头添加空行
    if (ui->checkBox_UDP_Receive_Time->isChecked() || ui->checkBox_UDP_For_Client->isChecked())
    {
        if (!ui->textEdit_UDP_Receive->toPlainText().isEmpty())
            ui->textEdit_UDP_Receive->append("");
    }

    // 显示接收时间
    if (ui->checkBox_UDP_Receive_Time->isChecked())
    {
        ui->textEdit_UDP_Receive->append(QString("[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));
    }

    // 显示客户端来源
    if (ui->checkBox_UDP_For_Client->isChecked())
    {
        ui->textEdit_UDP_Receive->append(QString("[From %1:%2]").arg(clientAddress.toString()).arg(clientPort));
    }

    // 显示Hex
    if (ui->radioButton_UDP_Receive_A->isChecked())
        ui->textEdit_UDP_Receive->append(ReceiveData);
    else
        ui->textEdit_UDP_Receive->append(QByteArrayToHex(ReceiveData));
}

// UDP 错误信息
void MainWindow::udp_Socket_Error(QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError;
}

// 手动增加客户端
void MainWindow::on_pushButton_UDP_ADD_clicked()
{
    InsertUDPClientUI(ui->lineEdit_UDP_ADD_IP->text(), ui->spinBox_UDP_ADD_Port->text().toShort());
}
