#include "ChooseByDirection.h"
#include<QKeyEvent>
#include<QPaintEvent>
#include<Base.h>
#include<QApplication>
/**
 * @brief ChooseByDirection::ChooseByDirection
 * @param parent
 */
ChooseByDirection::ChooseByDirection(QWidget* parent)
    :QLabel(parent)
{
    parent->installEventFilter(this);
    this->setAlignment(Qt::AlignHCenter);
    update();
}
/**
 * @brief ChooseByDirection::setItems
 * 更新items
 * @param newItems
 */
void ChooseByDirection::setItems(const QVector<QString> &newItems)
{
    items = newItems;
}
/**
 * @brief ChooseByDirection::addItems
 * 添加item
 * @param newItem
 */
void ChooseByDirection::addItems(const QString &newItem)
{
    if(newItem == ""){
        throw "item shouldn't be null.\n";
    }
    this->items.append(newItem);
}
/**
 * @brief ChooseByDirection::getCurrentItems
 * 返回当前item
 * @return
 */
const QString ChooseByDirection::getCurrentItems() const
{
    if(this->items.isEmpty())return "";
    return this->items[indexForItems];
}
/**
 * @brief ChooseByDirection::keyPressEvent
 * 处理方向键点击事件
 * @param e
 */
void ChooseByDirection::keyPressEvent(QKeyEvent *e)
{
    if(this->items.isEmpty())return;
    switch (e->key()) {
    case Qt::Key_Left:{
        indexForItems--;
        indexForItems = (indexForItems+items.size())%items.size();
        e->accept();
        break;}
    case Qt::Key_Right:{
        indexForItems++;
        indexForItems = indexForItems%items.size();
        e->accept();
        break;}
    default:
        break;
    }
    this->repaint();
    QLabel::keyPressEvent(e);
}
/**
 * @brief ChooseByDirection::paintEvent
 * @param e
 */
void ChooseByDirection::paintEvent(QPaintEvent *e)
{
    if(!items.isEmpty()){
        this->clear();
        this->setText("<   "+items[indexForItems]+"    >");
    }
    QLabel::paintEvent(e);
}
/**
 * @brief ChooseByDirection::focusInEvent
 * 获取焦点时的事件槽函数
 * @param e
 */
void ChooseByDirection::focusInEvent(QFocusEvent *e)
{
    this->setStyleSheet("border:2px solid skyblue;");
}
/**
 * @brief ChooseByDirection::focusOutEvent
 * 失去焦点事件槽函数
 * @param e
 */
void ChooseByDirection::focusOutEvent(QFocusEvent *e)
{
    this->setStyleSheet("border:transparent;");
}
/**
 * @brief ChooseByDirection::eventFilter
 * 事件过滤器,捕捉父对象的鼠标点击事件
 * @param watched
 * @param e
 * @return
 */
bool ChooseByDirection::eventFilter(QObject *watched, QEvent *e)
{
    if(e->type() == QMouseEvent::MouseButtonPress)
    {
        auto p = static_cast<QMouseEvent*>(e);
//        dout<<p->pos();
        if(p->button() == Qt::LeftButton)
            return true;//捕获点击事件,只会由this的mousePressEvent处理
    }
//    dout<<e->type();
    return QLabel::eventFilter(watched,e);//交给父类处理
}
/**
 * @brief ChooseByDirection::mousePressEvent
 * 鼠标点击事件
 * @param e
 */
void ChooseByDirection::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        this->setFocus();

        if(e->pos().x() < this->width()*0.3)
        {
            indexForItems--;
            indexForItems = (indexForItems+items.size())%items.size();
        }else if(e->pos().x() > this->width()*0.6)
        {
            indexForItems++;
            indexForItems = indexForItems%items.size();
        }

        e->accept();
    }
    e->ignore();
    QLabel::mousePressEvent(e);
}
/**
 * @brief ChooseByDirection::enterEvent
 * 鼠标进入事件，获取焦点
 * @param e
 */
void ChooseByDirection::enterEvent(QEnterEvent *e)
{
    this->setFocus();
    repaint();
    QLabel::enterEvent(e);
}
