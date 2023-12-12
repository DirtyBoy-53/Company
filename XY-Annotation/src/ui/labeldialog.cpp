#include "labeldialog.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox> 


#include "qtfunctions.h"
#include "common.h"
LabelDialog::LabelDialog(const LabelManager &labelManager, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Label Attribute"));
    initUi();
    setFixedSize(350,450);
    for (auto label:labelManager.getLabels()){
        QListWidgetItem *item = new QListWidgetItem(ColorUtils::iconFromColor(label.color),label.label, m_LabelListWidget);
        m_LabelListWidget->addItem(item);
    }

    m_edtiLabel->setLabelListWidget(m_LabelListWidget);
}

QString LabelDialog::getLabel() const
{
    return m_edtiLabel->text();
}

void LabelDialog::initUi()
{
    m_edtiLabel = new LabelLineEdit;
    m_edtiLabel->setPlaceholderText(tr("Label"));

    m_editGroup = new QLineEdit;
    m_editGroup->setMaximumWidth(90);
    m_editGroup->setPlaceholderText(tr("Group ID"));
    QHBoxLayout* hlayout_Edit = genHBoxLayout();
    hlayout_Edit->addWidget(m_edtiLabel);
    hlayout_Edit->addWidget(m_editGroup);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    //QPushButton* btnConfirm = genPushButton(QPixmap(":confirm"),tr("Confirm"));
    //QPushButton* btnCancel = genPushButton(QPixmap(":cancel"),tr("Cancel"));
    //QHBoxLayout* hlayout_Btn = genHBoxLayout();
    //hlayout_Btn->addStretch();
    //hlayout_Btn->addWidget(btnConfirm);
    //hlayout_Btn->setSpacing(5);
    //hlayout_Btn->addWidget(btnCancel);

    m_LabelListWidget = new QListWidget;
    m_editDescription = new QTextEdit;
    m_LabelListWidget->setMinimumWidth(150);
    m_editDescription->setMinimumHeight(50);
    m_editDescription->setPlaceholderText(tr("Label description"));

    QVBoxLayout* vlayout = genVBoxLayout();
    vlayout->addLayout(hlayout_Edit);
    vlayout->addSpacing(5);
    vlayout->addWidget(btns);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_LabelListWidget);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_editDescription);
    setLayout(vlayout);

}

void LabelDialog::initConnect(){}

void LabelLineEdit::setLabelListWidget(QListWidget *listWidget)
{
    labelListWidget = listWidget;

    // 功能 a: 输入时根据 labelListWidget 的 items 的 text 提供自动补全
    QCompleter* completer = new QCompleter(this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModel(labelListWidget->model());
    this->setCompleter(completer);

    // 功能 b: 当 labelListWidget 的选中项改变时，相应的改变输入框中的text
    connect(labelListWidget, &QListWidget::currentItemChanged, [this](QListWidgetItem *currentItem){
        this->setText(currentItem->text());
    });
}

void LabelLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up){
        if (labelListWidget!=nullptr && labelListWidget->count()>0){
            int newRow = std::max(0, std::min(labelListWidget->currentRow() - 1, labelListWidget->count()-1));
            labelListWidget->setCurrentRow(newRow);
            this->setText(labelListWidget->currentItem()->text());
        }
    }else if (event->key() == Qt::Key_Down){
        if (labelListWidget!=nullptr && labelListWidget->count()>0){
            int newRow = std::max(0, std::min(labelListWidget->currentRow() + 1, labelListWidget->count()-1));
            labelListWidget->setCurrentRow(newRow);
            this->setText(labelListWidget->currentItem()->text());
        }
    }else{
        QLineEdit::keyPressEvent(event);
    }
}
