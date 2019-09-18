/*
* Copyright (c) 2019,浙江智澜科技有限公司
* All rights reserved.
*
* 摘 要：串口类，实现串口相关功能
*
* 当前版本：1.5
* 作 者：聂咸丰
* 完成日期：2019 年 9 月 5 日
*/
#ifndef SERIAL_H
#define SERIAL_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include "mymessagebox.h"
#include "common.h"

class Serial:public QObject // 要继承QObject，才能使用 connect() 函数
{
    Q_OBJECT

public:
    Serial();
    QStringList scanSerial(); // 扫描可用串口
    bool open(QString serialName, int baudRate); // 打开串口
    void close(); // 关闭串口
    void sendData(QByteArray &sendData); // 发送数据给下位机
    void sendDataFrame(char cmd, char passage, float kp, float ki, float kd); // 发送数据帧给下位机
    QByteArray getReadBuf(); // 获得读取数据缓冲区
    void clearReadBuf(); // 清除读取数据缓冲区
    bool isOpen(); // 判断串口是否打开

signals:
    void readOneFrame(); //读取满一帧数据，就会触发这个信号，以提示其它类对象
    void errorSignal(); // 串口发生错误的信号
    void onNewSerialPort(QStringList); // 扫描到新串口的信号

public slots:
    void readData(); // 读取下位机发来数据
    void handleSerialError(QSerialPort::SerialPortError error); // 处理串口错误（热插拔）
    void onTimeOut(); // 周期扫描是否有新串口

private:
    QSerialPort *m_serialPort; // 实例化一个指向串口的指针，可以用于访问串口
    QTimer *serialTimer; // 周期扫描串口号定时器
    QStringList oldPortStrList; // 存放旧的串口号列表
    QByteArray m_readBuf; // 存储接受数据的缓冲区
    QByteArray m_tempAll; // 存储接受数据的临时缓冲区
};

#endif // SERIAL_H
