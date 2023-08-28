#include "MessageTips.h"

#include <QScreen>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include<QApplication>
/**
 * @brief MessageTips::MessageTips
 * @param showStr 显示的文字
 * @param parent 父对象
 */
MessageTips::MessageTips(QString showStr,QWidget *parent)
    : QWidget(parent),
   opacityValue(1.00),
   textSize(18),
   textColor(QColor(255,255,255)),
   backgroundColor(QColor(192,192,192)),
   frameColor(QColor(211,211,211)),
   frameSize(2),
   showTime(3500),
   closeTime(100),
   hBoxlayout(new QHBoxLayout(this)),
   mText(new QLabel(this)),
   closeSpeed(0.2)
{
    setWindowFlags(Qt::Window|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::Tool|Qt::X11BypassWindowManagerHint);
    this->setAttribute(Qt::WA_TranslucentBackground); //半透明
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);// 禁止鼠标事件
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->showStr = showStr;
    hBoxlayout->addWidget(mText);
    InitLayout();
}
/**
 * @brief MessageTips::~MessageTips
 */
MessageTips::~MessageTips()
{
    delete font;
    font = nullptr;
}
/**
 * @brief MessageTips::InitLayout
 * 初始化界面
 */
void MessageTips::InitLayout()
{
    this->setWindowOpacity(opacityValue);

    //文字显示居中，设置字体，大小，颜色
    font = new QFont("微软雅黑",textSize,QFont::Bold);
    mText->setFont(*font);
    mText->setText(showStr);
    mText->setAlignment(Qt::AlignCenter);
    QPalette label_pe;//设置字体颜色
    label_pe.setColor(QPalette::WindowText, textColor);
    mText->setPalette(label_pe);

    QPointer<QTimer> mtimer = new QTimer(this);//隐藏的定时器
    mtimer->setTimerType(Qt::PreciseTimer);
    connect(mtimer,&QTimer::timeout,this,[this](){
        if(opacityValue<=0){ this->close(); }
        opacityValue -= closeSpeed;
        this->setWindowOpacity(opacityValue);    //设置窗口透明度
        });


    QPointer<QTimer> mShowtimer = new QTimer(this);//显示时间的定时器
    mShowtimer->setTimerType(Qt::PreciseTimer);// 修改定时器对象的精度
    connect(mShowtimer,&QTimer::timeout,this,[=](){
        mtimer->start(closeTime);//执行延时自动关闭
        });
    mShowtimer->start(showTime);
    ///todo:改成自定义位置/移动的
    ///note:居中效果不好，原因未知
    //设置屏幕水平居中显示
    auto desktop = QGuiApplication::primaryScreen()->geometry(); // =qApp->desktop();也可以
    this->move((desktop.width() - this->width())/2, 20);
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);// 禁止鼠标事件
}
/**
 * @brief MessageTips::paintEvent
 * @param event
 */
void MessageTips::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setBrush(QBrush(backgroundColor));//窗体的背景色

    painter.setPen(QPen(frameColor,frameSize));//窗体边框的颜色和笔画大小
    QRectF rect(0, 0, this->width(), this->height());
    painter.drawRoundedRect(rect, 15, 15); // round rect
}

/**
 * @brief MessageTips::setCloseTimeSpeed
 * 设置关闭的时间和速度，speed大小限定0~1
 * @param closeTime
 * @param closeSpeed
 */
void MessageTips::setCloseTimeSpeed(int closeTime, double closeSpeed)
{
    if(closeSpeed>0 && closeSpeed<=1){
       this->closeSpeed = closeSpeed;
    }
   this->closeTime = closeTime;
    InitLayout();
}

/**
 * @brief MessageTips::getShowTime
 * @return
 */
int MessageTips::getShowTime() const
{
    return showTime;
}
/**
 * @brief MessageTips::setShowTime
 * @param value
 */
void MessageTips::setShowTime(int value)
{
    showTime = value;
    InitLayout();
}
/**
 * @brief MessageTips::getFrameSize
 * @return
 */
int MessageTips::getFrameSize() const
{
    return frameSize;
}
/**
 * @brief MessageTips::setFrameSize
 * @param value
 */
void MessageTips::setFrameSize(int value)
{
    frameSize = value;
}
/**
 * @brief MessageTips::getFrameColor
 * @return
 */
QColor MessageTips::getFrameColor() const
{
    return frameColor;
}
/**
 * @brief MessageTips::setFrameColor
 * @param value
 */
void MessageTips::setFrameColor(const QColor &value)
{
    frameColor = value;
}
/**
 * @brief MessageTips::getBackgroundColor
 * @return
 */
QColor MessageTips::getBackgroundColor() const
{
    return backgroundColor;
}
/**
 * @brief MessageTips::setBackgroundColor
 * @param value
 */
void MessageTips::setBackgroundColor(const QColor &value)
{
    backgroundColor = value;
}
/**
 * @brief MessageTips::getTextColor
 * @return
 */
QColor MessageTips::getTextColor() const
{
    return textColor;
}
/**
 * @brief MessageTips::setTextColor
 * @param value
 */
void MessageTips::setTextColor(const QColor &value)
{
    textColor = value;
    InitLayout();
}
/**
 * @brief MessageTips::getTextSize
 * @return
 */
int MessageTips::getTextSize() const
{
    return textSize;
}
/**
 * @brief MessageTips::setTextSize
 * @param value
 */
void MessageTips::setTextSize(int value)
{
    textSize = value;
    InitLayout();
}
/**
 * @brief MessageTips::getOpacityValue
 * @return
 */
double MessageTips::getOpacityValue() const
{
    return opacityValue;
}
/**
 * @brief MessageTips::setOpacityValue
 * @param value
 */
void MessageTips::setOpacityValue(double value)
{
    opacityValue = value;
    InitLayout();
}

