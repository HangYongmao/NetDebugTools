#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
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

private:
    Ui::MainWindow *ui;
    QTcpSocket *clientSocket;   // Client Socket
};

#endif // MAINWINDOW_H
