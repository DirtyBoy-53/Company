#include "window.h"
#include "appdef.h"
#include "confile.h"
#include "qtstyles.h"
#include "shape.h"
#include "labeldialog.h"
#include "ycommon.h"
#include "filemanager.h"
#include "CanvasView.h"
#include "ImageAdj.h"
#include "ydefine.h"
#include "yfunction.h"
#include "CanvasBase.h"
#include "shapefactory.h"
Window::Window(QWidget *parent)
    : QMainWindow(parent)
    , m_canvasView(new CanvasView(this))
    , m_imageAdj(new ImageAdj)
{
    setWindowIcon(QIcon(":ICON"));
    initParams();
    initUI();
    initConnect();

    statusBar()->showMessage(tr("No Message!"), 5000);
}

Window::~Window()
{}

void Window::initUI()
{
    setCentralWidget(m_canvasView);

    initDockWidget();
    initMenu();
}

void Window::initConnect()
{
    connect(m_canvasView->canvas(), &Canvas2D::sigSetProperty, this, &Window::slotSetProperty);


    // label changed -> ui list changed
    connect(this, &Window::sigLabelAdded,
            &m_dockWidget.labelListWidget(), &CustomListWidget::addCustomItem);   

    connect(this, &Window::sigLabelAdded,
            &m_dockWidget.annoListWidget(), &CustomListWidget::addCustomItem);

    connect(&m_dockWidget.fileListWidget(), &QListWidget::clicked, this, [=](QModelIndex index){
        if(!m_fileManager.hasChangeNotSaved()){
            m_fileManager.selectFile(index.row());
            QString path = m_fileManager.getCurrentImageFile();
            m_canvasView->loadImage(QImage(path));
        }else{
            //提示保存
            auto res = QMessageBox::warning(this, "提示", "有文件未保存,是否保存?");
			if(res == QMessageBox::Yes){
				saveFile();
			}
            m_fileManager.selectFile(index.row());
            QString path = m_fileManager.getCurrentImageFile();
            m_canvasView->loadImage(QImage(path));
        }
//        m_canvasView->loadPixmap();
    });

    

    connect(m_imageAdj, &ImageAdj::contrastChanged, this, [=](int contrast) {
        m_canvasView->canvas()->setContrast(contrast);
        });
    connect(m_imageAdj, &ImageAdj::brightnessChanged, this, [=](int brightness) {
        m_canvasView->canvas()->setBrightness(brightness);
        });
}

void Window::initParams()
{
    int contrast        = g_config->Get<int>("default_contrast",   "image", DEFAULT_CONTRAST);
    int brightness      = g_config->Get<int>("default_brightness", "image", DEFAULT_BRIGHTNESS);

    m_canvasView->canvas()->setContrast(contrast);
    m_canvasView->canvas()->setBrightness(brightness);

}

void Window::initDockWidget()
{
    addDockWidget(Qt::RightDockWidgetArea,&m_dockWidget.annoDockWidget());
    addDockWidget(Qt::RightDockWidgetArea,&m_dockWidget.labelDockWidget());
    addDockWidget(Qt::RightDockWidgetArea,&m_dockWidget.fileDockWidget());
}
void Window::saveFile()
{
    QString path = m_fileManager.getSavePath();

    if (path.isEmpty()) {
        QMessageBox::warning(this, "警告", "保存路径为空，请重新选择");
        return;
    }
       
    QString filename = path + "/" + m_fileManager.getLabelFile();
    auto list = m_canvasView->canvas()->getShapeList();
    if(list.size() <= 0) return;
    shape_json::root_s root;
    if (!m_fileManager.IsOpenJsonFile()) {
        QString ImgPath{ m_fileManager.getCurrentImageFile() };
        QImage img(ImgPath);
        root.imageData = m_fileManager.imgToBase64(img);
        root.imageHeight = std::to_string(img.height());
        root.imagePath = ImgPath.toStdString();
        root.imageWidth = std::to_string(img.width());
    }else root = m_fileManager.root;
    root.shapes.clear();
    QVector<shape_json::shape_s> shapelist;
    for(ShapePtr shape : list){
        shape_json::shape_s s;
        s.description = shape->label()->m_description.toStdString();
        s.flags = std::string();
        s.group_id = shape->label()->m_groupId;
        s.label = shape->label()->m_label.toStdString();
        s.mask = 0;
        s.points = shape->points().getImgPoints();
        s.shape_type = Shape::drawModeToStr(shape->type());
        shapelist.append(s);
    }

    root.version = APP_VERSION;
    root.flags = std::string();
    for(auto &p : shapelist)
        root.shapes.push_back(p);

 
    FileManager::saveJson(root, filename);
}
void Window::openFile()
{
    //TODO:注意保存当前已操作的画布
    m_canvasView->canvas()->clean();
    m_canvasView->clean();
    m_dockWidget.labelListWidget().clean();
    m_dockWidget.annoListWidget().clean();
    m_canvasView->canvas()->changeOperatMode(CanvasBase::edit);
    shape_json::root_s root;
    m_fileManager.openFile(root, chooseFile());
    m_canvasView->loadImage(m_fileManager.Base64ToImg(root.imageData));
    for (auto it : root.shapes) {
        ShapePtr newShape = ShapeFactory::create(m_canvasView->canvas()->getDrawMode());
        newShape->setimgWH(m_canvasView->canvas()->getImage().size());
        std::for_each(it.points.begin(), it.points.end(), [=](auto point) {newShape->appendPoint(m_canvasView->canvas()->mapFromImg(point)); });
        newShape->setClosed();
        m_canvasView->canvas()->addShape(newShape);

        QString name(it.label.c_str());
        if (name.isEmpty()) name = "unknonw"; 
        LabelProperty label(name, ColorUtils::randomColor(), true, 0);
        newShape->setLabel(label);
        emit sigLabelAdded(label.m_label, label.m_color, label.m_visible);
    }

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

    connect(actOpenFile, &QAction::triggered, this, &Window::openFile);
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
       
       m_canvasView->canvas()->clean();
       m_canvasView->clean();
       m_dockWidget.labelListWidget().clean();
       m_dockWidget.annoListWidget().clean();
        m_fileManager.setMultiImage(path);
        m_dockWidget.fileListWidget().addItems(m_fileManager.allImageFiles());
        m_fileManager.selectFile(0);
        QString imgpath = m_fileManager.getCurrentImageFile();
        m_canvasView->loadImage(QImage(imgpath));
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



    QAction* actImageAdj = new QAction(QIcon(":check"),tr("亮度/对比度 "));

    

    connect(actImageAdj, &QAction::triggered, this, [=](){
        auto contrast = m_canvasView->canvas()->getContrast();
        auto brightness = m_canvasView->canvas()->getBrightness();
        m_imageAdj->setParam(contrast, brightness);
        m_imageAdj->show();
    });
    viewMenu->addAction(actImageAdj);
    viewToolbar->addAction(actImageAdj);

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

void Window::closeEvent(QCloseEvent* event)
{
    m_imageAdj->close();
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
