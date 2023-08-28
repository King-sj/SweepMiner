QT       += core gui multimedia network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BackgroundMusicPlayer.cpp \
    Board.cpp \
    ChooseByDirection.cpp \
    Chunk.cpp \
    DenotationMod.cpp \
    Leaderboard.cpp \
    MainWindow.cpp \
    MessageTips/MessageTips.cpp \
    NetBoard.cpp \
    NetDenotationMod.cpp \
    SettingWindow.cpp \
    main.cpp

HEADERS += \
    BackgroundMusicPlayer.h \
    Base.h \
    Board.h \
    ChooseByDirection.h \
    Chunk.h \
    DenotationMod.h \
    Leaderboard.h \
    MainWindow.h \
    MessageTips/MessageTips.h \
    NetBoard.h \
    NetDenotationMod.h \
    Packet/Packet.h \
    Packet/Packet.cpp \
    SettingWindow.h

FORMS += \
    Board.ui \
    Leaderboard.ui \
    MainWindow.ui \
    SettingWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
	resource.qrc

dataFiles.files+=src/readme.txt
dataFiles.files+=src/pic.png
dataFiles.path = /assets/data
INSTALLS += dataFiles

QMAKE_LFLAGS+="-fsanitize=address"
QMAKE_LFLAGS+="-fsanitize=undefined"
QMAKE_LFLAGS+="-fsanitize=leak"
