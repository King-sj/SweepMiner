#include "Player.h"
#include"debug.h"
#include <QSqlError>
#include <QSqlQuery>
#include<QSettings>
#include"Smtp.h"
#include<QRandomGenerator>
#include<QHostAddress>
#include<QDateTime>
#include <QUuid>
///todo：显然这个项目的Net部分还有很多尚未处理，比如安全性、掉线等；留坑待补
/**
 * @brief Player::Player
 */
Player::Player()
    :QObject(),lastGameMod("",0,0,0),packet(this)
{
    this->socket = nullptr;
    init();
}
/**
 * @brief Player::Player
 * @param s 为每个socket建立一个玩家对象
 */
Player::Player(QTcpSocket *s)
    :QObject(),lastGameMod("",0,0,0),packet(this)
{
    init();
    this->socket = s;
    QString ip = socket->peerAddress().toString();
    QString port = QString::number(socket->peerPort());
    QString all = ip + ":" + port;
    dout<<"ip:port --"<<all<<"\n";
}
/**
 * @brief Player::~Player
 */
Player::~Player()
{
    //todo:delete socket may be better
	antiPlayer = nullptr;
}
/**
 * @brief Player::dealConnected
 * 连接成功处理函数
 */
void Player::dealConnected()
{
    dout<<"连接成功"<<dendl;
}
/**
 * @brief Player::dealDisconnected
 * 掉线处理函数
 */
void Player::dealDisconnected()
{
    dout<<this->email<<"掉线"<<dendl;
    this->isLogin = false;
	dout<<antiPlayer;
    if(this->antiPlayer == nullptr)return;
	try{
		this->antiPlayer->setAntiPlayer(nullptr);//note:清空,但会对断线重连有影响(后续采用其它机制恢复)
		dout<<"处理掉线成功";
	}catch(...)
	{
		dout<<"掉线时发生了些奇怪的error";
		dout<<antiPlayer;
	}
}
/**
 * @brief Player::getEmail
 * @return 返回对象的email属性
 */
const QString &Player::getEmail() const
{
    return email;
}

/**
 * @brief Player::login
 * 登录时间的回调函数
 * @param list length:2 {邮箱,密码}
 */
void Player::login(QStringList list)
{

    auto em = list[0];//邮箱
    auto pas = list[1];//密码
    dout<<em<<pas<<dendl;
    auto pFailed = [&](){
        QStringList tmp;
        tmp<<"login_response"<<"0";
        socket->write(packet.formatMes(tmp).toStdString().c_str());
        socket->flush();
    };
    if(!emailIsLegal(em))
    {
        dout<<"email is illegal\n";
        pFailed();
        return;
    }
    if(!emailIsExist(em)){
        dout << "this doesn't exist." << dendl;
        pFailed();
        return;
    }
    else{
        dout << "this email exists." << dendl;
         //player(email varchar(100) primary key, name varchar(100),password varchar(100))
        QSqlQuery query;
        query.prepare("select password from player where email=:email");
        query.bindValue(":email", em);
        if (!query.exec()) {
            dout << query.lastError().text();
            pFailed();
            return;
        }
        while (query.next()) {
            dout<<query.value("password").toString();
            this->password = query.value("password").toString();
        }
        if(this->password == pas)
        {
            QStringList tmp;
            tmp<<"login_response"<<"1";
            socket->write(packet.formatMes(tmp).toStdString().c_str());
            socket->flush();
            this->email = em;
			this->password = pas;
            this->isLogin = true;
            return;
        }else{
			dout<<"password is not right\n";
            pFailed();
            return;
        }
    }
}
/**
 * @brief Player::signUp
 * 注册事件的回调函数
 * @param list length :2 {email,password}
 */
