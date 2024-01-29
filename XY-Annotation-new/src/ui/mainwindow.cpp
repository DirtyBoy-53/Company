#include "mainwindow.h"

#include <QScrollArea>

#include "appdef.h"
#include "confile.h"
#include "qtstyles.h"

#include "canvasbase.h"
#include "canvas2d.h"
#include "common.h"
#include "customlistwidget.h"
#include "labeldialog.h"
using namespace StringConstants;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scrollArea(new QScrollArea)
    , m_labelManager(this)
    , m_annoContainer(this)
    , m_fileManager(this)
{
    initUI();
    initConnect();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    setWindowIcon(QIcon(":ICON"));

    setCentralWidget(m_scrollArea);
    m_2DCanvas = new Canvas2D(&m_labelManager,&m_annoContainer,m_scrollArea);
    m_curCanvas = m_2DCanvas;

    m_curCanvas->setVisible(true);
    m_curCanvas->setEnabled(true);

    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidget(m_curCanvas);

    m_curCanvas->changeTask(SEGMENTATION);
    m_curCanvas->changeDrawMode(getDrawModeFromText("Polygonal Contour "));


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


    m_labelListWidget->setSortingEnabled(true);

    initMenu();

    statusBar()->showMessage(tr("No Message!"));
}

void MainWindow::initConnect()
{
    // label的右键菜单: change color & delete
    m_labelListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_labelListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::provideLabelContextMenu);

    // label的visible改变
    connect(m_labelListWidget, &QListWidget::itemChanged,
            [this](QListWidgetItem *item){
                if (item->checkState()==Qt::Checked){
                    m_labelManager.setVisible(item->text(),true);
                }else{
                    m_labelManager.setVisible(item->text(),false);
                }
            });

    // label changed -> canvas update
    connect(&m_labelManager, &LabelManager::labelChanged, this, &MainWindow::canvasUpdate);

    // label changed -> ui list changed
    connect(&m_labelManager, &LabelManager::labelAdded,
            m_labelListWidget, &CustomListWidget::addCustomItem);
    connect(&m_labelManager, &LabelManager::labelRemoved,
            m_labelListWidget, &CustomListWidget::removeCustomItem);
    connect(&m_labelManager, &LabelManager::colorChanged,
            m_labelListWidget, &CustomListWidget::changeIconColor);
    connect(&m_labelManager, &LabelManager::visibelChanged,
            m_labelListWidget, &CustomListWidget::changeCheckState);
    connect(&m_labelManager, &LabelManager::allCleared,
            m_labelListWidget, &QListWidget::clear);

    // 执行undo的时候可能会造成已经被删除的label又重新出现的情况
    connect(&m_annoContainer, &AnnotationContainer::labelGiveBack, this, &MainWindow::newLabelRequest);

    // 来自canvas的关于新标注的请求
//    connect(m_2DCanvas, &Canvas2D::newRectangleAnnotated, this, &MainWindow::getNewRect);
    connect(m_2DCanvas, &Canvas2D::newStrokesAnnotated, this, &MainWindow::getNewStrokes);

    // anno changed -> canvas update
    connect(&m_annoContainer, &AnnotationContainer::annoChanged, this, &MainWindow::canvasUpdate);
    connect(&m_annoContainer, &AnnotationContainer::selectedChanged, this, &MainWindow::canvasUpdate);

    // anno changed -> ui list change
    connect(&m_labelManager, &LabelManager::colorChanged, [this](QString label, QColor color){
        for (int i=0;i<m_annoListWidget->count();i++){
            auto item = m_annoListWidget->item(i);
            if (item->text().split(' ')[0]==label)
                m_annoListWidget->changeIconColorByIdx(i, color);
        }
    });
    connect(&m_annoContainer, &AnnotationContainer::AnnotationAdded,[this](const AnnoItemPtr &item){
        m_annoListWidget->addCustomItemUncheckable(item->toStr(), m_labelManager.getColor(item->getLabel()));
    });
    connect(&m_annoContainer, &AnnotationContainer::AnnotationInserted,[this](const AnnoItemPtr &item, int idx){
        m_annoListWidget->insertCustomItemUncheckable(item->toStr(), m_labelManager.getColor(item->getLabel()),idx);
    });
    connect(&m_annoContainer, &AnnotationContainer::AnnotationModified,[this](const AnnoItemPtr &item, int idx){
        m_annoListWidget->changeTextByIdx(idx, item->toStr());
    });
    connect(&m_annoContainer, &AnnotationContainer::AnnotationRemoved,[this](int idx){
        m_annoListWidget->removeCustomItemByIdx(idx);
    });
    connect(&m_annoContainer, &AnnotationContainer::allCleared,
            m_annoListWidget, &QListWidget::clear);
    connect(&m_annoContainer, &AnnotationContainer::AnnotationSwap, [this](int idx){
        int selectedIdx = m_annoContainer.getSelectedIdx();

        auto item = m_annoListWidget->takeItem(idx);
        m_annoListWidget->insertItem(idx+1, item);

        if (selectedIdx == idx)
            m_annoListWidget->item(idx)->setSelected(true);
        else if (selectedIdx == idx+1)
            m_annoListWidget->item(idx+1)->setSelected(true);
    });

    // fileListSetup -> update ui list
    connect(&m_fileManager, &FileManager::fileListSetup, [this](){
        m_fileListWidget->clear();
        if (m_fileManager.getMode() == Close) return;
        for (const QString &image: m_fileManager.allImageFiles()){
            m_fileListWidget->addItem(FileManager::getNameWithExtension(image));
        }
        m_fileListWidget->item(m_fileManager.getCurIdx())->setSelected(true);
    });
}

