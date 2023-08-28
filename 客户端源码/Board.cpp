#include "Board.h"
#include"Base.h"
#include<QPainter>
#include<QTimer>
#include<QRandomGenerator>
#include"BackgroundMusicPlayer.h"
/**
 * @brief Board::Board
 * @param parent
 */
/**
 * @brief Board::Board
 * @param rowNum 行数
 * @param colNum 列数
 * @param bombNum 雷数
 * @param parent
 */
Board::Board(qint32 rowNum, qint32 colNum, qint32 bombNum, QWidget *parent,QString GameMod)
    :QMainWindow{parent},rowNum(rowNum),colNum(colNum),bombNum(bombNum),
      pix(new QPixmap(":/img/Resource/img/background.png")),GameMod(GameMod)
{
    setupUi(this);
    this->init();    
//    setWindowFlags((windowFlags() & ~Qt::WindowCloseButtonHint & Qt::WindowMinMaxButtonsHint & Qt::WindowMinimizeButtonHint));
    dout<<this->thread()->currentThreadId();

    this->timeEditGameTime->setDisplayFormat("HH:mm:ss"); 
    gameTimer = new QTimer();
    connect(gameTimer,&QTimer::timeout,[this](){
        this->timeEditGameTime->setTime(this->timeEditGameTime->time().addSecs(1));
    });
    gameTimer->start(1000);
    this->lcdNumberRemainBombNum->display(this->bombNum);
    this->setWindowTitle("扫雷");
}
/**
 * @brief Board::~Board
 */
Board::~Board()
{
    dout<<"close";
    gameTimer->stop();
    backgroundPlayerThread->quit();
    backgroundPlayerThread->wait();
}
/**
 * @brief Board::getRowNum
 * @return
 */
qint32 Board::getRowNum() const
{
    return rowNum;
}
/**
 * @brief Board::getColNum
 * @return
 */
qint32 Board::getColNum() const
{
    return colNum;
}
/**
 * @brief Board::getBombNum
 * @return
 */
qint32 Board::getBombNum() const
{
    return bombNum;
}
/**
 * @brief Board::getGameTimer
 * 获得游戏计时器对象
 * @return
 */
QPointer<QTimer> Board::getGameTimer() const
{
    return gameTimer;
}
/**
 * @brief Board::setIsFirstClick
 * @param newIsFirstClick 是否是第一次点击
 */
void Board::setIsFirstClick(bool newIsFirstClick)
{
    isFirstClick = newIsFirstClick;
}
/**
 * @brief 初始化...
 */
void Board::init()
{
    isFirstClick = true;
    minedNum = 0;
    this->chunks.resize(this->rowNum);
    for(qint32 i = 0 ; i < this->rowNum ; i++)
    {
        this->chunks[i].resize(this->colNum);
        for(qint32 j = 0 ; j < this->colNum ; j++)
        {
            this->chunks[i][j] = new Chunk(this);
            auto& p=this->chunks[i][j];
            //            p->setParent(this);
            p->setRowCol(i,j);
            //事件绑定
            connect(p,&Chunk::signalExploded,this,&Board::dealSignalExploded);
            connect(p,&Chunk::signalClickChunk,this,&Board::dealClickChunk);
            connect(p,&Chunk::signalDoubleClickChunk,this,&Board::dealDoubleClickChunk);
            connect(p,&Chunk::signalFlagBombChanged,[this](qint32 changedNum){
                this->flagBombNum+=changedNum;
                this->lcdNumberRemainBombNum->display(this->bombNum - this->flagBombNum);
            });
            connect(this,&Board::signalMove,p,[this,p,i,j]()->void{
                auto len = qMin(this->width()/(this->colNum+2),this->height()/(this->rowNum+2));
                auto horBorder=(this->width()-len*colNum)/2;
                auto verBorder=(this->height()-len*rowNum)/2;

                p->resize(len,len);
                p->move(horBorder+j*p->size().width(),
                        verBorder+i*p->size().height());
//                dout<<p->pos()<<dendl;
//                dout<<this->size()<<dendl;

            });
            this->chunks[i][j]->show();
        }
    }
//    update();
    connect(this,&Board::signalMove,[this](){update();});    
    /**
     * 绑定背景音乐播放器
     */
     auto bp = new BackgroundMusicPlayer();
     backgroundPlayerThread = new QThread();
     bp->moveToThread(backgroundPlayerThread);
     connect(backgroundPlayerThread,&QThread::finished,bp,&QObject::deleteLater);
     connect(backgroundPlayerThread,&QThread::finished,backgroundPlayerThread,&QObject::deleteLater);
     connect(this,&Board::signalPlayNewBackGroundMusic,bp,&BackgroundMusicPlayer::playNewBackgroundMusc);
     backgroundPlayerThread->start();
}
/**
 * @brief Board::setBombs
 * 初始化雷区,若有物理引擎将会调用其来生成随机数
 * @param firstClickedRC 第一次点击的坐标
 */
