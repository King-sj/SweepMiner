#pragma once

#include "ui_SettingWindow.h"
/**
 * @brief The SettingWindow class
 * 设置界面
 */
class SettingWindow : public QMainWindow, private Ui::SettingWindow
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr);
    virtual void reloadApp();
private slots:
    void on_pushButtonCancel_clicked();
    void on_pushButtonOk_clicked();
    virtual void mousePressEvent(QMouseEvent* e)override;
    void on_pushButtonExitLogin_clicked();
};

