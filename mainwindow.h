#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QDateTime>
#include <QByteArray>
#include <QList>
#include <QTableWidget>
#include <QScrollBar>
#include <QUdpSocket>

namespace Ui {
class MainWindow;
}

typedef struct{
    QHostAddress IP;
    quint16 Port;
}UDPClient;

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

// UDP
    void on_pushButton_UDP_Open_clicked();      // UDP 连接/断开
    void on_pushButton_UDP_Send_clicked();      // UDP 发送数据
    void udp_Socket_Read_Data();                // UDP 接收数据
    void udp_Socket_Error(QAbstractSocket::SocketError socketError); // UDP 错误信息
    void on_pushButton_UDP_ADD_clicked();       // 手动增加客户端

private:
    Ui::MainWindow *ui;
    void paintEvent(QPaintEvent *event);    // 控制TableWidget的列宽
    QByteArray QByteArrayToHex(QByteArray byteArray);  // QByteArray转Hex 含空格

// Client
    QTcpSocket *clientSocket;   // Client Socket

// Server
    QTcpServer *tcpServer;
    QList<QTcpSocket*> tcpClientSocketList;         // Server 储存所有的客户端连接
    void InitClientListTableWidgetUI();             // Server 客户端列表
    void InsertClientIntoTableWidget(QString IP, quint16 Port); // Server 客户端列表中添加数据
    void RemoveClientRow(QString IP, int Port);     // Server 删除某个客户端连接
    void server_Close_All_Client();                 // 断开所有客户端

// UDP
    QUdpSocket *udpSocket;
    QList<UDPClient> UDPClientList;                 // UDP 储存所有的客户端连接
    void InitUDPClientTableWidget();                // UDP 客户端列表
    void InsertUDPClientUI(QString IP, quint16 Port);   // UDP 客户端列表中添加数据
};

#endif // MAINWINDOW_H
