#include "MainWindow.h"
#include<QFileInfo>
#include<QMessageBox>
#include<QUrl>
#include"Base.h"
#include"SettingWindow.h"
#include"DenotationMod.h"
#include<QFormLayout>
#include<QDialogButtonBox>
#include<QLineEdit>
#include<QPushButton>
#include<QRegularExpressionValidator>
#include"Leaderboard.h"
#include"NetBoard.h"
#include"MessageTips/MessageTips.h"
#include"NetDenotationMod.h"
#include<QTimer>
#include<QDataStream>
#include<QFile>
#include<QFileInfo>
#include<QDir>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),packet{this}
{
    setupUi(this);    
    this->setWindowTitle("扫雷");
    this->socket = nullptr;
    this->board = nullptr;

    //todo:delete last line
//    this->pushButtonLeaderboard->setVisible(false);
    update();

    init();
    //添加socket事件处理
    packet.installClassFunctionEvent("login_response",1,&MainWindow::dealLoginResponse);
    packet.installClassFunctionEvent("signUp_response",2,&MainWindow::dealSignUpResponse);
    packet.installClassFunctionEvent("captcha_response",3,&MainWindow::dealCaptchaResponse);
    packet.installClassFunctionEvent("tansferHistoryFileHead",1,&MainWindow::dealTansferHistoryFileHead);
    packet.installClassFunctionEvent("tansferingHistoryFile",10,&MainWindow::dealTansferingHistoryFile);
    packet.installClassFunctionEvent("tansferHistoryFileEnd",0,&MainWindow::dealTansferHistoryFileEnd);
}
/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    database.close();
    if(board != nullptr)
    {
        delete board;
        board = nullptr;
    }
    if(socket != nullptr)
    {
        socket->disconnect();
        delete socket;
        socket = nullptr;
    }
    this->isLogin = false;
}
/**
 * @brief MainWindow::init
 * 初始化
 */
void MainWindow::init()
{
    this->board = nullptr;
    QFileInfo fileInfo("./config.ini");
    if (!fileInfo.isFile())
    {        
        MessageTips* mMessageTips = new MessageTips("缺少config.ini文件！",this);
        mMessageTips->show();
        return;
    }

    this->socket = new QTcpSocket(this);
    //todo
    this->socket->connectToHost(IP,2001);//连接服务器
    connect(socket,&QTcpSocket::readyRead,this,&MainWindow::dealRecv);
    connect(socket,&QTcpSocket::connected,this,&MainWindow::dealConnected);
    connect(socket,&QTcpSocket::disconnected,this,&MainWindow::dealDisconnected);
    if(!socket->waitForConnected(5000))//默认30s
    {
        //todo:连接超时        
        MessageTips* mMessageTips = new MessageTips("连接服务器超时,请检查网络",this);
        mMessageTips->show();
    }
}
/**
 * @brief MainWindow::tryLogin
 * 尝试登录
 * @param email
 * @param password
 * @return 成功返回true
 */
bool MainWindow::tryLogin(QString email,QString password)
{
    if(email == "" || password == "")
        return false;
    QStringList list;
    list<<"login"<<email<<password;
    socket->write(packet.formatMes(list).toStdString().c_str());
    socket->flush();
    return true;
}
/**
 * @brief MainWindow::on_pushButtonBeginGame_clicked
 * 启动classic game 的按钮的clicked 槽
 */
