#include "window.h"
#include "appdef.h"
#include "confile.h"
#include "qtstyles.h"
#include "shape.h"
#include "labeldialog.h"
#include "ycommon.h"
#include "filemanager.h"
#include "CanvasView.h"


Window::Window(QWidget *parent)
    : QMainWindow(parent)
    , m_canvasView(new CanvasView(this))
{
    initUI();
    initConnect();
}

Window::~Window()
{}

void Window::initUI()
{
    setWindowIcon(QIcon(":ICON"));

    setCentralWidget(m_canvasView);

    initDockWidget();
    initMenu();

    statusBar()->showMessage(tr("No Message!"));
}

void Window::initConnect()
{
    connect(m_canvasView->canvas(), &Canvas2D::sigSetProperty, this, &Window::slotSetProperty);


    // label changed -> ui list changed
    connect(this, &Window::sigLabelAdded,
            &m_dockWidget.labelListWidget(), &CustomListWidget::addCustomItem);

    connect(&m_dockWidget.fileListWidget(), &QListWidget::clicked, this, [=](QModelIndex index){
        if(!m_fileManager.hasChangeNotSaved()){
            m_fileManager.selectFile(index.row());
            QString path = m_fileManager.getCurrentImageFile();
            m_canvasView->loadImage(path);
        }else{
            //提示保存
        }


//        m_canvasView->loadPixmap();
    });
}

void Window::initDockWidget()
{
    addDockWidget(Qt::RightDockWidgetArea,&m_dockWidget.annoDockWidget());
    addDockWidget(Qt::RightDockWidgetArea,&m_dockWidget.labelDockWidget());
    addDockWidget(Qt::RightDockWidgetArea,&m_dockWidget.fileDockWidget());
}
void Window::saveFile()
{
    auto list = m_canvasView->canvas()->getShapeList();
    if(list.size() <= 0) return;
    shape_json::root_s root;

    QVector<shape_json::shape_s> shapelist;
    for(ShapePtr shape : list){
        shape_json::shape_s s;
        s.description = "des";
        s.flags = "1";
        s.group_id = 1;
        s.label = shape->label()->m_label.toStdString();
        s.mask = 1;
        s.points = shape->points().getImgPoints();
        s.shape_type = Shape::drawModeToStr(shape->type());
        shapelist.append(s);
    }

    root.version = APP_VERSION;
    root.imageData = "1";
    root.imageHeight = "2";
    root.imagePath = "3";
    root.imageWidth = "4";
    root.flags = "5";
    for(auto &p : shapelist)
        root.shapes.push_back(p);

    QString filename = QApplication::applicationDirPath() + "/test.json";
    FileManager::saveJson(root, filename);
}
void Window::initMenu()
{
    Qt::ToolBarArea toolBar_location = static_cast<Qt::ToolBarArea>(g_config->Get<int>("toolbar_location", "ui", TOOLBAR_LOCATION));

    m_undoAction = m_canvasView->undoGroup()->createUndoAction(this);
    m_redoAction = m_canvasView->undoGroup()->createRedoAction(this);

    //File
    QMenu *fileMenu = menuBar()->addMenu(tr("&文件"));
    QToolBar *fileToolbar = addToolBar(tr("&文件"));
    fileToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBar(toolBar_location,fileToolbar);//set toolbar location

    //File->OpenFile
    QAction* actOpenFile = new QAction(QIcon(":open_file"),"打开文件  ");
    actOpenFile->setShortcut(QKeySequence("Ctrl+O"));

    connect(actOpenFile, &QAction::triggered, this, [=](){
        //m_canvasView->loadPixmap();
    });
    fileMenu->addAction(actOpenFile);
    fileToolbar->addAction(actOpenFile);

    //File->OpenDir
    QAction* actOpenDir = new QAction(QIcon(":open_file"), "打开文件夹 ");
    actOpenDir->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(actOpenDir, &QAction::triggered, this, [=](){
       QString path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                          "/home",
                                          QFileDialog::ShowDirsOnly
                                          | QFileDialog::DontResolveSymlinks);
        m_fileManager.setMultiImage(path);
        m_dockWidget.fileListWidget().addItems(m_fileManager.allImageFiles());
    });
    fileMenu->addAction(actOpenDir);
    fileToolbar->addAction(actOpenDir);

    //File->SaveFile
    QAction* actSaveFile = new QAction(QIcon(":save_file"),"保存文件  ");
    actSaveFile->setShortcut(QKeySequence("Ctrl+S"));
    connect(actSaveFile,&QAction::triggered,this,[=](){
        saveFile();
        //        on_actionSave_File_triggered();
    });
    fileMenu->addAction(actSaveFile);
    fileToolbar->addAction(actSaveFile);


    //Edit
    QMenu *editMenu = menuBar()->addMenu(tr("&编辑"));
    QToolBar *editToolbar = addToolBar(tr("&编辑"));
    editToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBar(toolBar_location,editToolbar);//set toolbar location

    m_taskComboBox = new QComboBox(editToolbar);
    m_taskComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_taskComboBox->insertItem(CanvasBase::detection,"检测");
    m_taskComboBox->insertItem(CanvasBase::segmentation,"分割");
    m_taskComboBox->setCurrentIndex(CanvasBase::segmentation);
    editToolbar->addWidget(m_taskComboBox);
    m_canvasView->canvas()->changeTask(CanvasBase::segmentation);
    connect(m_taskComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int idx){
        m_canvasView->canvas()->changeTask(static_cast<CanvasBase::task_mode_e>(idx));
    });

    m_drawComboBox = new QComboBox(editToolbar);
    m_drawComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_drawComboBox->insertItem(YShape::Rectangle,"矩形");
    m_drawComboBox->insertItem(YShape::Line,"直线");
    m_drawComboBox->insertItem(YShape::Curve,"曲线");
    m_drawComboBox->insertItem(YShape::Polygon,"多边形");
    m_drawComboBox->setCurrentIndex(YShape::Polygon);
    editToolbar->addWidget(m_drawComboBox);
    m_canvasView->canvas()->changeDrawMode(YShape::Polygon);
    connect(m_drawComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int idx){
        m_canvasView->canvas()->changeDrawMode(static_cast<YShape::draw_mode_e>(idx));
    });

    m_optComboBox = new QComboBox(editToolbar);
    m_optComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_optComboBox->insertItem(CanvasBase::draw,"绘制");
    m_optComboBox->insertItem(CanvasBase::edit,"编辑");
    m_optComboBox->setCurrentIndex(CanvasBase::draw);
    editToolbar->addWidget(m_optComboBox);
    m_canvasView->canvas()->changeOperatMode(CanvasBase::draw);
    connect(m_optComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int idx){
        CanvasBase::operat_mode_e mode = static_cast<CanvasBase::operat_mode_e>(idx);
        m_canvasView->canvas()->changeOperatMode(mode);
        if(mode == CanvasBase::edit){
//            actionEnable(true);
        }else if(mode == CanvasBase::draw){
//            actionEnable(false);
        }
    });

    QAction* actCreatPolygon = new QAction(QIcon(":creat_polygon"),tr("创建多边形  "));
    actCreatPolygon->setShortcut(QKeySequence("Ctrl+N"));

    connect(actCreatPolygon, &QAction::triggered, this, [=](){
        qInfo() << "Creat polygon";
        m_canvasView->addShape();
    });
    editMenu->addAction(actCreatPolygon);
    editToolbar->addAction(actCreatPolygon);

    QAction* actOpt = new QAction(QIcon(":check"),tr("绘制  "));

    actOpt->setCheckable(true);
    connect(actOpt, &QAction::triggered, this, [=](){
        static bool flag{true};
        static QString str = actOpt->text();
        flag = !flag;
        actOpt->setChecked(flag);
        actOpt->setIcon(flag == true ? QIcon(":check") : QIcon(":uncheck"));
        if(!flag){
            //            m_curCanvas->changeCanvasMode(SELECT);
            actOpt->setText("编辑  ");
        }else{
            //            m_curCanvas->changeCanvasMode(DRAW);
            actOpt->setText(str);
        }
    });
    editMenu->addAction(actOpt);
    editToolbar->addAction(actOpt);


    m_undoAction->setIcon(QIcon(":undo"));
    m_redoAction->setIcon(QIcon(":redo"));
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editToolbar->addAction(m_undoAction);
    editToolbar->addAction(m_redoAction);