void MainWindow::initMenu()
{
    Qt::ToolBarArea toolBar_location = static_cast<Qt::ToolBarArea>(g_config->Get<int>("toolbar_location", "ui", TOOLBAR_LOCATION));

    //File
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolbar = addToolBar(tr("&File"));
    fileToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBar(toolBar_location,fileToolbar);//set toolbar location

    //File->OpenFile
    QAction* actOpenFile = new QAction(QIcon(":open_file"),tr("打开文件  "));
    actOpenFile->setShortcut(QKeySequence("Ctrl+O"));

    connect(actOpenFile, &QAction::triggered, this, [=](){
        on_actionOpen_File_triggered();
    });
    fileMenu->addAction(actOpenFile);
    fileToolbar->addAction(actOpenFile);

    //File->SaveFile
    QAction* actSaveFile = new QAction(QIcon(":save_file"),tr("保存文件  "));
    actSaveFile->setShortcut(QKeySequence("Ctrl+S"));
    connect(actSaveFile,&QAction::triggered,this,[=](){
        on_actionSave_File_triggered();
    });
    fileMenu->addAction(actSaveFile);
    fileToolbar->addAction(actSaveFile);


    //Edit
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolbar = addToolBar(tr("&Edit"));
    editToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBar(toolBar_location,editToolbar);//set toolbar location


    QAction* actCreatPolygon = new QAction(QIcon(":creat_polygon"),tr("创建多边形  "));
    actCreatPolygon->setShortcut(QKeySequence("Ctrl+N"));

    connect(actCreatPolygon, &QAction::triggered, this, [=](){
        qInfo() << "Creat polygon";
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
            m_curCanvas->changeCanvasMode(SELECT);
            actOpt->setText("编辑  ");
        }else{
            m_curCanvas->changeCanvasMode(DRAW);
            actOpt->setText(str);
        }
    });
    editMenu->addAction(actOpt);
    editToolbar->addAction(actOpt);

    //View
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    QToolBar *viewToolbar = addToolBar(tr("&Edit"));
    viewToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addToolBar(toolBar_location,viewToolbar);//set toolbar location

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
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(QIcon(":help"),tr(" &About"), this, SLOT(about()));
}

void MainWindow::about()
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


void MainWindow::mv_fullscreen()
{
    
}

void MainWindow::canvasUpdate()
{
    if (m_curCanvas==m_2DCanvas){
        m_2DCanvas->update();
    }else {
        //        if (canvas3d->getTaskMode() == DETECTION3D){
        //            canvas3d->updateChildren();
        //        }else{
        //            canvas3d->repaintSegAnnotation();
        //        }
    }
}

QString MainWindow::getCurrentLabel() const
{
    auto selectedLabels = m_labelListWidget->selectedItems();// ui->labelListWidget->selectedItems();
    if (selectedLabels.length()==1){
        return selectedLabels[0]->text();
    }else if (selectedLabels.length()==0){
        return "";
    }else {
        throw "selected mutiple label in the list";
    }
}
void MainWindow::newLabelRequest(QString newLabel)
{
    if (newLabel.isNull() || newLabel.isEmpty()) return;
    if (!m_labelManager.hasLabel(newLabel)){
        QColor newColor = ColorUtils::randomColor();
        m_labelManager.addLabel(newLabel, newColor, true);
    }
}
QString MainWindow::_labelRequest()
{
    QString curLabel = getCurrentLabel();
    if (curLabel==""){
        LabelDialog dialog(m_labelManager, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString newLabel = dialog.getLabel();
            // newLabel 也有可能是 "" ，说明dialog被点击了取消
            newLabelRequest(newLabel);
            return newLabel;
        }else {
            return "";
        }
    }else{
        return curLabel;
    }
}
void MainWindow::getNewStrokes(const QList<SegStroke> &strokes)
{
    QString label = _labelRequest();
    if (label=="") return;
    std::shared_ptr<SegAnnotationItem> item =
        std::make_shared<SegAnnotationItem>(strokes, label,
                                            m_annoContainer.newInstanceIdForLabel(label));
    m_annoContainer.push_back(std::static_pointer_cast<AnnotationItem>(item));
}