void Player::signUp(QStringList list)
{
    auto em = list[0];
    auto ps = list[1];
    dout<<em<<ps;
    if(!emailIsLegal(em) || !passwordIsLegal(ps))
    {
        QStringList list;
        list<<"signUp_response"<<"0"<<"账号或密码不合法";
        socket->write(packet.formatMes(list).toStdString().c_str());
        socket->flush();
        return;
    }
    if(emailIsExist(em))
    {
        QStringList list;
        list<<"signUp_response"<<"0"<<"账号已存在";
        socket->write(packet.formatMes(list).toStdString().c_str());
        socket->flush();
        return;
    }
    this->email=em ;
    this->password = ps;

    QSettings settings("./config.ini", QSettings::IniFormat);
    auto acc = settings.value("SMTP/account").toString();
    auto pw = settings.value("SMTP/pw").toString();
    Smtp s(acc.toStdString().c_str(),pw.toStdString().c_str());

    auto rng = QRandomGenerator(*QRandomGenerator::system());
    this->captchaStr = QString::number(rng.bounded(static_cast<qint32>(1e5),static_cast<qint32>(1e6)));
    s.send(em.toStdString().c_str(),"扫雷在线版账号注册验证码:","你的邮箱"+email+"本次注册扫雷在线版的验证码是:"+captchaStr);

    QStringList l;
    l<<"signUp_response"<<"1"<<"请输入验证码";
    socket->write(packet.formatMes(l).toStdString().c_str());
    socket->flush();
    //接下来交给处理captcha的函数验证验证码
}
/**
 * @brief Player::captcha
 * 验证码验证事件的回调函数
 * @param list length:1 {0/1}
 */
void Player::captcha(QStringList list)
{
    dout<<list;
    QStringList li;
    if(list[0] == this->captchaStr)
    {
        li<<"captcha_response"<<"1"<<email<<password;
        this->isLogin = true;
		dout<<"begin add new player\n";
        addPlayerInDB();
    }else{
        li<<"captcha_response"<<"0"<<"null"<<"null";
		dout<<"captcha is not right\n";
        //note:此处在验证失败后没清空数据可能会带来被盗号风险(目前猪脑过载，想不出来，先不管)
//        this->email = "";
//        this->password = "";

    }
    socket->write(packet.formatMes(li).toStdString().c_str());
    socket->flush();
}
/**
 * @brief Player::match
 * 进行新的匹配事件的回调函数
 * @param list length:4{模式,行数，列数,雷数}
 */
void Player::match(QStringList list)
{
    QString gameMod = list[0];
    qint32 rowNum = list[1].toInt();
    qint32 colNum = list[2].toInt();
    qint32 bombNum = list[3].toInt();
    lastGameMod = GameMode(gameMod,rowNum,colNum,bombNum);
    emit signalMatchNewGame(GameMode(gameMod,rowNum,colNum,bombNum));
}
/**
 * @brief Player::NetInitState
 * 转发网络对战更新事件给对手
 * @param list length: {1}{12* rowNum * colNum in lastGamMod split by /}
 */
void Player::NetInitState(QStringList list)
{
    QStringList newList;
    newList<<"NetInitState"<<list;
    dout<<newList;
    if(antiPlayer == nullptr)return;
    antiPlayer->sendMesBySocket(newList);
}
/**
 * @brief
 * 更新自己的游戏积分事件的回调函数
 * @param list length:1{integral}
 */
void Player::updateIntegral(QStringList list)
{
    //list[0]实际上是自己的积分
    QStringList newList;

    this->updateIntegral(this->lastGameMatchID,list[0]);
    if(antiPlayer == nullptr)return;
    newList<<"updateAntiIntegral"<<list;
    antiPlayer->sendMesBySocket(newList);//转发到对手,让对手更新她的对手(我的分数)
    //todo:更新数据库的积分
}
/**
 * @brief Player::gameOver
 * 游戏结束事件的回调函数
 * 这个事件同时会转发给对手
 * @param list length :0
 */
