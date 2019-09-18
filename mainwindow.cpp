#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置窗体标题
    this->setWindowTitle("PidAssistant");
    // 禁止最大化按钮
    this->setWindowFlags(windowFlags() &~ Qt::WindowMaximizeButtonHint);
    // 禁止拖动窗口大小
    this->setFixedSize(this->width(),this->height());
    // 默认不使能暂停更新按钮
    ui->pauseUpdateButton->setEnabled(false);
    // 默认设置按钮样式
    ui->pauseUpdateButton->setStyleSheet("background-color: rgb(255, 255, 255); border-radius:8px;");
    ui->windowTopButton->setStyleSheet("background-color: rgb(255, 255, 255); border-radius:8px;");
    // 限制输入栏只能输入数字和小数点
    // 通过正则表达式设置三行"密码输入栏"，只能输入6位数字
    QValidator *accountValidator = new QRegExpValidator(QRegExp("^(-?[0]|-?[1-9][0-9]{0,5})(?:\\.\\d{1,4})?$|(^\\t?$)"));
    ui->pLineEdit->setValidator(accountValidator);
    ui->dLineEdit->setValidator(accountValidator);
    ui->iLineEdit->setValidator(accountValidator);

    // 初始化成员变量
    m_vecPidData.resize(5); // 存放5个通道的PID结果值
    m_channel = 0x00; // 有效通道

    // 默认打开通道1
    ui->ch1Frame->setStyleSheet("color: rgba(255, 0, 0, 255);");
    m_channel |= 0x01;
    // 为ch[1~5]Frame添加事件过滤器
    ui->ch1Frame->installEventFilter(this);
    ui->ch2Frame->installEventFilter(this);
    ui->ch3Frame->installEventFilter(this);
    ui->ch4Frame->installEventFilter(this);
    ui->ch5Frame->installEventFilter(this);
    // label安装事件过滤器，实现鼠标点击事件
    ui->taobaoLabel->installEventFilter(this);
    ui->chartWidget->installEventFilter(this);

    // 初始化串口
    m_serial = new Serial;
    // 寻找可用串口
    QStringList serialStrList;
    serialStrList = m_serial->scanSerial();
    for (int i=0; i<serialStrList.size(); i++)
    {
        ui->portComboBox->addItem(serialStrList[i]); // 在comboBox那添加串口号
    }
    // 默认设置波特率为115200（第5项）
    ui->baudComboBox->setCurrentIndex(5);
    // 当下位机中有数据发送过来时就会响应这个槽函数
    connect(m_serial, SIGNAL(readOneFrame()), this, SLOT(readFromSerial()));
    // 处理串口错误
    connect(m_serial, SIGNAL(errorSignal()), this, SLOT(handleSerialError()));
    // 连接onNewSerialPort信号槽，定时获取串口列表
    connect(m_serial, SIGNAL(onNewSerialPort(QStringList)),this, SLOT(onNewPortList(QStringList)));

    // 初始化右键菜单(下个版本添加)
    // initRightClickMenu();

    // 初始化图表实例指针
    m_chart = new Chart(this);
    // 初始化图表
    m_chart->initChart(ui->chartWidget);
    // 读取到一帧数据，则更新一次图表
    connect(this, &MainWindow::updataSignal, this, &MainWindow::runUpdataChart);
    // 在下方状态栏上显示鼠标所在位置
    connect(m_chart, SIGNAL(sendCoordToMain(float, float)), this, SLOT(showCoor(float, float)));

    QStyle* style = QApplication::style();
    // 添加Qt内置的"加载"图标
    QIcon icon = style->standardIcon(QStyle::SP_DialogOpenButton);
    ui->loadAction->setIcon(icon);
    // 添加Qt内置的"保存"图标
    icon = style->standardIcon(QStyle::SP_DialogSaveButton);
    ui->saveAction->setIcon(icon);    
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 运行更新绘图函数的函数
void MainWindow::runUpdataChart()
{
    // 在右侧的数码管上显示各个通道的PID输出值
    ui->ch1LcdNumber->display(QString("%1").arg(m_vecPidData[0]));
    ui->ch2LcdNumber->display(QString("%1").arg(m_vecPidData[1]));
    ui->ch3LcdNumber->display(QString("%1").arg(m_vecPidData[2]));
    ui->ch4LcdNumber->display(QString("%1").arg(m_vecPidData[3]));
    ui->ch5LcdNumber->display(QString("%1").arg(m_vecPidData[4]));

    // 更新绘图
    m_chart->updataChart(m_channel, m_vecPidData);
}

