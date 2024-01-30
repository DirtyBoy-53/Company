#include "matchfusionwidget.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include "fusionalgorithm.h"
#include <QMessageBox>
#include <QDebug>
#include <thread>

#define HAVDATA_CHECKTIME   150     //是否有数据检测   间隔150ms检测一次
QImage cvMat2QImage(const cv::Mat& mat);
MatchFusionWidget::MatchFusionWidget(QWidget *parent)
    : QWidget(parent)
{
    mSensor_match = new SensorMatch;
    mRunFlag = true;
    new std::thread(&MatchFusionWidget::doWork, this);
    
    DataCollector::instance()->setSensorMatch(mSensor_match);
    DataCollector::instance()->setChannel(0);
    // m_timer_id = startTimer(m_timer_interval);
    mTimer.start(HAVDATA_CHECKTIME);
    initUI();
    initConnect();
}

void MatchFusionWidget::timerEvent(QTimerEvent *event)
{
    if(event->timerId() != m_timer_id) return;
    if(m_is_fusion_start == true) return;
    if(mMatch_data.isTakeOut == true){
        // QImage image = cvMat2QImage(mMatch_data.cameraData);
        // QPixmap pixmap = QPixmap::fromImage(image);
        // this->lbl_show_result->setPixmap(pixmap);
        // this->lbl_show_result->resize(pixmap.width(),pixmap.height());
    }
    
    // if(mSensor_match->getMatchData().size() <= 1) return;
    // mMatch_data = mSensor_match->getMatchData().front();
    // mSensor_match->getMatchData().pop_front();
    // m_future = QtConcurrent::run([=](){
    //     start_fusion(mMatch_data);
    // });
    // m_watcher.setFuture(m_future);
}


MatchFusionWidget::~MatchFusionWidget()
{
    mRunFlag = false;
    delete mSensor_match;
    mSensor_match = nullptr;
}

void MatchFusionWidget::initUI()
{
    // QLineEdit* edit_value1 = new QLineEdit("1.18460728");
	// QLineEdit* edit_value2 = new QLineEdit("-1.13551925");
	// QLineEdit* edit_value3 = new QLineEdit("1.25224818");
	// QLineEdit* edit_value4 = new QLineEdit("0.48083475");
	// QLineEdit* edit_value5 = new QLineEdit("0.10228526");
	// QLineEdit* edit_value6 = new QLineEdit("0.23415749");
	// QPushButton* btn_value = new QPushButton("写入");
	// connect(btn_value,&QPushButton::clicked,[=](){
	// 	std::vector<double> value;
	// 	value.push_back(edit_value1->text().toDouble());
	// 	value.push_back(edit_value2->text().toDouble());
	// 	value.push_back(edit_value3->text().toDouble());
	// 	value.push_back(edit_value4->text().toDouble());
	// 	value.push_back(edit_value5->text().toDouble());
	// 	value.push_back(edit_value6->text().toDouble());
	// 	FusionAlgorithm::instance()->writeParam(value);
	// });

    //进制该页面上的滚轮事件，防止误触：
    auto qComboBoxGroup= this->findChildren<QComboBox*>();
    for(auto each:qComboBoxGroup) {
        each->installEventFilter(this);
    }

    QComboBox* comBox_match_type = new QComboBox;
    QLabel* lbl_match_type = new QLabel("模式选择");

    QComboBox* comBox_channel = new QComboBox;
    QLabel* lbl_channel = new QLabel("相机通道选择");

    btn_start = new QPushButton("开始配准");

    QHBoxLayout* hLayout_match_type = new QHBoxLayout;
    // hLayout_match_type->addWidget((QWidget*)edit_value1);
    // hLayout_match_type->addWidget((QWidget*)edit_value2);
    // hLayout_match_type->addWidget((QWidget*)edit_value3);
    // hLayout_match_type->addWidget((QWidget*)edit_value4);
    // hLayout_match_type->addWidget((QWidget*)edit_value5);
    // hLayout_match_type->addWidget((QWidget*)edit_value6);
    // hLayout_match_type->addWidget(btn_value);

    hLayout_match_type->addStretch();
    hLayout_match_type->addWidget(lbl_channel);
    hLayout_match_type->addWidget(comBox_channel);
    hLayout_match_type->setSpacing(10);
    hLayout_match_type->addWidget(lbl_match_type);
    hLayout_match_type->addWidget(comBox_match_type);
    hLayout_match_type->setSpacing(10);
    hLayout_match_type->addWidget(btn_start);

    connect(btn_start,&QPushButton::clicked,[=](){
        if(btn_start->text() == "开始配准"){
            btn_start->setText("停止配准");
            mSensor_match->cleanALLData();
            comBox_channel->setEnabled(false);
            comBox_match_type->setEnabled(false);
            mPauseFlag = false;
            qDebug() << "channel:" << mCameraChannel << "  type:" << mMatch_type;

            mSensor_match->setCameraChannel(mCameraChannel);
            mSensor_match->setMatchType(mMatch_type);

            mSensor_match->setStopRecv(false);
            mTimer.start(HAVDATA_CHECKTIME);
        }else{
            btn_start->setText("开始配准");
            mPauseFlag = true;
            comBox_channel->setEnabled(true);
            comBox_match_type->setEnabled(true);
            mSensor_match->setStopRecv(true);
            mTimer.stop();
            
        }
        
    });
    connect(comBox_channel,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int index) {
        mCameraChannel = index;
    });

    connect(comBox_match_type,QOverload<int>::of(&QComboBox::currentIndexChanged),[=](int index) {
        mMatch_type = static_cast<EnumType::match_type_e>(index);
    });
    for(auto i = 0;i < CAMERA_NUM;i++){
        comBox_channel->insertItem(i,"通道"+QString::number(i+1));
    }

    comBox_match_type->insertItem(EnumType::none_match,"无");
    comBox_match_type->insertItem(EnumType::lidar_camera_match,"激光-相机");
    comBox_match_type->insertItem(EnumType::radar_camera_match,"毫米波-相机");
    comBox_match_type->insertItem(EnumType::lidar_radar_match,"激光-毫米波");
    comBox_match_type->insertItem(EnumType::lidar_radar_camera_match,"激光-毫米波-相机");
    comBox_match_type->setCurrentIndex(EnumType::none_match);




    // lbl_show_result = new QLabel();
    mImageWidget = new ImageWidget(this);
    QVBoxLayout* vLayout = new QVBoxLayout;
    // lbl_show_result->setScaledContents(true);
    vLayout->addLayout(hLayout_match_type);
    vLayout->addWidget(mImageWidget);
    setLayout(vLayout);

}

