#include "Leaderboard.h"
#include"Base.h"
#include<QTableWidget>
#include<QLabel>
#include<QIcon>
#include<QPixmap>
#include<QListWidget>
#include<QHeaderView>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
#include<QSettings>
#include<QVector>
/**
 * @brief Leaderboard::Leaderboard
 * @param socket
 * @param parent
 */
Leaderboard::Leaderboard(QTcpSocket *socket, QWidget *parent)
    :QMainWindow(parent),socket(socket),packet(this)
{
    setupUi(this);    

    allHistoryWidget = new QWidget;
    initAllHistoryWidget();
    tabWidget->addTab(allHistoryWidget, "所有");

    classicOrderWidget = new QWidget;
    initSelfOrderWidget(classicOrderWidget,"Classic");
    tabWidget->addTab(classicOrderWidget,"经典单机模式");

    denotationOrderWidget = new QWidget;
    initSelfOrderWidget(denotationOrderWidget,"Denotation");
    tabWidget->addTab(denotationOrderWidget,"经典爆炸模式");

    netBoardOrderWidget = new QWidget;
    initSelfOrderWidget(netBoardOrderWidget,"NetBoard");
    tabWidget->addTab(netBoardOrderWidget,"经典对战模式");

    netDenotationOrderWidget = new QWidget;
    initSelfOrderWidget(netDenotationOrderWidget,"NetDenotation");
    tabWidget->addTab(netDenotationOrderWidget,"爆炸对战模式");
}
/**
 * @brief Leaderboard::dealMainSocketNewRecvMessage
 * 接受mainWindow转发的服务器传过来的消息
 * @param mes
 */
void Leaderboard::dealMainSocketNewRecvMessage(QByteArray mes)
{
    dout<<mes;
    this->packet.pushMessage(mes);
}
/**
 * @brief Leaderboard::initAllHistoryWidget
 */
void Leaderboard::initAllHistoryWidget()
{
    ////create table playHistory(matchID varchar(40) primary key,Date varchar(20),\
    ///gameMod varchar(20),rowNum varchar(4),colNum varchar(4),bombNum varchar(4),\
    ///player1 varchar(50),player2 varchar(50),player1Integral varchar(128),player2Integral varchar(128))"
    /// 共计10个参数
    QPointer<QListWidget>list = new QListWidget(this->allHistoryWidget);
    list->setAlternatingRowColors(true);
    list->resize(800,600);

    QSettings settings("./config.ini", QSettings::IniFormat);
    QString email =settings.value("Account/email").toString();
    QSqlQuery query;
    query.prepare("select * from playHistory where player1=:player1 or player2=:player2");
    query.bindValue(":player1",email);
    query.bindValue(":player2",email);
    if(!query.exec())
    {
        dout<<query.lastError().text();
        //todo:添加下载失败的事件
        return;
    }else{
        qint32 total = 0;
        try {
            while(query.next())
            {
                QListWidgetItem *item1 = new QListWidgetItem(list);
                item1->setSizeHint(QSize(list->width(), 20));
                QPointer<QLabel>label=new QLabel(list);
                list->addItem(item1);
                list->setItemWidget(item1,label);

                QString matchID = query.value(0).toString();
                QString Date = query.value(1).toString();
                QString gameMod = query.value(2).toString();
                QString rowNum = query.value(3).toString();
                QString colNum = query.value(4).toString();
                QString bombNum =query.value(5).toString();
                QString player1 =query.value(6).toString();
                QString player2 =query.value(7).toString();
                QString player1Integral = query.value(8).toString();
                QString player2Integral = query.value(9).toString();
                QString tmp;
                if(email == player1)
                {
                    tmp = formatEachGameItem( matchID, Date,
                                        gameMod, rowNum, colNum, bombNum,
                                        player1, player2, player1Integral, player2Integral);
                }else{

                    tmp = formatEachGameItem( matchID, Date,
                                        gameMod, rowNum, colNum, bombNum,
                                        player2, player1, player2Integral, player1Integral);
                }
                label->setText(tmp);
                total++;
            }
        } catch (...) {
            dout<<"传输第"<<total<<"条记录时出错";
        }
    }
    update();
}
/**
 * @brief Leaderboard::initSelfOrderWidget
 * @param widget
 * @param gameMod
 */
