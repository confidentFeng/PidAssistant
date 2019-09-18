/*
* Copyright (c) 2019,浙江智澜科技有限公司
* All rights reserved.
*
* 摘 要：实现主窗口的基本功能
*
* 当前版本：1.5
* 作 者：聂咸丰
* 完成日期：2019 年 8 月 28 日
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTime>
#include <QDebug>
#include <assert.h>
#include "qcustomplot.h"
#include "common.h"
#include "serial.h"
#include "chart.h"
#include "mymessagebox.h"
#include "aboutdialog.h"
#include <QScreen>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void runUpdataChart(); // 运行更新绘图函数的函数
    bool eventFilter(QObject *obj, QEvent *event); // 事件过滤器
    void initRightClickMenu(); // 初始化右键菜单

private slots:
    void readFromSerial(); // 读取来自串口类的数据
    void handleSerialError(); // 处理串口错误
    void onNewPortList(QStringList portName); // 用于响应SerialPortList信号，周期获取串口列表
    void on_openSerialButton_clicked(); // 打开串口按钮-点击槽函数    
    void on_readPidButton_clicked(); // 读取PID按钮-点击槽函数
    void on_writePidButton_clicked(); // 写入PID按钮-点击槽函数
    void on_resButton_clicked(); // 恢复出厂按钮-点击槽函数
    void on_setButton_clicked(); // 设置出厂按钮-点击槽函数
    void on_pauseUpdateButton_clicked(); // 暂停更新按钮-点击槽函数
    void on_windowTopButton_clicked(); // 窗口置顶按钮-点击槽函数
    void showCoor(float x_val, float y_val); // 在下方状态栏上显示鼠标所在位置

    void on_saveAction_triggered(); // 保存数据-触发槽函数
    void on_loadAction_triggered(); // 载入数据-触发槽函数
    void on_screenAction_triggered(); // 截屏动作-触发槽函数
    void on_aboutAction_triggered(); // 关于动作-触发槽函数
    void on_exitAction_triggered(); // 退出动作-触发槽函数

signals:
    void updataSignal(); // 更新图表信号

private:
    Ui::MainWindow *ui;

    Serial *m_serial; // 自定义串口类实例指针
    Chart *m_chart; // 图表实例指针
    QVector<int> m_vecPidData; // 存放5个通道的PID结果值    
    char m_channel; // 有效通道
    bool m_isPause = true; // 是否暂停更新的标志位
    int m_currIndex;
    Qt::WindowFlags m_windowFlags; // 窗口置顶标志
};

#endif // MAINWINDOW_H
