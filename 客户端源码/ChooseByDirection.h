#pragma once

#include <QLabel>
#include<QVector>
/**
 * @brief The ChooseByDirection class
 * 自定义的根据方向键切换模式的控件,支持鼠标点击，会自动获取焦点
 * T 为可选选项的数据类型
 */
class ChooseByDirection : public QLabel
{
    Q_OBJECT
public:
    ChooseByDirection(QWidget* parent);
public:
    virtual void setItems(const QVector<QString> &newItems);
    virtual void addItems(const QString& newItem);
    virtual const QString getCurrentItems()const;
private:
    QVector<QString> items;//
    qint32 indexForItems=0;
private slots:
    virtual void keyPressEvent(QKeyEvent* e)override;
    virtual void paintEvent(QPaintEvent*e)override;
    virtual void focusInEvent(QFocusEvent *e)override;
    virtual void focusOutEvent(QFocusEvent *e)override;
    virtual bool eventFilter(QObject *watched,QEvent *e)override;
    virtual void mousePressEvent(QMouseEvent* e)override;
    virtual void enterEvent(QEnterEvent *e)override;
//    virtual void leaveEvent(QEvent *e)override;//do nothing now
};

