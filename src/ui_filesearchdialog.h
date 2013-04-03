/********************************************************************************
** Form generated from reading UI file 'filesearchdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef FILESEARCHDIALOG_H
#define FILESEARCHDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AeFileSearchDialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *patternInput;
    QListView *resultFiles;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *resourcePath;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AeFileSearchDialog)
    {
        if (AeFileSearchDialog->objectName().isEmpty())
            AeFileSearchDialog->setObjectName(QStringLiteral("AeFileSearchDialog"));
        AeFileSearchDialog->resize(473, 448);
        verticalLayout = new QVBoxLayout(AeFileSearchDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(AeFileSearchDialog);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        patternInput = new QLineEdit(AeFileSearchDialog);
        patternInput->setObjectName(QStringLiteral("patternInput"));

        horizontalLayout->addWidget(patternInput);


        verticalLayout->addLayout(horizontalLayout);

        resultFiles = new QListView(AeFileSearchDialog);
        resultFiles->setObjectName(QStringLiteral("resultFiles"));

        verticalLayout->addWidget(resultFiles);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_2 = new QLabel(AeFileSearchDialog);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_2->addWidget(label_2);

        resourcePath = new QLineEdit(AeFileSearchDialog);
        resourcePath->setObjectName(QStringLiteral("resourcePath"));
        resourcePath->setReadOnly(true);

        horizontalLayout_2->addWidget(resourcePath);


        verticalLayout->addLayout(horizontalLayout_2);

        buttonBox = new QDialogButtonBox(AeFileSearchDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(AeFileSearchDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AeFileSearchDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AeFileSearchDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AeFileSearchDialog);
    } // setupUi

    void retranslateUi(QDialog *AeFileSearchDialog)
    {
        AeFileSearchDialog->setWindowTitle(QApplication::translate("AeFileSearchDialog", "Open Resource", 0));
        label->setText(QApplication::translate("AeFileSearchDialog", "Pattern:", 0));
        label_2->setText(QApplication::translate("AeFileSearchDialog", "File Location:", 0));
    } // retranslateUi

};

namespace Ui {
    class AeFileSearchDialog: public Ui_AeFileSearchDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // FILESEARCHDIALOG_H
