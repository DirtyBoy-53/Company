#include "inputmessagebox.h"
#include "ui_inputmessagebox.h"
#include <QKeyEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <configinfo.h>
#include <QMessageBox>
#include <QThread>
#include <QCoreApplication>
#include <thread>
#include <QElapsedTimer>

InputMessageBox::InputMessageBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InputMessageBox)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::WindowTitleHint;
    setWindowFlags(flags);
    ui->lineEdit->setFocus();
    ui->stackedWidget->setCurrentIndex(0);
    ui->label->setWordWrap(true);

//    ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "按钮选择器", m_useSelector);
//    if (m_useSelector) {
//        m_btnSelector = new CButtonSelector;
//        qDebug() << "使用按钮选择器";
//        if (!m_btnSelector->getHandle()) {
//            qDebug() << "获取按钮选择器句柄失败";
//            QMessageBox::information(this, "错误", "<font size='26' color='red'>初始化按钮选择器错误</font>");
//        } else {
//            QString comport;
//            ConfigInfo::getInstance()->getValueString("GLOBAL", "按钮选择器端口", comport);
//            qDebug() << "开启端口：" << comport;
//            if (!m_btnSelector->open(comport)) {
//                qDebug() << "开启按钮选择器失败:" << m_btnSelector->errorString();
//                QMessageBox::information(nullptr, "错误", "<font size='26' color='red'>初始化按钮选择器错误: "+ m_btnSelector->errorString() +"</font>");
//                // exit(0);
//            } else {
//                qDebug() << "开启按钮选择器OK";
//            }
//        }
//    }
}

InputMessageBox::~InputMessageBox()
{
//    m_btnSelector->close();
//    m_btnSelector->freeHandle();
    qInfo("out msg...");
    delete ui;
}

void InputMessageBox::setMsg(QString sMesg, int type)
{
    m_type = type;
    m_notifyStop = true;
    if(sMesg.length() <= 0) {
        hide();
        return;
    }
    ui->lineEdit->setFocus();
    ui->label->setText(sMesg);
    QString strStyle ;
    strStyle.append("color:rgba(0,0,200,1);");
    strStyle.append("background-color:rgba(240,240,220,150);");
    strStyle.append("font-size: 30px;");
    strStyle.append("font-weight: Bold;");
    strStyle.append("text-align: center;");
    strStyle.append("qproperty-alignment: AlignCenter;");
    this->move(900, QApplication::desktop()->geometry().height()/2 + height());

    ui->pushButton->setText("点击回车确认");
    if(type == InputBox_NgTips) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->lineEdit->hide();
        strStyle.append("color: rgb(220, 0, 0);");
    } else if(type == InputBox_OkNg) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->labelTips->setText(sMesg);
        ui->labelTips->setStyleSheet("font-size:30px;background-color:rgba(240,240,220,150);"
                                     "color:rgba(0,0,200,1);font-weight: Bold;text-align: center;"
                                     "qproperty-alignment: AlignCenter;");
    } else if(type == InputBox_OkTips || type == InputBox_OkTips1) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->lineEdit->hide();
        strStyle = "font-size:30px;background-color:rgba(240,240,220,150);"
                   "color:rgba(0,0,200,1);font-weight: Bold;text-align:center;"
                   "qproperty-alignment: AlignCenter;";
    } else if (type == InputBox_ShowOnly) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->lineEdit->hide();
        strStyle = "font-size:30px;background-color:rgba(240,240,220,150);"
                   "color:rgba(0,0,200,1);font-weight: Bold;text-align:center;"
                   "qproperty-alignment: AlignCenter;";
        ui->pushButton->setText("点按钮选择器确认");
    } else if (type == InputBox_Input){
        ui->stackedWidget->setCurrentIndex(0);
        ui->lineEdit->setText("");
        ui->lineEdit->show();
    } else {
        QMessageBox::information(nullptr, "错误", "<font size='26' color='red'>无对应类型弹框MsgBox</font>");
        return;
    }

    ui->label->setStyleSheet(strStyle);
    ui->label->setText(sMesg);
    m_notifyStop = false;
    show();    
}

void InputMessageBox::keyPressEvent(QKeyEvent *event)
{
    if(ui->stackedWidget->currentIndex() != 0) return;

    if (m_type == InputBox_ShowOnly) {
        return;
    }

    qDebug() << event->key();
    switch(event->key())
    {
    case Qt::Key_Return:
        onOut();
        break;
    case Qt::Key_Enter:
        onOut();
        break;
    default:
        break;
    }
}

void InputMessageBox::onOut()
{
    qDebug() << __FUNCTION__ << __threadid();
    QString txt = ui->lineEdit->text().toUpper();
    emit signalConfirmClicked(txt);
    close();
}

void InputMessageBox::closeEvent(QCloseEvent *event)
{
    qDebug() << __FUNCTION__ << __threadid();
    ui->label->clear();
    m_msg.clear();
    hide();
    m_notifyStop = true;
}

void InputMessageBox::showEvent(QShowEvent *event)
{
    if(!ui->lineEdit->isHidden()) {
        ui->lineEdit->setFocus();
        ui->lineEdit->setText("");
    }
}

void InputMessageBox::on_btnSuc_clicked()
{
    qDebug() << __FUNCTION__ << __threadid();
    emit signalClickedFinish(true);
    close();
}

void InputMessageBox::on_btnFail_clicked()
{
    qDebug() << __FUNCTION__ << __threadid();
    emit signalClickedFinish(false);
    close();
}
