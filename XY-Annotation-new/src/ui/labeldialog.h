#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "labelmanager.h"
#include "singleton.hpp"
#include "CanvasBase.h"

QT_FORWARD_DECLARE_CLASS(QListWidget)
QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(LabelDialog)

class LabelLineEdit : public QLineEdit{
    friend LabelDialog;
public:
    LabelLineEdit() = default;

    void setLabelListWidget(QListWidget* listWidget);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QListWidget* labelListWidget{ nullptr };

};



class LabelDialog : public QDialog/*, public Singleton<LabelDialog>*/
{
    Q_OBJECT
public:
    explicit LabelDialog(const QVector<ShapePtr> shapeList,QWidget* parent = nullptr);
    QString getLabel() const;

protected:
    void initUi();
    void initConnect();
private:
    LabelLineEdit* m_edtiLabel{nullptr};
    QLineEdit* m_editGroup{nullptr};
    QListWidget* m_LabelListWidget{nullptr};
    QTextEdit* m_editDescription{nullptr};
};

#endif // LABELDIALOG_H
