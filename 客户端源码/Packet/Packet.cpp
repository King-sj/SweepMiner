// Packet.cpp -- 模板类的实现
#pragma once
#include "Packet.h"
#include<QList>
#include<QDebug>
#define dout qDebug()<<"["<<__LINE__<<","<<__FUNCTION__<<","<<__FILE__<<"]"//< debug out (line,function name,file)
#define dendl Qt::endl//< debug endl
template<typename T>
Packet<T>::Packet(T* parent)
    :parent(parent)
{

}
template<typename T>
void Packet<T>::pushMessage(QString newMes)
{
    QStringList newMesList = splitMes(newMes);
    if(newMesList.length() == 0)return;
    newMesList[0] = recvBuff+newMesList[0];
    if(newMesList[newMesList.length()-1] != "")
    {
        recvBuff = newMesList[newMesList.length()-1];
    }
    else
    {
        recvBuff.clear();
    }
    newMesList.removeLast();
    this->recvList << newMesList;

    this->distributerEvent();//尝试分配事件与处理函数
}

template<typename T>
QString Packet<T>::formatMes(QStringList newMesList)
{
    QString res="";
    foreach (const QString& s, newMesList) {
        res+=s+separator;
    }
    return res;
}

template<typename T>
QString Packet<T>::formatMes(QString newMes)
{
    return newMes+separator;
}

template<typename T>
void Packet<T>::installClassFunctionEvent(QString funcName, qint32 parameterNum, void (T::*callBack)(QStringList))
{
    this->funcEvents.push_back(new FunctionEvent(funcName,parameterNum,callBack));
}

/**
 * @brief Packet::distributerEvent
 */
//todo:添加对长时间未处理等的（bad request）的处理
template<typename T>
void Packet<T>::distributerEvent()
{
    dout<<this->recvList;
    bool flag = false;//是否触发过
    for(qint32 i = 0 ; i < this->funcEvents.length() ; i++)
    {
        if(recvList.length() > 0 && recvList[0] == funcEvents[i]->funcName)
        {
            if(recvList.length() > funcEvents[i]->parameterNum)
            {
                recvList.removeFirst();
                QStringList par;
                for(qint32 j = 0 ; j < funcEvents[i]->parameterNum ; j++)
                {
                    par<<recvList[0];
                    recvList.removeFirst();
                }
                auto p = funcEvents[i]->callBack;
//                dout<<p;
                (parent->*p)(par);
                flag = true;
            }
        }
    }
    if(flag)
        distributerEvent();//继续看是否还能触发其它事件
}

template<typename T>
inline QStringList Packet<T>::splitMes(QString mes)
{
    return mes.split(Packet<T>::separator);
}

template<typename T>
const QString Packet<T>::separator = "#####";
