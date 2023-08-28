#include "Chunk.h"
#include<QMenu>
#include<QPainter>
#include"Base.h"
#include<QTimer>
#include<QPointer>
/**
 * @brief Chunk::Chunk
 * @param parent
 */
Chunk::Chunk(QWidget *parent):
    QLabel(parent)
{
    this->pix = QPixmap(":/img/Resource/img/unknownChunk.png").scaled(this->pixSize);
}
/**
 * @brief Chunk::setRowCol
 * 设置坐标
 * @param row 行坐标
 * @param col 列坐标
 */
void Chunk::setRowCol(qint32 row, qint32 col)
{
    this->rowCol = RowCol(row,col);
}
/**
 * @brief Chunk::setMineType
 * 设置类型
 * @param mt
 */
void Chunk::setMineType(const MineType mt)
{
    this->mineType = mt; 
}
/**
 * @brief Chunk::getMineType
 * 返回类型
 * @return
 */
Chunk::MineType Chunk::getMineType() const
{
    return this->mineType;
}
/**
 * @brief Chunk::paintEvent
 * @param e
 */
void Chunk::paintEvent(QPaintEvent *e)
{
//    QLabel::paintEvent(e);
    QPainter p;
    p.begin(this);
    p.drawPixmap(0,0,this->width(),this->height(),this->pix.scaled(this->size()));
    p.end();    
}
/**
 * @brief Chunk::mousePressEvent
 * @param e
 */
void Chunk::mousePressEvent(QMouseEvent *e)
{    
    if(!clickable)return;
    if(e->button() == Qt::LeftButton && mineState != MineState::Mined)//左键单击
    {        
        if(this->mineState == MineState::Mined || this->mineState == MineState::FlagBomb)return;//
        if(this->mineType == MineType::Bomb){
            this->pix = QPixmap(":/img/Resource/img/explodedBomb.png").scaled(this->pixSize);
            update();
            mineState = MineState::Mined;
            emit signalExploded();//爆炸
            return;
        }
        emit signalClickChunk(this->rowCol);        
    }
    if(e->button() == Qt::RightButton && mineState != MineState::Mined)
    {        
        switch (mineState) {
        case MineState::UnMined:
            this->pix = QPixmap(":/img/Resource/img/flagBomb.png").scaled(this->pixSize);
            this->mineState = MineState::FlagBomb;
            emit signalFlagBombChanged(1);
            break;
        case MineState::FlagBomb:
            this->pix = QPixmap(":/img/Resource/img/flagQuestion.png").scaled(this->pixSize);
            this->mineState = MineState::FlagQuestion;
            emit signalFlagBombChanged(-1);
            break;
        default:
            this->pix = QPixmap(":/img/Resource/img/unknownChunk.png").scaled(this->pixSize);
            this->mineState = MineState::UnMined;
//            emit signalFlagBombChanged(-1);
            break;
        }
        update();
        //initRightKeyMenu();//游戏操作理解错误
    }
}

/**
 * @brief 实现在右键点击的地方显示菜单,已暂时废弃,暂时留着，以备后续迭代时可能会重新启用
 * @param void
 */
[[deprecated]] void Chunk::initRightKeyMenu()
{
    //创建菜单对象
    QMenu *pMenu = new QMenu(this);

    QAction *pFlagBomb = new QAction(tr("标记为炸弹"), this);
    pFlagBomb->setData(static_cast<qint32>(RIGHT_KEY_MENU::FlagBomb));
    //把QAction对象添加到菜单上
    pMenu->addAction(pFlagBomb);
    //连接鼠标右键点击信号
    connect(pFlagBomb,&QAction::triggered,this,&Chunk::onTaskBoxContextMenuEvent);

    QAction *pFlagQueStion = new QAction(tr("标记为问题"), this);
    pFlagQueStion->setData(static_cast<qint32>(RIGHT_KEY_MENU::FlagQuestion));
    pMenu->addAction(pFlagQueStion);
    connect(pFlagQueStion,&QAction::triggered,this,&Chunk::onTaskBoxContextMenuEvent);

    //在鼠标右键点击的地方显示菜单
    pMenu->exec(cursor().pos());

    //释放内存
    QList<QAction*> list = pMenu->actions();
    foreach (QAction* pAction, list) delete pAction;
    delete pMenu;
}
/**
 * @brief Chunk::onTaskBoxContextMenuEvent
 *
 */