void Player::gameOver(QStringList list)
{
    this->lastGameMod = GameMode("none",0,0,0);
    this->lastGameMatchID = "none";

    if(antiPlayer == nullptr)return;
    QStringList newList;
    newList<<"antiGameOver"<<list;
    dout<<newList;
    antiPlayer->sendMesBySocket(newList);\
    antiPlayer = nullptr;//清空对手
}
/**
 * @brief Player::upLoadHistory
 * 更新单机模式的战绩
 * @param list length:5
 */
void Player::upLoadHistory(QStringList list)
{
    this->lastGameMatchID = QUuid::createUuid().toString();
    this->insertPlayHistory(GameMode(list[0],list[1].toInt(),list[2].toInt(),list[3].toInt()),
            this->getEmail(),"none",list[4],"none");
    dout<<"done upload history";
    this->lastGameMatchID = "";
}
/**
 * @brief Player::exitMatch
 * 退出匹配事件的回调函数
 * @param list length:0
 */
void Player::exitMatch(QStringList list)
{
    //nothing need be done
    //Date:23/6/7
    //note:中途退出等,转交由gameOver处理
    this->gameOver(list);
}
/**
 * @brief Player::downLoadHistoryFile
 * 下载历史战绩事件的回调函数
 * @param list length:0
 */
void Player::downLoadHistoryFile(QStringList)
{
    ////create table playHistory(matchID varchar(40) primary key,Date varchar(20),\
    ///gameMod varchar(20),rowNum varchar(4),colNum varchar(4),bombNum varchar(4),\
    ///player1 varchar(50),player2 varchar(50),player1Integral varchar(128),player2Integral varchar(128))"
    /// 共计10个参数
    QSqlQuery query;
    query.prepare("select * from playHistory where player1=:player1 or player2=:player2");
    query.bindValue(":player1",this->email);
    query.bindValue(":player2",this->email);
    if(!query.exec())
    {
        dout<<query.lastError().text();
        //todo:添加下载失败的事件
        return;
    }else{
        QStringList head;
        head<<"tansferHistoryFileHead"<<"gameHistory.db";
        this->socket->write(packet.formatMes(head).toStdString().c_str());

        qint64 total=0;
        try {
            while(query.next())
            {
                QStringList list;
                list<<"tansferingHistoryFile";
                for(qint32 i = 0 ;i < 10 ; i++)
                {
                    list<<query.value(i).toString();
                }
                this->socket->write(packet.formatMes(list).toStdString().c_str());
                dout<<list;
                total++;
            }
        } catch (...) {
            dout<<"传输第"<<total<<"条记录时出错";
        }

        //todo 后续可以添加检验文件传输完整性的功能
        QStringList end;
        end<<"tansferHistoryFileEnd";
        this->socket->write(packet.formatMes(end).toStdString().c_str());
        this->socket->flush();
    }
}
/**
 * @brief Player::init
 * 初始化函数,主要是注册事件和绑定相应的回调函数
 */
void Player::init()
{
    ///
    this->packet.installClassFunctionEvent("login",2,&Player::login);//登录
    this->packet.installClassFunctionEvent("signUp",2,&Player::signUp);//注册
    this->packet.installClassFunctionEvent("captcha",1,&Player::captcha);//发送验证码
    //Classic,NetClassic,Denotation
    this->packet.installClassFunctionEvent("match",4,&Player::match);//匹配玩家
//    this->packet.installClassFunctionEvent("NetInitState",2*lastGameMod.rowNum*lastGameMod.colNum,&Player::NetInitState);//分发对局初始化信息
    this->packet.installClassFunctionEvent("NetInitState",1,&Player::NetInitState);//分发对局初始化信息
    this->packet.installClassFunctionEvent("updateIntegral",1,&Player::updateIntegral);//更新对方的积分
    this->packet.installClassFunctionEvent("gameOver",0,&Player::gameOver);//游戏结束事件
    this->packet.installClassFunctionEvent("upLoadHistory",5,&Player::upLoadHistory);
    this->packet.installClassFunctionEvent("exitMatch",0,&Player::exitMatch);
    this->packet.installClassFunctionEvent("downLoadHistoryFile",0,&Player::downLoadHistoryFile);
}
/**
 * @brief Player::dealRecv
 * socket有新消息时的槽函数,将消息托管给packet
 */
