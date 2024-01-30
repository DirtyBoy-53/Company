#include "eventwidget.h"
#include "ui_eventwidget.h"
#include <QDebug>
#include <QDateTime>
#include <QDir>
EventWidget::EventWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EventWidget)
{
    ui->setupUi(this);
    createFileDir(QCoreApplication::applicationDirPath() + "/EventLog");
    m_tagFile = QCoreApplication::applicationDirPath() + "/EventLog/tag.txt";
    ui->editTagPath->setText(m_tagFile);
}

EventWidget::~EventWidget()
{
    delete ui;
}
QString EventWidget::createFileDir(QString filePath)
{
    QDir dir(filePath);
    if (dir.exists(filePath)){
        return filePath;
    }

    QString parentDir = createFileDir(filePath.mid(0,filePath.lastIndexOf('/')));
    QString dirname = filePath.mid(filePath.lastIndexOf('/') + 1);
    QDir parentPath(parentDir);
    if ( !dirname.isEmpty()){
        parentPath.mkpath(dirname);
    }

    return parentDir + "/" + dirname;
}
void EventWidget::addTag(QString value, int index)
{
    QFile file(m_tagFile);
    if(file.open(QIODevice::Append)) {
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss.zzz");
        QString msg = QString("%1 : %2_%3").arg(time).arg(value).arg(index);
        QTextStream text(&file);
        text << msg << "\r\n";
        file.flush();
        file.close();
        ui->listWidget->addItem(msg);
    }
}

void EventWidget::slot_btn_clicked()
{
    auto btn = (QPushButton*)sender();
    if(btn->objectName() == "btnReboot"){
        m_times.btnReboot++;
        addTag("重启", m_times.btnReboot);
    }else if(btn->objectName() == "btnAlarmError"){
        m_times.btnAlarmError++;
        addTag("误报", m_times.btnAlarmError);
    }else if(btn->objectName() == "btnCheckError"){
        m_times.btnCheckError++;
        addTag("误检", m_times.btnCheckError);
    }else if(btn->objectName() == "btnCheckMissed"){
        m_times.btnAlarmMissed++;
        addTag("漏报", m_times.btnAlarmMissed);
    }else if(btn->objectName() == "btnCustom"){
        QString txt = ui->editCustom->text();
        m_times.btnCustom++;
        addTag(txt, m_times.btnCustom);
    }else{}
}
