#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <exception>
#include <string>
#include <QFileDialog>

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
    static QString getPath(QString fileName);
    static QString getBaseName(QString fileName);
    static QString getSuffix(QString fileName);
    static QString getFileName(QString fileName);

    static bool saveJson(const shape_json::root_s &root, QString fileName);
    static bool readJson(shape_json::root_s &root, QString fileName);

    explicit FileManager(QObject *parent = nullptr);

    QString imageFileNameAt(int idx) const { return imageFiles[idx]; }
    bool hasChangeNotSaved() const { return changeNotSaved; }
    QString getCurrentImageFile() const { return m_path + imageFiles[curIdx]; }
    QString getCurrentOutputFile() const { return mode==ThirdDImage ? outputFiles[0] : outputFiles[curIdx]; }
    const QString &getLabelFile();
    const QString& getSavePath();


    FileMode getMode() const { return mode; }
    const QStringList &allImageFiles() const { return imageFiles; }
    int getCurIdx() const  { return curIdx; }
    int count() const { return imageFiles.length(); }


    void close();


    void setSingleImage(QString fileName, QString outputSuffix);
    void setMultiImage(QString path, QStringList suffix=QStringList() << "*.jpg" << "*.png" << "*.bmp");
    void set3DImage(QStringList fileNames, QString outputSuffix);

    QStringList getImageFiles() const;

    std::string bmpToBase64(const QImage &img);

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
    QString m_path;
    QString m_savePath;

    int curIdx;
    bool changeNotSaved{false};
    FileMode mode;


    void emitPrevNextEnable();
};

#endif // FILEMANAGER_H
