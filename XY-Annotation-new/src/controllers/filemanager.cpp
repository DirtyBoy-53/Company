#include "filemanager.h"
#include "ycommon.h"
#include <QtDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
FileException::FileException(std::string message):message(message) {}

const char *FileException::what() const noexcept {
    return message.c_str();
}



QString FileManager::getPath(QString fileName)
{
    QFileInfo info(fileName);
    return info.absolutePath();
}

QString FileManager::getBaseName(QString fileName)
{
    QFileInfo info(fileName);
    return info.baseName();
}


QString FileManager::getSuffix(QString fileName)
{
    QFileInfo info(fileName);
    return info.suffix();
}

QString FileManager::getFileName(QString fileName)
{
    QFileInfo info(fileName);
    return info.fileName();
}

bool FileManager::saveJson(const shape_json::root_s &root, QString fileName)
{
    return JsonManager::writeJson(fileName.toStdString(), root);
}

bool FileManager::readJson(shape_json::root_s &root, QString fileName)
{
    return JsonManager::readJson(fileName.toStdString(), root);
}


FileManager::FileManager(QObject *parent) : QObject(parent)
{
    changeNotSaved=false;
    mode = Close;
}

 bool FileManager::openFile(shape_json::root_s &root_, QString&& fileName)
{
    if (!readJson(root_, fileName)) {
        qWarning() << "无法读取数据:" << fileName;
        return false;
    }
    root = root_;
    isOpenJsonFile = true;
    clearAllImg();
    m_savePath = getPath(fileName);
    labelFile = getFileName(fileName);
    return true;
}

const QString &FileManager::getLabelFile()
{
    if(!allImageFiles().isEmpty())
        labelFile = getBaseName(imageFileNameAt(curIdx)) + StringConstants::FILENAME_DIR_LABEL;
    return labelFile;
}

const QString& FileManager::getSavePath()
{
    if (m_savePath.isEmpty()) {
        m_savePath = QFileDialog::getExistingDirectory(NULL, "保存路径", m_path);
    }
    return m_savePath;
}

void FileManager::close()
{
    mode = Close;
    imageFiles.clear();
    outputFiles.clear();
    labelFile = QString();
    curIdx=0;
    changeNotSaved=false;
    emit prevEnableChanged(false);
    emit nextEnableChanged(false);
    emit fileListSetup();
}

void FileManager::setSingleImage(QString fileName, QString outputSuffix)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;

    mode = SingleImage;
    curIdx = 0;
    imageFiles<<fileName;
//    outputFiles<<getDir(fileName) + getName(fileName) + outputSuffix;
    emitPrevNextEnable();
    emit fileListSetup();
}

void FileManager::setMultiImage(QString path, QStringList suffix)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;
    m_path = path+"/";
    qDebug() << m_path;
    mode = MultiImage;
    curIdx = 0;
    QDir dir(m_path);
    imageFiles = dir.entryList(suffix, QDir::Files | QDir::Readable, QDir::Name);
    imageFiles.sort();
    if(imageFiles.size() > 0)
        labelFile = getPath(imageFiles[0]) + StringConstants::FILENAME_DIR_LABEL;
    emitPrevNextEnable();
    emit fileListSetup();
}

void FileManager::set3DImage(QStringList fileNames, QString outputSuffix)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;

    mode = ThirdDImage;
    curIdx = 0;
    fileNames.sort();
    imageFiles = fileNames;
//    outputFiles << getDir(fileNames[0])+outputSuffix;
    emitPrevNextEnable();
    emit fileListSetup();
}

void FileManager::prevFile(){
    if (curIdx>0) selectFile(curIdx-1);
}

void FileManager::nextFile(){
    if (curIdx<count()-1) selectFile(curIdx+1);
}

void FileManager::selectFile(int idx){
    if (curIdx==idx) return;
    curIdx=idx;
    emitPrevNextEnable();
}

QStringList FileManager::getImageFiles() const
{
    return imageFiles;
}

QImage FileManager::Base64ToImg(const std::string& base64)
{
    // 将 Base64 字符串解码为字节数组
    QByteArray imageData = QByteArray::fromBase64(base64.c_str());

    // 将字节数组转换为 QImage
    QImage image;
    image.loadFromData(imageData);

    if (image.isNull()) {
        qWarning() << "Failed to convert Base64 to QImage";
        return QImage();
    }

    return image;
}

void FileManager::emitPrevNextEnable(){
    emit prevEnableChanged(curIdx>0);
    emit nextEnableChanged(curIdx<count()-1);
}


std::string FileManager::imgToBase64(const QImage& img)
{
    auto _img = img;
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);

    _img.save(&buf, "BMP");
    QByteArray ba2 = ba.toBase64();
    QString b64str = QString::fromLatin1(ba2);

    return b64str.toStdString();
}