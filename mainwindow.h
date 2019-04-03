#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QDateTime>
#include <QByteArray>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
// Client
    void on_pushButton_Clien_Connect_clicked(); // Client 连接/断开
    void on_pushButton_Client_Send_clicked();   // Client 发送数据
    void client_Socket_Read_Data();             // Client 接收数据
    void client_Socket_DisConnected();          // Client 断开Socket
    void client_Socket_Error(QAbstractSocket::SocketError socketError); // Client 错误信息
// Server
    void on_pushButton_Server_Open_clicked();   // Server 打开/关闭
    void on_pushButton_Server_Send_clicked();   // Server 发送数据
    void server_New_connect();                  // Server 创建新连接
    void server_Read_Data();                    // Server 接收数据
    void server_Disconnected();                 // Server 断开Socket

private:
    Ui::MainWindow *ui;
    QTcpSocket *clientSocket;   // Client Socket

    QTcpServer *tcpServer;
    QTcpSocket *serverSocket;   // Server Socket

    QByteArray QByteArrayToHex(QByteArray byteArray);  // QByteArray转Hex 含空格
};

#endif // MAINWINDOW_H