void Leaderboard::initSelfOrderWidget(QWidget* widget, QString gameMod)
{
    ////create table playHistory(matchID varchar(40) primary key,Date varchar(20),\
    ///gameMod varchar(20),rowNum varchar(4),colNum varchar(4),bombNum varchar(4),\
    ///player1 varchar(50),player2 varchar(50),player1Integral varchar(128),player2Integral varchar(128))"
    /// 共计10个参数
    QPointer<QListWidget>list = new QListWidget(widget);
    list->setAlternatingRowColors(true);
    list->resize(800,600);

    QSettings settings("./config.ini", QSettings::IniFormat);
    QString email =settings.value("Account/email").toString();
    QSqlQuery query;
//    bug:这里的排序变成按字典序排序了,未按数字大小排序,原因是储存方式是varchar,不是num,需要改的地方比较多，暂时不改了
    query.prepare("select * from playHistory \
        where (player1=:player1 or player2=:player2) and gameMod=:gameMod \
        order by player1Integral desc");
    query.bindValue(":player1",email);
    query.bindValue(":player2",email);
    query.bindValue(":gameMod",gameMod);
    if(!query.exec())
    {
        dout<<query.lastError().text();
        //todo:添加下载失败的事件
        return;
    }else{
        qint32 total = 0;
        try {
            while(query.next())
            {
                QListWidgetItem *item1 = new QListWidgetItem(list);
                item1->setSizeHint(QSize(list->width(), 20));
                QPointer<QLabel>label=new QLabel(list);
                list->addItem(item1);
                list->setItemWidget(item1,label);

                QString matchID = query.value(0).toString();
                QString Date = query.value(1).toString();
                QString gameMod = query.value(2).toString();
                QString rowNum = query.value(3).toString();
                QString colNum = query.value(4).toString();
                QString bombNum =query.value(5).toString();
                QString player1 =query.value(6).toString();
                QString player2 =query.value(7).toString();
                QString player1Integral = query.value(8).toString();
                QString player2Integral = query.value(9).toString();
                QString tmp;
                if(email == player1)
                {
                    tmp = formatEachGameItem( matchID, Date,
                                        gameMod, rowNum, colNum, bombNum,
                                        player1, player2, player1Integral, player2Integral);
                }else{

                    tmp = formatEachGameItem( matchID, Date,
                                        gameMod, rowNum, colNum, bombNum,
                                        player2, player1, player2Integral, player1Integral);
                }
                label->setText(tmp);
                total++;
            }
        } catch (...) {
            dout<<"传输第"<<total<<"条记录时出错";
        }
    }
    update();
}
//合成每局游戏结果对应显示的str
QString Leaderboard::formatEachGameItem(QString matchID,QString Date,
    QString gameMod,QString rowNum,QString colNum,QString bombNum,
    QString selfEmail,QString antiEmail,QString selfIntegral,QString antiIntegral)
{
    QString tmp;
    if(antiIntegral != "none")
    {
        if(selfIntegral > antiIntegral)
        {
            tmp += "胜利";
        }else if(selfIntegral == antiIntegral){
            tmp += "失败";
        }else{
            tmp+="平";
        }
        tmp+="  ";
    }

    tmp+="模式:"+gameMod + "行:"+rowNum +" 列:"+colNum+" 雷数:"+bombNum+" ";
    tmp+="积分:"+selfIntegral;
    tmp+="对手:"+antiEmail+" 对方积分:"+antiIntegral;
    tmp+=" "+Date;
    return tmp;
}
