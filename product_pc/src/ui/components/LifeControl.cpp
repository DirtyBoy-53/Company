#include "LifeControl.h"
#include "ui_LifeControl.h"
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTextCodec>
#include "configinfo.h"

LifeControl::LifeControl(QWidget *parent) :
    QLabel(parent),
    ui(new Ui::LifeControl)
{
    ui->setupUi(this);
    setText("耗材管控");
    setMinimumSize(250, 50);
    setMaximumSize(250, 50);

    setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    m_lifeSetup = new LifeControlSetup();
    m_lifeSetup->hide();
    updateStatusLife(false);

    connect(m_lifeSetup, &LifeControlSetup::sigResetLabelStatus, this, [this](){
        setStyleSheet("background-color:rgba(0,220,0,255);color:rgba(255,255,255,255)");
        emit sigOutLimitation(false);
    });

}

LifeControl::~LifeControl()
{
    delete ui;
}

void LifeControl::updateStatusLife(bool usrPlusOne)
{
    bool bMng = true;
    ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "耗材寿命管控", bMng);
    if (!bMng) {
        qDebug() << "无寿命管控逻辑存在";
        return;
    }

    QString path = QCoreApplication::applicationDirPath() + "/config/";
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }
    path += "lifeSetup.ini";

    QSettings setting(path, QSettings::IniFormat);
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    setting.setIniCodec(codec);
    qDebug() << path;

    if (!QFile::exists(path)) {
        QMessageBox::information(this, "注意", "无寿命管控配置文件，请先配置寿命管控内容[若此站无寿命管控，忽略此提示]");
        return;
    }

    QStringList groups = setting.childGroups();
    qDebug() << groups;
    float max_rate = 0.0f;
    foreach (QString grp, groups) {
        setting.beginGroup(grp);
        int used = setting.value("used").toInt();
        int total = setting.value("total").toInt();
        int added = setting.value("added").toInt();
        int plus = total + added;
        if (plus == 0) plus = 1;

        if (usrPlusOne) {
            used++;
            setting.setValue("used", QVariant(used));
        }

        float rate = 100.f * float(float(used) / float(plus));
        if (max_rate < rate) max_rate = rate;

        setting.endGroup();
    }

    if (max_rate >= 100) {
        setStyleSheet("background-color:rgba(220,0,0,255);color:rgba(255,255,255,255)");
//    } else if (max_rate > 98) {
//        setStyleSheet("background-color:rgba(220,0,0,255);color:rgba(255,255,255,255)");
    } else if(max_rate > 95) {
        setStyleSheet("background-color:rgba(250,250,0,255);color:rgba(0,0,0,255)");
    } else {
        setStyleSheet("background-color:rgba(0,220,0,255);color:rgba(255,255,255,255)");
    }

    if (max_rate >= 100) {
        emit sigOutLimitation(true);
    } else {
        emit sigOutLimitation(false);
    }
}

void LifeControl::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (1 == ConfigInfo::getInstance()->cacheInfo().Permissionmode) {
        m_lifeSetup->show(true);
    } else {
        m_lifeSetup->show(false);
    }
}


