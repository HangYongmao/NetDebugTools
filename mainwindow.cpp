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
    }
    else
    {
        // 断开连接
        clientSocket->disconnectFromHost();

        ui->pushButton_Clien_Connect->setText("连接");
        ui->pushButton_Client_Send->setEnabled(false);
        ui->comboBox_Client_IP->setEnabled(true);
        ui->comboBox_Client_Port->setEnabled(true);
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
        ui->textEdit_Client_Receive->append(QString("\r\n[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));
    if (ui->radioButton_Client_Receive_A->isChecked())
        ui->textEdit_Client_Receive->append(buffer);
    else
        ui->textEdit_Client_Receive->append(buffer.toHex().toUpper());
}

// Client 断开Socket
void MainWindow::client_Socket_DisConnected()
{
    ui->pushButton_Client_Send->setEnabled(false);
    ui->pushButton_Clien_Connect->setText("连接");
    ui->comboBox_Client_IP->setEnabled(true);
    ui->comboBox_Client_Port->setEnabled(true);
    qDebug() << "DisConnected.";
}
