#ifndef UIRESULTLABEL_H
#define UIRESULTLABEL_H

#include <QLabel>
#include <QObject>

class XyResultLabel : public QLabel
{
public:
    XyResultLabel(QWidget* parent);

    void setFontSize(int size); //. 20 pt defalt...
    /// \brief setDefault
    /// \ui:background-color: light-gray, font: white 20 bold Yahei
    void xyClearLabel();
    /// \brief setDefault
    /// \ui:background-color: light-green, font: white 20 bold Yahei
    void xySetPassResult();
    /// \brief setDefault
    /// \ui:background-color: light-red, font: white 20 bold Yahei
    void xySetFailResult(QString errString);
    void xySetNormalStatus(QString log);

private:
    QRgb mRgb;
    int mFontSize;
};

#endif // UIRESULTLABEL_H
