#include "NetBoard.h"
#include"Base.h"
#include<QTimer>
#include<QTime>
#include<QRandomGenerator>
#include<QVector>
/**
 * @brief NetBoard::NetBoard
 * @param socket
 * @param rowNum
 * @param colNum
 * @param bombNum
 * @param parent
 * @param GameMod
 */
NetBoard::NetBoard(QTcpSocket *socket, qint32 rowNum, qint32 colNum, qint32 bombNum, QWidget *parent,QString GameMod)
    :Board(rowNum,colNum,bombNum,parent,GameMod),socket(socket),packet(this)
{
    initPacket();
///    /note:暂不处理连接成功的事件,后期若要添加断线重连时可以加上,(当然，也可以让MainWindow来处理。。。以后再说)
///    connect(this->socket,QTcpSocket::connected,);
///    /
///    / note:暂不处理，理由同上
///    connect(this->socket,&QTcpSocket::disconnected);
///    note:此处的消息在MainWindow读取后已经被清空了
///    connect(this->socket,&QTcpSocket::readyRead,[&]{
///        QString newMes = this->socket->readAll();
///        dout<<newMes;
///        this->packet.pushMessage(newMes);
///    });
    matchingMessageTips = new MessageTips("匹配中...",this);
    matchingMessageTips->setShowTime(1000*60*60*60);
    matchingMessageTips->setCloseTimeSpeed(10000,0);
    matchingMessageTips->show();
    queryNewMatch();    
    this->setEnabled(false);
    gameTimer->stop();
}
/**
 * @brief NetBoard::initPacket
 * 绑定event和回调函数
 */
void NetBoard::initPacket()
{
    packet.installClassFunctionEvent("match_response",2,&NetBoard::dealMatchResponse);
    packet.installClassFunctionEvent("NetInitState",1,&NetBoard::dealNetInitState);
    packet.installClassFunctionEvent("updateAntiIntegral",1,&NetBoard::dealUpdateAntiIntegral);
    packet.installClassFunctionEvent("antiGameOver",0,&NetBoard::dealAntiGameOver);
}
/**
 * @brief NetBoard::queryNewMatch
 * 向服务器请求新的匹配
 */
void NetBoard::queryNewMatch()
{
    dout<<this->GameMod;
    QStringList list;
    list<<"match"<<this->GameMod<<QString::number(this->getRowNum()) \
       <<QString::number(this->getColNum())<<QString::number(this->getBombNum());
    socket->write(packet.formatMes(list).toStdString().c_str());
    socket->flush();
}
/**
 * @brief NetBoard::initNetBoardThenSendBySocket
 * 初始化雷田
 */
void NetBoard::initNetBoardThenSendBySocket()
{
    QStringList list;
    QString res;
    list<<"NetInitState";
    for(qint32 i = 0 ; i < Board::getRowNum() ; i++)
    {
        for(qint32 j = 0 ; j < Board::getColNum() ; j++)
        {
            qint32 t = static_cast<qint32>(chunks[i][j]->getMineType());
            qint32 s = static_cast<qint32>(chunks[i][j]->getMineState());
            res=res+QString::number(t)+"/";
            res=res+QString::number(s)+"/";
        }
    }
    list<<res;
    dout<<list;
    socket->write(packet.formatMes(list).toStdString().c_str());
    socket->flush();
}
/**
 * @brief NetBoard::generateGameStateStr
 * 生成游戏状态字符串
 * @param selfIntegral
 * @param antiIntegral
 * @param antiEmail
 * @return
 */
QString NetBoard::generateGameStateStr(qint32 selfIntegral, qint32 antiIntegral, QString antiEmail)
{
    return QString("Your integral:%1  |  Anti integral(%2):%3")\
            .arg(selfIntegral).arg(antiEmail).arg(antiIntegral);
}
/**
 * @brief NetBoard::gameOver
 * 重载游戏结束事件，
 * @param loseOrWin
 */
void NetBoard::gameOver(QString loseOrWin)
{
    this->antiPlayerEmail = "";
    Board::gameOver(loseOrWin);
    sendIntegralToServer(selfCurrentIntegral);
    QStringList list;
    list<<"gameOver";
    socket->write(packet.formatMes(list).toStdString().c_str());
    socket->flush();

    dout<<list;
}

/**
 * @brief NetBoard::sendIntegralToServer
 * 发送自己的积分到服务器
 * @param integral
 */

void NetBoard::sendIntegralToServer(qint32 integral)
{
    QStringList list;
    list<<"updateIntegral"<<QString::number(integral);
    this->socket->write(packet.formatMes(list).toStdString().c_str());
    this->socket->flush();
}
/**
 * @brief NetBoard::dealMatchResponse
 * 处理匹配结果
 * @param list length :2 {1/0}{email}
 */
