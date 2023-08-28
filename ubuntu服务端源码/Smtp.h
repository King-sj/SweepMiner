#ifndef __SMTP_H__
#define __SMTP_H__

#include<QByteArray>
#include<QString>
#include<QTcpSocket>
/**
 * @brief The Smtp class
 * 实现简单的通过网易163邮箱发送plain text的功能
 * 代码借鉴自csdn
 */
class Smtp
{
public:
    Smtp(QByteArray username,QByteArray password);
    ~Smtp();
	void send(QByteArray recvaddr,QString subject,QString content);
private:
    QTcpSocket * clientsocket;
    QByteArray username;//发送方邮箱
    QByteArray password;//授权码
    QByteArray mailfrom = "mail from:<";
    QByteArray rcptto = "rcpt to:<";
    QByteArray prefrom = "from:";
    QByteArray preto = "to:";
    QByteArray presubject ="subject:";
    QByteArray recvaddr;//接收方邮箱
    QString subject;//邮件标题
    QString content; //发送内容
    QByteArray recvdata;//接收到的数据
};

#endif // __SMTP_H__
