#include "LSideWindow.h"
#include "ui_LSideWindow.h"
#include "YFunction.h"
#include "confile.h"
#include "appdef.h"
#include <QMessageBox>

LSideWindow::LSideWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LSideWindow)
{
    ui->setupUi(this);
    initUI();
    initConnect();
}

LSideWindow::~LSideWindow()
{
    delete ui;
}

void LSideWindow::btn_clicked()
{
    QString name = qobject_cast<QPushButton*>(sender())->objectName();
    if(name.contains("Btn_VideoSavePath")){
        //选择视频保存路径
        auto path = choosePath();
        if(path.isEmpty()){
            QMessageBox::warning(this, APP_NAME, "保存路径异常，请重新选择.", QMessageBox::Ok);
            return;
        }
        ui->Edit_VideoSavePath->setText(path);
        g_config->set<std::string>("videoSavePath", path.toStdString(), "root");
    }

    if(name.contains("Btn_PicSavePath")){
        //选择截图保存路径
        auto path = choosePath();
        if(path.isEmpty()){
            QMessageBox::warning(this, APP_NAME, "保存路径异常，请重新选择.", QMessageBox::Ok);
            return;
        }
        ui->Edit_PicSavePath->setText(path);
        g_config->set<std::string>("picSavePath", path.toStdString(), "root");
    }



}

void LSideWindow::initUI()
{
    auto videoPath = g_config->get<std::string>("videoSavePath", "root", (g_exec_dir_path + "/record_video/").toStdString());
    auto picPath = g_config->get<std::string>("videoSavePath", "root", (g_exec_dir_path + "/screen_shot/").toStdString());

    ui->Edit_VideoSavePath->setText(QString::fromStdString(videoPath));
    ui->Edit_PicSavePath->setText(QString::fromStdString(picPath));
}

void LSideWindow::initConnect()
{
    connect(ui->Btn_PicSavePath,    &QPushButton::clicked, this, &LSideWindow::btn_clicked);
    connect(ui->Btn_VideoSavePath,  &QPushButton::clicked, this, &LSideWindow::btn_clicked);
}
