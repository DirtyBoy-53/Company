#ifndef YFUNCTION_H
#define YFUNCTION_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QFile>
#include "ylog.h"
#include "YDefine.h"
#define DESKTOP_WIDTH   qApp->desktop()->availableGeometry().width()
#define DESKTOP_HEIGHT  qApp->desktop()->availableGeometry().height()

inline void setFont(int size) {
    QFont font = qApp->font();
    font.setPointSize(size);
    qApp->setFont(font);
}

inline void loadSkin(const char *skin) {
    QFile file(QString(":%1").arg(skin));
    if (file.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }else{
        YLog::Logger->warn("read {}.qss error.",skin);
    }
}

inline void setPalette(QColor clr) {
    qApp->setPalette(QPalette(clr));
}

inline bool confirmFile(const QString file) {
    QFileInfo info(file);
    if(!info.exists()){
        QDir dir(info.absolutePath());
        if(!dir.exists())
            if(!dir.mkpath(info.absolutePath())) return false;
        QFile qfile(file);
        if(!qfile.open(QIODevice::WriteOnly)) return false;
        qfile.close();
    }
    return true;
}

inline QPushButton* genPushButton(QPixmap pixmap, QString tooltip = QString(), QSize sz = QSize(0,0), QWidget* parent = NULL) {
    QPushButton* btn = new QPushButton(parent);
    btn->setFlat(true);
    if (sz.isEmpty()) {
        sz = pixmap.size();
    }
    btn->setFixedSize(sz);
    btn->setIconSize(sz);
    btn->setIcon(pixmap.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    btn->setToolTip(tooltip);
    return btn;
}

inline QLabel* genLabel(QPixmap pixmap, QSize sz = QSize(0,0), QWidget* parent = NULL) {
    QLabel* lbl = new QLabel(parent);
    if (sz.isEmpty()) {
        sz = pixmap.size();
    }
    lbl->setFixedSize(sz);
    lbl->setPixmap(pixmap.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    return lbl;
}

inline QHBoxLayout* genHBoxLayout() {
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setContentsMargins(10,1,10,1);
    hbox->setSpacing(1);
    return hbox;
}

inline QVBoxLayout* genVBoxLayout() {
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setContentsMargins(1,1,1,1);
    vbox->setSpacing(1);
    return vbox;
}

inline void setBgFg(QWidget* wdg, QColor bg, QColor fg = Qt::white) {
    wdg->setAutoFillBackground(true);
    QPalette pal = wdg->palette();
    pal.setColor(QPalette::Background, bg);
    pal.setColor(QPalette::Foreground, fg);
    wdg->setPalette(pal);
}

inline void connectButtons(QPushButton* btn1, QPushButton* btn2) {
    QObject::connect(btn1, SIGNAL(clicked(bool)), btn1, SLOT(hide()) );
    QObject::connect(btn1, SIGNAL(clicked(bool)), btn2, SLOT(show()) );

    QObject::connect(btn2, SIGNAL(clicked(bool)), btn2, SLOT(hide()) );
    QObject::connect(btn2, SIGNAL(clicked(bool)), btn1, SLOT(show()) );
}

inline void centerWidget(QWidget* wdg) {
    int w = wdg->width();
    int h = wdg->height();
    if (w < DESKTOP_WIDTH && h < DESKTOP_HEIGHT) {
        wdg->setGeometry((DESKTOP_WIDTH-w)/2, (DESKTOP_HEIGHT-h)/2, w, h);
    }
}

inline QRect adjustRect(QPoint pt1, QPoint pt2) {
    int x1 = qMin(pt1.x(), pt2.x());
    int x2 = qMax(pt1.x(), pt2.x());
    int y1 = qMin(pt1.y(), pt2.y());
    int y2 = qMax(pt1.y(), pt2.y());
    return QRect(QPoint(x1,y1), QPoint(x2,y2));
}

inline QRect adjustPos(QRect rc, QRect rcParent) {
    int x = qMax(rc.left(), rcParent.left());
    int y = qMax(rc.top(), rcParent.top());
    int w = qMin(rc.width(), rcParent.width());
    int h = qMin(rc.height(), rcParent.height());

    if (rc.right() >= rcParent.right()) {
        x = rcParent.right() - w;
    }
    if (rc.bottom() >= rcParent.bottom()) {
        y = rcParent.bottom() - h;
    }

    return QRect(x,y,w,h);
}

inline void toggle(QWidget* wdg) {
    wdg->setVisible(!wdg->isVisible());
}


inline void setBtnStyle(QPushButton *btn, QString normalColor, QString normalTextColor,
               QString hoverColor, QString hoverTextColor, QString pressedColor, QString pressedTextColor)
{
    QStringList qss;
    qss.append(QString("QPushButton{border-style:none;padding:10px;border-radius:5px;color:%1;background:%2;}").arg(normalTextColor).arg(normalColor));
    qss.append(QString("QPushButton:hover{color:%1;background:%2;}").arg(hoverTextColor).arg(hoverColor));
    qss.append(QString("QPushButton:pressed{color:%1;background:%2;}").arg(pressedTextColor).arg(pressedColor));
    btn->setStyleSheet(qss.join(""));
}

inline void yprint(QString msg,print_color_e color=PRINT_RED){
    qDebug().noquote() << QString("\033[%1m%2\033[0m").arg(color).arg(msg);
}

//byteArray 数据
//jump      多少个数据跳转行
inline QString byteArrayToHexStr(QByteArray byteArray, int jump=10)
{
    QString hexString = "";
    for (int i = 0; i < byteArray.size(); i++) {
        hexString += QString("%1").arg(static_cast<unsigned char>(byteArray[i]), 2, 16, QLatin1Char('0')).toUpper();
        if((i+1)%jump == 0){
            hexString.append("\n");
            continue;
        }
        if (i != byteArray.size() - 1) {
            hexString.append(" ");
        }
    }
    return hexString;
}

inline uint16_t byteArrayToUint16(const QByteArray &byteArray, bool &ok) {
    // 确保字节数组至少包含2个字节
    if (byteArray.size() < 2) {
        qWarning() << "Byte array too short to convert to uint16_t.";
        ok = false;
        return 0;
    }

    // 将字节数组转换为uint16_t
    uint16_t value = static_cast<uint16_t>((static_cast<unsigned char>(byteArray[0]) << 8) |
                                           static_cast<unsigned char>(byteArray[1]));
    ok = true;

    return value;
}


inline bool confirmFolder(const QString path){
    QDir dir(path);
    if(!dir.exists())
        if(!dir.mkpath(path)) return false;
    return true;
}

inline QString choosePath(){
    return QFileDialog::getExistingDirectory(nullptr, "选择路径", "./", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
}

inline QString chooseFile(QString ext="*.*"){
    return QFileDialog::getOpenFileName(nullptr, "选择路径", "./", ext);
}

inline QString exePath(){
    return QApplication::applicationDirPath();
}

#endif // YFUNCTION_H
