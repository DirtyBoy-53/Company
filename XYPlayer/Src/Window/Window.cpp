#include "Window.h"
#include <QMessageBox>
#include <QEvent>
#include <QMenuBar>
#include <QStatusBar>
#include <QDebug>
#include <QKeyEvent>

#include "CentralWidget.h"
#include "MulWindow.h"
#include "appdef.h"
#include "VideoWidget.h"
#include "VideoPlayer.h"
#include "QtEventLoopCheck.h"
#include "styles.h"
#include "confile.h"
#include "LSideWindow.h"

Window::Window(QWidget *parent)
    : QMainWindow(parent)
    , m_centerWidger(new CentralWidget)
{
    initUI();
    initConnect();
    initMenu();
    onPlayScreenLayout(2);

    EventLoopCheck * EventLoop = new EventLoopCheck(this);
}

Window::~Window(){
}

void Window::initUI()
{
    setWindowIcon(QIcon(":XYPlayer"));
    setCentralWidget(m_centerWidger);

    statusBar()->showMessage("No Message",5000);
}

void Window::initConnect()
{

}

void Window::initMenu()
{
    Qt::ToolBarArea _toolBar_location = static_cast<Qt::ToolBarArea>(g_config->get<int>("toolbar_location", "ui", TOOLBAR_LOCATION));

    // Media
    QMenu *mediaMenu = menuBar()->addMenu("媒体");
    QToolBar *mediaToolbar = addToolBar("媒体");
    addToolBar(_toolBar_location,mediaToolbar);//set toolbar location

    // View
    QMenu *viewMenu = menuBar()->addMenu("视图");
#if WITH_MV_STYLE
    QToolBar *viewToolbar = addToolBar("视图");
    addToolBar(_toolBar_location,viewToolbar);//set toolbar location
    toolbars.push_back(viewToolbar);

    QAction *actMVS;
    QSignalMapper *smMVS = new QSignalMapper(this);
#define VISUAL_MV_STYLE(id, row, col, label, image) \
    actMVS = new QAction(QIcon(image), tr(label), this);\
        actMVS->setToolTip(tr(label)); \
        smMVS->setMapping(actMVS, id); \
        connect( actMVS, SIGNAL(triggered(bool)), smMVS, SLOT(map()) ); \
        viewMenu->addAction(actMVS); \
        if (row * col <= 16){        \
            viewToolbar->addAction(actMVS); \
    }

    FOREACH_MV_STYLE(VISUAL_MV_STYLE)
#undef VISUAL_MV_STYLE

    connect( smMVS, SIGNAL(mapped(int)), this, SLOT(onPlayScreenLayout(int)) );
#endif

    actMvFullscreen = new QAction(tr(" MV Fullscreen F12"));
    actMvFullscreen->setCheckable(true);
    actMvFullscreen->setChecked(false);
    connect( actMvFullscreen, &QAction::triggered, this, &Window::mv_fullscreen );
    viewMenu->addAction(actMvFullscreen);
    viewMenu->addSeparator();

    actFullscreen = new QAction(tr(" Fullscreen F11"));
    actFullscreen->setCheckable(true);
    actFullscreen->setChecked(false);
    connect( actFullscreen, &QAction::triggered, this, &Window::fullScreen );
    viewMenu->addAction(actFullscreen);

    actMenubar = new QAction(tr(" Menubar F10"));
    actMenubar->setCheckable(true);
    bool menubar_visible = g_config->get<bool>("menubar_visible", "ui", true);
    actMenubar->setChecked(menubar_visible);
    menuBar()->setVisible(menubar_visible);
    connect(actMenubar, &QAction::triggered, [=](bool check) {
        menuBar()->setVisible(check);
        g_config->set<bool>("menubar_visible", check, "ui");
    });
    viewMenu->addAction(actMenubar);

    QAction *actToolbar = new QAction(tr(" Toolbar"));
    actToolbar->setCheckable(true);
    bool toolbar_visible = g_config->get<bool>("toolbar_visible", "ui", true);
    actToolbar->setChecked(toolbar_visible);
    foreach(auto toolbar, toolbars) {
        toolbar->setVisible(toolbar_visible);
    }
    connect(actToolbar, &QAction::triggered, [=](bool check) {
        foreach(auto toolbar, toolbars) {
            toolbar->setVisible(check);
        }
        g_config->set<bool>("toolbar_visible", check, "ui");
    });
    viewMenu->addAction(actToolbar);

    QAction *actStatusbar = new QAction(tr(" Statusbar"));
    actStatusbar->setCheckable(true);
    bool statusbar_visible = g_config->get<bool>("statusbar_visible", "ui", true);
    actStatusbar->setChecked(statusbar_visible);
    statusBar()->setVisible(statusbar_visible);
    connect(actStatusbar, &QAction::triggered, [=](bool check) {
        statusBar()->setVisible(check);
        g_config->set<bool>("statusbar_visible", check, "ui");
    });
    viewMenu->addAction(actStatusbar);

    QAction *actLside = new QAction(tr(" Leftside"));
    actLside->setCheckable(true);
    bool lside_visible = g_config->get<bool>("lside_visible", "ui", false);
    actLside->setChecked(lside_visible);
    m_centerWidger->m_lsideWind->setVisible(lside_visible);
    connect(actLside, &QAction::triggered, [=](bool check) {
        m_centerWidger->m_lsideWind->setVisible(check);
        g_config->set<bool>("lside_visible", check, "ui");
    });
    viewMenu->addAction(actLside);

    QAction *actRside = new QAction(tr(" Rightside"));
    actRside->setCheckable(true);
    bool rside_visible = g_config->get<bool>("rside_visible", "ui", false);
    actRside->setChecked(rside_visible);
    m_centerWidger->m_rsideWind->setVisible(rside_visible);

    connect(actRside, &QAction::triggered, [=](bool check) {
        m_centerWidger->m_rsideWind->setVisible(check);
        g_config->set<bool>("rside_visible", check, "ui");
    });
    viewMenu->addAction(actRside);

    // Help
    QMenu *helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction(" 帮助", this, SLOT(about()));
}

