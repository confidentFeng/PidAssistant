#include "chart.h"

Chart::Chart(QMainWindow *parent) : QMainWindow(parent)
{
    // 初始化成员变量
    m_pauseTime = 0;
    m_vecPidLastData.resize(5); // 存放5个通道的PID上一次结果值
    m_ismoving = false;
}

void Chart::initChart(QCustomPlot *ChartWidget)
{
    // 初始化图表实例指针
    m_customPlot = ChartWidget;
    // 将customPlot的mouseMove信号连接到mouseCoordConver函数
    connect(m_customPlot, &QCustomPlot::mouseMove, this, &Chart::mouseCoordConver);
    // 安装事件过滤器
    m_customPlot->installEventFilter(this);
    // 设置背景色
    m_customPlot->setBackground(QBrush(QColor(0, 0, 0)));
    // 设置x/y轴刻度标签颜色
    m_customPlot->yAxis->setTickLabelColor(QColor(255, 255, 255));
    m_customPlot->xAxis->setTickLabelColor(QColor(255, 255, 255));
    // 支持鼠标拖拽轴的范围、滚动缩放轴的范围，左键点选图层（每条曲线独占一个图层）
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // 添加通道1曲线
    m_customPlot->addGraph(); // 向绘图区域QCustomPlot添加一条曲线
    m_customPlot->graph(0)->setPen(QPen(Qt::red, 3)); // 设置曲线颜色
    // 添加通道2曲线
    m_customPlot->addGraph();
    m_customPlot->graph(1)->setPen(QPen(Qt::magenta, 3));
    // 添加通道3曲线
    m_customPlot->addGraph();
    m_customPlot->graph(2)->setPen(QPen(Qt::green, 3));
    // 添加通道4曲线
    m_customPlot->addGraph();
    m_customPlot->graph(3)->setPen(QPen(Qt::blue, 3));
    // 添加通道5曲线
    m_customPlot->addGraph();//添加graph等价于添加新曲线
    m_customPlot->graph(4)->setPen(QPen(Qt::cyan, 3));

    // 创建时间坐标轴
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    timeTicker->setTickCount(10); // 设置越大，同屏显示的时间栅格越多
    m_customPlot->xAxis->setTicker(timeTicker);

    for(int i=0; i<CHANNEL_COUNT; i++)
    {
        // 自动设定graph曲线x和y轴的范围
        m_customPlot->graph(i)->rescaleAxes(true);
        // 解决点击曲线，曲线变蓝色的问题
        m_customPlot->graph(i)->setSelectable(QCP::stNone);
    }

    // 立即刷新图表
    m_customPlot->replot();
}

// 更新图表
void Chart::updataChart(char channel, QVector<int> vecPidData)
{
    // 自串口打开以来经过的时间-暂停时间，即x轴的时间值，以s为单位
    double XAxisTime = (m_serialTime.elapsed() - this->m_pauseTime)/1000.0;

    // 关闭通道后停止向graph里面添加数据
    if((channel&0x01) == 0x01)
    {
        if(m_vecPidLastData[0] != vecPidData[0]) //值改变才添加数据到graph中
        {
            // 将数据添加到曲线
            m_customPlot->graph(0)->addData(XAxisTime, vecPidData[0]);
            // 添加到本地数据vector，方便保存和重新加载
            m_mapChannel[0].insert(XAxisTime, vecPidData[0]);
            // 获得上一次的数据(曲线不光滑的原因是好几次的Y值都一样，导致横线[时间过长]过长)
            m_vecPidLastData[0] = vecPidData[0];
        }
    }
    if((channel&0x02) == 0x02)
    {
        if(m_vecPidLastData[1] != vecPidData[1])
        {
            m_customPlot->graph(1)->addData(XAxisTime, vecPidData[1]);
            m_mapChannel[1].insert(XAxisTime, vecPidData[1]);
            m_vecPidLastData[1] = vecPidData[1];
        }
    }
    if((channel&0x04) == 0x04)
    {
        if(m_vecPidLastData[2] != vecPidData[2])
        {
            m_customPlot->graph(2)->addData(XAxisTime, vecPidData[2]);
            m_mapChannel[2].insert(XAxisTime, vecPidData[2]);
            m_vecPidLastData[2] = vecPidData[2];
        }
    }
    if((channel&0x08) == 0x08)
    {
        if(m_vecPidLastData[3] != vecPidData[3])
        {
            m_customPlot->graph(3)->addData(XAxisTime, vecPidData[3]);
            m_mapChannel[3].insert(XAxisTime, vecPidData[3]);
            m_vecPidLastData[3] = vecPidData[3];
        }
    }
    if((channel&0x10) == 0x10)
    {
        if(m_vecPidLastData[4] != vecPidData[4])
        {
            m_customPlot->graph(4)->addData(XAxisTime, vecPidData[4]);
            m_mapChannel[4].insert(XAxisTime, vecPidData[4]);
            m_vecPidLastData[4] = vecPidData[4];
        }
    }

    // 自适应设置X和Y轴的范围
    if(!m_ismoving)
    {
        // 自定义x轴值的显示范围(起始值为elapsedTime，终止值为TIMER_COUNT)
        m_customPlot->xAxis->setRange(XAxisTime, 8, Qt::AlignRight);

        for(int i=0; i<CHANNEL_COUNT; i++)
            m_customPlot->graph(i)->rescaleValueAxis(true);
    }

    // 立即刷新图像
    m_customPlot->replot();
}