void Board::setBombs(Chunk::RowCol firstClickedRC)
{//生成初始的雷区,这应该在第一次点击之后调用
    auto rng = QRandomGenerator(*QRandomGenerator::system());
    QVector<Chunk::RowCol>tmp(this->bombNum);
    for(qint32 i = 0 ,num = 0; i < this->rowNum ; i++){
        for(qint32 j = 0 ; j < this->colNum ; j++){
            if(i == firstClickedRC.row && j == firstClickedRC.col){
                continue;
            }
            if(num < this->bombNum){
                tmp[num].row = i;
                tmp[num++].col = j;
            }else{
                //蓄水池算法
                int res = rng.bounded(0,i*this->colNum+j+1);
                if(res < this->bombNum){
                    tmp[res].row = i;
                    tmp[res++].col = j;
                }
            }
        }
    }
    for(qint32 i = 0 ; i < this->bombNum ; i++){
        this->chunks[tmp[i].row][tmp[i].col]->setMineType(Chunk::MineType::Bomb);
    }
    initSurroundBomb();
    update();
}
/**
 * @brief Board::detect
 * 递归探索周围的雷区
 * @param rc
 */
void Board::detect(Chunk::RowCol rc)
{
    //数据需保证rc对应的位置不是雷，且没有被标记为雷
    auto& chunk = *chunks[rc.row][rc.col];    
    if(chunk.getMineType() == Chunk::MineType::Bomb || chunk.getMineState() == Chunk::MineState::FlagBomb)
    {
        throw QString("bad call detect in "+this->objectName()+" " + __FILE__ +":"+QString::number(__LINE__));
    }
//    dout<<"detect :"<<rc.row<<","<<rc.col<<dendl;
    if(this->chunks[rc.row][rc.col]->getMineState() != Chunk::MineState::Mined)minedNum++;
    this->chunks[rc.row][rc.col]->setMineState(Chunk::MineState::Mined);

    if (this->chunks[rc.row][rc.col]->getSurroundBomb() == 0) {//没有雷
        for (qint32 i = 0; i < SurroundDirectionNum; i++) {
            qint32 tx = rc.row + SurroundDirection[i][0];
            qint32 ty = rc.col + SurroundDirection[i][1];
            if (inBoard(tx,ty) &&(this->chunks[tx][ty]->getMineState() == Chunk::MineState::UnMined || this->chunks[tx][ty]->getMineState() == Chunk::MineState::FlagQuestion))//其肯定不是雷，若其在有效区域内，且未被点击过(防止无限循环)，则递归的进行（其若被标记也会停止）
            {
                detect(Chunk::RowCol(tx,ty));
            }
        }
    }
}
/**
 * @brief Board::initSurroundBomb
 * 每个chunk初始化周围的雷数
 */
void Board::initSurroundBomb()
{
    for(qint32 i = 0 ; i < this->rowNum ; i++)
    {
        for(qint32 j = 0 ; j < this->colNum ; j++)
        {
            qint32 bombNum = 0;
            for(qint32 k = 0 ; k < SurroundDirectionNum ; k++)//遍历邻近的格子
            {
                qint32 tr = this->chunks[i][j]->getRowCol().row + SurroundDirection[k][0];
                qint32 tc = this->chunks[i][j]->getRowCol().col + SurroundDirection[k][1];
                if(inBoard(tr,tc) && chunks[tr][tc]->getMineType() == Chunk::MineType::Bomb)bombNum++;
            }
            chunks[i][j]->setSurroundBomb(bombNum);
        }
    }
}
/**
 * @brief Board::inBoard
 * 判断是否在棋盘内部
 * @param rc
 * @return
 */
inline bool Board::inBoard(Chunk::RowCol rc)
{
    return inRect<qint32>(0,0,this->rowNum-1,this->colNum-1,rc.row,rc.col);
}
/**
 * @brief Board::inBoard
 * 重载
 * @param row
 * @param col
 * @return
 */
inline bool Board::inBoard(qint32 row, qint32 col)
{
    return inBoard(Chunk::RowCol(row,col));
}
/**
 * @brief Board::gameOver
 * 游戏结束槽函数
 * @param loseOrWin 输赢？
 */
