#ifndef ANNOTATIONCONTAINER_H
#define ANNOTATIONCONTAINER_H

#include <QObject>
#include <QList>
#include <QRect>
#include <QStack>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <memory>

#include "annotationitem.h"
#include "canvasbase.h"

using AnnoItemPtr = std::shared_ptr<AnnotationItem>;

enum ContainerOp{
    PUSH, REMOVE, MODIFY, SWAP
};

struct AnnotationOp{
    ContainerOp opClass;
    int idx;
    AnnoItemPtr item, item2;
};


class AnnotationContainer: public QObject
{
    Q_OBJECT
public:
    explicit AnnotationContainer(QObject *parent = nullptr);

    int length() const{ return items.length(); }
    AnnoItemPtr operator [](int idx) const { checkIdx(idx); return items[idx]; }
    AnnoItemPtr at(int idx) const { checkIdx(idx); return items[idx]; }

    int getSelectedIdx() const { return selectedIdx; }
    AnnoItemPtr getSelectedItem() const { return (selectedIdx>=0 && selectedIdx<items.length())?items[selectedIdx]:nullptr; }

    QJsonArray toJsonArray();

    void fromJsonObject(QJsonObject json, TaskMode task);
    void fromJsonArray(QJsonArray json, TaskMode task);


    bool hasData(QString label) const;

    int newInstanceIdForLabel(QString label);


signals:

    void selectedChanged();

    void annoChanged();

    void labelGiveBack(QString label);

    void UndoEnableChanged(bool);
    void RedoEnableChanged(bool);

    void AnnotationAdded(AnnoItemPtr item);
    void AnnotationInserted(AnnoItemPtr item, int idx);
    void AnnotationModified(AnnoItemPtr item, int idx);
    void AnnotationRemoved(int idx);
    void AnnotationSwap(int idx);
    void allCleared();
public slots:
    void push_back(const AnnoItemPtr &item);
    void remove(int idx);
    void modify(int idx,const AnnoItemPtr &item);
    void swap(int idx);

    void allClear();

    void redo();
    void undo();

    void setSelected(int idx);

private:
    QList<AnnoItemPtr> items;
    QStack<AnnotationOp> ops;

    int curVersion;

    int selectedIdx;

    void checkIdx(int idx) const;
    void pushBackOp(AnnotationOp op);
    void emitUndoRedoEnable();
};

#endif // ANNOTATIONCONTAINER_H
