#include "DenotationMod.h"
#include<QTimer>
#include<QTime>
#include<QRandomGenerator>
#include<set>
#include<QEvent>
/**
 * @brief DenotationMod::DenotationMod
 * @param rowNum
 * @param colNum
 * @param bombNum
 * @param parent
 * @param GameMod
 */
DenotationMod::DenotationMod(qint32 rowNum, qint32 colNum, qint32 bombNum, QWidget *parent,QString GameMod)
    :Board(rowNum,colNum,bombNum,parent,GameMod)
{
    remainBombNum = bombNum;
    this->moveNum = remainBombNum + 5;

    this->labelForGameTime->setText("Remain Move Number:"+QString::number(this->moveNum));

    this->isFirstClick = false;
    this->setBombs(Chunk::RowCol(-1,-1));
    randomOpenNotBombChunk(this->rowNum*this->colNum/10);
    //note:此处把gameTimer->stop注释掉可能会导致时间计算不准确
//    this->gameTimer.data()->stop();
    this->timeEditGameTime->hide();

    for (qint32 i = 0; i < this->rowNum; ++i) {
        for(qint32 j = 0 ; j < this->colNum ; ++j)
        {
            this->chunks[i][j]->installEventFilter(this);
            connect(this->chunks[i][j],&Chunk::signalClickChunk,[this](){
                this->moveNum--;
                if(this->moveNum < this->remainBombNum)
                {
                    gameOver("lose");
                }
                this->labelForGameTime->setText("Remain Move Number:"+QString::number(this->moveNum));
            });
        }
    }
}
/**
 * @brief DenotationMod::randomOpenNotBombChunk
 * 随即翻开非雷的chunk
 * @param num 翻开的数量
 */
void DenotationMod::randomOpenNotBombChunk(qint32 num)
{
    auto rng = QRandomGenerator(*QRandomGenerator::system());
    std::set<Chunk::RowCol>rcs;
    while(rcs.size() < num)
    {
        int r = rng.bounded(0,this->rowNum);
        int c = rng.bounded(0,this->colNum);
        if(this->chunks[r][c]->getMineType() != Chunk::MineType::Bomb && this->chunks[r][c]->getSurroundBomb() != 0 )
        {
            rcs.insert(Chunk::RowCol(r,c));
        }
    }
    foreach (auto& rc,rcs) {
        this->chunks[rc.row][rc.col]->openThenShow();
        dealClickChunk(rc);
    }
    update();
}
/**
 * @brief DenotationMod::dealSignalExploded
 * 重载触雷事件
 */
void DenotationMod::dealSignalExploded()
{
    auto p = (Chunk*)(sender());
    p->setMineType(Chunk::MineType::NotBomb);//
    dealClickChunk(p->getRowCol());
    p->setMineType(Chunk::MineType::Bomb);
    p->setMineState(Chunk::MineState::FlagBomb);

    this->moveNum--;
    this->remainBombNum--;
    this->lcdNumberRemainBombNum->display(this->remainBombNum);
    if(remainBombNum == 0)
    {
        gameOver("win");
    }
    this->labelForGameTime->setText("Remain Move Number:"+QString::number(this->moveNum));

    p->setPix(QPixmap(":/img/Resource/img/safeBomb.png").scaled(p->getPix().size()));
    //todo:改成bfs
    for(qint32 u = 0 ; u < 5 ; u++)
    for(qint32 i = 0 ; i < this->rowNum ; i++)
    {
        for(qint32 j = 0 ; j < this->colNum ; j++)
        {
            if(this->chunks[i][j]->getMineState() == Chunk::MineState::Mined)
            {
                Board::dealDoubleClickChunk(Chunk::RowCol(i,j),true);
            }
        }
    }
    update();
}
/**
 * @brief DenotationMod::eventFilter
 * 截取鼠标右键事件
 * @param watched
 * @param e
 * @return
 */
bool DenotationMod::eventFilter(QObject *watched, QEvent *e)
{
    if(e->type() == QEvent::MouseButtonPress)
    {
        auto p = static_cast<QMouseEvent*>(e);
        if(p->button() == Qt::RightButton)
        {
            e->accept();
            return true;
        }
    }
    return false;
}
