#include "OpenMediaDlg.h"

#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>


#include "YDefine.h"
#include "confile.h"
FileTab::FileTab(int id, QWidget *parent) : QWidget(parent), m_id(id) {
    QVBoxLayout* vbox = new QVBoxLayout;

    vbox->addStretch();
    vbox->addWidget(new QLabel("文件:"));

    QHBoxLayout* hbox = new QHBoxLayout;
    edit = new QLineEdit;
    std::string key = QString("last_file_source_%1").arg(m_id).toStdString();
    std::string str = g_config->get<std::string>(key, "media", "");
    if (!str.empty()) {
        edit->setText(QString::fromUtf8(str.c_str()));
    }
    hbox->addWidget(edit);
    btnBrowse = new QPushButton("...");
    connect(btnBrowse, &QPushButton::clicked, this, [=]() {
        QString file = QFileDialog::getOpenFileName(this, tr("Open Meida File"), "",
                                                    "Video Files(*.bin *.raw *.y8 *.y16)\n"
                                                    "All Files(*)");
        if (!file.isEmpty()) {
            edit->setText(file);
        }
    });
    hbox->addWidget(btnBrowse);

    vbox->addLayout(hbox);
    vbox->addStretch();

    setLayout(vbox);
}

NetworkTab::NetworkTab(int id, QWidget *parent) : QWidget(parent), m_id(id) {
    QVBoxLayout* vbox = new QVBoxLayout;

    vbox->addStretch();
    vbox->addWidget(new QLabel(tr("URL:")));

    edit = new QLineEdit;
    std::string key = QString("last_network_source_%1").arg(m_id).toStdString();
    std::string str = g_config->get<std::string>(key, "media");
    if (!str.empty()) {
        edit->setText(QString::fromUtf8(str.c_str()));
    }

    vbox->addWidget(edit);
    vbox->addStretch();

    setLayout(vbox);
}

PcieCaptureTab::PcieCaptureTab(int id, QWidget *parent) : QWidget(parent), m_id(id) {

    m_confMap.insert("IR_640x512_3G_1ch","Gaode_FIR_Camera_1ch.json");
    m_confMap.insert("IR_640x512_3G_2ch","Gaode_FIR_Camera_2ch.json");
    m_confMap.insert("IR_384x288_3G_1ch","Gaode_FIR_Camera_lowPixel_1ch.json");
    m_confMap.insert("IR_384x288_3G_2ch","Gaode_FIR_Camera_lowPixel_2ch.json");
    m_confMap.insert("IR_XD01A_ivi_1ch","Gaode_ivi_1ch.json");
    m_confMap.insert("IR_640x512_6G_1ch","alg_XD01B_1ch.json");
    m_confMap.insert("IR_640x512_6G_2ch","alg_XD01B_2ch.json");
    m_confMap.insert("IR_XP05_1ch","Gaode_FIR_1280x1024_Camera_1CH.json");
    m_confMap.insert("IR_XP05_2ch","Gaode_FIR_1280x1024_Camera_2CH.json");
    m_confMap.insert("ALG_1920x1280_1ch","alg_ox03c_1_ch_slave.json");
    m_confMap.insert("ALG_1920x1280_2ch","alg_ox03c_2_ch.json");
    m_confMap.insert("ALG_3840x2160_1ch","alg_ox08b_1_ch.json");

    m_confBox    = new QComboBox();
    m_channelBox = new QComboBox();
    m_confBox->addItems(QStringList()   << "IR_640x512_3G_1ch" << "IR_640x512_3G_2ch"
                                        << "IR_384x288_3G_1ch" << "IR_384x288_3G_2ch"
                                        << "IR_XD01A_ivi_1ch"
                                        << "IR_640x512_6G_1ch" << "IR_640x512_6G_2ch"
                                        << "IR_XP05_1ch"       << "IR_XP05_2ch"
                                        << "ALG_1920x1280_1ch" << "ALG_1920x1280_2ch"
                                        << "ALG_3840x2160_1ch");

    for(auto i = 0;i < 8; ++i)
        m_channelBox->addItem(QString("%1 通道").arg(i+1));

    std::string key_conf = QString("last_pcie_source_%1").arg(m_id).toStdString();
    std::string str_conf = g_config->get<std::string>(key_conf, "media", "IR_640x512_3G_1ch");
    if (!str_conf.empty()) {
        m_confBox->setCurrentText(QString::fromUtf8(str_conf.c_str()));
    }

    std::string key_ch = QString("last_pcie_channel_%1").arg(m_id).toStdString();
    int ch = g_config->get<int>(key_ch, "media", 0);
    if (ch >= 0) {
        m_channelBox->setCurrentText(QString::fromUtf8(QString("%1 通道").arg(ch+1).toStdString().c_str()));
    }
    QString content = m_confBox->currentText();
    qDebug() << "ch:" << ch << "conf:" << QString::fromStdString(str_conf);
    if(m_confMap.contains(content)){
        m_chooseConf = m_confMap[content];
    }
    m_curConf = content;
    m_chooseCh = m_channelBox->currentIndex();

    // 布局
    QHBoxLayout* hbox_conf  = new QHBoxLayout;
    QHBoxLayout* hbox_ch    = new QHBoxLayout;
    QVBoxLayout* vbox       = new QVBoxLayout;

    vbox->addStretch();
    hbox_ch->addWidget(new QLabel("通道:"));
    hbox_ch->addSpacing(10);
    hbox_ch->addWidget(m_channelBox);

    hbox_conf->addWidget(new QLabel("配置:"));
    hbox_conf->addSpacing(10);
    hbox_conf->addWidget(m_confBox);

    vbox->addLayout(hbox_ch);
    vbox->addSpacing(20);
    vbox->addLayout(hbox_conf);
    vbox->addStretch();

    setLayout(vbox);
    initConnect();
}


