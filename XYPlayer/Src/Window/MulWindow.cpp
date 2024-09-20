#include "MulWindow.h"
#include "VideoWidget.h"
#include "styles.h"
#include "confile.h"
#include "YFunction.h"

#include <QLabel>
#include <QMouseEvent>
#include <QDateTime>
MulWindow::MulWindow(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnect();
    bStretch = false;
}

MulWindow::~MulWindow()
{

}

VideoWidget *MulWindow::getPlayerByID(int playerid)
{
    for (int i = 0; i < views.size(); ++i) {
        VideoWidget *player = (VideoWidget*)views[i];
        if (player->playerid == playerid) {
            return player;
        }
    }
    return NULL;
}

VideoWidget *MulWindow::getPlayerByCh(int ch)
{
    for (int i = 0; i < views.size(); ++i) {
        VideoWidget *player = (VideoWidget*)views[i];
        if(player){
            if (player->getMedia().channel == ch) {
                // qDebug() << "Media:" << player->getMedia().toString();
                return player;
            }
        }else{
            qWarning() << "player is null.";
        }

    }
    return nullptr;
}

VideoWidget *MulWindow::getPlayerByPos(QPoint pt)
{
    for (int i = 0; i < views.size(); ++i) {
        QWidget* wdg = views[i];
        if (wdg->isVisible() && wdg->geometry().contains(pt)) {
            return (VideoWidget*)wdg;
        }
    }
    return NULL;
}

VideoWidget *MulWindow::getIdlePlayer()
{
    for (int i = 0; i < views.size(); ++i) {
        VideoWidget *player = (VideoWidget*)views[i];
        if (player->isVisible() && player->status == VideoWidget::STOP) {
            return player;
        }
    }
    return NULL;
}

void MulWindow::setLayout(int row, int col)
{
    saveLayout();
    table.init(row,col);
    updateUI();
    g_config->set<int>("mv_row", row, "ui");
    g_config->set<int>("mv_col", col, "ui");
}

void MulWindow::saveLayout()
{
    prev_table = table;
}

void MulWindow::restoreLayout()
{
    YTable tmp = table;
    table = prev_table;
    prev_table = tmp;
    updateUI();
}

void MulWindow::mergeCells(int lt, int rb)
{
    return;
#if 1
    // find first non-stop player as lt
    for (int i = lt; i <= rb; ++i) {
        VideoWidget* player = getPlayerByID(i);
        if (player && player->status != VideoWidget::STOP) {
            exchangeCells(player, getPlayerByID(lt));
            break;
        }
    }
#endif

    saveLayout();
    table.merge(lt,rb);
    updateUI();
}

void MulWindow::exchangeCells(VideoWidget *player1, VideoWidget *player2)
{
    return;
    QRect rcTmp = player1->geometry();
    int idTmp = player1->playerid;
    Media mediaTmp = player1->media;
    player1->setGeometry(player2->geometry());
    player1->playerid = player2->playerid;
    player1->media = player2->media;

    player2->setGeometry(rcTmp);
    player2->playerid = idTmp;
    player2->media = mediaTmp;
}

void MulWindow::stretch(QWidget *wdg)
{
    if (table.row == 1 && table.col == 1) return;
    if (bStretch) {
        restoreLayout();
        bStretch = false;
    }
    else {
        saveLayout();
        for (int i = 0; i < views.size(); ++i) {
            views[i]->hide();
        }
        wdg->setGeometry(rect());
        wdg->show();
        bStretch = true;
    }
}

void MulWindow::initUI()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    for (int i = 0; i < MV_STYLE_MAXNUM; ++i) {
        VideoWidget* player = new VideoWidget(this);
        player->playerid = i+1;
        views.push_back(player);
        connect(player, &VideoWidget::sigSendMsg, this, &MulWindow::onSendMsg);
    }

    int row = g_config->get<int>("mv_row", "ui", MV_STYLE_ROW);
    int col = g_config->get<int>("mv_col", "ui", MV_STYLE_COL);
    setLayout(row, col);

    labDrag = new QLabel(this);
    labDrag->setFixedSize(DRAG_WIDTH, DRAG_HEIGHT);
    labDrag->hide();
    labDrag->setStyleSheet(DRAG_QSS);

    labRect = new QLabel(this);
    labRect->hide();
    labRect->setStyleSheet(RECT_QSS);
}
void MulWindow::onSendMsg(QString &msg)
{
    qDebug() << msg;
}