void Player::dealRecv()
{
    auto buffer = socket->readAll();
    QString newMes = buffer;
    dout<<newMes<<dendl;
    packet.pushMessage(newMes);
}
/**
 * @brief Player::emailIsLegal
 * 判断邮箱是否有效
 * @param em 邮箱
 * @return 有效返回true
 */
bool Player::emailIsLegal(QString em)
{
    if(!em.contains("@") || !em.contains("com") || em.contains(" ")|| em.contains("\n") || em.contains("\r"))
        return false;
	return true;
}
/**
 * @brief Player::passwordIsLegal
 * 判断密码是否有效
 * @param ps 密码
 * @return 有效返回true
 */
bool Player::passwordIsLegal(QString ps)
{
	if(ps.length() < 8|| ps.contains(" ") || ps.contains("\n") || ps.contains("\r"))
	return false;
    return true;
}

/**
 * @brief Player::emailIsExist
 * 检验邮箱是否存在于已注册的数据库中
 * @param em 邮箱
 * @return 存在返回true
 */
bool Player::emailIsExist(QString em)
{//player(email varchar(100) primary key, name varchar(100),password varchar(100))
    QSqlQuery query;
	query.exec(QString("select email from player where email='%1'").arg(em));
	if (!query.exec()) {
		dout << query.lastError().text();
		return false;
    }
	return query.next();
}
/**
 * @brief Player::addPlayerInDB
 * 将玩家账号加入数据库中
 */
void Player::addPlayerInDB()
{
    //player(email varchar(100) primary key, name varchar(100),password varchar(100))
    QSqlQuery query;
	query.exec("begin"); // 开始事务
    QString insert_sql = "insert into player values (?, ?, ?)";
    query.prepare(insert_sql);
    query.addBindValue(this->email);
    query.addBindValue(QUuid::createUuid().toString());//todo:使用uuid作为name
    query.addBindValue(this->password);
    if(!query.exec())
    {
        dout << query.lastError();
    }
    else
    {
        dout << "insert "<<this->email<<"succeessfully";
    }
	query.exec("commit"); // 提交事务，保存更改
    dout<<"add new player successfully\n";
}
/**
 * @brief Player::createHistoryTable
 * 创建存储历史战绩的数据库表
 */
void Player::createHistoryTable()
{
    QSqlQuery sql;
    QString tmp = "create table playHistory(matchID varchar(40) primary key,Date varchar(20),\
            gameMod varchar(20),rowNum varchar(4),colNum varchar(4),bombNum varchar(4),\
            player1 varchar(50),player2 varchar(50),player1Integral varchar(128),player2Integral varchar(128))";
    sql.prepare(tmp);
    if(!sql.exec())
    {
        dout<<sql.lastError().text();
    }else{
        dout<<"successfully create history table \n";
    }
    sql.finish();
}
/**
 * @brief Player::setLastGameMatchID
 * 给最近的匹配绑定一个uuid
 * @param newLastGameMatchID uuid
 */

void Player::setLastGameMatchID(const QString &newLastGameMatchID)
{
    lastGameMatchID = newLastGameMatchID;
}
/**
 * @brief Player::insertPlayHistory
 * 插入新的对局记录
 * @param gameMod 模式
 * @param player1 玩家1email
 * @param player2 玩家2email
 * @param player1Integral 玩家1积分
 * @param player2Integral 玩家2积分
 */
