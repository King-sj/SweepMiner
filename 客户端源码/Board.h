#pragma once

#include <QMainWindow>
#include <QObject>
#include <QVector>
#include"Chunk.h"
#include<QThread>
#include<memory>
#include<QResizeEvent>
#include"ui_Board.h"
#include<QCloseEvent>
/// 前置声明
class DenotationMod;
/**
 * @brief The Board class
 * 经典模式的游戏类
 */
class Board : public QMainWindow,protected Ui::Board
{
    Q_OBJECT    
    friend class DenotationMod;
public:
    Board(qint32 rowNum,qint32 colNum,qint32 bombNum,QWidget *parent = nullptr,QString GameMod = "Classic");
    ~Board();
    qint32 getRowNum() const;

    qint32 getColNum() const;

    qint32 getBombNum() const;

    QPointer<QTimer> getGameTimer() const;

    void setIsFirstClick(bool newIsFirstClick);

protected:
    const QString GameMod;
    QVector<QVector<QPointer<Chunk>>>chunks;//<
    const qint32 rowNum;//<
    const qint32 colNum;//<
    const qint32 bombNum;//<
    qint32 flagBombNum=0;
    qint32 minedNum;
    const static qint32 SurroundDirectionNum = 8;//<
    constexpr static qint32 SurroundDirection[SurroundDirectionNum][2] = {
        {1,0},{0,1},{-1,0},{0,-1},
        {1,1},{1,-1},{-1,1},{-1,-1}};//<
    QPointer<QTimer> gameTimer;
    bool isFirstClick;//<
    qint32 selfCurrentIntegral=0;
private:

    std::unique_ptr<QPainter>painter();
//    QPixmap* pix;//<
    std::unique_ptr<QPixmap>pix;
    QPointer<QThread> backgroundPlayerThread;

private:
    void init();
protected:
    void setBombs(Chunk::RowCol firstClickedRC);
    virtual void detect(Chunk::RowCol rc);
    void initSurroundBomb();
    inline bool inBoard(Chunk::RowCol rc);//<
    inline bool inBoard(qint32 row,qint32 col);//<
    virtual void gameOver(QString loseOrWin);//<
    virtual void upLoadHistory();
    virtual qint32 calculateCurrentIntegral();
private slots:
    virtual void paintEvent(QPaintEvent* e)override;
    virtual void resizeEvent(QResizeEvent* e)override;
    virtual void closeEvent(QCloseEvent* e)override;    
protected slots:
    //自定义事件槽
    virtual void dealSignalExploded();//<
    virtual void dealClickChunk(Chunk::RowCol rc);//<
    virtual void dealDoubleClickChunk(Chunk::RowCol rc,bool system=false);//<
signals:
    void signalGameOver();    
    void signalPlayNewBackGroundMusic(QString path);
    void signalMove();
    void signalUpLoadHistory(QString gameMod,QString rowNum,QString colNum,QString bombNum,QString integral);
};