// 事件过滤器
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 通道标签选择是否高亮
    if(event->type()==QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if(obj==ui->ch1Frame && mouseEvent->buttons()==Qt::LeftButton)
        {
            if(ui->ch1Frame->styleSheet() == "color: rgba(255, 0, 0, 70);")
            {
                ui->ch1Frame->setStyleSheet("color: rgba(255, 0, 0, 255);");
                m_channel |= 0x01;
            }
            else
            {
                ui->ch1Frame->setStyleSheet("color: rgba(255, 0, 0, 70);");
                m_channel &= 0xFE;
                m_chart->clearChannel(0); // 清空数据
            }
        }
        else if(obj==ui->ch2Frame && mouseEvent->buttons()==Qt::LeftButton)
        {
            if(ui->ch2Frame->styleSheet() == "color: rgba(255, 0, 255, 70);")
            {
                ui->ch2Frame->setStyleSheet("color: rgba(255, 0, 255, 255);");
                m_channel |= 0x02;
            }
            else
            {
                ui->ch2Frame->setStyleSheet("color: rgba(255, 0, 255, 70);");
                m_channel &= 0xFD;
                m_chart->clearChannel(1);
            }
        }
        else if(obj==ui->ch3Frame && mouseEvent->buttons()==Qt::LeftButton)
        {
            if(ui->ch3Frame->styleSheet() == "color: rgba(0, 128, 0, 70);")
            {
                ui->ch3Frame->setStyleSheet("color: rgba(0, 128, 0, 255);");
                m_channel |= 0x04;
            }
            else
            {
                ui->ch3Frame->setStyleSheet("color: rgba(0, 128, 0, 70);");
                m_channel &= 0xFB;
                m_chart->clearChannel(2);
            }
        }
        else if(obj==ui->ch4Frame && mouseEvent->buttons()==Qt::LeftButton)
        {
            if(ui->ch4Frame->styleSheet() == "color: rgba(0, 0, 255, 70);")
            {
                ui->ch4Frame->setStyleSheet("color: rgba(0, 0, 255, 255);");
                m_channel |= 0x08;
            }
            else
            {
                ui->ch4Frame->setStyleSheet("color: rgba(0, 0, 255, 70);");
                m_channel &= 0xF7;
                m_chart->clearChannel(3);
            }
        }
        else if(obj==ui->ch5Frame && mouseEvent->buttons()==Qt::LeftButton)
        {
            if(ui->ch5Frame->styleSheet() == "color: rgba(0, 255, 255, 70);")
            {
                ui->ch5Frame->setStyleSheet("color: rgba(0, 255, 255, 255);");
                m_channel |= 0x10;
            }
            else
            {
                ui->ch5Frame->setStyleSheet("color: rgba(0, 255, 255, 70);");
                m_channel &= 0xEF;
                m_chart->clearChannel(4);
            }
        }
    }

    // 只有单个通道有效时，读写按钮才能使用
    if(m_channel==0x01 || m_channel==0x02 || m_channel==0x04 || m_channel==0x08 || m_channel==0x10)
    {
        ui->readPidButton->setEnabled(true);
        ui->writePidButton->setEnabled(true);
        ui->resButton->setEnabled(true);
        ui->setButton->setEnabled(true);
    }
    else
    {
        ui->readPidButton->setEnabled(false);
        ui->writePidButton->setEnabled(false);
        ui->resButton->setEnabled(false);
        ui->setButton->setEnabled(false);
    }

    // 按下"官方淘宝商铺"Label，打开公司淘宝网页
    if(obj==ui->taobaoLabel && event->type()==QEvent::MouseButtonPress)
    {
        QDesktopServices::openUrl(QUrl(QLatin1String("https://shop360222626.taobao.com/index.htm?"
                                                     "spm=2013.1.w5002-21081213145.2.4a80f31bmzWRiF")));
    }

    return QWidget::eventFilter(obj,event);
}

