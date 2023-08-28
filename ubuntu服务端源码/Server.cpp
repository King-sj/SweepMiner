#include "Server.h"
#include"debug.h"
#include <QSqlError>
#include <QSqlQuery>
#include<QUuid>
/**
 * @brief Server::Server
 * @param parent
 */
Server::Server(QObject* parent)
	:QObject(parent)
{
	dout<<"server begin\n";

	connectLocalMainDataBase();

	this->server = new QTcpServer();
	if(server == nullptr)
    {
        dout<<"服务端创建失败\n";
    }
    if(!server->listen(QHostAddress::Any, PORT))
    {
        dout<<"监听失败:"<<server->errorString()<<dendl;
        return;
    }
    connect(this->server,&QTcpServer::newConnection,this,&Server::dealNewConnection);
}
/**
 * @brief Server::~Server
 */
Server::~Server()
{
	dout<<"server end\n";
    database.close();
}
/**
 * @brief Server::connectLocalMainDataBase
 * 连接本地数据库
 */
void Server::connectLocalMainDataBase()
{
	QStringList ds=QSqlDatabase::drivers();
    foreach(const QString& strTmp,ds)
        dout<<strTmp;
	// database = new QSqlDatabase();
	if (QSqlDatabase::contains("qt_sql_default_connection"))//默认链接
	{
		database = QSqlDatabase::database("qt_sql_default_connection");
	}
	else
	{
		// 建立和SQlite数据库的连接
		database = QSqlDatabase::addDatabase("QSQLITE");
		// 设置数据库文件的名字,已存在则返回已存在的对象
		database.setDatabaseName("mainDataBase.db");
	}
	if (!database.open())
	{
		dout<< "Error: Failed to connect database." << database.lastError();
	}
	else
	{
		// do something
		dout<<"success\n";
	}
	QSqlQuery sql_query;
	//创建存储玩家信息的
    if(!sql_query.exec("create table player(email varchar(100) primary key, name varchar(100),password varchar(100))"))
	{
        dout << "Error: Fail to create table." << sql_query.lastError();
	}
}
/**
 * @brief Server::dealNewConnection
 * 有新的客户端连接时的处理函数
 */
void Server::dealNewConnection()
{
    dout<<"有新连接"<<dendl;
	auto socket = server->nextPendingConnection();
    if(socket == nullptr)
	{
		dout<<"连接出现未知错误\n";
		return ;
	}
    auto p = new Player(socket);
    connect(socket,&QTcpSocket::disconnected,p,&Player::dealDisconnected);
    //note:这里赋值p=nullptr报错了,所以只是释放了资源，会导致问题吗？
    connect(socket,&QTcpSocket::disconnected,
            [=](){
		dout<<"在线人数--";
		dout<<p;
        if(p != nullptr)
        {
            dout<<"delete "<<p->getEmail();
            delete p;
        }
        dout<<"处理掉线成功";
        });//todo:断开连接时直接释放资源,对断线重连有影响吗？（还没做）
    connect(socket,&QTcpSocket::readyRead,p,&Player::dealRecv);
    connect(socket,&QTcpSocket::connected,p,&Player::dealConnected);
    connect(p,&Player::signalMatchNewGame,this,&Server::dealMatchNewGame);
    //    players.push_back(t);
}
/**
 * @brief Server::dealMatchNewGame
 * 给客户端匹配新的对局
 * @param gm 游戏模式
 *
 */
void Server::dealMatchNewGame(Player::GameMode gm)
{
    //note:有人掉线时会崩溃，待解决
    auto& list  = matchMapList[gm.mod][gm.rowNum][gm.colNum][gm.bombNum];
    auto player = static_cast<Player*>(sender());
    if(player == nullptr)
    {
        dout<<"player已经掉线了\n";
        return;
    }
    if(list.length() > 0)
    {
        auto antiPlayer = list.first();
        while(antiPlayer == nullptr){
            dout<<"antiPlayer已经掉线了\n";
            list.removeFirst();
            antiPlayer = list.first();
        }
        antiPlayer->setAntiPlayer(player);
        player->setAntiPlayer(antiPlayer,true);

        QString matchID  =QUuid::createUuid().toString();
        player->setLastGameMatchID(matchID);
        antiPlayer->setLastGameMatchID(matchID);
        player->insertPlayHistory(gm,player->getEmail(),antiPlayer->getEmail(),"none","none");

        list.removeFirst();
        dout<<"match successfully "<<player->getEmail()<<antiPlayer->getEmail();
    }else{
        list.push_back(player);
		dout<<"failed";
		dout<<list;
    }
    //todo:增加匹配超时，等功能
}
