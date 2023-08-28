#pragma once

#include "Board.h"
//#include"Chunk.h"
/**
 * @brief The DenotationMod class
 * 爆炸模式，触雷不会死，但点击次数有限
 */
class DenotationMod : public Board
{
    Q_OBJECT
public:
    DenotationMod(qint32 rowNum,qint32 colNum,qint32 bombNum,QWidget *parent = nullptr,QString GameMod = "Denotation");
private:
    virtual void randomOpenNotBombChunk(qint32 num);
private slots:
    virtual void dealSignalExploded()override;
//    virtual void upLoadHistory()override;
private:
    virtual bool eventFilter(QObject *watched,QEvent *e)override;
private:
//    const QString GameMod = "Denotation";
    qint32 moveNum;
    qint32 remainBombNum;
};

