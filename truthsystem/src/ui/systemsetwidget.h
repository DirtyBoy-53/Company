#ifndef SYSTEMSETWIDGET_H
#define SYSTEMSETWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include "configinfo.h"
#include <QTreeWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalMapper>
#include "initeractionbypcie.h"
#include "ArtosynNet.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SystemSetWidget; }
QT_END_NAMESPACE


class SystemSetWidget : public QWidget
{
    Q_OBJECT
private:
    enum CameraType{
        type_hw,
        type_light,
    };
public:
    SystemSetWidget(QWidget *parent = nullptr);
    ~SystemSetWidget();
    void Init();


    void TableInit();
    void TreeWidgetInit();
    //    QString (const char* str){return QString::fromUtf8(str);}

public slots:
    void slot_SaveSenceParam_btClicked();
    void slot_browsePath_btnClicked();
    void slot_setPath_btnClicked();
    void slot_ComBox_Change_ItemType(int idx);
    void slot_makeTag_btnClicked();
signals:
    void signal_sendSavePath(QString &path);
    void signal_lidar_init(QString ip);
    void signal_radar_init(int port);
protected:
    bool eventFilter(QObject *target, QEvent *event);
private:
    Ui::SystemSetWidget *ui;
    void readTableData(CSenceInfo &info);
    void TableAddData(QTableWidget *widget,QString name,QString content,QString brief,QComboBox* box=nullptr);

    void treeWidgetAddIterm_IIC_Camera(QTreeWidget *_treeWidget,QTreeWidgetItem* topItem,int &channel,CameraType type=type_hw);
    void treeWidgetAddIterm_Lidar(QTreeWidget *_treeWidget, QTreeWidgetItem *topItem, int &channel);
    void treeWidgetAddIterm_Radar(QTreeWidget *_treeWidget, QTreeWidgetItem *topItem, int &channel);
    void treeWidgetAddIterm_Net_Camera(QTreeWidget *_treeWidget, QTreeWidgetItem *topItem, int &channel);

    void btnConnectSlot(QTreeWidget *_treeWidget,QTreeWidgetItem* childItem,QString &&objName,QString btnName="配置");

    QTreeWidgetItem* m_topItem[MAX_ITEM_NUM];

    QComboBox* m_ComBox_item_Type[MAX_ITEM_NUM]{nullptr};
    QComboBox* m_ComBox_jsonfile[MAX_ITEM_NUM]{nullptr};
    QComboBox* m_ComBox_videoType[MAX_ITEM_NUM]{nullptr};
    QComboBox* m_ComBox_colorType[MAX_ITEM_NUM]{nullptr};
    QComboBox* m_ComBox_disType[MAX_ITEM_NUM]{nullptr};


    QCheckBox* m_CheckBox_algSwitch[MAX_ITEM_NUM]{nullptr};

    QLineEdit* m_Edit_LidarIP[MAX_ITEM_NUM]{nullptr};
    QLineEdit* m_Edit_CanPort[MAX_ITEM_NUM]{nullptr};
    QLineEdit* m_Edit_CameraIP[MAX_ITEM_NUM]{nullptr};

    QString m_FilePath{""};

    QMap<int,QString> m_weaterMap;
    QMap<int,QString> m_TimeQuantumMap;
    QMap<int,QString> m_ProjectMap[MAX_PANLE_NUM];
    QMap<int,QString> m_SenceMap;

    QSignalMapper  m_writeJson_Mapper;
    QSignalMapper  m_videoType_Mapper;
    QSignalMapper  m_colorSwitch_Mapper;
    QSignalMapper  m_algSwitch_Mapper;

    ArtosynNet* m_artosVideo[MAX_ITEM_NUM]{nullptr};

};
#endif // SYSTEMSETWIDGET_H
