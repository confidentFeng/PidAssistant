#include "serial.h"

Serial::Serial()
{
    // 初始化串口
    m_serialPort = new QSerialPort;

    // 初始化成员变量
    m_readBuf.clear();
    m_tempAll.clear();

    // 串口错误处理信号槽（热插拔）
    connect(m_serialPort,SIGNAL(errorOccurred(QSerialPort::SerialPortError)),this,SLOT(handleSerialError(QSerialPort::SerialPortError)));

    // 串口号列表更新定时器
    serialTimer = new QTimer(this);
    connect(serialTimer, SIGNAL(timeout()), SLOT(onTimeOut()));
    serialTimer->start(1500);
}

// 扫描可用串口
QStringList Serial::scanSerial()
{
    QStringList serialStrList;

    // 读取串口信息
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        serialStrList.append(info.portName());
    }

    return serialStrList;
}

// 打开串口
bool Serial::open(QString serialName, int baudRate)
{
    // 设置串口名
    m_serialPort->setPortName(serialName);
    // 打开串口(以读写方式)
    if(m_serialPort->open(QIODevice::ReadWrite))
    {
        m_serialPort->setBaudRate(baudRate); // 设置波特率(默认为115200)
        m_serialPort->setDataBits(QSerialPort::Data8); // 设置数据位(数据位为8位)
        m_serialPort->setParity(QSerialPort::NoParity); // 设置校验位(无校验位)
        m_serialPort->setStopBits(QSerialPort::OneStop); // 设置停止位(停止位为1)
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl); // 设置流控制(无数据流控制)

        // 当下位机中有数据发送过来时就会响应这个槽函数
        connect(m_serialPort, &QSerialPort::readyRead, this, &Serial::readData);

        return true;
    }

    return false;
}

// 关闭串口
void Serial::close()
{
    m_serialPort->clear();
    m_serialPort->close();
}

// 发送数据给下位机
void Serial::sendData(QByteArray &sendData)
{
    // 发送数据帧
    m_serialPort->write(sendData);
}

// 发送数据帧给下位机
void Serial::sendDataFrame(char cmd, char passage, float kp, float ki, float kd)
{
    TempAll tempAll;
    // 帧头、帧长度、命令类型、通道
    tempAll.int16Data[0] = static_cast<qint16>(FRAME_HEAD);
    tempAll.cData[2] =FRAME_LENGTH;
    tempAll.cData[3] =cmd;
    tempAll.cData[4] =passage;
    // PID值
    TempPid tempPid;
    tempPid.fData[0] = kp;
    tempPid.fData[1] = ki;
    tempPid.fData[2] = kd;
    for(int i=0; i<12; i++)
    {
        tempAll.cData[5+i] = static_cast<char>(tempPid.cData[i]);
    }
    // 校验和
    qint16 checkSum = 0;
    for(int i=0; i<FRAME_LENGTH-4; i++)
    {
        checkSum += static_cast<unsigned char>(tempAll.cData[i]);
    }
    tempAll.cData[17] = static_cast<char>((checkSum & 0x00ff));
    tempAll.cData[18] = static_cast<char>((checkSum & 0xff00) >> 8);
    // 帧尾
    tempAll.cData[19] = static_cast<char>((FRAME_TAIL & 0xff00) >> 8);
    tempAll.cData[20] = static_cast<char>((FRAME_TAIL & 0x00ff));

    // 发送数据帧
    m_serialPort->write(tempAll.cData, FRAME_LENGTH);
}

// 读取下位机发来数据
void Serial::readData()
{
    //这个判断尤为重要,否则的话直接延时再接收数据,空闲时和会出现高内存占用
    if(m_serialPort->bytesAvailable()<=0)
    {
        return;
    }

    // 将下位机发来数据存储在数据缓冲区
    QByteArray temp = m_serialPort->readAll();
    m_tempAll.append(temp);
    //qDebug() <<"temp: " << temp;

    // 由于Qt串口固有缺陷，每次只能接受一帧数据的一部分，所以协议添加帧尾，进而分离出完整的一帧数据
    if(!m_tempAll.isEmpty())
    {
        if(m_tempAll.contains("\r\n"))
        {
            m_readBuf = m_tempAll.split('\r').at(0) + "\r\n";
            m_tempAll.clear();

            if(m_readBuf.length()>LIMIT_LENGTH && m_readBuf.length()==m_readBuf.at(2)) // 判断帧长度
            {
                emit readOneFrame();
            }
        }
    }
}

// 处理串口错误（热插拔）
void Serial::handleSerialError(QSerialPort::SerialPortError error)
{
    if (this->isOpen() && error!=QSerialPort::NoError) // 调试PID时突然拔掉串口线(PermissionError)、突然按下关机键(ResourceError)等
    {
        // 首先关闭串口
        m_serialPort->close();

        emit errorSignal();
    }
}

// 周期扫描是否有新串口
void Serial::onTimeOut()
{
    if(!this->isOpen()) // 串口打开时，不扫描串口
    {
        // 扫描可用串口
        QStringList newPortStrList = this->scanSerial();

        //更新旧的串口列表
        if(newPortStrList.size() != oldPortStrList.size())
        {
            oldPortStrList = newPortStrList;
            emit onNewSerialPort(oldPortStrList);
        }
    }
}

// 获得读取数据缓冲区
QByteArray Serial::getReadBuf()
{
    return m_readBuf;
}

// 清除读取数据缓冲区
void Serial::clearReadBuf()
{
    m_readBuf.clear();
}

// 判断串口是否打开
bool Serial::isOpen()
{
    return m_serialPort->isOpen();
}
