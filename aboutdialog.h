/*
* Copyright (c) 2019,浙江智澜科技有限公司
* All rights reserved.
*
* 摘 要：关于对话框类，显示关于信息
*
* 当前版本：1.5
* 作 者：聂咸丰
* 完成日期：2019 年 9 月 1 日
*/
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include <QDesktopServices>
#include "mytitlebar.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();    

protected:
    bool eventFilter(QObject *obj, QEvent *event); // 事件过滤器：只实现点击label打开网页的功能

private:
    Ui::AboutDialog *ui;

    MyTitleBar* m_titleBar; // 自定义标题栏
};

#endif // ABOUTDIALOG_H
