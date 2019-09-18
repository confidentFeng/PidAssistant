/*
* Copyright (c) 2019,浙江智澜科技有限公司
* All rights reserved.
*
* 摘 要：图表类，主要显示5个通道的曲线
*
* 当前版本：1.5
* 作 者：聂咸丰
* 完成日期：2019 年 9 月 10 日
*/
#ifndef CHART_H
#define CHART_H

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTime>
#include <QDebug>
#include <assert.h>
#include "common.h"
#include "serial.h"
#include "mymessagebox.h"
#include "aboutdialog.h"
#include "qcustomplot.h"

#define CHANNEL_COUNT 5 // 通道数量

class Chart : public QMainWindow
{
    Q_OBJECT

public:
    Chart(QMainWindow *parent = nullptr);

    void initChart(QCustomPlot *ChartWidget); // 初始化图表
    void updataChart(char channel, QVector<int> vecPidData); // 更新图表
    bool eventFilter(QObject *obj, QEvent *event); // 事件过滤器
    void startTimer(); // 启动串口计时器
    void clearPauseTime();
    void addChannelData(int channel, double x, int y); // 往通道曲线中添加数据
    void clearChannel(int channel); // 清除通道对应曲线
    void clearChart(); // 清除图表
    void replotChart(); // 刷新图表
    void clearChannelData(); // 清空本地各通道的数据
    void saveChannelData(int channel, double x, int y); // 本地保存通道数据
    QMap<double, int>* getMapData(); // 获得本地保存的各通道数据
    void adaptiveAll(); // 自适应设置所有通道的X和Y轴范围
    void mouseCoordConver(QMouseEvent *event); // 鼠标坐标点转换函数
    void controlUpdata(bool flag); // 控制是否暂停更新

signals:
    void sendCoordToMain(float, float); // 发送鼠标坐标点给主窗口的信号

private:
    QCustomPlot *m_customPlot; // 图表实例指针
    QVector<int> m_vecPidLastData; // 存放5个通道的PID上一次结果值
    QMap<double, int> m_mapChannel[5]; // 本地保存各通道的数据
    bool m_ismoving; // 鼠标左键是否移动和滚轮是否使用的标志位

    QTime m_pauseTimer; // 暂停时间计时器
    int m_pauseTime; // 暂停时间
    QTime m_serialTime; // 串口打开后的计时器
};

#endif // CHART_H