// 事件过滤器
bool Chart::eventFilter(QObject *obj, QEvent *event)
{
    // 检测是否使用鼠标
    if (obj == m_customPlot && event->type()==QEvent::MouseButtonPress)
    {
        QMouseEvent *event_ = static_cast<QMouseEvent*>(event);
        if(m_customPlot->rect().contains(event_->pos()) && event_->button() == Qt::LeftButton)
        {
            // 鼠标左键移动，则将m_ismoving设置为true
            m_ismoving = true;
        }
    }
    else if (event->type()==QEvent::MouseButtonRelease)
    {
        // 鼠标左键释放，则将m_ismoving设置为false
        m_ismoving = false;
    }
    // 检测是否使用滚轮
    if(event->type()==QEvent::Wheel)
    {
        m_ismoving = true;
    }

    return QWidget::eventFilter(obj,event);
}

// 启动串口计时器
void Chart::startTimer()
{
    m_serialTime.start();
}

// 清零暂停时间
void Chart::clearPauseTime()
{
    m_pauseTime = 0;
}

// 往通道曲线中添加数据
void Chart::addChannelData(int channel, double x, int y)
{
    m_customPlot->graph(channel)->addData(x, y);
}

// 清除通道对应曲线
void Chart::clearChannel(int channel)
{
    // 写入空的vector即可清除曲线
    m_customPlot->graph(channel)->setData({}, {});
}

// 清除图表
void Chart::clearChart()
{
    for(int i=0; i<CHANNEL_COUNT; i++)
        m_customPlot->graph(i)->setData({}, {});
}

// 刷新图表
void Chart::replotChart()
{
    m_customPlot->replot();
}

// 清空本地各通道的数据
void Chart::clearChannelData()
{
    for(int i=0; i<CHANNEL_COUNT; i++)
        m_mapChannel[0].clear();
}

// 本地保存通道数据
void Chart::saveChannelData(int channel, double x, int y)
{
    m_mapChannel[channel].insert(x, y);
}

// 获得本地保存的各通道数据
QMap<double, int>* Chart::getMapData()
{
    return m_mapChannel;
}

// 自适应设置所有通道的X和Y轴范围
void Chart::adaptiveAll()
{
    // 自适应设置X和Y轴的范围
    for(int i=0; i<CHANNEL_COUNT; i++)
        m_customPlot->graph(i)->rescaleAxes(true);
}

// 鼠标坐标点转换函数
void Chart::mouseCoordConver(QMouseEvent *event)
{
    // 把鼠标坐标点 转换为 QCustomPlot 内部坐标值 （pixelToCoord 函数）
//    float x_val = static_cast<float>(m_customPlot->xAxis->pixelToCoord(event->pos().x()));
//    float y_val = static_cast<float>(m_customPlot->yAxis->pixelToCoord(event->pos().y()));
    double x_val = m_customPlot->xAxis->pixelToCoord(event->pos().x());
    double y_val = m_customPlot->yAxis->pixelToCoord(event->pos().y());

    QString xValStr = QString::number(x_val, 'f', 2);
    QString yValStr = QString::number(y_val, 'f', 2);
    // 在图表区域中鼠标移动，则将内部鼠标坐标点发送给下方状态栏中的X和Y标签
    emit sendCoordToMain(xValStr.toFloat(), yValStr.toFloat());
}

// 控制是否暂停更新
void Chart::controlUpdata(bool pauseFlag)
{
    if(!pauseFlag)
        m_pauseTime += this->m_pauseTimer.elapsed(); // 计算总暂停时间
    else
        m_pauseTimer.restart(); // 暂停更新后开始计时
}
