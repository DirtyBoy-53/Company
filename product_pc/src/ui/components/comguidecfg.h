#ifndef COMGUIDECFG_H
#define COMGUIDECFG_H

#include <QWidget>
#include <QDialog>

struct GuideCfgInfo
{
    QString tips;
    QString msgTips;
    QString path;
};

namespace Ui {
class ComGuideCfg;
}


class ComGuideCfg : public QDialog
{
    Q_OBJECT

public:
    explicit ComGuideCfg(QWidget *parent = nullptr);
    ~ComGuideCfg();
    void initWidget();
    static void getGuideCfgList(QList<GuideCfgInfo>& list);
    void changeProduce(const QString& produce);

private slots:
    void on_btnDelete_clicked();

    void on_btnAdd_clicked();

    void on_btnSave_clicked();

    void on_pushButton_clicked();

private:
    Ui::ComGuideCfg *ui;
    QString m_configPath;
    static QList<GuideCfgInfo> m_list;
    QString m_produceName;
    QString m_imgPath;
};

#endif // COMGUIDECFG_H