// 初始化右键菜单
void MainWindow::initRightClickMenu()
{    // 初始化动作
    QAction *action1 = new QAction("新建",this);
    // 初始化右键菜单
    QMenu *rightClickMenu = new QMenu(this);
    // 动作添加到右键菜单
    rightClickMenu->addAction(action1);
    rightClickMenu->addAction(ui->newAction);
    rightClickMenu->addSeparator();
    rightClickMenu->addAction(ui->exitAction);
    // 给动作设置信号槽
    //connect(exitAction, &QAction::triggered, this, &MainWindow::on_exitAction_triggered);

    // 给控件设置上下文菜单策略:鼠标右键点击控件时会发送一个void QWidget::customContextMenuRequested(const QPoint &pos)信号
    ui->chartWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    // 给信号设置相应的槽函数
    connect(ui->chartWidget, &QCustomPlot::customContextMenuRequested, [=](const QPoint &pos)
    {
        Q_UNUSED(pos);
        rightClickMenu->exec(QCursor::pos());
    });
}

// 打开串口按钮-点击槽函数
void MainWindow::on_openSerialButton_clicked()
{
    if(ui->openSerialButton->text() == tr("打开串口"))
    {
        // 打开串口
        if(m_serial->open(ui->portComboBox->currentText(), ui->baudComboBox->currentText().toInt()))
        {
            // 打开串口前清除图表，避免加载数据产生曲线和串口接受曲线混杂
            m_chart->clearChart();
            // 清空本地各通道的数据(避免加载数据中本地保存通道数据和打开串口后接受通道数据混淆)
            m_chart->clearChannelData();
            // 暂停时间清零
            m_chart->clearPauseTime();
            // 启动串口计时器，以获得串口打开后的经过时间
            m_chart->startTimer();

            // 关闭下拉列表使能
            ui->portComboBox->setEnabled(false);
            ui->baudComboBox->setEnabled(false);
            // 使能暂停更新按钮
            ui->pauseUpdateButton->setEnabled(true);
            // 修改按钮名称
            ui->openSerialButton->setText(tr("关闭串口"));
            // 修改串口提示标签的文本，且颜色修改为蓝色
            ui->serialTipLabel->setText(tr("串口已开启"));
            ui->serialTipLabel->setStyleSheet("color:blue");
        }
    }
    else
    {
        // 避免当暂停更新(会disconnect)后关闭串口，再打开串口不接受数据的情况
        m_isPause = false;
        emit ui->pauseUpdateButton->clicked();

        // 关闭串口
        m_serial->close();
        // 重新开启下拉列表使能
        ui->portComboBox->setEnabled(true);
        ui->baudComboBox->setEnabled(true);
        // 设置不使能暂停更新按钮
        ui->pauseUpdateButton->setEnabled(false);
        // 恢复按钮名称
        ui->openSerialButton->setText(tr("打开串口"));
        // 恢复串口提示标签的文本及颜色
        ui->serialTipLabel->setText(tr("串口未开启"));
        ui->serialTipLabel->setStyleSheet("color:red");
    }
}

