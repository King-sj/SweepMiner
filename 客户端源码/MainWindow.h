#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>
#include"Board.h"
#include<QSettings>
#include<QTcpSocket>
#include"Packet/Packet.h"
#include"Leaderboard.h"
#include<QByteArray>
#include<QSqlDatabase>
/**
 * @brief The MainWindow class
 * 主窗口对象
 */
class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();    
private:
    void init();
    qint32 getSettingsIntValue(QString sectinName,QString ValueName);
    virtual bool tryLogin(QString email,QString password);
    virtual void showSignUpAndLogin();
    virtual void showCaptcha();
    virtual void downloadRemoteHistoryFile();
private slots:
    void on_pushButtonBeginGame_clicked();
    void on_pushButtonSetting_clicked();
    void on_pushButtonDenotation_clicked();
    void on_pushButtonLeaderboard_clicked();
private slots:
    virtual void dealConnected();
    virtual void dealDisconnected();
    virtual void dealRecv();
    virtual void dealGameOver();
private:
//    const QString IP = "101.42.8.164";//服务器ip
    const QString IP = "127.0.0.1";//连接本地测试
    Board* board;
    QPointer<Leaderboard> leaderBoard;
    QTcpSocket* socket;
    QString difficulty;
    bool isLogin = false;    
    QSqlDatabase database;
private:
    friend class Packet<MainWindow>;
    Packet<MainWindow> packet;
    virtual void dealLoginResponse(QStringList list);
    virtual void dealSignUpResponse(QStringList list);
    virtual void dealCaptchaResponse(QStringList list);
    virtual void dealSignalUpLoadHistory(QString gameMod,QString rowNum,QString colNum,QString bombNum,QString integral);    
    virtual void dealTansferHistoryFileHead(QStringList list);
    virtual void dealTansferingHistoryFile(QStringList list);
    virtual void dealTansferHistoryFileEnd(QStringList list);
signals:
    void signalMainSocketNewRecvMessage(QByteArray mes);
};
