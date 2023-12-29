#pragma once
#include <QMainWindow>
#include <QDockWidget>

#include "qtheaders.h"
#include "qtstyles.h"
#include "singleton.h"


#include "labelmanager.h"
#include "annotationcontainer.h"
#include "filemanager.h"
#include "segannotationitem.h"

class QScrollArea;
class CanvasBase;
class Canvas2D;
class CustomListWidget;

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    enum window_state_e {
        NORMAL = 0,
        MINIMIZED,
        MAXIMIZED,
        FULLSCREEN,
    } window_state;


    void _saveSegmentImageResults();
    void on_actionSave_triggered();
    bool _checkUnsaved();
    void unableFileActions();
    void enableFileActions();
    void adjustFitWindow();
    qreal scaleFitWindow() const;
    void _loadJsonFile(QString fileName);
    void getNewStrokes(const QList<SegStroke> &strokes);
    QString _labelRequest();
    QString getCurrentLabel() const;
    void newLabelRequest(QString newLabel);
    void removeLabelRequest(QString label);
protected:
    void initUI();
    void initConnect();

    void initMenu();

public slots:
    void about();
    void mv_fullscreen();

    void on_actionOpen_File_triggered();
    void provideLabelContextMenu(const QPoint& pos);
    void on_actionClose_triggered();
    void canvasUpdate();
private:
    QAction *mFileAction{nullptr};
    QAction *mEditAction{nullptr};
    QAction *mViewAction{nullptr};

    QVector<QToolBar*> mToolbars;


    QScrollArea *m_scrollArea{nullptr};
    CanvasBase  *m_curCanvas;
    Canvas2D    *m_2DCanvas;
//    Canvas3D    *m_3DCanvas;

    LabelManager m_labelManager;
    AnnotationContainer m_annoContainer;
    FileManager m_fileManager;

    QList<QAction*> m_fileRelatedActions;

    QDockWidget *m_annoDockWidget{nullptr};
    QDockWidget *m_labelDockWidget{nullptr};
    QDockWidget *m_fileDockWidget{nullptr};
    CustomListWidget *m_annoListWidget{nullptr};
    CustomListWidget *m_labelListWidget{nullptr};
    CustomListWidget *m_fileListWidget{nullptr};
};
