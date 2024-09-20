#ifndef MULWINDOW_H
#define MULWINDOW_H

#include <QWidget>
#include "YTable.h"
#include <QElapsedTimer>
#define MV_STYLE_MAXNUM     16
// F(id, row, col, label, image)
#define FOREACH_MV_STYLE(F) \
    F(MV_STYLE_1,  1, 1, " MV1",  ":style1")     \
    F(MV_STYLE_2,  1, 2, " MV2",  ":style2")     \
    F(MV_STYLE_4,  2, 2, " MV4",  ":style4")     \
    F(MV_STYLE_9,  3, 3, " MV9",  ":style9")     \
    F(MV_STYLE_16, 4, 4, " MV16", ":style16")    \

enum MV_STYLE {
#define ENUM_MV_STYLE(id, row, col, label, image) id,
        FOREACH_MV_STYLE(ENUM_MV_STYLE)
};

QT_FORWARD_DECLARE_CLASS(VideoWidget)
QT_FORWARD_DECLARE_CLASS(QLabel)
class MulWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MulWindow(QWidget *parent = nullptr);
    ~MulWindow();
    enum Action {
        STRETCH,
        EXCHANGE,
        MERGE,
    };
    VideoWidget* getPlayerByCh(int ch);
    VideoWidget* getPlayerByID(int playerid);
    VideoWidget* getPlayerByPos(QPoint pt);
    VideoWidget* getIdlePlayer();
signals:

public slots:
    void setLayout(int row, int col);
    void saveLayout();
    void restoreLayout();

    void mergeCells(int lt, int rb);
    void exchangeCells(VideoWidget* player1, VideoWidget* player2);
    void stretch(QWidget* wdg);

    void onSendMsg(QString &msg);
protected:
    void initUI();
    void initConnect();
    void updateUI();

    virtual void resizeEvent(QResizeEvent* e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
public:
    YTable table;
    YTable prev_table;
    QVector<QWidget*> views;
    QLabel *labRect;
    QLabel *labDrag;

    QPoint ptMousePress;
    QElapsedTimer tsMousePress;
    Action action;
    bool bStretch;
};

#endif // MULWINDOW_H