void Player::insertPlayHistory(GameMode gameMod, QString player1, QString player2, QString player1Integral, QString player2Integral)
{
    ////""create table playHistory(matchID varchar(40) primary key,Date varchar(20),\
    ///gameMod varchar(20),rowNum varchar(4),colNum varchar(4),bombNum varchar(4),\
    ///player1 varchar(50),player2 varchar(50),player1Integral varchar(128),player2Integral varchar(128))"
    /// 共计10个参数
    createHistoryTable();
    QSqlQuery query;
    query.exec("begin"); // 开始事务
    QString insert_sql = QString("insert into playHistory values (?,?,?, ?,?,?, ?,?,?,?)");
    query.prepare(insert_sql);
//    this->lastGameMatchID = QUuid::createUuid().toString();

    query.addBindValue(this->lastGameMatchID);
    query.addBindValue(QDateTime::currentDateTime().toString());
    query.addBindValue(gameMod.mod);

    query.addBindValue(QString::number(gameMod.rowNum));
    query.addBindValue(QString::number(gameMod.colNum));
    query.addBindValue(QString::number(gameMod.bombNum));

    query.addBindValue(player1);
    query.addBindValue(player2);
    query.addBindValue(player1Integral);
    query.addBindValue(player2Integral);
    if(!query.exec())
    {
        dout << query.lastError();
    }
    else
    {
        dout << "insert "<<this->email<<"succeessfully";
    }
    query.exec("commit"); // 提交事务，保存更改
}
/**
 * @brief Player::updateIntegral
 * 更新自己的分数
 * @param integral 积分
 *
 */
void Player::updateIntegral(QString matchID,QString integral)
{
    ////create table playHistory(matchID varchar(40) primary key,Date varchar(20),\
    ///gameMod varchar(20),rowNum varchar(4),colNum varchar(4),bombNum varchar(4),\
    ///player1 varchar(50),player2 varchar(50),player1Integral varchar(128),player2Integral varchar(128))"
    /// 共计10个参数
    ///
    if(matchID == "")
    {
        dout<<"badrequest";
        return;
    }
    QSqlQuery query;
    query.exec("begin");
    query.prepare("select * from playHistory where matchID=:matchID");
    query.bindValue(":matchID",matchID);
    dout<<query.lastQuery();
    QString playerIntegral;//="player2Integral";
    if(!query.exec())
    {
        dout<<query.lastError().text();
    }else{
        while(query.next())
        {
            if(query.value("player1") == this->email)
                playerIntegral = "player1Integral";
            else playerIntegral = "player2Integral";
            dout<<playerIntegral;
        }
    }
    QString tmp = QString("update playHistory set %1='%2' where matchID='%3'")\
            .arg(playerIntegral).arg(integral).arg(matchID);

    dout<<matchID<<integral;
    dout<<tmp;
    if(!query.exec(tmp))dout<<query.lastError().text();
    dout<<query.lastQuery();
    query.exec("commit");
}
/**
 * @brief Player::setAntiPlayer
 * 设置自己的对手
 * @param newAntiPlayer 对手的指针
 * @param isHeadStart 是否是先手->更新棋盘
 */
void Player::setAntiPlayer(Player *newAntiPlayer, bool isHeadStart)
{
    antiPlayer = newAntiPlayer;
    if(antiPlayer == nullptr)return;
    //todo:这样写是不是耦合度高了一点，先不管
    QStringList list;

    //note:当时设计时没考虑到QWQ
    if(isHeadStart)list<<"match_response"<<"1hs"<<newAntiPlayer->email;
    else list<<"match_response"<<"1"<<newAntiPlayer->email;

    this->socket->write(packet.formatMes(list).toStdString().c_str());
    this->socket->flush();
}
/**
 * @brief Player::sendMesBySocket
 * 发送消息给对应的客户端
 * @param list
 */
void Player::sendMesBySocket(QStringList list)
{
    socket->write(packet.formatMes(list).toStdString().c_str());
    socket->flush();
    dout<<list;
}