// 读取来自串口类的数据
void MainWindow::readFromSerial()
{
    QByteArray readBuf = m_serial->getReadBuf();
    //qDebug() << "mainRecv:" << readBuf;

    // 校验帧头
    TempAll tempAll;
    tempAll.cData[0] = readBuf.at(0);
    tempAll.cData[1] = readBuf.at(1);
    if(tempAll.int16Data[0] != static_cast<qint16>(FRAME_HEAD))
    {
        qDebug() << "校验帧头错误";
        m_serial->clearReadBuf();
        return;
    }

    // 检验校验位 【注意所有帧的长度不一致】
    qint16 checkSum = 0;
    for(int i=0; i<readBuf.at(2)-4; i++)
    {
        checkSum += static_cast<unsigned char>(readBuf.at(i));
    }
    char checkSumH = static_cast<char>((checkSum & 0xff00) >> 8);
    char checkSumL = static_cast<char>((checkSum & 0x00ff));
    if(checkSumH!=readBuf.at(readBuf.at(2)-3) || checkSumL!=readBuf.at(readBuf.at(2)-4))
    {
        qDebug() << "校验位错误";
        m_serial->clearReadBuf();
        return;
    }

    // 校验命令
    switch (readBuf.at(3))
    {
        case RECV_DATA_CMD: // 命令：接受下位机上传数据
        {
            // 通道1-5（2byte）
            Temp1 temp1;
            for(int i=0; i<10; i++)
            {
                temp1.cData[i] = readBuf[4+i];
            }

            // 更新图表数据
            for(int i=0; i<CHANNEL_COUNT; i++)                
                m_vecPidData[i] = temp1.int16Data[i];

            // 发射更新图表信号
            emit updataSignal();
        }
        break;
        case RECV_PID_CMD: // 命令：上位机读取PID值，下位机回复格式也一样
        {
            // 通道(1byte)、KP-KI-KD(4byte)
            TempPid tempPid;
            for(int i=0; i<12; i++)
            {
                tempPid.cData[i] = readBuf[5+i];
            }

            //显示默认PID值
            ui->pLineEdit->setText(QString("%1").arg(static_cast<double>(tempPid.fData[0])));
            ui->iLineEdit->setText(QString("%1").arg(static_cast<double>(tempPid.fData[1])));
            ui->dLineEdit->setText(QString("%1").arg(static_cast<double>(tempPid.fData[2])));
        }
        break;
        case RECV_DEFAULT_PID_CMD: // 命令：上位机读取并设置默认PID值，下位机回复格式也一样
        {
            //qDebug() << "读到出厂PID";
            // 通道(1byte)、KP-KI-KD(4byte)
            TempPid tempPid;
            for(int i=0; i<12; i++)
            {
                tempPid.cData[i] = readBuf[5+i];
            }

            //显示默认PID值
            ui->pLineEdit->setText(QString("%1").arg(static_cast<double>(tempPid.fData[0])));
            ui->iLineEdit->setText(QString("%1").arg(static_cast<double>(tempPid.fData[1])));
            ui->dLineEdit->setText(QString("%1").arg(static_cast<double>(tempPid.fData[2])));

            // 恢复出厂后，还要将读出的出厂PID写入到相应通道
            m_serial->sendDataFrame(SEND_PID_CMD, m_channel, tempPid.fData[0], tempPid.fData[1], tempPid.fData[2]);
        }
        break;
    }

    // 清除读取数据缓冲区
    m_serial->clearReadBuf();
}

// 处理串口错误
void MainWindow::handleSerialError()
{
    MyMessageBox *msgBox = new MyMessageBox();
    msgBox->showMyMessageBox(nullptr, tr("错误提示"), tr("串口连接中断，请检查是否正确连接!"), MESSAGE_WARNNING,
                                        BUTTON_OK, true);

    // 再次触发"打开串口"按钮槽函数，以关闭串口
    ui->openSerialButton->setText(tr("关闭串口"));
    emit ui->openSerialButton->clicked();
}

// 用于响应SerialPortList信号，周期获取串口列表
void MainWindow::onNewPortList(QStringList portName)
{
    ui->portComboBox->clear();
    ui->portComboBox->addItems(portName);
}

