#ifndef __SERVER_H__
#define __SERVER_H__

#include<QTcpSocket>
#include<QTcpServer>
#include <QSqlDatabase>
#include<QPointer>
#include"Player.h"
#include<vector>
#include<QMap>
#include<QList>
/**
 * @brief The Server class
 * 服务器类,管理多个客户端
 */
class Server:public QObject
{
	Q_OBJECT
public:
	Server(QObject* parent = nullptr);
	~Server();
private:
	virtual void connectLocalMainDataBase();
protected slots:
	virtual void dealNewConnection();
    virtual void dealMatchNewGame(Player::GameMode gm);
private:
	QPointer<QTcpServer> server;
//    std::vector<QPointer<Player>>players;
	QSqlDatabase database;
	const qint32 PORT = 2001;
    //note : 太长且复杂了
//	QMap<std::vector<QString>,QList<Player*>> matchMapList;
     QMap<QString,QMap<qint32,QMap<qint32,QMap<qint32,QList<Player*>>>>>matchMapList;
};
#endif // __SERVER_H__
