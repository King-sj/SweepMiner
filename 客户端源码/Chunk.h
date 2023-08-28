#pragma once
#include <QLabel>
#include<QPixmap>
#include<QPaintEvent>
#include<QMouseEvent>
/**
 * @brief The Chunk class
 * 单个块对象
 */
class Chunk : public QLabel
{
    Q_OBJECT
public:
    /**
     * @brief
     */
    struct RowCol
    {
        qint32 row;
        qint32 col;
        RowCol(qint32 row = 0,qint32 col = 0):row(row),col(col) {};
        friend bool operator<(const RowCol& l,const RowCol& r)
        {
            if(l.row == r.row)
            {
                return l.col < r.col;
            }
            return l.row < r.row;
        }
//        friend bool operator==(const RowCol& l,const RowCol& r)
//        {
//            return (r.row == l.row) && (l.col == r.col);
//        }
    };
    enum class MineType{Bomb,NotBomb};//<
    enum class MineState{UnMined, Mined, FlagBomb, FlagQuestion};//<
    enum class RIGHT_KEY_MENU{FlagBomb,FlagQuestion};
public:
    explicit Chunk(QWidget *parent = nullptr);
//    Chunk& operator=(const Chunk& h);
//    Chunk(const Chunk& h);
public:
    virtual void setRowCol(qint32 row,qint32 col);
    const qint32 getChunkSize();
    virtual void setMineType(const MineType mt);
    virtual MineType getMineType()const;
    virtual qint32 getSurroundBomb() const;
    virtual void setSurroundBomb(qint32 newSurroundBomb);
    virtual const RowCol getRowCol() const;
    virtual MineState getMineState() const;
    virtual void setMineState(MineState newMineState);
    virtual void floatByDoubleClick();

//    void setClickable(bool newClickable);
    virtual void showBomb();
    virtual void openThenShow();
    void setPix(const QPixmap &newPix);

    const QPixmap &getPix() const;

private:
    virtual void drawSurroundBombNum(qint32 num);
    [[deprecated]] virtual void initRightKeyMenu();
    virtual void onTaskBoxContextMenuEvent();
private:
    virtual void paintEvent(QPaintEvent* e)override;
    virtual void mousePressEvent(QMouseEvent* e)override;
    virtual void mouseDoubleClickEvent(QMouseEvent* e)override;
private:
    const static qint32 DirectorNum = 4;//<
private:        
    QPixmap pix;//<全局画布,
    QSize pixSize{500,500};
    RowCol rowCol;//<    
    MineType mineType = MineType::NotBomb;//<
    MineState mineState = MineState::UnMined;//<
    qint32 surroundBomb = 0;//<
    const qint32 director[DirectorNum][2] = {{1,0},{0,1},{-1,0},{0,-1}};//<
    bool clickable = true;
signals:
    void signalExploded();//<
    void signalClickChunk(Chunk::RowCol rc);//<
    void signalDoubleClickChunk(Chunk::RowCol rc,bool system=false);//<
    ///只有点击不爆炸后才会发出该信号
    void signalFlagBombChanged(qint32 changedNum);
};
