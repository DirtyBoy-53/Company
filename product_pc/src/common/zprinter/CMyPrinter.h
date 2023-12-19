#ifndef CMYPRINTER_H
#define CMYPRINTER_H

#include <QObject>
#include <Ole2.h>
#include <comdll.h>


class Q_API_FUNCTION CMyPrinter : public QObject
{
    Q_OBJECT
public:
    explicit CMyPrinter(QObject *parent = 0);
    ~CMyPrinter();

    /// \brief errString
    /// \return 错误信息
    QString errString();

    /// \brief printerName
    /// \return 返回当前系统打印机列表
	QStringList printerName();

    QString defalutPrinterName();

    /// \brief items 获取具名数据源列表，前提是需要先加载模板
    /// \return 返回具名数据源列表，返回形式为：key:value的QString类型
    QStringList items();

    /// \brief load
    /// \param p 打印机名称
    /// \param t 打印模板，需要传入绝对路径
    /// \param c 需要打印的数量
    /// \param s 需要序列化打印的数量
    /// \return true/false
    bool load(QString p, QString t, int c, int s);

    /// \brief setItem 为具名数据源设置数据，设置完成后会进行回读比对
    /// \param key 具名数据源名称
    /// \param value 需设置的数据
    /// \return true/fase
    bool setItem(QString key, QString value);

    /// \brief print 进行打印
    /// \return 局限于调用SDK问题，打印会返回false，当前不用对结果进行判断
    bool print();

private:
    void* mPrintApplication;
    void* mPrintFormat;
    QString mErrString;
signals:

public slots:
};

#endif // CMYPRINTER_H