void PcieCaptureTab::initConnect()
{
    connect(m_confBox, &QComboBox::currentTextChanged, this, [=](const QString &content){
        if(m_confMap.contains(content)){
            m_chooseConf = m_confMap[content];
        }
        m_curConf = content;
        qDebug() << "content:" << content << "m_chooseConf" << m_chooseConf;
    });

    connect(m_channelBox, QOverload<int>::of(&QComboBox::currentIndexChanged),this, [=](const int &index){
        m_chooseCh = index;
        qDebug() << "index:" << index << "m_chooseCh" << m_chooseCh;
    });

}


OpenMediaDlg::OpenMediaDlg(int id, QWidget *parent)
    : QDialog(parent), m_id(id)
{
    initUI();
    initConnect();
}

void OpenMediaDlg::initUI()
{
    setWindowTitle("打开媒体");
    setFixedSize(600, 300);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);


    tab = new QTabWidget;
    tab->addTab(new FileTab(m_id), QIcon(":fileTab"), "文件");
    tab->addTab(new NetworkTab(m_id), QIcon(":networkTab"), "网络");
    tab->addTab(new PcieCaptureTab(m_id), QIcon(":pcieCaptureTab"), "PCIE卡");

    std::string key = QString("last_tab_%1").arg(m_id).toStdString();
    tab->setCurrentIndex(g_config->get<int>(key, "media", DEFAULT_MEDIA_TYPE));

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(tab);
    vbox->addWidget(btns);

    setLayout(vbox);
}

void OpenMediaDlg::initConnect()
{

}

void OpenMediaDlg::accept()
{
switch (tab->currentIndex()) {
    case MEDIA_TYPE_FILE: {
        FileTab* filetab = qobject_cast<FileTab*>(tab->currentWidget());
        if (filetab) {
            media.type = MEDIA_TYPE_FILE;
            media.src = filetab->edit->text().toUtf8().data();
            media.id = filetab->m_id;
            // 保存最后打开的媒体源
            std::string key = QString("last_file_source_%1").arg(filetab->m_id).toStdString();
            g_config->set<std::string>(key, media.src.c_str(), "media");
            g_config->save();
        }
    } break;
    case MEDIA_TYPE_NETWORK: {
        NetworkTab* nettab = qobject_cast<NetworkTab*>(tab->currentWidget());
        if (nettab) {
            media.type      = MEDIA_TYPE_NETWORK;
            media.src       = nettab->edit->text().toUtf8().data();
            media.id        = nettab->m_id;
            media.channel   = nettab->m_channle;
            // 保存最后打开的媒体源
            std::string key = QString("last_network_source_%1").arg(nettab->m_id).toStdString();
            g_config->set<std::string>(key, media.src.c_str(), "media");
            g_config->save();
        }
    } break;
    case MEDIA_TYPE_PCIE: {
        PcieCaptureTab* captab = qobject_cast<PcieCaptureTab*>(tab->currentWidget());
        if (captab) {
            media.type      = MEDIA_TYPE_PCIE;
            media.src       = qPrintable(captab->m_chooseConf);
            media.id        = captab->m_id;
            media.channel   = captab->m_chooseCh;
            std::string key = QString("last_pcie_source_%1").arg(captab->m_id).toStdString();
            g_config->set<std::string>(key, captab->m_curConf.toStdString().c_str(), "media");
            std::string key2 = QString("last_pcie_channel_%1").arg(captab->m_id).toStdString();
            g_config->set<int>(key2, media.channel, "media");
            g_config->save();
            qDebug() << "choose media:" << media.toString();
        }
    } break;
    default: break;
    }

    if ( media.type == MEDIA_TYPE_NONE ||
        (media.src.empty() && media.id < 0)) {
        QMessageBox::information(this, tr("Info"), tr("Invalid media source!"));
        return;
    }

    std::string key = QString("last_tab_%1").arg(m_id).toStdString();
    g_config->set<int>(key, tab->currentIndex(), "media");
    g_config->save();

    QDialog::accept();
}
