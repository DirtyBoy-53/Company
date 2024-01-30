#include "recordconfig.h"
#include "ui_recordconfig.h"
#include <QFileDialog>
#include "memcache.h"


RecordConfig::RecordConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecordConfig)
{
    ui->setupUi(this);
}

RecordConfig::~RecordConfig()
{
    delete ui;
}

void RecordConfig::on_pushButton_clicked()
{
    QCheckBox * check[MAXREC] = {
        ui->checkBox,
        ui->checkBox_2,ui->checkBox_3,ui->checkBox_4,ui->checkBox_5,ui->checkBox_6,ui->checkBox_7,ui->checkBox_8
    };
    for(int i = 0; i < MAXREC; i ++) {
        if(check[i]->isChecked()) {
            m_recorder[i].start(i);
        }else {
            m_recorder[i].stop();
        }
    }
    hide();
}

void RecordConfig::on_btSelFile_clicked()
{
    QString sFilePath = QFileDialog::getExistingDirectory(this, "选择路径", ".");
    ui->lineEditSaveile->setText(sFilePath);
    MemCache::instance()->setValue("recordPath", sFilePath);
}
