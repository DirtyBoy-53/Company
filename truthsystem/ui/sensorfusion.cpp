#include "sensorfusion.h"
#include "sensorfusion.h"
#include "sensorfusion.h"
#include "sensorfusion.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <thread>
SensorFusion::SensorFusion(QWidget* parent)
	:QWidget(parent)
{
	initUI();
	initConnect();
}

SensorFusion::~SensorFusion()
{
}

bool getFileList(const QString& fileDirPath, const QStringList& filter, std::list<QString>& fileList)
{
	bool _ret{ false };
	QDir _dir(fileDirPath);
	if (!_dir.exists()) return _ret;

	_dir.setNameFilters(filter);
	QStringList names = _dir.entryList();
	for (QString& name : names) {
		fileList.emplace_back(fileDirPath + "/" + name);
		_ret = true;
	}
	return _ret;
}

void SensorFusion::initUI()
{
	QPushButton* btn_start_fusion = new QPushButton("开始融合");
	QHBoxLayout* hLayout_start_fusion = new QHBoxLayout;
	connect(btn_start_fusion,&QPushButton::clicked,[=](){
		if(!m_run){
			btn_start_fusion->setText("停止融合");
			m_run = true;
			new std::thread(&SensorFusion::sendMsgByConsole,this);
		} 
		else{
			btn_start_fusion->setText("开始融合");
			m_run = false;
		}
	});
	hLayout_start_fusion->addWidget(btn_start_fusion);


	QLabel* label_chose_depth = new QLabel("选择融合数据-depth");
	QLineEdit* edit_chose_depth = new QLineEdit();
	QPushButton* btn_chose_depth = new QPushButton("选择");
	QHBoxLayout* hLayout_chose_depth = new QHBoxLayout;
	connect(btn_chose_depth, &QPushButton::clicked, [=]() {
		m_play_timer.stop();
		m_pic_vector.clear();
		m_play_path = choose_path();
		if (m_play_path.isEmpty())  QMessageBox::warning(this, "warning", "请重新选择.");
		else edit_chose_depth->setText(m_play_path);
	});
	hLayout_chose_depth->addStretch();
	hLayout_chose_depth->addWidget(label_chose_depth);
	hLayout_chose_depth->addWidget(edit_chose_depth);
	hLayout_chose_depth->addWidget(btn_chose_depth);

	QVBoxLayout* vLayout_tool = new QVBoxLayout;
	vLayout_tool->addLayout(hLayout_start_fusion);
	vLayout_tool->addLayout(hLayout_chose_depth);


	QVBoxLayout* vLayout_player = new QVBoxLayout;
    // QHBoxLayout* hLayout_player = new QHBoxLayout;
	label_image = new QLabel();
    label_image->setScaledContents(true);
    // hLayout_player->addStretch();
    // hLayout_player->addWidget(label_image);
    // hLayout_player->addStretch();
	btn_play_stop = new QPushButton("播放");
	connect(btn_play_stop, &QPushButton::clicked, [=]() {
		if (m_play_path.isEmpty()){
			QMessageBox::warning(this, "warning", "请重新选择路径，再点击播放.");
			return;
		} 

		if(btn_play_stop->text() == "停止"){
			btn_play_stop->setText("播放");
			m_play_timer.stop();
			return;
		}
		if(!m_pic_vector.empty() && btn_play_stop->text() == "播放"){
			m_play_timer.start(PLAYF_REQUENCY);
			btn_play_stop->setText("停止");
			return;
		}
		if (getFileList(m_play_path, QStringList() << "*.jpg", m_pic_vector)) {
			m_play_timer.start(PLAYF_REQUENCY);
			btn_play_stop->setText("停止");
			return;
		}
		else {
			QMessageBox::warning(this, "warning", "未找到jpg文件,无法播放.");
		}	
		});

	label_image->setMinimumSize(640, 512);
    // vLayout_player->addLayout(hLayout_player);
	vLayout_player->addWidget(label_image);
	vLayout_player->addWidget(btn_play_stop);

	QVBoxLayout* vLayout = new QVBoxLayout;
	vLayout->addLayout(vLayout_tool);
	vLayout->addLayout(vLayout_player);
	setLayout(vLayout);

}