// 读取PID按钮-点击槽函数
void MainWindow::on_readPidButton_clicked()
{
    MyMessageBox *msgBox = new MyMessageBox();
    // 判断串口是否打开
    if(!m_serial->isOpen())
    {
        msgBox->showMyMessageBox(this, tr("错误提示"), tr("串口未打开，请先打开串口!"), MESSAGE_WARNNING, \
                                            BUTTON_OK, true);
        return;
    }

    // 只有单个通道有效时，才能读取PID，达到读取单个通道PID的功能
    if(m_channel==0x01 || m_channel==0x02 || m_channel==0x04 || m_channel==0x08 || m_channel==0x10)
    {
        // 发送全零的PID值给下位机，下位机收到后则上发有效通道的PID
        float pValue = 0;
        float iValue = 0;
        float dValue = 0;

        // 读取PID成功后，给用户提示
        MyMessageBox *msgBox = new MyMessageBox();
        msgBox->showMyMessageBox(this, tr("读取PID"), tr("成功读取通道 %1 的PID!").arg(common::getChannelIndex(m_channel)), \
                                 MESSAGE_INFORMATION, BUTTON_OK, true);

        //qDebug() << "发送读取PID值帧";
        for(int i=0; i<7; i++)
        {
            m_serial->sendDataFrame(RECV_PID_CMD, m_channel, pValue, iValue, dValue);
            common::delayMSec(100);
        }
    }
}

// 写入PID按钮-点击槽函数
void MainWindow::on_writePidButton_clicked()
{
    MyMessageBox *msgBox = new MyMessageBox();
    // 判断串口是否打开
    if(!m_serial->isOpen())
    {
        msgBox->showMyMessageBox(this, tr("错误提示"), tr("串口未打开，请先打开串口!"), MESSAGE_WARNNING, \
                                            BUTTON_OK, true);
        return;
    }

    if(ui->pLineEdit->text().isEmpty() || ui->iLineEdit->text().isEmpty() || ui->dLineEdit->text().isEmpty())
    {
        MyMessageBox *msgBox = new MyMessageBox();
        msgBox->showMyMessageBox(this, tr("写入PID"), tr("P、I、D值都需要输入!"), MESSAGE_WARNNING, \
                                    BUTTON_OK, true);
        return;
    }

    float kp = ui->pLineEdit->text().toFloat();
    float ki = ui->iLineEdit->text().toFloat();
    float kd = ui->dLineEdit->text().toFloat();
    m_serial->sendDataFrame(SEND_PID_CMD, m_channel, kp, ki, kd);

    // 写入PID成功后，给用户提示
    msgBox->showMyMessageBox(this, tr("写入PID"), tr("成功写入通道 %1 的PID!").arg(common::getChannelIndex(m_channel)), \
                             MESSAGE_INFORMATION, BUTTON_OK, true);
}

// 恢复出厂按钮-点击槽函数
void MainWindow::on_resButton_clicked()
{

    MyMessageBox *msgBox = new MyMessageBox();
    // 判断串口是否打开
    if(!m_serial->isOpen())
    {
        msgBox->showMyMessageBox(this, tr("错误提示"), tr("串口未打开，请先打开串口!"), MESSAGE_WARNNING, \
                                            BUTTON_OK, true);
        return;
    }

    if(msgBox->showMyMessageBox(this, tr("恢复出厂"), tr("确定恢复出厂PID吗？"), MESSAGE_QUESTION, \
                                BUTTON_OK_AND_CANCEL, true) == ID_OK)
    {
        //qDebug() << "恢复出厂-确定按钮";
        // 只有单个通道有效时，才能恢复出厂PID
        if(m_channel==0x01 || m_channel==0x02 || m_channel==0x04 || m_channel==0x08 || m_channel==0x10)
        {
            for(int i=0; i<7; i++)
            {
                m_serial->sendDataFrame(RECV_DEFAULT_PID_CMD, m_channel, 0, 0, 0);
                common::delayMSec(100);
            }
        }
    }
}

