/**
 * 调试相关的宏定义
 */
#include<QDebug>
#define dout qDebug()<<"["<<__LINE__<<","<<__FUNCTION__<<","<<__FILE__<<"]"//< debug out (line,function name,file)
#define dendl Qt::endl//< debug endl
