#ifndef MESSAGETIPS_H
#define MESSAGETIPS_H

#include <QWidget>
#include<QPointer>
//#pragma execution_character_set(push) // push the previous character
# pragma execution_character_set("utf-8")

class QHBoxLayout;
class QLabel;
/**
 * @brief The MessageTips class
 * 实现自动消失的消息框,由于时间原因,此代码借鉴于csdn
 * 必须使用指针类型或者指定父对象
 * @my doing
 * 添加了对qt6的兼容
 * 添加关闭时自动删除，释放资源,避免内存泄露
 * @todo 添加move功能
 * 改成更好看的样式
 *
 */
class MessageTips : public QWidget
{
    Q_OBJECT
public:
    explicit MessageTips(QString showStr="none", QWidget *parent = nullptr);
    ~MessageTips();
    double getOpacityValue() const;
    void setOpacityValue(double value);

    qint32 getTextSize() const;
    void setTextSize(int value);

    QColor getTextColor() const;
    void setTextColor(const QColor &value);

    QColor getBackgroundColor() const;
    void setBackgroundColor(const QColor &value);

    QColor getFrameColor() const;
    void setFrameColor(const QColor &value);

    qint32 getFrameSize() const;
    void setFrameSize(int value);

    qint32 getShowTime() const;
    void setShowTime(int msec);

    void setCloseTimeSpeed(int closeTime = 100,double closeSpeed = 0.1);
public:
//    virtual void show()override;
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void InitLayout();//初始化窗体的布局和部件
    QPointer<QHBoxLayout> hBoxlayout;//布局显示控件布局
    QPointer<QLabel> mText;//用于显示文字的控件
    QString showStr;//显示的字符串

    double opacityValue;//窗体初始化透明度
    QFont* font;
    qint32     textSize;//显示字体大小
    QColor  textColor;//字体颜色
    QColor  backgroundColor;//窗体的背景色
    QColor  frameColor;//边框颜色
    qint32     frameSize;//边框粗细大小

    qint32     showTime;//显示时间
    qint32     closeTime;//关闭需要时间
    double  closeSpeed;//窗体消失的平滑度，大小0~1

signals:

};
//#pragma execution_character_set(pop) // pop the previous character set
#endif // MESSAGETIPS_H