void MainWindow::on_pushButtonBeginGame_clicked()
{
    if(this->board != nullptr){
        try {
            dout<<"let's got new turn";
            delete this->board;
        } catch (...) {
            dout<<"delete board failed";
        }
    }
    QSettings settings("./config.ini", QSettings::IniFormat);
    this->difficulty = QString(settings.value("Difficulty/Difficulty").toString());//配置文件读取后，要对其进行类型转换
    qint32 rowNum = getSettingsIntValue(this->difficulty,"RowNum");
    qint32 colNum = getSettingsIntValue(this->difficulty,"ColNum");
    qint32 bombNum = getSettingsIntValue(this->difficulty,"BombNum");
    if(settings.value("GameMod/gameMod").toString() == "单机")
    {
        this->board = new Board(rowNum,colNum,bombNum);
        connect(board,&Board::signalUpLoadHistory,this,&MainWindow::dealSignalUpLoadHistory);
    }else{
        if(!isLogin)
        {
            auto mt = new MessageTips("请先登录");
            mt->show();
            update();
            return;
        }
        ///note:暂时选取共用一个socket的连接方式,由于本人Packet的设计,在NetBoard中注册socket消息的响应和MainWindow
        ///是独立的(实际上这样会让MainWindow多遍历一遍，倘若项目过于庞大会导致性能的消耗)
        ///
        this->board = new NetBoard(this->socket,rowNum,colNum,bombNum);
        connect(this,&MainWindow::signalMainSocketNewRecvMessage,
                static_cast<NetBoard*>(board),&NetBoard::dealMainSocketNewRecvMessage);
//        connect(this,&MainWindow::signalMainSocketNewRecvMessage,
//                [](QByteArray mes){dout<<mes;});
    }    
    connect(board,&Board::signalGameOver,this,&MainWindow::dealGameOver);
    this->board->show();
    this->hide();
}
/**
 * @brief 返回指定的整值（本来想用模板的，但是发生了一些问题(无法转化，无法特例化等)）
 * @param sectinName
 * @param ValueName
 * @return
 */
qint32 MainWindow::getSettingsIntValue(QString sectinName, QString valueName)
{
    //使用 settings.value(“节名/键名”)方法访问配置文件的值
    QSettings settings("./config.ini", QSettings::IniFormat);
    QString ver = sectinName+"/"+valueName;
    qint32 val = settings.value(ver).toInt();//配置文件读取后，要对其进行类型转换
    return val;
}
/**
 * @brief MainWindow::on_pushButtonSetting_clicked
 * 启动设置界面
 */
void MainWindow::on_pushButtonSetting_clicked()
{
    QPointer<QMainWindow> sw = new SettingWindow(this);
    sw->setWindowModality(Qt::WindowModal);
    sw->setAttribute(Qt::WA_DeleteOnClose);
    sw->show();
}
/**
 * @brief MainWindow::dealConnected
 * 和和服务器连接成功的槽函数
 */
void MainWindow::dealConnected()
{
    dout<<"连接成功\n";
    QSettings settings("./config.ini", QSettings::IniFormat);
    if(!tryLogin(settings.value("Account/email").toString(),settings.value("Account/password").toString()))
    {                
        showSignUpAndLogin();
    }
}
/**
 * @brief MainWindow::dealDisconnected
 * 处理与服务器断开连接时事件
 */
void MainWindow::dealDisconnected()
{
    MessageTips* mMessageTips = new MessageTips("掉线辣",this);
    mMessageTips->show();
    this->isLogin = false;
    this->labelAccount->setText("未登录");
}
/**
 * @brief MainWindow::dealRecv
 * 将服务器发送的消息托管给packet
 */
void MainWindow::dealRecv()
{
    auto buffer = socket->readAll();
    emit signalMainSocketNewRecvMessage(buffer);
    QString newMes = buffer;
    dout<<newMes;
    packet.pushMessage(newMes);
}
/**
 * @brief MainWindow::dealGameOver
 * 处理游戏结束事件
 */
void MainWindow::dealGameOver()
{
    this->show();
    this->board->hide();
    if(this->socket != nullptr && this->isLogin)
    {
        QStringList list;
        list<<"exitMatch";
        socket->write(packet.formatMes(list).toStdString().c_str());
    }
}
/**
 * @brief MainWindow::on_pushButtonDenotation_clicked
 * 启动爆炸模式
 */
void MainWindow::on_pushButtonDenotation_clicked()
{
    if(this->board != nullptr){
        delete this->board;
    }
    QSettings settings("./config.ini", QSettings::IniFormat);
    this->difficulty = QString(settings.value("Difficulty/Difficulty").toString());//配置文件读取后，要对其进行类型转换
    qint32 rowNum = getSettingsIntValue(this->difficulty,"RowNum");
    qint32 colNum = getSettingsIntValue(this->difficulty,"ColNum");
    qint32 bombNum = getSettingsIntValue(this->difficulty,"BombNum");
    if(settings.value("GameMod/gameMod").toString() == "单机")
    {
        this->board = new DenotationMod(rowNum,colNum,bombNum);
        connect(board,&Board::signalUpLoadHistory,this,&MainWindow::dealSignalUpLoadHistory);
    }else{        
        if(!isLogin)
        {
            auto mt = new MessageTips("请先登录");
            mt->show();
            update();
            return;
        }
        this->board = new NetDenotationMod(this->socket,rowNum,colNum,bombNum);
        connect(this,&MainWindow::signalMainSocketNewRecvMessage,
                static_cast<NetDenotationMod*>(board),&NetBoard::dealMainSocketNewRecvMessage);
    }    
    connect(board,&Board::signalGameOver,this,&MainWindow::dealGameOver);
    this->board->show();
    this->hide();
}
/**
 * @brief MainWindow::dealLoginResponse
 * 处理登录的返回结果
 * @param list length:1 value:1 / 0
 */
