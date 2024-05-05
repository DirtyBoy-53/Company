#include "window.h"
#include "appdef.h"
#include "confile.h"
#include "qtstyles.h"
#include "shape.h"
Window::Window(QWidget *parent)
    : QMainWindow(parent)
    , m_canvas(new CanvasWidget(this))
{
    initUI();
    initConnect();
}

Window::~Window()
{}

void Window::initUI()
{
    setWindowIcon(QIcon(":ICON"));

    setCentralWidget(m_canvas);

    initDockWidget();
    initMenu();

    statusBar()->showMessage(tr("No Message!"));
}

void Window::initConnect()
{

}

void Window::initDockWidget()
{
    m_annoDockWidget    = new QDockWidget(tr("Annotation List"));
    m_labelDockWidget   = new QDockWidget(tr("Lable List"));
    m_fileDockWidget    = new QDockWidget(tr("File List"));

    m_annoListWidget    = new CustomListWidget(m_annoDockWidget);
    m_labelListWidget   = new CustomListWidget(m_labelDockWidget);
    m_fileListWidget    = new CustomListWidget(m_fileDockWidget);

    m_annoDockWidget->setWidget(m_annoListWidget);
    m_labelDockWidget->setWidget(m_labelListWidget);
    m_fileDockWidget->setWidget(m_fileListWidget);

    addDockWidget(Qt::RightDockWidgetArea,m_annoDockWidget);
    addDockWidget(Qt::RightDockWidgetArea,m_labelDockWidget);
    addDockWidget(Qt::RightDockWidgetArea,m_fileDockWidget);
}

void Window::initMenu()
{
    Qt::ToolBarArea toolBar_location = static_cast<Qt::ToolBarArea>(g_config->Get<int>("toolbar_location", "ui", TOOLBAR_LOCATION));

    //File
    QMenu *fileMenu = menuBar()->addMenu(tr("&文件"));
    QToolBar *fileToolbar = addToolBar(tr("&文件"));
    fileToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBar(toolBar_location,fileToolbar);//set toolbar location

    //File->OpenFile
    QAction* actOpenFile = new QAction(QIcon(":open_file"),tr("打开文件  "));
    actOpenFile->setShortcut(QKeySequence("Ctrl+O"));

    connect(actOpenFile, &QAction::triggered, this, [=](){
        m_canvas->loadPixmap();
    });
    fileMenu->addAction(actOpenFile);
    fileToolbar->addAction(actOpenFile);

    //File->SaveFile
    QAction* actSaveFile = new QAction(QIcon(":save_file"),tr("保存文件  "));
    actSaveFile->setShortcut(QKeySequence("Ctrl+S"));
    connect(actSaveFile,&QAction::triggered,this,[=](){
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
    m_taskComboBox->insertItem(DocumentBase::detection,"检测");
    m_taskComboBox->insertItem(DocumentBase::segmentation,"分割");
    m_taskComboBox->setCurrentIndex(DocumentBase::segmentation);
    editToolbar->addWidget(m_taskComboBox);
    m_canvas->m_doc->changeTask(DocumentBase::segmentation);
    connect(m_taskComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int idx){
        m_canvas->m_doc->changeTask(static_cast<DocumentBase::task_mode_e>(idx));
    });

    m_drawComboBox = new QComboBox(editToolbar);
    m_drawComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_drawComboBox->insertItem(YShape::Rectangle,"矩形");
    m_drawComboBox->insertItem(YShape::Line,"直线");
    m_drawComboBox->insertItem(YShape::Curve,"曲线");
    m_drawComboBox->insertItem(YShape::Polygon,"多边形");
    m_drawComboBox->setCurrentIndex(YShape::Polygon);
    editToolbar->addWidget(m_drawComboBox);
    m_canvas->m_doc->changeDrawMode(YShape::Polygon);
    connect(m_drawComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int idx){
        m_canvas->m_doc->changeDrawMode(static_cast<YShape::draw_mode_e>(idx));
    });

    m_optComboBox = new QComboBox(editToolbar);
    m_optComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_optComboBox->insertItem(DocumentBase::draw,"绘制");
    m_optComboBox->insertItem(DocumentBase::edit,"编辑");
    m_optComboBox->setCurrentIndex(DocumentBase::draw);
    editToolbar->addWidget(m_optComboBox);
    m_canvas->m_doc->changeOperatMode(DocumentBase::draw);
    connect(m_optComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int idx){
        m_canvas->m_doc->changeOperatMode(static_cast<DocumentBase::operat_mode_e>(idx));
    });

    QAction* actCreatPolygon = new QAction(QIcon(":creat_polygon"),tr("创建多边形  "));
    actCreatPolygon->setShortcut(QKeySequence("Ctrl+N"));

    connect(actCreatPolygon, &QAction::triggered, this, [=](){
        qInfo() << "Creat polygon";
        m_canvas->addShape();
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


    QAction *m_undoAction = m_canvas->m_undoGroup->createUndoAction(this);
    QAction *m_redoAction = m_canvas->m_undoGroup->createRedoAction(this);

    m_undoAction->setIcon(QIcon(":undo"));
    m_redoAction->setIcon(QIcon(":redo"));
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editToolbar->addAction(m_undoAction);
    editToolbar->addAction(m_redoAction);

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
    connect(zoom_in_Action, &QAction::triggered, this, [=](){m_canvas->m_doc->setScale(m_canvas->m_doc->getScale()*1.1);});
    connect(zoom_out_Action, &QAction::triggered, this, [=](){m_canvas->m_doc->setScale(m_canvas->m_doc->getScale()*0.9);});
    connect(zoom_fit_Action, &QAction::triggered, this, [=](){m_canvas->adjustFitWindow();});



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
