#include "labeldialog.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox> 


#include "qtfunctions.h"
#include "ycommon.h"


LabelDialog::LabelDialog(const QVector<ShapePtr> shapeList, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("标签属性");
    initUi();
    setFixedSize(350,450);
    for(ShapePtr shape : shapeList){
        auto label = shape->label();
        auto item = new QListWidgetItem(ColorUtils::iconFromColor(label->m_color),label->m_label, m_LabelListWidget);
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
    m_edtiLabel->setPlaceholderText("Label");

    m_editGroup = new QLineEdit;
    m_editGroup->setMaximumWidth(90);
    m_editGroup->setPlaceholderText(tr("Group ID"));
    QHBoxLayout* hlayout_Edit = genHBoxLayout();
    hlayout_Edit->addWidget(m_edtiLabel);
    hlayout_Edit->addWidget(m_editGroup);

//    std::string btnstyle = R"(background-color:transparent;)";
    std::string btnstyle = R"(background-color:rgb(228,231,230);)";
    QString picStr{""};
    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::Cancel);
    for(auto btn : btns->buttons()){
        if(btns->buttonRole(btn) == QDialogButtonBox::ButtonRole::YesRole){
            picStr = ":confirm";
        }else picStr = ":cancel";

        btn->setStyleSheet(QString::fromStdString(btnstyle));
        auto pix = QPixmap(picStr);     auto sz = pix.size();
        btn->setFixedSize(sz);          btn->setIconSize(sz);
        btn->setIcon(pix.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    m_LabelListWidget = new QListWidget;
    m_editDescription = new QTextEdit;
    m_LabelListWidget->setMinimumWidth(150);
    m_editDescription->setMinimumHeight(50);
    m_editDescription->setPlaceholderText(tr("Label描述"));

    QVBoxLayout* vlayout = genVBoxLayout();
    vlayout->addLayout(hlayout_Edit);
    vlayout->addSpacing(5);
    vlayout->addWidget(btns);
//    vlayout->addLayout(hlayout_Btn);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_LabelListWidget);
    vlayout->addSpacing(5);
    vlayout->addWidget(m_editDescription);
    setLayout(vlayout);

}



/**************************************************************************************************
 * LabelLineEdit(输入Label名称)
 *
 **************************************************************************************************/
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