void MainWindow::dealLoginResponse(QStringList list)
{
    auto res = list[0];
    if(res == "1")
    {
        QSettings settings("./config.ini", QSettings::IniFormat);
        this->labelAccount->setText( settings.value("Account/email").toString());
        MessageTips* mMessageTips = new MessageTips("登录成功",this);
        mMessageTips->show();
        this->isLogin = true;
        downloadRemoteHistoryFile();
    }else{
        showSignUpAndLogin();
    }
}
/**
 * @brief MainWindow::showSignUpAndLogin
 * 显示注册窗口
 */
void MainWindow::showSignUpAndLogin()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel("登陆失败，请重新登录"));
    // 账号
    QString value1 = QString("账号:");
    auto acc = QLineEdit(&dialog);
    form.addRow(value1, &acc);
    // 密码
    QString value2 = QString("密码:");
    auto pas = QLineEdit(&dialog);
    pas.setContextMenuPolicy(Qt::NoContextMenu);
    pas.setEchoMode(QLineEdit::Password);
    form.addRow(value2,&pas);
    // Add button
    auto login = QPushButton("登录",&dialog);
    auto signUp = QPushButton("立即注册",&dialog);
    form.addRow(&login,&signUp);
    QObject::connect(&login, &QPushButton::clicked,[&](){
        dialog.hide();
        QSettings settings("./config.ini", QSettings::IniFormat);
        if(tryLogin(acc.text(),pas.text()))
        {
            settings.setValue("Account/email",acc.text());
            settings.setValue("Account/password",pas.text());
        }else{
            showSignUpAndLogin();
        }
    });
    QObject::connect(&signUp,&QPushButton::clicked,[&](){
        dialog.hide();
        QSettings settings("./config.ini", QSettings::IniFormat);
        settings.setValue("Account/email",acc.text());
        settings.setValue("Account/password",pas.text());
        QStringList list;
        list<<"signUp"<<acc.text()<<pas.text();
        socket->write(packet.formatMes(list).toStdString().c_str());
        socket->flush();
    });
    dialog.exec();
}
/**
 * @brief MainWindow::showCaptcha
 * 显示验证码填写窗口
 */
void MainWindow::showCaptcha()
{
    QDialog dia(this);
    QFormLayout fm(&dia);
    auto captcha = QLineEdit(&dia);
    captcha.setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]+$")));
    fm.addRow("验证码:",&captcha);
    auto ok = QPushButton(&dia);
    ok.setText("确定");
    fm.addRow("   ",&ok);
    connect(&ok,&QPushButton::clicked,[&]{
        dia.hide();
        QStringList list;
        list<<"captcha"<<captcha.text();
        socket->write(packet.formatMes(list).toStdString().c_str());
        socket->flush();
    });
    dia.exec();
}
/**
 * @brief MainWindow::downloadRemoteHistoryFile
 * 从服务器下载历史战绩文件
 */
void MainWindow::downloadRemoteHistoryFile()
{
    //todo:添加下载远程文件的方法
    QStringList list;
    list<<"downLoadHistoryFile";
    this->socket->write(packet.formatMes(list).toStdString().c_str());
    socket->flush();
}
/**
 * @brief MainWindow::dealSignUpResponse
 * 处理注册结果
 * @param list length:2 { 1 / 0}{detail information}
 */
