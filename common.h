/*
* Copyright (c) 2019,浙江智澜科技有限公司
* All rights reserved.
*
* 摘 要：common.h/cpp，存放一些公用的宏、枚举、类
*
* 当前版本：1.5
* 作 者：聂咸丰
* 完成日期：2019 年 8 月 28 日
*/
#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QStringList>
#include <QEventLoop>
#include <QTimer>

/******************************协议帧******************************/
#define FRAME_HEAD 0xC33C // 帧头
#define FRAME_LENGTH 0x15 // 帧长度（接受5通道的帧长度为0x13）
#define FRAME_TAIL 0x0D0A // 帧头
#define RECV_DATA_CMD 0x01 // 命令：接受下位机上传数据【通道1-5（2byte），共18个字节】
#define SEND_PID_CMD 0x02 // 命令：上位机发送PID给下位机【通道(1byte)、KP-KI-KD(4byte)，共21个字节】
#define RECV_PID_CMD 0x03 // 命令：上位机读取PID值，下位机回复格式也一样【通道(1byte)、KP-KI-KD(4byte)】
#define RECV_DEFAULT_PID_CMD 0x04 // 命令：上位机读取并设置默认PID值，下位机回复格式也一样【通道(1byte)、KP-KI-KD(4byte)】
#define SEND_DEFAULT_PID_CMD 0x05 // 命令：上位机发送PID值给下位机，重新系统默认PID值【通道(1byte)、KP-KI-KD(4byte)】
#define LIMIT_LENGTH 10 // 避免串口有时候只接受到几个字节，后面调用QByteArray.at(i)导致崩溃

typedef union _Temp
{
    qint16 int16Data[11];
    char cData[22];
}TempAll;

typedef union _TempPid
{
    float fData[3];
    char cData[12];
}TempPid;

typedef union _Temp1
{
    qint16 int16Data[5];
    char cData[10];
}Temp1;

class common
{
public:
    common();

    static int getChannelIndex(char channel); // 获得有效通道序号
    static int getDecimal(float value); // 获得浮点数的小数部分
    static void delayMSec(int msec); // 延时函数
};

#endif // COMMON_H
