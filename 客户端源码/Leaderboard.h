#pragma once

#include "ui_Leaderboard.h"
#include<QTcpSocket>
#include"Packet/Packet.h"
#include<QWidget>
#include<QPointer>
/**
 * @brief The Leaderboard class
 * 显示历史战绩
 */
class Leaderboard : public QMainWindow, public Ui::Leaderboard
{
    Q_OBJECT

public:
    explicit Leaderboard(QTcpSocket* socket,QWidget *parent = nullptr);
public slots:
    virtual void dealMainSocketNewRecvMessage(QByteArray mes);
private:
    QTcpSocket* socket;
    friend class Packet<Leaderboard>;
    Packet<Leaderboard> packet;    
private:
    QPointer<QWidget>allHistoryWidget=nullptr;
    QPointer<QWidget>classicOrderWidget=nullptr;
    QPointer<QWidget>denotationOrderWidget=nullptr;
    QPointer<QWidget>netBoardOrderWidget=nullptr;
    QPointer<QWidget>netDenotationOrderWidget=nullptr;
    void initAllHistoryWidget();
    void initSelfOrderWidget(QWidget* widget,QString gameMod);
    QString formatEachGameItem(QString matchID,QString Date,
        QString gameMod,QString rowNum,QString colNum,QString bombNum,
        QString selfEmail,QString antiEmail,QString selfIntegral,QString antiIntegral);
};