void MainWindow::dealSignUpResponse(QStringList list)
{
    auto res = list[0];
    if(res == "0")
    {
        QMessageBox::information(nullptr,"注册失败",list[1]);
        showSignUpAndLogin();
    }else if(res == "1"){
        showCaptcha();
    }else{
        dout<<"error"<<list;
        throw "error";
    }
}
/**
 * @brief MainWindow::dealCaptchaResponse
 * 处理验证码验证结果
 * @param list length:3 {0/1}{email、ps}
 */
void MainWindow::dealCaptchaResponse(QStringList list)
{
    if(list[0] == "0")
    {
        MessageTips* mMessageTips = new MessageTips("验证码错误",this);
        mMessageTips->show();
        showCaptcha();
    }else{
        MessageTips* mMessageTips = new MessageTips("注册成功,已自动登录。",this);
        mMessageTips->show();
        this->isLogin = true;
        this->labelAccount->setText(list[1]);
        QSettings settings("./config.ini", QSettings::IniFormat);
        settings.setValue("Account/email",list[1]);
        settings.value("Account/password",list[0]);
        downloadRemoteHistoryFile();
    }
}
/**
 * @brief MainWindow::dealSignalUpLoadHistory
 * 上传历史战绩
 * @param gameMod
 * @param rowNum
 * @param colNum
 * @param bombNum
 * @param integral
 */
void MainWindow::dealSignalUpLoadHistory(QString gameMod, QString rowNum, QString colNum, QString bombNum, QString integral)
{
    QStringList list;
    list<<"upLoadHistory"<<gameMod<<rowNum<<colNum<<bombNum<<integral;
    socket->write(packet.formatMes(list).toStdString().c_str());
    socket->flush();
}
/**
 * @brief MainWindow::dealTansferHistoryFileHead
 * 下载历史文件的请求头处理函数
 * @param list length:0
 */
void MainWindow::dealTansferHistoryFileHead(QStringList list)
{
    QString fileName = list[0];
    QFileInfo info(fileName);
    if(info.exists())
    {
        QFile::remove(fileName);
    }
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
        database.setDatabaseName(fileName);
    }
    if (!database.open())
    {
        dout<< "Error: Failed to connect database." << database.lastError();
    }
    else
    {
        dout<<"success\n";
    }
    QSqlQuery sql_query;
    QString tmp = "create table playHistory(matchID varchar(40) primary key,Date varchar(20),\
            gameMod varchar(20),rowNum varchar(4),colNum varchar(4),bombNum varchar(4),\
            player1 varchar(50),player2 varchar(50),player1Integral varchar(128),player2Integral varchar(128))";
    //创建存储云端历史信息的数据库
    if(!sql_query.exec(tmp))
    {
        dout << "Error: Fail to create table." << sql_query.lastError();
    }
}
/**
 * @brief MainWindow::dealTansferingHistoryFile
 * 传输历史文件过程中的处理函数
 * @param list length : 10
 */
void MainWindow::dealTansferingHistoryFile(QStringList list)
{
    QString sql = QString("insert into playHistory values (?,?,?, ?,?,?, ?,?,?,?)");
    QSqlQuery query;
    query.exec("begin");
    query.prepare(sql);
//    this->lastGameMatchID = QUuid::createUuid().toString();
    dout<<list.size();
    for(qint32 i = 0; i < list.size() ; i++)
    {
        query.bindValue(i,list[i]);
    }
    if(!query.exec())
    {
        dout<<query.lastError().text();
    }else{
        dout<<"success ++";
    }
    query.exec("commit");
}
/**
 * @brief MainWindow::dealTansferHistoryFileEnd
 * 传输历史文件结束的处理函数
 * @param list : length :0
 */
void MainWindow::dealTansferHistoryFileEnd(QStringList list)
{
    dout<<"down load game history successfully";
}
/**
 * @brief MainWindow::on_pushButtonLeaderboard_clicked
 * 启动显示战绩的界面
 */
void MainWindow::on_pushButtonLeaderboard_clicked()
{    
    if(!isLogin)
    {
        auto mt = new MessageTips("请先登录");
        mt->show();
        update();
        return;
    }
    if(leaderBoard != nullptr)
    {
        delete leaderBoard;
        leaderBoard = nullptr;
    }
    leaderBoard = new Leaderboard(this->socket);
    connect(this,&MainWindow::signalMainSocketNewRecvMessage,
            leaderBoard,&Leaderboard::dealMainSocketNewRecvMessage);
    leaderBoard->show();
}
