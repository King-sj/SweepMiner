#pragma once
#include<QDebug>
/**
* @FileName	Base.h
* @brief         摘要
* 调试宏
*/
#define dout qDebug()<<"["<<__LINE__<<","<<__FUNCTION__<<","<<__FILE__<<"]"//< debug out (line,function name,file)
#define dendl Qt::endl//< debug endl
/**
 * @brief 检查是否在闭区间内部
 * @param l 左区间
 * @param r 右区间
 * @param p 点
 * @return
 *
 */
template<typename T>
inline bool inRange(T l,T r,T p)
{
    return l <= p && p <= r;
}
/**
 * @brief 判断(px,py)是否在矩形(topx,topy,width,height)
 * @param topx 矩形左上顶点横坐标
 * @param topy 矩形左上顶点纵坐标
 * @param width 矩形宽度
 * @param height
 * @param px
 * @param py
 * @return 若在其内部返回true,否则返回false
 */
template<typename T>
inline bool inRect(T topx,T topy,T width,T height,T px,T py)
{
    return inRange<T>(topx,width+topx,px) && inRange<T>(topy,height+topy,py);
}
