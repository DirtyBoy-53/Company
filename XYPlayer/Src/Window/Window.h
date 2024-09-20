#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include "YStruct.h"


QT_FORWARD_DECLARE_CLASS(CentralWidget)

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);
    ~Window();
    enum window_state_e {
        NORMAL = 0,
        MINIMIZED,
        MAXIMIZED,
        FULLSCREEN,
    } window_state;

    CentralWidget *centerWidger() const;
    void sendFrame(XYZJFramePtr frame, const int channelId);

protected:
    void initUI();
    void initConnect();
    void initMenu();

    // QWidget interface
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void changeEvent(QEvent *event) override;

public slots:
    void about();
    void fullScreen();
    void mv_fullscreen();
    void onPlayScreenLayout(int num);

private:
    QAction*            actMenubar;
    QAction*            actFullscreen;
    QAction*            actMvFullscreen;

    QVector<QToolBar*>  toolbars;

    CentralWidget*      m_centerWidger{nullptr};


    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;
};
#endif // WINDOW_H