void Window::fullScreen()
{
    static QRect _rc_old;
    if(isFullScreen()){
        menuBar()->setVisible(true);
        showNormal();
        setGeometry(_rc_old);
    }
    else{
        _rc_old = geometry();
        menuBar()->setVisible(false);
        showFullScreen();
    }
    actFullscreen->setChecked(isFullScreen());
    actMenubar->setCheckable(menuBar()->isVisible());
}

void Window::mv_fullscreen()
{
    MulWindow* mv = m_centerWidger->m_mulWind;;
    bool is_mv_fullscreen = false;
    if (mv->windowType() & Qt::Window) {
        mv->setWindowFlags(Qt::SubWindow);
        this->show();
    }
    else {
        mv->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        this->hide();
        mv->showFullScreen();
        mv->raise();
        is_mv_fullscreen = true;
    }
    this->grabKeyboard();
    actMvFullscreen->setChecked(is_mv_fullscreen);
}

void Window::onPlayScreenLayout(int num)
{
    int r,c;
    switch (num) {
#define CASE_MV_STYLE(id, row, col, lable, image) \
    case id: \
            r = row; c = col; \
            break;

        FOREACH_MV_STYLE(CASE_MV_STYLE)
#undef  CASE_MV_STYLE
    default:
        r = 1; c = 1;
        break;
    }
    m_centerWidger->m_mulWind->setLayout(r,c);
}

#include "AlgSdkManager.h"
void Window::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent will exit.";
    // AlgSdkManager::instance()->close();
    // QThread::msleep(1000);
    QMainWindow::closeEvent(event);
}

CentralWidget *Window::centerWidger() const
{
    return m_centerWidger;
}

void Window::sendFrame(XYZJFramePtr frame, const int channelId)
{
    VideoWidget *widget = m_centerWidger->m_mulWind->getPlayerByCh(channelId);

    if(widget && widget->getPImpl_player()){
        widget->getPImpl_player()->addData(frame);
    }
}



void Window::keyPressEvent(QKeyEvent *e)
{
       if (m_centerWidger->m_mulWind->windowType() & Qt::Window) {
       if (e->key() == Qt::Key_F12 || e->key() == Qt::Key_Escape) {
           mv_fullscreen();
       }
   }
   else {
       switch(e->key()) {
       case Qt::Key_F10:
           // toggle(menuBar());
           actMenubar->setChecked(menuBar()->isVisible());
           return;
       case Qt::Key_F11:
       case Qt::Key_Escape:
           fullScreen();
           return;
       case Qt::Key_F12:
           mv_fullscreen();
           return;
       default:
           return QMainWindow::keyPressEvent(e);
       }
   }
}

void Window::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        update();
    }
    else if (event->type() == QEvent::WindowStateChange) {
        if (isFullScreen()) {
            window_state = FULLSCREEN;
        }
        else if (isMaximized()) {
            window_state = MAXIMIZED;
        }
        else if (isMinimized()) {
            window_state = MINIMIZED;
        }
        else {
            window_state = NORMAL;
        }
        qInfo("window_state=%d", (int)window_state);
    }
}

void Window::about()
{
    QString strAbout = APP_NAME " " APP_VERSION "\n\n";

    strAbout += "Build on ";
    strAbout += QString::asprintf("%s %s\n\n", __DATE__, __TIME__);

    strAbout += "Qt version: ";
    strAbout += qVersion();
    strAbout += "\n\n";

    strAbout += "Copyright 2018-2028 " COMPANY_NAME " Company.\n";
    strAbout += "All rights reserved.\n";

    QMessageBox::information(this, tr("About Application"), strAbout);
}

