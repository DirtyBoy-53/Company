#include "XyResultLabel.h"

XyResultLabel::XyResultLabel(QWidget *parent) :
    QLabel(parent)
{
    mFontSize = 35;
    xySetNormalStatus("");
}

void XyResultLabel::setFontSize(int size)
{
    mFontSize = size;
}

void XyResultLabel::xyClearLabel()
{
    QString strStyle = "background-color:rgba(200, 200, 200, 0);";
    strStyle.append("color: rgb(255, 255, 255);");
    strStyle.append("font-family: Microsoft YaHei UI;");
    strStyle.append(QString("font-size: %1pt;").arg(mFontSize));
    strStyle.append("font-weight: Bold;");
    strStyle.append("text-align: center;");
    strStyle.append("qproperty-alignment: AlignCenter;");
    setStyleSheet(strStyle);
    setText("");
}

void XyResultLabel::xySetPassResult()
{
    QString strStyle = "background-color:rgb(0, 200, 0);";
    strStyle.append("color: rgb(255, 255, 255);");
    strStyle.append("font-family: Microsoft YaHei UI;");
    strStyle.append(QString("font-size: %1pt;").arg(mFontSize));
    strStyle.append("font-weight: Bold;");
    // strStyle.append("text-align: center;");
    strStyle.append("qproperty-alignment: AlignCenter;");
    setStyleSheet(strStyle);
    setText("PASS");
}

void XyResultLabel::xySetFailResult(QString errString)
{
    int fontSize = mFontSize;
    if (errString.size() > 40) {
        fontSize = 15;
    }
    QString strStyle = "background-color:rgb(240, 0, 0);";
    strStyle.append("color: rgb(255, 255, 255);");
    strStyle.append("font-family: Microsoft YaHei UI;");
    strStyle.append(QString("font-size: %1pt;").arg(fontSize));
    strStyle.append("font-weight: Bold;");
    strStyle.append("text-align: center;");
    strStyle.append("qproperty-alignment: AlignCenter;");
    setStyleSheet(strStyle);
    QString text = "FAIL";
    if (!errString.isEmpty()) {
        text += " : " + errString;
    }
    setText(text);
}

void XyResultLabel::xySetNormalStatus(QString log)
{
    int fontSize = mFontSize;
    if (log.size() > 40) {
        fontSize = 15;
    }
    QString strStyle = "background-color:rgb(200, 200, 250);";
    strStyle.append("color: rgb(25, 25, 180);");
    strStyle.append("font-family: Microsoft YaHei UI;");
    strStyle.append(QString("font-size: %1pt;").arg(fontSize));
    strStyle.append("font-weight: Bold;");
    strStyle.append("text-align: center;");
    strStyle.append("qproperty-alignment: AlignCenter;");
    setStyleSheet(strStyle);
    setText(log);
}
