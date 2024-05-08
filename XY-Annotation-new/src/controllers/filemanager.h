#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <exception>
#include <string>
#include "jsonManager.h"
enum FileMode{
    Close, SingleImage, MultiImage, ThirdDImage
};

class FileException: public std::exception{
public:
    FileException(std::string message);
    const char * what() const noexcept;
private:
    std::string message;
};

class FileManager : public QObject
{
    Q_OBJECT
public:
    static QString getDir(QString fileName);
    static QString getName(QString fileName);
    static QString getSuffix(QString fileName);
    static QString getNameWithExtension(QString fileName);
    static void saveJson(QJsonObject json, QString fileName);
    static QJsonObject readJson(QString fileName);
    static bool saveJson(const shape_json::root_s &root, QString fileName);
    static bool readJson(shape_json::root_s &root, QString fileName);

    explicit FileManager(QObject *parent = nullptr);

    QString imageFileNameAt(int idx) const { return imageFiles[idx]; }
    bool hasChangeNotSaved() const { return changeNotSaved; }
    QString getCurrentImageFile() const { return imageFiles[curIdx]; }
    QString getCurrentOutputFile() const { return mode==ThirdDImage ? outputFiles[0] : outputFiles[curIdx]; }
    QString getLabelFile() const { return labelFile; }
    FileMode getMode() const { return mode; }
    const QStringList &allImageFiles() const { return imageFiles; }
    int getCurIdx() const  { return curIdx; }
    int count() const { return imageFiles.length(); }


    void close();


    void setSingleImage(QString fileName, QString outputSuffix);
    void setMultiImage(QStringList fileNames, QString outputSuffix);
    void set3DImage(QStringList fileNames, QString outputSuffix);

signals:
    void prevEnableChanged(bool);
    void nextEnableChanged(bool);
    void fileListSetup();
public slots:
    void setChangeNotSaved() { changeNotSaved=true; }
    void resetChangeNotSaved(){ changeNotSaved=false; }
    void prevFile();
    void nextFile();
    void selectFile(int idx);

private:
    QStringList imageFiles;
    QStringList outputFiles;
    QString labelFile;

    int curIdx;
    bool changeNotSaved;
    FileMode mode;

    void emitPrevNextEnable();
};

#endif // FILEMANAGER_H
