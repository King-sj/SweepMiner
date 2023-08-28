#include "NetDenotationMod.h"
#include<QTimer>
#include<QTime>
#include<QRandomGenerator>
#include<set>
#include<QEvent>
//todo:这里是直接粘贴的DenotationMod.cpp的代码，不利于维护,后期再想办法吧
//todo:还没重写双方同步的代码,最好改成由服务器生成,包括NetBoard的部分
/**
 * @brief NetDenotationMod::NetDenotationMod
 * @param socket
 * @param rowNum
 * @param colNum
 * @param bombNum
 * @param parent
 * @param GameMod
 */
NetDenotationMod::NetDenotationMod(QTcpSocket *socket, qint32 rowNum, qint32 colNum, qint32 bombNum, QWidget *parent,QString GameMod)
    :NetBoard(socket,rowNum,colNum,bombNum,parent,GameMod),socket(socket),packet(this)
{
    remainBombNum = bombNum;
    this->moveNum = remainBombNum + 5;

    this->labelForGameTime->setText("Remain Move Number:"+QString::number(this->moveNum));

    this->isFirstClick = false;
    this->setBombs(Chunk::RowCol(-1,-1));

    this->gameTimer.data()->stop();
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
 * @brief NetDenotationMod::randomOpenNotBombChunk
 * @param num
 */
void NetDenotationMod::randomOpenNotBombChunk(qint32 num)
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
 * @brief NetDenotationMod::dealSignalExploded
 */
void NetDenotationMod::dealSignalExploded()
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
 * @brief NetDenotationMod::dealMatchResponse
 * @param list
 */
void NetDenotationMod::dealMatchResponse(QStringList list)
{
    if(list[0] == "1hs"){
       this->randomOpenNotBombChunk(this->rowNum*this->colNum/10);
    }
    NetBoard::dealMatchResponse(list);
}
/**
 * @brief NetDenotationMod::eventFilter
 * @param watched
 * @param e
 * @return
 */
bool NetDenotationMod::eventFilter(QObject *watched, QEvent *e)
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
