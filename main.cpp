#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
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
