/********************************************************************************
** Form generated from reading UI file 'LSideWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LSIDEWINDOW_H
#define UI_LSIDEWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LSideWindow
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QLineEdit *Edit_VideoSavePath;
    QPushButton *Btn_VideoSavePath;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *Edit_PicSavePath;
    QPushButton *Btn_PicSavePath;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *LSideWindow)
    {
        if (LSideWindow->objectName().isEmpty())
            LSideWindow->setObjectName(QString::fromUtf8("LSideWindow"));
        LSideWindow->resize(355, 650);
        verticalLayout = new QVBoxLayout(LSideWindow);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(LSideWindow);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        Edit_VideoSavePath = new QLineEdit(LSideWindow);
        Edit_VideoSavePath->setObjectName(QString::fromUtf8("Edit_VideoSavePath"));

        horizontalLayout->addWidget(Edit_VideoSavePath);

        Btn_VideoSavePath = new QPushButton(LSideWindow);
        Btn_VideoSavePath->setObjectName(QString::fromUtf8("Btn_VideoSavePath"));
        Btn_VideoSavePath->setMaximumSize(QSize(60, 16777215));

        horizontalLayout->addWidget(Btn_VideoSavePath);


        verticalLayout->addLayout(horizontalLayout);

        label_2 = new QLabel(LSideWindow);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout->addWidget(label_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        Edit_PicSavePath = new QLineEdit(LSideWindow);
        Edit_PicSavePath->setObjectName(QString::fromUtf8("Edit_PicSavePath"));

        horizontalLayout_2->addWidget(Edit_PicSavePath);

        Btn_PicSavePath = new QPushButton(LSideWindow);
        Btn_PicSavePath->setObjectName(QString::fromUtf8("Btn_PicSavePath"));
        Btn_PicSavePath->setMaximumSize(QSize(60, 16777215));

        horizontalLayout_2->addWidget(Btn_PicSavePath);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer = new QSpacerItem(20, 497, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(LSideWindow);

        QMetaObject::connectSlotsByName(LSideWindow);
    } // setupUi

    void retranslateUi(QWidget *LSideWindow)
    {
        LSideWindow->setWindowTitle(QCoreApplication::translate("LSideWindow", "Form", nullptr));
        label->setText(QCoreApplication::translate("LSideWindow", "\350\247\206\351\242\221\344\277\235\345\255\230\350\267\257\345\276\204", nullptr));
        Btn_VideoSavePath->setText(QCoreApplication::translate("LSideWindow", "\351\200\211\346\213\251", nullptr));
        label_2->setText(QCoreApplication::translate("LSideWindow", "\350\247\206\351\242\221\344\277\235\345\255\230\350\267\257\345\276\204", nullptr));
        Btn_PicSavePath->setText(QCoreApplication::translate("LSideWindow", "\351\200\211\346\213\251", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LSideWindow: public Ui_LSideWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LSIDEWINDOW_H
