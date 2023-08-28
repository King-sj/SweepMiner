#pragma once
#include "NetBoard.h"
#include<QTcpSocket>
#include"Packet/Packet.h"
/**
 * @brief The NetDenotationMod class
 * 爆炸模式网络对战
 */
class NetDenotationMod : public NetBoard
{
    Q_OBJECT
public:
    NetDenotationMod(QTcpSocket* socket,qint32 rowNum,qint32 colNum,qint32 bombNum,QWidget *parent = nullptr,QString GameMod = "NetDenotation");
protected:
private:
    void randomOpenNotBombChunk(qint32 num);
protected slots:
    virtual void dealSignalExploded()override;
    virtual void dealMatchResponse(QStringList list)override;
private:
    virtual bool eventFilter(QObject *watched,QEvent *e)override;
//    virtual void detect(Chunk::RowCol rc)override;
private:
    qint32 moveNum;
    qint32 remainBombNum;
    QTcpSocket* socket;
    friend class Packet<NetDenotationMod>;
    Packet<NetDenotationMod> packet;
};

