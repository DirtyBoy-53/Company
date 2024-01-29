#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include "qtheaders.h"
#include "qtstyles.h"
#include "singleton.h"
#include "canvaswidget.h"
#include "customlistwidget.h"
class Window : public QMainWindow
{
    Q_OBJECT
public:
    explicit Window(QWidget* parent = nullptr);
    ~Window();
    enum window_state_e {
        NORMAL = 0,
        MINIMIZED,
        MAXIMIZED,
        FULLSCREEN,
    } window_state;
protected:
    void initUI();
    void initConnect();
    void initDockWidget();

    void initMenu();

public slots:
    void about();
    void mv_fullscreen();

private:
    QAction *mFileAction{nullptr};
    QAction *mEditAction{nullptr};
    QAction *mViewAction{nullptr};

    CanvasWidget *m_canvas{nullptr};

    QDockWidget *m_annoDockWidget{nullptr};
    QDockWidget *m_labelDockWidget{nullptr};
    QDockWidget *m_fileDockWidget{nullptr};
    CustomListWidget *m_annoListWidget{nullptr};
    CustomListWidget *m_labelListWidget{nullptr};
    CustomListWidget *m_fileListWidget{nullptr};

    QComboBox *m_taskComboBox{nullptr};
    QComboBox *m_drawComboBox{nullptr};
    QComboBox *m_optComboBox{nullptr};
};

#endif // WINDOW_H
