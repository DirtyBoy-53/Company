#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include "qtheaders.h"
#include "qtstyles.h"
#include "singleton.hpp"
#include "canvaswidget.h"
#include "ydockwidget.h"
#include "filemanager.h"

QT_FORWARD_DECLARE_CLASS(CanvasView)
QT_FORWARD_DECLARE_CLASS(ImageAdj)

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

    void saveFile();
    
protected:
    void initUI();
    void initConnect();
    void initParams();
    void initDockWidget();

    void initMenu();
    void actionEnable(bool state);

    void closeEvent(QCloseEvent* event);
signals:
    void sigLabelAdded(QString label, QColor color, bool checked);
public slots:
    void about();
    void mv_fullscreen();
    void slotSetProperty(ShapePtr shape);
    void openFile();
private:
    QAction         *mFileAction{nullptr};
    QAction         *mEditAction{nullptr};
    QAction         *mViewAction{nullptr};

    //CanvasWidget *m_canvas{nullptr};
    CanvasView      *m_canvasView{nullptr};

    YDockWidget     m_dockWidget;

    QComboBox       *m_taskComboBox{nullptr};
    QComboBox       *m_drawComboBox{nullptr};
    QComboBox       *m_optComboBox{nullptr};

    QAction         *m_undoAction{nullptr};
    QAction         *m_redoAction{nullptr};

    FileManager     m_fileManager;

    ImageAdj*       m_imageAdj{nullptr}; // image adjustment widget
};

#endif // WINDOW_H
