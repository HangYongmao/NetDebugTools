#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss("style.qss");
    if (qss.open(QFile::ReadWrite))
    {
        a.setStyleSheet(qss.readAll());
        qss.close();
    }

    MainWindow w;
    w.show();

    // 设置编码方式，防止中文乱码
#if defined(Q_CC_MSVC)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("gb2312"));
#elif defined(Q_CC_MINGW)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
#endif

    return a.exec();
}
