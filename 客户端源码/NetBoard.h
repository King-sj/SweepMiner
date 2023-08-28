#pragma once

#include "Board.h"
#include<QTcpSocket>
#include"Packet/Packet.h"
#include"MessageTips/MessageTips.h"
#include<QByteArray>
/**
 * @brief The NetBoard class
 * 经典模式的网络对战
 */
class NetBoard : public Board
{
    Q_OBJECT
public:
    NetBoard(QTcpSocket* socket,qint32 rowNum,qint32 colNum,qint32 bombNum,QWidget *parent = nullptr,QString GameMod = "NetBoard");
private:
    void initPacket();

    virtual void initNetBoardThenSendBySocket();
    QString generateGameStateStr(qint32 selfIntegral,qint32 antiIntegral,QString antiEmail);
protected:
    virtual void gameOver(QString loseOrWin)override;//<
    virtual void sendIntegralToServer(qint32 integral);
    virtual void queryNewMatch();
private:
    friend class Packet<NetBoard>;
    QTcpSocket* socket;
    Packet<NetBoard> packet;
    QString antiPlayerEmail;
    qint32 antiCurrentIntegral=0;
protected slots:
    virtual void dealMatchResponse(QStringList list);
    virtual void dealClickChunk(Chunk::RowCol rc)override;//<
public slots:
    virtual void dealMainSocketNewRecvMessage(QByteArray mes);
    virtual void dealNetInitState(QStringList list);
    virtual void dealUpdateAntiIntegral(QStringList list);
    virtual void dealAntiGameOver(QStringList list);
private:
    MessageTips* matchingMessageTips;
};

