#ifndef __PLAYER_H__
#define __PLAYER_H__
#include<QTcpSocket>
#include<QPointer>
#include"Packet/Packet.h"
#include<QObject>
/**
 * @brief The Player class
 * 玩家对象
 */
class Player : public QObject
{
    Q_OBJECT
public:
    struct GameMode
    {
        QString mod;
        qint32 rowNum;
        qint32 colNum;
        qint32 bombNum;
        GameMode(QString mod,qint32 rowNum,qint32 colNum,qint32 bombNum)
            :mod(mod),rowNum(rowNum),colNum(colNum),bombNum(bombNum){};
    };
public:
	Player();
    Player(QTcpSocket* socket );
    ~Player();
    const QString &getEmail() const;

    void setAntiPlayer(Player *newAntiPlayer,bool isHeadStart=false);
    void sendMesBySocket(QStringList List);
    virtual void updateIntegral(QString matchID,QString integral);
    void setLastGameMatchID(const QString &newLastGameMatchID);
    virtual void insertPlayHistory(GameMode gameMod,QString player1,QString player2,QString player1Integral,QString player2Integral);
public slots:
    void dealConnected();
    void dealRecv();
    void dealDisconnected();
private:
	QPointer<QTcpSocket>socket;
	QString email;
    QString password;
    QString captchaStr;//验证码
    bool isLogin = false;//记录是否登录成功
    virtual bool emailIsLegal(QString em);
    virtual bool passwordIsLegal(QString ps);
    virtual bool emailIsExist(QString em);
    virtual void addPlayerInDB();
    virtual void createHistoryTable();
private:
    Player* antiPlayer=nullptr;
    GameMode lastGameMod;
    QString lastGameMatchID;
protected:
    virtual void login(QStringList list);
    virtual void signUp(QStringList list);
    virtual void captcha(QStringList list);
    virtual void match(QStringList list);
    virtual void NetInitState(QStringList list);
    virtual void updateIntegral(QStringList list);
    virtual void gameOver(QStringList list);
    virtual void upLoadHistory(QStringList list );
    virtual void exitMatch(QStringList list);//todo
    virtual void downLoadHistoryFile(QStringList list);
private:
    friend class Packet<Player>;
    Packet<Player> packet;
    void init();
signals:
    void signalMatchNewGame(GameMode gm);
};
#endif // __PLAYER_H__