void NetBoard::dealMatchResponse(QStringList list)
{
    this->matchingMessageTips->hide();
    if(list[0] == "0")
    {
        dout<<"匹配失败\n";
        matchingMessageTips = new MessageTips("匹配失败",this);
        matchingMessageTips->setShowTime(3000);
        matchingMessageTips->show();
        return;
    }else if(list[0] == "1hs"){
        auto rng = QRandomGenerator(*QRandomGenerator::system());
        qint32 firstClickRow = rng.bounded(0,this->getRowNum());
        qint32 firstClickCol = rng.bounded(0,this->getColNum());
        this->antiPlayerEmail = "tmp";//note:拙劣的补丁，后面改成服务器刷新雷区就好了,目前不这样会导致dealClickChunk不能正常刷新
        dealClickChunk(Chunk::RowCol(firstClickRow,firstClickCol));

        initNetBoardThenSendBySocket();
        this->setEnabled(true);
        gameTimer->start();
        dout<<"begin\n";
    }else{
        this->antiPlayerEmail = "tmp";
        setIsFirstClick(false);
    }
    this->antiPlayerEmail = list[1];    
    dout<<"匹配成功\n";
    matchingMessageTips = new MessageTips("匹配成功",this);
    matchingMessageTips->setShowTime(1500);
    matchingMessageTips->show();
    this->labelMoreDetailState->setText(generateGameStateStr(0,0,this->antiPlayerEmail));
    update();
}
/**
 * @brief NetBoard::dealClickChunk
 * 重载单击事件
 * @param rc
 */
void NetBoard::dealClickChunk(Chunk::RowCol rc)
{
    if(this->antiPlayerEmail == "")
    {
        dout<<"bad request";
        return;
    }
    selfCurrentIntegral = calculateCurrentIntegral();
    this->labelMoreDetailState->setText(generateGameStateStr(\
        selfCurrentIntegral,antiCurrentIntegral,this->antiPlayerEmail));
    if(!isFirstClick)
    {
        /**
         * @note:玄学改bug法,如若不这样,“后手”方无法正常init,(慢一步)
         */
        sendIntegralToServer(selfCurrentIntegral);
    }
    Board::dealClickChunk(rc);
    update();
}
/**
 * @brief NetBoard::dealMainSocketNewRecvMessage
 * 接受mainWindow转发的消息
 * @param mes
 */
void NetBoard::dealMainSocketNewRecvMessage(QByteArray mes)
{
    dout<<mes;
    this->packet.pushMessage(mes);
}
/**
 * @brief NetBoard::dealNetInitState
 * @param list length: {1}{12* rowNum * colNum in lastGamMod split by /}
 */
void NetBoard::dealNetInitState(QStringList list)
{
    auto res = list[0].split("/");
    qint32 tail = 0;
    dout<<list;
    QVector<QVector<qint32>>ms;//存储探索状态
    ms.resize(getRowNum());
    for(qint32 i = 0 ; i < Board::getRowNum() ; i++)
        ms[i].resize(getColNum());
    for(qint32 i = 0 ; i < Board::getRowNum() ; i++)
    {
        for(qint32 j = 0 ; j < Board::getColNum() ; j++)
        {
            qint32 t = res[tail++].toInt();
            qint32 s = res[tail++].toInt();
            this->chunks[i][j]->setMineType(static_cast<Chunk::MineType>(t));
            ms[i][j] = s;
            if(static_cast<Chunk::MineState>(s) == Chunk::MineState::Mined)minedNum++;//note:因为是直接更新的，所以要在这手动维护
        }
    }
    initSurroundBomb();
    //要在更新Chunk类型后再更新翻开状态，否则不能正常initSurroudBomb
    for(qint32 i = 0 ; i < Board::getRowNum() ; i++)
        for(qint32 j = 0 ; j < Board::getColNum() ; j++)
            this->chunks[i][j]->setMineState(static_cast<Chunk::MineState>(ms[i][j]));

    this->setEnabled(true);
    gameTimer->start();
    update();
    dout<<"begin\n";
}
/**
 * @brief NetBoard::dealUpdateAntiIntegral
 * 更新对手的积分
 * @param list length:1{}
 */
void NetBoard::dealUpdateAntiIntegral(QStringList list)
{
    dout<<list;
    this->antiCurrentIntegral = list[0].toInt();
    this->labelMoreDetailState->setText(generateGameStateStr(\
        selfCurrentIntegral,antiCurrentIntegral,this->antiPlayerEmail));
    update();
}
/**
 * @brief NetBoard::dealAntiGameOver
 * 处理对手游戏结束事件
 * @param list
 */
void NetBoard::dealAntiGameOver(QStringList list)
{
//    this->antiPlayerEmail = "";
    dout<<list;
    auto mt = new MessageTips("对方已结束");
    mt->show();
    update();
}
