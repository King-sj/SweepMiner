#pragma once

#include <QObject>
#include<QMediaPlayer>
#include<QPointer>
/**
 * @brief The BackgroundMusicPlayer class
 * 这个类设置了退出自动删除，不必也不应该使用智能指针
 * 该类是多线程类,将其moveToThread后通过信号使用它
 */
class BackgroundMusicPlayer : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundMusicPlayer(QObject *parent = nullptr);

private:
    QPointer<QMediaPlayer> player;
    QPointer<QAudioOutput>audioOutput;
signals:
public slots:
    virtual void playNewBackgroundMusc(QString path);
};

