#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "labelmanager.h"
class QListWidget;
class QTextEdit;
class LabelDialog;

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



class LabelDialog : public QDialog
{
    Q_OBJECT
public:
    LabelDialog(const LabelManager& labelManager,QWidget* parent = nullptr);
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
