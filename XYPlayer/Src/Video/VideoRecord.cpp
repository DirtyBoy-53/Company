#include "VideoRecord.h"
#include <QDateTime>
#include <QString>
#include <QDir>

#include "confile.h"
#include "YDefine.h"
#include "YFunction.h"
VideoRecord::VideoRecord()
{
    save_path = g_exec_dir_path+"/record_video/";
}

VideoRecord::~VideoRecord()
{

}

void VideoRecord::run()
{
    XYZJFramePtr data;
    QByteArray frame;
    file_name_cur = createFile();
    elapsedTimer.start();

    while(isRunning)
    {
        if(mQueue.wait_and_pop_fortime(data)){
            if(!openFile()){
                yprint(QString("Error: Open file fail.file:%1").arg(file_name_cur));
                return;
            }
            saveXYZJVideo(data, frame);
        }

    }
}

void VideoRecord::stop()
{
    isRunning = false;
}
void VideoRecord::close()
{
    if(file_handle.isOpen()){
        file_handle.close();
    }
}

void VideoRecord::addFrame(XYZJFramePtr frame)
{
    if(mQueue.size() > 1000){
        QString msg = QString("视频保存队列缓存信息过大，无法继续保存,请暂停视频后排查问题。 缓存大小:%1").arg(mQueue.size());
        yprint(msg);
        return;
    }
    mQueue.push(frame);
}

void VideoRecord::setPath(const QString &path)
{
    save_path = path;
}

void VideoRecord::startRun()
{
    isRunning = true;
}

void VideoRecord::setMedia(Media &media)
{
    mMedia = media;
}

QString VideoRecord::createFile()
{
    save_path.append(QString("/Window%1/record_video/").arg(mMedia.id));
    QDir dir(save_path);
    if(!dir.exists()){
        if(!dir.mkpath(save_path)) return "";
    }
    std::string suffix = g_config->get<std::string>("suffix", "video", DEFAULT_SUFFIX);
    std::string prefix = g_config->get<std::string>("prefix", "video", DEFAULT_PREFIX);
    QString curTime = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
    QString file_format = QString("ch%4_%1_%2.%3").arg(QString::fromStdString(prefix)).arg(curTime).arg(QString::fromStdString(suffix)).arg(mMedia.channel+1);
    QString msg = QString("create file:%1%2").arg(save_path).arg(file_format);
    yprint(msg, PRINT_GRAY);
    return (save_path + file_format);
}

bool VideoRecord::openFile()
{
    if(elapsedTimer.elapsed() > VIDEO_RECORD_INTERVAL){
        file_name_cur = createFile();
        elapsedTimer.start();
    }
    if(file_name_last != file_name_cur){
        file_name_last = file_name_cur;
        if(file_handle.isOpen()){
            file_handle.close();
        }
        file_handle.setFileName(file_name_cur);
        if(!file_handle.open(QIODevice::WriteOnly | QIODevice::Append)) {
            yprint("Error: file_handle open fail.  file="+file_name_cur);
            return false;
        }
        writeVideoHead(file_handle);//首次打开文件需要写入视频头
    }
    return file_handle.isOpen();
}

bool VideoRecord::saveFrame(const QByteArray &frame)
{
    if(!file_handle.isOpen()) return false;
    auto size = file_handle.write(frame);
    return (size >= frame.size());
}

void VideoRecord::createFrame(XYZJFramePtr data, QByteArray &frame)
{
    int size = FRAME_HEADSIZE_MAX+data->size;
    if(frame.size() != size){
        frame.resize(size);
    }
    memcpy(frame.data(), &data->head, sizeof(FrameInfo_S));             //帧头数据
    memcpy(frame.data()+FRAME_HEADSIZE_MAX, data->data, data->size);    //视频数据
}

void VideoRecord::writeVideoHead(QFile &handle)
{
    QByteArray videoHead(VIDEO_HEADSIZE_MAX,0);
    if(handle.isOpen()){
        handle.write(videoHead, VIDEO_HEADSIZE_MAX);
    }
}

void VideoRecord::saveXYZJVideo(XYZJFramePtr data, QByteArray &frame)
{
    bool ret{false};
    createFrame(data, frame);
    ret = saveFrame(frame);
    if(!ret){
        yprint("saveFrame fail");
    }
}


