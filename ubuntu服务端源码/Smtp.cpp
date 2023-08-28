//smtp.cpp
#include "Smtp.h"
#include "debug.h"
/**
 * @brief Smtp::Smtp
 * @param username 163账号
 * @param password 授权码
 */
Smtp::Smtp(QByteArray username,QByteArray password)
{
    dout<<username<<password;
	if(username.contains("@163"))
	{
		this->username = username;
		this->password = password;
		//NRAGIYGMLFOSGQPS
	}
	else
    {
        dout<<"NOT 163";
    }
}
/**
 * @brief Smtp::~Smtp
 */
Smtp::~Smtp()
{
	delete clientsocket;
	clientsocket = nullptr;
}
/**
 * @brief Smtp::send
 * 发送新的邮件
 * @param recvaddr
 * 接收方邮箱
 * @param subject 主题
 * @param content 内容
 */
void Smtp::send(QByteArray recvaddr,QString subject,QString content)
{
    dout<<recvaddr<<subject<<content;
    this->recvaddr = recvaddr;
    this->subject = subject;
    this->content = content;
    QByteArray usernametmp = this->username;
    QByteArray recvaddrtmp = this->recvaddr;
    clientsocket=new QTcpSocket();
    this->clientsocket->connectToHost("smtp.163.com",25,QTcpSocket::ReadWrite);
    this->clientsocket->waitForConnected(1000);
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
    this->clientsocket->write("HELO smtp.163.com\r\n");
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
    this->clientsocket->write("AUTH LOGIN\r\n");
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
//    dout<<"username:"<<username;
    this->clientsocket->write(username.toBase64().append("\r\n"));
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
//    dout<<"password:"<<password;
    this->clientsocket->write(password.toBase64().append("\r\n"));
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
    this->clientsocket->write(mailfrom.append(usernametmp.append(">\r\n")));
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
    //发送邮箱
    //dout<<"mail from:"<<mailfrom.append(usernametmp.append(">\r\n"));
    this->clientsocket->write(rcptto.append(recvaddrtmp.append(">\r\n")));
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
    //接收邮箱
    //dout<<"rcp to:"<<rcptto.append(recvaddrtmp.append(">\r\n"));
    //data表示开始传输数据
    this->clientsocket->write("data\r\n");
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
    usernametmp = this->username;
    recvaddrtmp = this->recvaddr;
    this->clientsocket->write(prefrom.append(usernametmp.append("\r\n")));
    this->clientsocket->write(preto.append(recvaddrtmp.append("\r\n")));
    this->clientsocket->write(presubject.append(subject.toLocal8Bit().append("\r\n")));
    this->clientsocket->write("\r\n");
    this->clientsocket->write(content.toLocal8Bit().append("\r\n"));
    this->clientsocket->write(".\r\n");
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
    this->clientsocket->write("quit\r\n");
    clientsocket->flush();
    this->clientsocket->waitForReadyRead(1000);
    recvdata = clientsocket->readAll();
    dout<<recvdata;
}
