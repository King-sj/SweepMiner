// Packet.cpp -- 模板类的实现
#pragma once
#include "Packet.h"
#include<QList>
#include<QDebug>
#define dout qDebug()<<"["<<__LINE__<<","<<__FUNCTION__<<","<<__FILE__<<"]"//< debug out (line,function name,file)
#define dendl Qt::endl//< debug endl
/**
 * @brief Packet::Packet
 * 构造函数,T为callBack所在的类名,会在触发事件的是否调用T::callBack(StringList)
 * @param parent 不能为空
 */
template<typename T>
Packet<T>::Packet(T* parent)
    :parent(parent)
{

}
/**
 * @brief Packet::pushMessage
 *压入信息,并判断是否满足触发event的条件
 * @param newMes
 */
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
/**
 * @brief Packet::formatMes
 * 返回封装好的信息
 * @param newMesList 要封装的信息
 * @return
 */
template<typename T>
QString Packet<T>::formatMes(QStringList newMesList)
{
    QString res="";
    foreach (const QString& s, newMesList) {
        res+=s+separator;
    }
    return res;
}
/**
 * @brief Packet::formatMes
 * 重载方法，方便使用
 * @param newMes
 * @return
 */
template<typename T>
QString Packet<T>::formatMes(QString newMes)
{
    return newMes+separator;
}
/**
 * @brief Packet::installClassFunctionEvent
 * 注册事件
 * @param funcName 触发事件对应的消息
 * @param parameterNum 回调函数的参数个数 （后期可维护成自动判断的可取消这个函数）//todo标记为废弃
 * @param void (T::*callBack)(QStringList) 回调函数
 */
template<typename T>
void Packet<T>::installClassFunctionEvent(QString funcName, qint32 parameterNum, void (T::*callBack)(QStringList))
{
    this->funcEvents.push_back(new FunctionEvent(funcName,parameterNum,callBack));
}

/**
 * @brief Packet::distributerEvent
 * 尝试分配事件
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
/**
 * @brief Packet::splitMes
 * 拆分信息
 * @param mes
 * @return
 */
template<typename T>
inline QStringList Packet<T>::splitMes(QString mes)
{
    return mes.split(Packet<T>::separator);
}
/**
 * @brief Packet::separator
 * 分割符 //todo:后期改为其它实现方式,而不是用分隔符
 */
template<typename T>
const QString Packet<T>::separator = "#####";
