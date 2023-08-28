#include "SettingWindow.h"
#include<QPointer>
#include<QSettings>
#include<QCheckBox>
#include<QMessageBox>
#include<QMouseEvent>
#include<QApplication>
#include<QProcess>
/**
 * @brief SettingWindow::SettingWindow
 * @param parent
 */
SettingWindow::SettingWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    this->labelChooseDifficulty->setItems(QVector<QString>{"Easy","Mid","Difficult"});
    this->labelChoosegameMod->setItems(QVector<QString>{"单机","网络对战"});
    setWindowFlags((windowFlags() & ~Qt::WindowCloseButtonHint ));
    this->customWidget->setVisible(false);
    connect(this->checkBoxCustom,&QCheckBox::stateChanged,[this](qint32 state){
        switch (state) {
        case Qt::Unchecked:{
            this->customWidget->setVisible(false);
            this->difficultyWidget->setEnabled(true);
            break;}
        case Qt::Checked:{
            this->customWidget->setVisible(true);
            this->difficultyWidget->setEnabled(false);
            break;}
        default:
            break;
        }
        update();
    });
    this->setWindowTitle("扫雷");
}
/**
 * @brief SettingWindow::reloadApp
 * 重启程序
 */
void SettingWindow::reloadApp()
{

    qApp->quit();
    QProcess::startDetached(qApp->applicationFilePath(), QStringList());

}
/**
 * @brief SettingWindow::on_pushButtonCancel_clicked
 * 取消修改
 */
void SettingWindow::on_pushButtonCancel_clicked()
{
    this->close();
}

/**
 * @brief SettingWindow::on_pushButtonOk_clicked
 * 确定修改
 */
void SettingWindow::on_pushButtonOk_clicked()
{
    QSettings settings("./config.ini", QSettings::IniFormat);
    QString ver = "Difficulty/Difficulty";
    if(this->checkBoxCustom->checkState() == Qt::Checked)
    {
        qint32 rowNum = this->spinBoxRowNum->value();
        qint32 colNum = this->spinBoxColNum->value();
        qint32 bombNum = this->spinBoxBombNum->value();
        if(bombNum > 0.8*rowNum*colNum )
        {
            QMessageBox::information(nullptr,"","参数不合法");
            return;
        }
        settings.setValue(ver,"Custom");
        settings.setValue("Custom/RowNum",rowNum);
        settings.setValue("Custom/ColNum",colNum);
        settings.setValue("Custom/BombNum",bombNum);
    }else settings.setValue(ver,this->labelChooseDifficulty->getCurrentItems());
    settings.setValue("GameMod/gameMod",this->labelChoosegameMod->getCurrentItems());
    this->close();
}
/**
 * @brief SettingWindow::mousePressEvent
 * @param e
 */
void SettingWindow::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        this->setFocus();
    }
}

/**
 * @brief SettingWindow::on_pushButtonExitLogin_clicked
 */
void SettingWindow::on_pushButtonExitLogin_clicked()
{
    //todo
    QSettings settings("./config.ini", QSettings::IniFormat);
    settings.setValue("Account/email","");
    settings.setValue("Account/password","");
    reloadApp();
}