void MulWindow::initConnect()
{

}

#define SEPARATOR_LINE_WIDTH 3  //video Widget之间的间距
void MulWindow::updateUI()
{
    int row = table.row;//窗口布局总行数
    int col = table.col;//窗口布局总列数
    if (row == 0 || col == 0) return;
    int cell_w = width()/col;//获取单个video widget宽度
    int cell_h = height()/row;//获取单个video widget高度
    auto w = width();
    auto h = height();
    int margin_x = (width() - cell_w * col) / 2;
    int margin_y = (height() - cell_h * row) / 2;
    int x = margin_x;//没有办法完全平分的时候 留一些 页边空白
    int y = margin_y;
    for (int i = 0; i < views.size(); ++i) {
        views[i]->hide();
    }

    int cnt = 0;
    YTableCell cell;
    for (int r = 0; r < row; ++r) {
        for (int c = 0; c < col; ++c) {
            int id = r*col + c + 1;
            if (table.getTableCell(id, cell)) {
                QWidget *wdg = getPlayerByID(id);
                if (wdg) {
                    wdg->setGeometry(x, y, cell_w*cell.colspan() - SEPARATOR_LINE_WIDTH, cell_h*cell.rowspan() - SEPARATOR_LINE_WIDTH);
                    wdg->show();
                    ++cnt;
                }
            }
            x += cell_w;//调整x起始点
        }
        x = margin_x;//multiview widget窗口左上角（考虑页边空白）
        y += cell_h;//调整y起始点
    }
    bStretch = (cnt == 1);
}

void MulWindow::resizeEvent(QResizeEvent *e)
{
    updateUI();
}

void MulWindow::mousePressEvent(QMouseEvent *e)
{
    ptMousePress = e->pos();
    tsMousePress.start();
}

void MulWindow::mouseReleaseEvent(QMouseEvent *e)
{
    if (action == EXCHANGE) {
        VideoWidget* player1 = getPlayerByPos(ptMousePress);
        VideoWidget* player2 = getPlayerByPos(e->pos());
        if (player1 && player2 && player1 != player2) {
            exchangeCells(player1, player2);
        }
    }
    else if (action == MERGE) {
        QRect rc = adjustRect(ptMousePress, e->pos());
        VideoWidget* player1 = getPlayerByPos(rc.topLeft());
        VideoWidget* player2 = getPlayerByPos(rc.bottomRight());
        if (player1 && player2 && player1 != player2) {
            mergeCells(player1->playerid, player2->playerid);
        }
    }

    labRect->setVisible(false);
    labDrag->setVisible(false);
    setCursor(Qt::ArrowCursor);
}

void MulWindow::mouseMoveEvent(QMouseEvent *e)
{
    VideoWidget *player = getPlayerByPos(ptMousePress);
    if (player == NULL) {
        return;
    }

    if (e->buttons() == Qt::LeftButton) {
        if (!labDrag->isVisible()) {
           if (tsMousePress.elapsed() < 300) return;
            action = EXCHANGE;
            setCursor(Qt::OpenHandCursor);
            labDrag->setPixmap(player->grab().scaled(labDrag->size()));
            labDrag->setVisible(true);
        }
        if (labDrag->isVisible()) {
            labDrag->move(e->pos()-QPoint(labDrag->width()/2, labDrag->height()));
        }
    }
    else if (e->buttons() == Qt::RightButton) {
        if (!labRect->isVisible()) {
            action = MERGE;
            setCursor(Qt::CrossCursor);
            labRect->setVisible(true);
        }
        if (labRect->isVisible()) {
            labRect->setGeometry(adjustRect(ptMousePress, e->pos()));
        }
    }
}

void MulWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    VideoWidget* player = getPlayerByPos(e->pos());
    if (player) {
        stretch(player);
    }
}


