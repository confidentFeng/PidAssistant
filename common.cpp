#include "common.h"

common::common()
{

}

// 延时函数
void common::delayMSec(int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

// 获得有效通道序号
int common::getChannelIndex(char channel)
{
    // 若channel=0x08，则返回4
    int validChIndex = 0;
    for(int i=0; i<5;i++)
    {
        validChIndex++;
        if(channel==0x01)
            break;
        channel >>= 1;
    }

    return validChIndex;
}

// 获得浮点数的小数部分
int common::getDecimal(float value)
{
    QString str = QString::number(static_cast<double>(value));
    QStringList strList  = str.split(".");

    return strList.at(1).toInt();
}
