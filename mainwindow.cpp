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

    clientSocket = new QTcpSocket();

    tcpServer    = new QTcpServer();
    serverSocket = NULL;


    // 连接信号槽
    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::client_Socket_Read_Data);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MainWindow::client_Socket_DisConnected);

    connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::server_New_connect);
}

MainWindow::~MainWindow()
{
    tcpServer->close();
    tcpServer->deleteLater();
    delete this->clientSocket;
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
    qDebug() << "DisConnected.";
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
        if (serverSocket != NULL)
            serverSocket->abort();

        // 关闭监听
        tcpServer->close();
        ui->pushButton_Server_Open->setText("打开");
        ui->pushButton_Server_Send->setEnabled(false);
    }
}

// Server 发送数据
void MainWindow::on_pushButton_Server_Send_clicked()
{
    serverSocket->write(ui->textEdit_Server_Send->toPlainText().toLatin1());
}

// Server 创建新连接
void MainWindow::server_New_connect()
{
    // 获取客户端连接
    serverSocket = tcpServer->nextPendingConnection();

    // 连接QTcpSocket的信号槽
    connect(serverSocket, &QTcpSocket::readyRead, this, &MainWindow::server_Read_Data);
    connect(serverSocket, &QTcpSocket::disconnected, this, &MainWindow::server_Disconnected);

    // 发送按键使能
    ui->pushButton_Server_Send->setEnabled(true);
    qDebug() << "A New Client Connect.";
}

// Server 接收数据
void MainWindow::server_Read_Data()
{
    QByteArray buffer;
    buffer = serverSocket->readAll();
    if (buffer.isEmpty())
        return;

    if (ui->checkBox_Server_Receive_Time->isChecked())
    {
        if (ui->textEdit_Server_Receive->toPlainText().isEmpty())
            ui->textEdit_Server_Receive->append(QString("[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));
        else
            ui->textEdit_Server_Receive->append(QString("\r\n[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));
    }
    if (ui->radioButton_Server_Receive_A->isChecked())
        ui->textEdit_Server_Receive->append(buffer);
    else
        ui->textEdit_Server_Receive->append(QByteArrayToHex(buffer));
}

// Server 断开Socket
void MainWindow::server_Disconnected()
{
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
