// Packet.h -- 模板类的声明
#pragma once
#include <QStringList>
#include <QString>
#include<QPointer>
/**
 * @brief 用于socket协议的信息封装和解包，可以绑定信息--回调函数,Packet.cpp和Packet.h都得放在头文件中( -I Packet.cpp Packet.h)
 * 如果要绑定私有行为，应该将Packet<T>声明为友元
 * T为parent对应的类名,installClassFunctionEvent 会在触发时调用parent的成员函数
 * 所有要绑定的函数都应该以void为返回值,QStringList为参数
 */
template<typename T>
class Packet
{
    struct FunctionEvent
    {
    public:
        QString funcName;
        qint32 parameterNum;
        void (T::*callBack)(QStringList);
        FunctionEvent(QString funcName, qint32 parameterNum, void (T::*callBack)(QStringList))
            :funcName(funcName),parameterNum(parameterNum),callBack(callBack){};
    };
public:
    Packet(T* parent);
    virtual void pushMessage(QString newMes);//压入信息，可能会触发callBack
    virtual QString formatMes(QStringList newMesList);//<将消息封装
    virtual QString formatMes(QString newMes);//<重载
    virtual void installClassFunctionEvent(QString funcName, qint32 parameterNum, void (T::*callBack)(QStringList));
private:
    virtual void distributerEvent();
    virtual inline QStringList splitMes(QString mes);
private:
    static const QString separator;
    QStringList recvList;
    QString recvBuff;
    T* parent;
    //warning:这里使用普通指针我也不知道会出什么问题不
    QList<FunctionEvent*> funcEvents;
};

#include "Packet.cpp" // 包含模板类的实现