void Chunk::onTaskBoxContextMenuEvent()
{
    QAction *pEven = qobject_cast<QAction *>(this->sender()); //this->sender()就是发信号者 QAction
        if(pEven == nullptr){
            dout<<"invalid triggered\n";
            return;
        }

        //获取发送信息类型
        int iType = pEven->data().toInt();
        dout<<"iType:"<<iType<<dendl;
        switch (iType)
        {
        case static_cast<qint32>(RIGHT_KEY_MENU::FlagBomb):
            this->pix = QPixmap(":/img/Resource/img/flagBomb.png").scaled(this->pixSize);
            this->mineState = MineState::FlagBomb;
            break;
        case static_cast<qint32>(RIGHT_KEY_MENU::FlagQuestion):
            this->pix = QPixmap(":/img/Resource/img/flagQuestion.png").scaled(this->pixSize);
            this->mineState = MineState::FlagQuestion;
            break;
        default:
            dout<<"do nothing"<<dendl;
            break;
        }
    update();
}
/**
 * @brief Chunk::mouseDoubleClickEvent
 * 双击
 * @param e
 */
void Chunk::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(!clickable)return;
    if(this->mineState == MineState::Mined)
        emit signalDoubleClickChunk(rowCol);
    //    this->setEnabled(true);
}
/**
 * @brief Chunk::getPix
 * 返回全局画布
 * @return
 */
const QPixmap &Chunk::getPix() const
{
    return pix;
}
/**
 * @brief Chunk::setPix
 * 设置画布
 * @param newPix
 */
void Chunk::setPix(const QPixmap &newPix)
{
    pix = newPix;
}

//void Chunk::setClickable(bool newClickable)
//{
//    clickable = newClickable;
//}
/**
 * @brief Chunk::showBomb
 * 翻开所有雷
 */
void Chunk::showBomb()
{
    if(this->getMineType() == MineType::Bomb && this->getMineState() != MineState::Mined && this->getMineState() != MineState::FlagBomb){
        openThenShow();
    }
}
/**
 * @brief Chunk::openThenShow
 * 翻开并显示
 */
void Chunk::openThenShow()
{
    if(this->mineType == MineType::NotBomb)
    {
        this->pix = QPixmap(":/img/Resource/img/safeChunk.png").scaled(this->pixSize);
        drawSurroundBombNum(this->surroundBomb);
    }else if(this->mineState != MineState::Mined){
        this->pix = QPixmap(":/img/Resource/img/unMinedBomb.png").scaled(this->pixSize);
    }
    this->mineState = MineState::Mined;
    update();
}

/**
 * @brief Chunk::getMineState
 * 返回探索状态
 * @return
 */
Chunk::MineState Chunk::getMineState() const
{
    return mineState;
}
/**
 * @brief Chunk::setMineState
 * @param newMineState
 */
void Chunk::setMineState(MineState newMineState)
{
    mineState = newMineState;
    switch (mineState) {
    case Chunk::MineState::Mined:
        if(this->mineType == Chunk::MineType::NotBomb){
            openThenShow();
        }
        break;
    default:
        //todo:do some other condition
        dout<<"do nothing expect set MineState"<<dendl;
        break;
    }
}
/**
 * @brief Chunk::floatByDoubleClick
 * 浮动效果
 */
void Chunk::floatByDoubleClick()
{
    auto temp = this->pix;
    this->pix =QPixmap(":/img/Resource/img/downChunk.png").scaled(this->pixSize);

    QTimer::singleShot(100, this, [this,temp](){
        this->pix = temp;
        this->clickable = true;
        update();
    });
    update();
}
/**
 * @brief Chunk::getRowCol
 * 返回坐标
 * @return
 */
const Chunk::RowCol Chunk::getRowCol() const
{
    return rowCol;
}
/**
 * @brief Chunk::drawSurroundBombNum
 * 根据周围的雷数画数字,数字颜色会根据num自动变化
 * @param num
 */
void Chunk::drawSurroundBombNum(qint32 num)
{
    if(!num)return;
    QPainter p;
    QFont font;
    QPen pen;
    p.begin(&this->pix);


    font.setFamily("Microsoft YaHei");
    font.setPointSize(qMin(this->pix.size().width(),this->pix.height())*0.8);
    p.setFont(font);
    //颜色随危险程度渐变
    qint32 r = -255*(num-8)*(num-8)/49+255;//0->255
    qint32 g = 255*(num-8)*(num-8)/49;//255->0
    pen.setColor(qRgba(r,g,0,0));
    p.setPen(pen);

    QRect rec(0,0,this->pix.width(),this->pix.height());
    p.drawText(rec,Qt::AlignHCenter | Qt::AlignVCenter,QString::number(num));
    p.end();
    update();
}
/**
 * @brief Chunk::getSurroundBomb
 * @return
 */
qint32 Chunk::getSurroundBomb() const
{
    return surroundBomb;
}
/**
 * @brief Chunk::setSurroundBomb
 * @param newSurroundBomb
 */
void Chunk::setSurroundBomb(qint32 newSurroundBomb)
{
    surroundBomb = newSurroundBomb;
}
