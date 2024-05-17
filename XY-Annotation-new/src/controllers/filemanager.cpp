#include "filemanager.h"
#include "ycommon.h"
#include <QtDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>

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

void FileManager::emitPrevNextEnable(){
    emit prevEnableChanged(curIdx>0);
    emit nextEnableChanged(curIdx<count()-1);
}


