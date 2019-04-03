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

    // 连接信号槽
    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::client_Socket_Read_Data);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MainWindow::client_Socket_DisConnected);
}

MainWindow::~MainWindow()
{
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
        if (!clientSocket->waitForConnected(30000))
        {
            qDebug() << "Connection Failed.";
            return;
        }
        qDebug() << "Connection Successfully.";

        ui->pushButton_Client_Send->setEnabled(true);

        ui->pushButton_Clien_Connect->setText("断开");
    }
    else
    {
        // 断开连接
        clientSocket->disconnectFromHost();

    }
}

// Client 发送数据
void MainWindow::on_pushButton_Client_Send_clicked()
{

}
