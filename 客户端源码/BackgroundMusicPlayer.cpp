#include "BackgroundMusicPlayer.h"
#include "Base.h"
#include<QThread>
#include<QAudioOutput>
/*
 * warning:QObject::killTimer: Timers cannot be stopped from another thread
 * 这个类有以上输出
*/
/**
 * @brief BackgroundMusicPlayer::BackgroundMusicPlayer
 * @param parent
 */
BackgroundMusicPlayer::BackgroundMusicPlayer(QObject *parent)
    : QObject{parent}
{    
    player = new QMediaPlayer();
    audioOutput = new QAudioOutput();
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(50);
    dout<<this->thread()->currentThreadId();
}
/**
 * @brief BackgroundMusicPlayer::playNewBackgroundMusc
 * 播放新的音乐
 * @param path 文件地址
 */
void BackgroundMusicPlayer::playNewBackgroundMusc(QString path)
{
//    dout<<path<<dendl;

//    connect(player,&QMediaPlayer::positionChanged,[](qint64 t){dout<<t<<dendl;});
    player->setSource(QUrl(path));
    player->setLoops(QMediaPlayer::Loops::Once);
    player->play();
}