// 设置出厂按钮-点击槽函数
void MainWindow::on_setButton_clicked()
{
    MyMessageBox *msgBox = new MyMessageBox();
    // 判断串口是否打开
    if(!m_serial->isOpen())
    {
        msgBox->showMyMessageBox(this, tr("错误提示"), tr("串口未打开，请先打开串口!"), MESSAGE_WARNNING, \
                                            BUTTON_OK_AND_CANCEL, true);
        return;
    }
    // 判断P、I、D值是否输入
    if(ui->pLineEdit->text().isEmpty() || ui->iLineEdit->text().isEmpty() || ui->dLineEdit->text().isEmpty())
    {
        msgBox->showMyMessageBox(this, tr("设置出厂"), tr("出厂P、I、D值都需要输入!"), MESSAGE_WARNNING, \
                                    BUTTON_OK, true);
        return;
    }

    if(msgBox->showMyMessageBox(this, tr("设置出厂"), tr("确定重新设置出厂PID吗？"), MESSAGE_QUESTION, \
                                BUTTON_OK_AND_CANCEL, true) == ID_OK)
    {
        //qDebug() << "设置出厂-确定按钮";
        // 只有单个通道有效时，才能重新设置出厂PID
        if(m_channel==0x01 || m_channel==0x02 || m_channel==0x04 || m_channel==0x08 || m_channel==0x10)
        {
            float kp = ui->pLineEdit->text().toFloat();
            float ki = ui->iLineEdit->text().toFloat();
            float kd = ui->dLineEdit->text().toFloat();

            m_serial->sendDataFrame(SEND_DEFAULT_PID_CMD, m_channel, kp, ki, kd);
            common::delayMSec(100);
            // 设置默认PID之后，还要写入当前PID
            m_serial->sendDataFrame(SEND_PID_CMD, m_channel, kp, ki, kd);
        }
    }
}

// 暂停更新按钮-点击槽函数
void MainWindow::on_pauseUpdateButton_clicked()
{
    if(m_isPause)
    {
        m_isPause = false;
        m_chart->controlUpdata(true);
        disconnect(this, &MainWindow::updataSignal, this, &MainWindow::runUpdataChart);
        ui->pauseUpdateButton->setStyleSheet("background-color:rgb(162, 188, 228); border-radius:8px;");
    }
    else
    {
        m_isPause = true;
        m_chart->controlUpdata(false);
        connect(this, &MainWindow::updataSignal, this, &MainWindow::runUpdataChart);
        ui->pauseUpdateButton->setStyleSheet("background-color: rgb(255, 255, 255); border-radius:8px;");
    }   
}

// 窗口置顶按钮-点击槽函数
void MainWindow::on_windowTopButton_clicked()
{
    if (!m_windowFlags)
    {
        m_windowFlags = windowFlags();
        setWindowFlags(m_windowFlags | Qt::WindowStaysOnTopHint);
        this->show();
        ui->windowTopButton->setStyleSheet("background-color:rgb(162, 188, 228); border-radius:8px;");
    }
    else
    {
        m_windowFlags = nullptr;
        setWindowFlags(m_windowFlags);
        this->show();
        ui->windowTopButton->setStyleSheet("background-color: rgb(255, 255, 255); border-radius:8px;");
    }
}

// 在下方状态栏上显示鼠标所在位置
void MainWindow::showCoor(float x_val, float y_val)
{
    ui->xLabel->setText(QString("X: %1").arg(static_cast<double>(x_val)));
    ui->yLabel->setText(QString("Y: %1").arg(static_cast<double>(y_val)));
}

// 保存数据-触发槽函数
void MainWindow::on_saveAction_triggered()
{
    // 保存数据前先暂停更新
    if(m_serial->isOpen()) // 串口打开的前提下
    {
        m_isPause = false;
        m_chart->controlUpdata(m_isPause);
        disconnect(m_serial, SIGNAL(readOneFrame()), this, SLOT(readFromSerial()));
    }

    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this, "保存数据", "", "Text File(*.txt)");
    if(fileName == "")
    {
        return;
    }
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        MyMessageBox *msgBox = new MyMessageBox();
        msgBox->showMyMessageBox(this, tr("错误提示"), tr("文件打开失败!"), MESSAGE_WARNNING, BUTTON_OK, true);
        return;
    }
    else
    {
        // 获得各通道数据
        QMap<double, int> *mapData;
        mapData = m_chart->getMapData();

        // 本地保存各通道的数据
        QTextStream textStream(&file);
        for(int i=0; i<CHANNEL_COUNT; i++)
        {
            textStream << QString("ch%1\r\n").arg(i+1);
            QMap<double, int>::iterator it = mapData[i].begin();
            for (it = mapData[i].begin(); it != mapData[i].end(); it++)
            {
                textStream << QString::number(it.key(), 10, 2) + " " + QString::number(it.value()) + "\r\n";
            }
        }

        file.close();
    }

    // 保存数据后再恢复更新
    if(m_serial->isOpen())
    {
        m_isPause = true;
        m_chart->controlUpdata(m_isPause);
        connect(m_serial, SIGNAL(readOneFrame()), this, SLOT(readFromSerial()));
    }
}