void MainWindow::_saveSegmentImageResults()
{
    QString fileName = m_fileManager.getCurrentImageFile();
    QImage colorImage = drawColorImage(m_2DCanvas->getPixmap().size(), &m_annoContainer, &m_labelManager);
    QString colorImagePath = FileManager::getDir(fileName) + FileManager::getName(fileName) + SUFFIX_SEG_COLOR;
    colorImage.save(colorImagePath);
    QImage labelIdImage = drawLabelIdImage(m_2DCanvas->getPixmap().size(), &m_annoContainer, &m_labelManager);
    QString labelIdImagePath = FileManager::getDir(fileName) + FileManager::getName(fileName) + SUFFIX_SEG_LABELID;
    labelIdImage.save(labelIdImagePath);
}

void MainWindow::on_actionSave_triggered()
{
    if (m_curCanvas == m_2DCanvas){ // 2D mode
        if (m_fileManager.getMode()==SingleImage){
            QJsonObject json;
            json.insert("labels", m_labelManager.toJsonArray());
            json.insert("annotations", m_annoContainer.toJsonArray());
            FileManager::saveJson(json, m_fileManager.getCurrentOutputFile());

            if (m_curCanvas->getTaskMode()==SEGMENTATION)
                _saveSegmentImageResults();
        }else if (m_fileManager.getMode()==MultiImage){
            QJsonObject labelJson;
            labelJson.insert("labels", m_labelManager.toJsonArray());
            FileManager::saveJson(labelJson, m_fileManager.getLabelFile());
            QJsonObject annoJson;
            annoJson.insert("annotations", m_annoContainer.toJsonArray());
            FileManager::saveJson(annoJson, m_fileManager.getCurrentOutputFile());

            if (m_2DCanvas->getTaskMode()==SEGMENTATION)
                _saveSegmentImageResults();
        }
    }
//    else if (curCanvas == canvas3d){ // 3D mode
//        if (canvas3d->getTaskMode() == DETECTION3D){
//            QJsonObject json;
//            json.insert("labels", labelManager.toJsonArray());
//            json.insert("annotations", annoContainer.toJsonArray());
//            FileManager::saveJson(json, fileManager.getCurrentOutputFile());
//        }else if (canvas3d->getTaskMode() == SEGMENTATION3D){
//            QJsonObject json;
//            json.insert("labels", labelManager.toJsonArray());
//            json.insert("annotations", annoContainer.toJsonArray());
//            FileManager::saveJson(json, fileManager.getCurrentOutputFile());

//            if (canvas3d->getTaskMode()==SEGMENTATION3D){
//                _saveSegment3dImageResults();
//            }
//        }
//    }
    m_fileManager.resetChangeNotSaved();
}

bool MainWindow::_checkUnsaved()
{
    if (m_fileManager.hasChangeNotSaved()){
//        if (ui->actionAuto_Save->isChecked())
//            on_actionSave_triggered();
//        else{
            int ret = QMessageBox::warning(this, QObject::tr("Warning"),
                                           QObject:: tr("The document has been modified.\n"
                                                       "Do you want to save your changes?\n"
                                                       "Note: you can check AutoSave option in the menu.\n"),
                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                           QMessageBox::Save);
            switch (ret) {
            case QMessageBox::Save:
                on_actionSave_triggered();
                break;
            case QMessageBox::Discard:
                break;
            case QMessageBox::Cancel:
                return false;
            default:
                break;
            }
//        }
    }
    return true;
}

void MainWindow::enableFileActions()
{
    for (auto action: m_fileRelatedActions)
            action->setEnabled(true);
//    taskComboBox->setEnabled(false);
}

void MainWindow::unableFileActions()
{
    for (auto action: m_fileRelatedActions)
            action->setEnabled(false);
//    taskComboBox->setEnabled(true);
}

void MainWindow::on_actionClose_triggered()
{
    if (m_fileManager.getMode() == Close) return;

    if (!_checkUnsaved()) return;

    m_curCanvas->close();
    m_labelManager.allClear();
    m_annoContainer.allClear();
    m_fileManager.close();
    unableFileActions();
}


