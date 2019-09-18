#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // 让窗口居中显示 | 隐藏标题栏
    this->setWindowFlags(Qt::Window |Qt::FramelessWindowHint);
    // 设置为模态对话框
    this->setModal(true);
    // label安装事件过滤器，实现鼠标点击事件
    ui->webLabel->installEventFilter(this);

    // 初始化标题栏
    m_titleBar = new MyTitleBar(this);
    // 设置窗口边框宽度
    m_titleBar->setWindowBorderWidth(0);
    // 设置标题栏颜色
    m_titleBar->setBackgroundColor(191, 191, 191);
    // 设置标题栏上按钮类型
    m_titleBar->setButtonType(ONLY_CLOSE_BUTTON);
    // 设置标题栏长度
    m_titleBar->setTitleWidth(this->width());
//    // 设置标题颜色
//    m_titleBar->setTitleColor(0, 0, 0);
    // 设置标题栏图标
    m_titleBar->setTitleIcon("://image/MyMessageBox/WindowIcon.png", QSize(16, 16));
    // 设置标题内容
    m_titleBar->setTitleContent("关于", 12);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

// 事件过滤器：只实现点击label打开网页的功能
bool AboutDialog::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==ui->webLabel && event->type()==QEvent::MouseButtonPress)
    {
        QDesktopServices::openUrl(QUrl(QLatin1String("http://sealandrobo.com/")));
    }

    return QWidget::eventFilter(obj,event);
}