void MatchFusionWidget::initConnect()
{
    // connect(&m_watcher,&QFutureWatcher<void>::finished,[this](){
    //     this->mMatch_data.isTakeOut = true;
    //     this->m_is_fusion_start = false;
    
    // });
    // connect(this,&MatchFusionWidget::sig_update_image,this,&MatchFusionWidget::slot_update_image);
    connect(this,&MatchFusionWidget::sig_update_image,mImageWidget,&ImageWidget::onShowImg);
    connect(&mTimer,&QTimer::timeout,[this](){
        bool havData = this->mSensor_match->checkSesorHavData();
        if(havData) return;
        this->mSensor_match->setStopRecv(true);//停止接收数据
        mTimer.stop();
        mSensor_match->cleanALLData();
        this->mPauseFlag = true;
        QMessageBox::information(this,"提示","请检查设备连接是否异常,目前无法正常匹配.");
        this->btn_start->setText("开始配准");
    });
}

void MatchFusionWidget::start_fusion(match_data_t &matchData,cv::Mat& mat)
{
    switch (mSensor_match->getMatchType())
    {
    case EnumType::none_match:  break;
    case EnumType::lidar_camera_match :  
        FusionAlgorithm::instance()->projectpcd(*(matchData.lidarData.get()),matchData.cameraData,mat);
    break;
    case EnumType::radar_camera_match :
       FusionAlgorithm::instance()->radar2camera(matchData.radarData,matchData.cameraData,mat);
    break;
    case EnumType::lidar_radar_match :  break;
    case EnumType::lidar_radar_camera_match :  
        FusionAlgorithm::instance()->projectpcd(*(matchData.lidarData.get()),matchData.cameraData,mat);
        FusionAlgorithm::instance()->radar2camera(matchData.radarData,matchData.cameraData,mat);
    break;
    default: break;
    }
    
}

bool MatchFusionWidget::eventFilter(QObject *watched, QEvent *event)
{
    //屏蔽鼠标滚轮事件
    if(event->type() == QEvent::Wheel&&(watched->inherits("QComboBox"))) {
        return true;
    }
    return false;
}

QImage cvMat2QImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}
#define MAX_CNT 1024
#include <QTime>
void MatchFusionWidget::doWork()
{
    match_data_t match_data;
    QImage image;  
    bool flag{false};
    cv::Mat mat;    

    while(mRunFlag){
        if(!mPauseFlag){
            if(mSensor_match != nullptr && mSensor_match->getMatchData().size() > 0){
                QTime timeDebug;
                timeDebug.start();
                match_data = mSensor_match->getMatchData().front();
                
                start_fusion(match_data,mat);
                // qDebug() << "match queue size:" << mSensor_match->getMatchData().size()
                // << "  alg use time:" << timeDebug.elapsed() << "ms";
                image = cvMat2QImage(mat);

                emit sig_update_image(image,image.width(),image.height());
                mSensor_match->getMatchData().pop_front();
                flag = true;
            }
        }
        if(flag == true){
            flag = false;
        }else{
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
}