void MainWindow::on_actionSave_File_triggered()
{
    QJsonObject json;
//    json.insert("labels", m_labelManager.toJsonArray());//添加
    json.insert("version",APP_VERSION);

    json.insert("shapes", m_annoContainer.toJsonArray());



    QString filename = m_fileManager.getCurrentImageFile();

    QPixmap pixmap = m_2DCanvas->getPixmap();
    QByteArray pixArray=QByteArray();
    QBuffer buffer(&pixArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer,FileManager::getSuffix(filename).toLocal8Bit());

    json.insert("imagePath",FileManager::getNameWithExtension(filename));
    json.insert("imageData",pixArray.toBase64().toStdString().c_str());
    json.insert("imageHeight",pixmap.height());
    json.insert("imageWidth",pixmap.width());
    FileManager::saveJson(json, m_fileManager.getCurrentOutputFile());

}

qreal MainWindow::scaleFitWindow() const
{
    int w1 = m_scrollArea->width() - 2; // -2 So that no scrollbars are generated.
    int h1 = m_scrollArea->height() - 2;
    qreal a1 = static_cast<qreal>(w1)/h1;
    int w2 = m_curCanvas->sizeUnscaled().width();
    int h2 = m_curCanvas->sizeUnscaled().height();
    qreal a2 = static_cast<qreal>(w2)/h2;
    return a2>=a1 ? static_cast<qreal>(w1)/w2 : static_cast<qreal>(h1)/h2;
}

void MainWindow::adjustFitWindow()
{
    m_curCanvas->setScale(scaleFitWindow());
}
void MainWindow::_loadJsonFile(QString fileName)
{
    QFileInfo checkFile=QFileInfo(fileName);
    if (checkFile.exists() && checkFile.isFile()){
            try {
                QJsonObject json = FileManager::readJson(fileName);
                m_labelManager.fromJsonObject(json);
                m_annoContainer.fromJsonObject(json, m_curCanvas->getTaskMode());
            } catch (FileException &e) {
                QMessageBox::warning(this, "File Error", e.what());
            } catch (JsonException &e) {
                QString msg;
                msg = QString("The saved json file is broken.\n")
                      +"Error message: "+e.what()+"\n"
                      +"Please check or delete the json file.\n";
                QMessageBox::warning(this, "Json Error", msg);
            }
    }
}
void MainWindow::on_actionOpen_File_triggered()
{
    on_actionClose_triggered();
    if (m_fileManager.getMode()!=Close) return; // cancel is selected when unsaved

    //QString fileName = QFileDialog::getOpenFileName(this, "open a file", "/",
    //                                                "Image Files (*.jpg *.png);;JPEG Files (*.jpg);;PNG Files (*.png)");
    QString fileName("E:/Desktop/vid_01.jpg");
    if (!fileName.isNull() && !fileName.isEmpty()){
        enableFileActions();

        m_2DCanvas->loadPixmap(fileName);
        adjustFitWindow();

        m_labelManager.allClear();
        m_annoContainer.allClear();

        if (m_2DCanvas->getTaskMode()==DETECTION){
            m_fileManager.setSingleImage(fileName, SUFFIX_DET_LABEL_ANNO);
        }else if (m_2DCanvas->getTaskMode()==SEGMENTATION){
            m_fileManager.setSingleImage(fileName, SUFFIX_SEG_LABEL_ANNO);
        }

//        _loadJsonFile(m_fileManager.getCurrentOutputFile());
        m_fileManager.resetChangeNotSaved();
    }
}

void MainWindow::removeLabelRequest(QString label)
{
    if (m_annoContainer.hasData(label)){
        QMessageBox::warning(this, "Warning", "This label has existing data! Please remove them first.");
    }else{
        m_labelManager.removeLabel(label);
    }
}
void MainWindow::provideLabelContextMenu(const QPoint& pos)
{
    QPoint globalPos = m_labelListWidget->mapToGlobal(pos);
    QModelIndex modelIdx = m_labelListWidget->indexAt(pos);
    if (!modelIdx.isValid()) return;
    int row = modelIdx.row();
    auto item = m_labelListWidget->item(row);

    QMenu submenu;
    submenu.addAction("Change Color");
    submenu.addAction("Delete");
    QAction* rightClickItem = submenu.exec(globalPos);
    if (rightClickItem) {
        if (rightClickItem->text().contains("Delete")) {
            removeLabelRequest(item->text());
        }
        else if (rightClickItem->text().contains("Change Color")) {
            QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
            if (color.isValid()) {
                m_labelManager.setColor(item->text(), color);
            }
        }
    }
}