//    actionEnable(false);

    //View
    QMenu *viewMenu = menuBar()->addMenu(tr("&视图"));
    QToolBar *viewToolbar = addToolBar(tr("&Edit"));
    viewToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBar(toolBar_location,viewToolbar);//set toolbar location

    QAction* zoom_in_Action = new QAction(QIcon(":zoom_in"),("&放大"));
    QAction* zoom_out_Action = new QAction(QIcon(":zoom_out"),("&缩小"));
    QAction* zoom_fit_Action = new QAction(QIcon(":zoom_fit"),("&自适应"));
    zoom_in_Action->setShortcut(QKeySequence("Ctrl+="));
    zoom_out_Action->setShortcut(QKeySequence("Ctrl+-"));
    viewMenu->addAction(zoom_in_Action);
    viewMenu->addAction(zoom_out_Action);
    viewMenu->addAction(zoom_fit_Action);
    viewToolbar->addAction(zoom_in_Action);
    viewToolbar->addAction(zoom_out_Action);
    viewToolbar->addAction(zoom_fit_Action);
    connect(zoom_in_Action, &QAction::triggered, this, [=](){m_canvasView->canvas()->zoomIn();});
    connect(zoom_out_Action, &QAction::triggered, this, [=](){m_canvasView->canvas()->zoomOut();});
    connect(zoom_fit_Action, &QAction::triggered, this, [=](){m_canvasView->adjustFitWindow();});



    QAction* actMark = new QAction(QIcon(":check"),tr("Mark  "));

    actMark->setCheckable(true);
    connect(actMark, &QAction::triggered, this, [=](){
        static bool flag{true};
        flag = !flag;
        actMark->setChecked(flag);
        actMark->setIcon(flag == true ? QIcon(":check") : QIcon(":uncheck"));
        qInfo() << "Mark:" << flag;
    });
    viewMenu->addAction(actMark);
    viewToolbar->addAction(actMark);

    // Help
    QMenu *helpMenu = menuBar()->addMenu(tr("&帮助"));
    helpMenu->addAction(QIcon(":help"),tr(" &关于"), this, SLOT(about()));
}

void Window::actionEnable(bool state)
{
//    m_undoAction->setEnabled(state);
//    m_redoAction->setEnabled(state);
    if(!m_undoAction || !m_redoAction) return;
    m_undoAction->setVisible(state);
    m_redoAction->setVisible(state);
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

void Window::mv_fullscreen()
{

}

void Window::slotSetProperty(ShapePtr shape)
{
    if(!shape) return;
    LabelDialog dialog(m_canvasView->canvas()->getShapeList(),this);
    if(dialog.exec() == QDialog::Accepted){
        QString name = dialog.getLabel();
        if(name.isEmpty()) return;
        LabelProperty label(name, ColorUtils::randomColor(), true, shape->id());
        shape->setLabel(label);
        emit sigLabelAdded(label.m_label, label.m_color, label.m_visible);
    }else{}
}