// 载入数据-触发槽函数
void MainWindow::on_loadAction_triggered()
{
    MyMessageBox *msgBox = new MyMessageBox();
    // 载入数据前，必须关闭串口
    if(m_serial->isOpen())
    {
        msgBox->showMyMessageBox(this, tr("错误提示"), tr("载入数据前，必须关闭串口!"), MESSAGE_WARNNING, BUTTON_OK, true);
        return;
    }

    // 载入数据前清除图表，避免加载数据产生曲线和串口接受曲线混杂
    m_chart->clearChart();

    // 载入数据
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, "载入数据", "", "Text File(*.txt)");
    if(fileName == "")
    {
        return ;
    }
    else
    {
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            msgBox->showMyMessageBox(this, tr("错误提示"), tr("文件打开失败!"), MESSAGE_WARNNING, BUTTON_OK, true);
            return;
        }
        else
        {
            if(!file.isReadable())
                msgBox->showMyMessageBox(this, tr("错误提示"), tr("文件不可读!"), MESSAGE_WARNNING, BUTTON_OK, true);
            else
            {
                QTextStream textStream(&file);
                int index = 0; // 通道序号
                while(!textStream.atEnd())
                {
                    QString line = textStream.readLine();
                    if(line.contains("ch")) // 获得通道序号
                    {
                        index = QString(line.split("ch").at(1)).toInt();
                        continue;
                    }
                    QStringList strList = line.split(" ");
                    // 将数据添加到曲线
                    m_chart->addChannelData(index-1, strList.at(0).toDouble(), strList.at(1).toInt());
                    // 同时本地保存数据(适用于打开软件后，先加载数据再保存数据的情况)
                    m_chart->saveChannelData(index-1, strList.at(0).toDouble(), strList.at(1).toInt());
                }
                file.close();
            }
        }
    }

    // 自适应设置所有通道的X和Y轴范围
    m_chart->adaptiveAll();

    // 刷新图表
    m_chart->replotChart();
}

// 截屏动作-触发槽函数
void MainWindow::on_screenAction_triggered()
{
    MyMessageBox *msgBox = new MyMessageBox();

    // 检查截图目录是否存在，若不存在则新建
    QString strDir = QCoreApplication::applicationDirPath() + "/screenshot";
    QDir dir;
    if (!dir.exists(strDir))
    {
        if(!dir.mkpath(strDir))
            msgBox->showMyMessageBox(this, tr("错误提示"), tr("新建截图目录失败!"), MESSAGE_WARNNING, \
                                            BUTTON_OK, true);
    }

    QPixmap pix = ui->chartWidget->grab(QRect(0,0,ui->chartWidget->width(),ui->chartWidget->height()));
    QString fileName= QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss")  + ".png";//通过时间命名文件
    if(!pix.save(QCoreApplication::applicationDirPath() + "/screenshot/" + fileName, "png"))
    {
        msgBox->showMyMessageBox(this, tr("错误提示"), tr("截图失败!"), MESSAGE_WARNNING, \
                                            BUTTON_OK, true);
    }
    else
    {
        msgBox->showMyMessageBox(this, tr("提示"), tr("截图已保存在：安装目录\\Screenshot目录下!"), MESSAGE_INFORMATION, \
                                            BUTTON_OK, true);
    }
}

// 关于动作-触发槽函数
void MainWindow::on_aboutAction_triggered()
{
    AboutDialog *aboutW = new AboutDialog(this);
    aboutW->show();
}

// 退出动作-触发槽函数
void MainWindow::on_exitAction_triggered()
{
    this->close();
}