void Board::gameOver(QString loseOrWin)
{
    this->gameTimer->stop();
    selfCurrentIntegral = this->calculateCurrentIntegral();
    if(loseOrWin == "lose")
    {
        emit signalPlayNewBackGroundMusic("qrc:/audio/Resource/audio/BombExplosionSound.wav");
    }else{        
        emit signalPlayNewBackGroundMusic("qrc:/audio/Resource/audio/Victory.wav");

        qint32 sumTime = 1+this->timeEditGameTime->time().hour()*3600 \
                +this->timeEditGameTime->time().minute()*60+\
                this->timeEditGameTime->time().second();            
        if(sumTime != 0){selfCurrentIntegral += 10000000/sumTime;}
        else {dout<<"多诡的零";}
    }
    for(qint32 i = 0 ; i < this->rowNum ; i++)
    {
        for(qint32 j = 0; j < this->colNum ; j++)
        {
            this->chunks[i][j]->setEnabled(false);
            this->chunks[i][j]->showBomb();
        }
    }
    auto len = qMin(this->width()/(this->colNum+2),this->height()/(this->rowNum+2));
    auto horBorder=(this->width()-len*colNum)/2;
    auto verBorder=(this->height()-len*rowNum)/2;

    QPainter p(this->pix.get());
    QFont font = p.font();

    font.setPixelSize(qMin(horBorder,verBorder)*0.6);
    p.setFont(font);

    QPen pen = p.pen();
    pen.setColor(loseOrWin == "win" ? Qt::green : Qt::red);
    p.setPen(pen);

    const QRect rectangle = QRect(0, 0,this->pix->width(),verBorder*this->height()/this->pix->height());
    p.drawText(rectangle,Qt::AlignHCenter, tr(loseOrWin.toStdString().c_str()));
    update();

    QTimer* timer = new QTimer();
    connect(timer,&QTimer::timeout,[&](){emit signalGameOver();});
    connect(timer,&QTimer::timeout,timer,&QObject::deleteLater);
    timer->start(3000);
    upLoadHistory();//上传成绩
}
/**
 * @brief Board::calculateCurrentIntegral
 * 计算积分，正相关于探索率，负相关于游戏时长
 * @return
 */
qint32 Board::calculateCurrentIntegral()
{
    //todo:need to do so many
    return 100*this->minedNum;
}
/**
 * @brief Board::upLoadHistory
 * 上传历史记录到服务器
 */
void Board::upLoadHistory()
{
    dout<<this->selfCurrentIntegral;
    emit signalUpLoadHistory(this->GameMod,QString::number(this->rowNum),\
        QString::number(this->colNum),QString::number(this->bombNum),QString::number(selfCurrentIntegral));
}
/**
 * @brief Board::paintEvent
 * @param e
 */
void Board::paintEvent(QPaintEvent *e)
{
    QPainter p;
    p.begin(this);
    p.drawPixmap(0,0,this->pix->scaled(this->size()));
    p.end();
    QMainWindow::paintEvent(e);
}
/**
 * @brief Board::resizeEvent
 * 实现自动调节棋盘大小
 * @param e
 */
void Board::resizeEvent(QResizeEvent *e)
{
    emit signalMove();
    QMainWindow::resizeEvent(e);
}
/**
 * @brief Board::closeEvent
 * 发送游戏结束事件
 * @param e
 */
void Board::closeEvent(QCloseEvent *e)
{    
    dout<<"close"<<this->objectName();
    emit signalGameOver();
    QMainWindow::close();    
}
/**
 * @brief Board::dealSignalExploded
 * 触雷事件槽函数
 */
void Board::dealSignalExploded()
{    
    gameOver("lose");
}
/**
 * @brief Board::dealClickChunk
 * 单击事件槽函数
 * @param rc
 */
void Board::dealClickChunk(Chunk::RowCol rc)
{    
//    if(isGameOver)return;//等待期间禁止点击
    if(isFirstClick)
    {
        isFirstClick = false;
        setBombs(rc);
    }    
    emit signalPlayNewBackGroundMusic("qrc:/audio/Resource/audio/rightClick.wav");
    detect(rc);    
    if(minedNum + bombNum == rowNum*colNum)
    {        
        gameOver("win");
    }
    update();
}
/**
 * @brief Board::dealDoubleClickChunk
 * 双击事件槽函数
 * @param rc 点击处坐标
 * @param system 是否是系统调用的
 */
void Board::dealDoubleClickChunk(Chunk::RowCol rc,bool system)
{ 
    qint32 cnt =0;
    bool flag = false;//是否有标记错误的
    for(qint32 i = 0 ; i < SurroundDirectionNum ; i++)
    {
        qint32 tr = rc.row + SurroundDirection[i][0];
        qint32 tc = rc.col + SurroundDirection[i][1];
        if(inBoard(tr,tc) && this->chunks[tr][tc]->getMineState() == Chunk::MineState::FlagBomb)
        {
            cnt++;
            if(this->chunks[tr][tc]->getMineType() == Chunk::MineType::NotBomb)flag = true;
        }
    }
    if(flag && cnt == this->chunks[rc.row][rc.col]->getSurroundBomb()){
        gameOver("lose");
        dout<<"lose with double clicking"<<dendl;
        return;
    }
    for(qint32 i = 0 ; i < SurroundDirectionNum ; i++)
    {
        qint32 tr = rc.row + SurroundDirection[i][0];
        qint32 tc = rc.col + SurroundDirection[i][1];
        if(inBoard(tr,tc) &&
            (chunks[tr][tc]->getMineState() == Chunk::MineState::UnMined ||
             chunks[tr][tc]->getMineState() == Chunk::MineState::FlagQuestion)
          )
        {
            if(cnt == this->chunks[rc.row][rc.col]->getSurroundBomb())
             dealClickChunk(Chunk::RowCol(tr,tc));
            else if(cnt < this->chunks[rc.row][rc.col]->getSurroundBomb() && !system)
            {
                this->chunks[tr][tc]->floatByDoubleClick();
                emit signalPlayNewBackGroundMusic("qrc:/audio/Resource/audio/doubleClickFailed.wav");
            }
        }
    }
    update();
}