void SensorFusion::initConnect()
{
	connect(&m_play_timer, &QTimer::timeout, this, &SensorFusion::slot_play_timer);
}

QString SensorFusion::choose_path()
{
	QString path = QFileDialog::getExistingDirectory(this, "choose dir", "./",
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (path.isEmpty())
		return QString("");
	else return path;
}

#include <QDebug>
#include <QDateTime>
void SensorFusion::slot_play_timer()
{
	if (!m_pic_vector.empty()) {
		QString name = m_pic_vector.front();
		m_pic_vector.pop_front();
		QPixmap pixmap(name);
		label_image->setPixmap(pixmap);
		label_image->resize(pixmap.width(),pixmap.height());


	}
	else {
		m_play_timer.stop();
        btn_play_stop->setText("播放");
		QMessageBox::warning(this, "warning", "播放完毕.");
	}
}

#include <QElapsedTimer>
#include <QCoreApplication>

void SensorFusion::sendMsgByConsole()
{
// #if 1
	QProcess m_pythonProcess;
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString spythonPath = sAppPath + "/yolov5-python/";
	QString path = spythonPath + "Truth_System_Lidar_Postprocess.py";
	bool bRst{false};
	QElapsedTimer timer;
	timer.start();
	QStringList options;
	options << path;
	qDebug() << options;
	// m_pythonProcess.start("bash");
    // m_pythonProcess.waitForStarted();
    // QString sCmd = "cd  "+spythonPath;
    // qInfo()<<sCmd;
    // m_pythonProcess.write(sCmd.toStdString().data());
	// QString pycmd = QString("%1 %2").arg("/usr/bin/python3").arg(path);
	// m_pythonProcess.write(pycmd.toStdString().data());
	m_pythonProcess.setWorkingDirectory(spythonPath);
	m_pythonProcess.start("/usr/bin/python3",options);
	while (timer.elapsed() < WAIT_FUSION_TIME && m_run) {
		m_pythonProcess.waitForFinished(); //等待运行结束，以不阻塞的形式
        m_pythonProcess.waitForReadyRead();
        QString msg = m_pythonProcess.readAll();  //命令行执行结果

		QString error =  m_pythonProcess.readAllStandardError(); //命令行执行出错的提示
		if(!error.isEmpty()){
			qDebug()<<"Warning:"<<error; //打印出错提示
			if(error.contains("Errno"))
				break;
		}


		if (msg.contains("fusion over!")) {
			bRst = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if(!msg.isEmpty())
			qDebug() << msg;
	}
	m_pythonProcess.close();
#if 0 
    m_pythonProcess.start("bash");
    m_pythonProcess.waitForStarted();
    QString sCmd = "cd  "+spythonPath;
    qInfo()<<sCmd;


    m_pythonProcess.write(sCmd.toStdString().data());

    sCmd = "python3 Truth_System_Lidar_Postprocess.py";
    qInfo()<<sCmd;
    bool bRst = false;

	


    int icount = 0;
    {
        m_pythonProcess.write(sCmd.toStdString().data());
        m_pythonProcess.waitForBytesWritten(10000);
        // m_pythonProcess.waitForFinished(10000);
        QElapsedTimer timer;
        timer.start();
        QByteArray outStd;
        while (timer.elapsed() < WAIT_FUSION_TIME && m_run) {
            QCoreApplication::processEvents();
            outStd += m_pythonProcess.readAll();
            if (outStd.contains("fusion over!")) {
                bRst = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
			qDebug() << outStd;
        }
        
    }

    // m_pythonProcess.write("exit\n");
    if (!bRst) {
        QMessageBox::information(NULL, "提示", "融合失败");
        return;
    }
    QMessageBox::information(NULL, "提示", "融合成功");
#endif
}