/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionOpen;
    QAction *actionRevert_to_Saved;
    QAction *actionDiff_to_Saved;
    QAction *actionSave;
    QAction *actionSave_As;
    QAction *actionClose;
    QAction *actionClose_Others;
    QAction *actionExit;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionDelete;
    QAction *actionSelect_All;
    QAction *actionShow_File_Manager;
    QAction *actionBuild_Run;
    QAction *actionOpen_Resource;
    QWidget *centralwidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuProject;
    QStatusBar *statusbar;
    QDockWidget *fileTree;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout;
    QTreeView *fileView;
    QDockWidget *console;
    QWidget *dockWidgetContents_3;
    QVBoxLayout *verticalLayout_2;
    QTextBrowser *consoleWindow;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(800, 600);
        actionNew = new QAction(MainWindow);
        actionNew->setObjectName(QStringLiteral("actionNew"));
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QStringLiteral("actionUndo"));
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName(QStringLiteral("actionRedo"));
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionRevert_to_Saved = new QAction(MainWindow);
        actionRevert_to_Saved->setObjectName(QStringLiteral("actionRevert_to_Saved"));
        actionDiff_to_Saved = new QAction(MainWindow);
        actionDiff_to_Saved->setObjectName(QStringLiteral("actionDiff_to_Saved"));
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QStringLiteral("actionSave"));
        actionSave_As = new QAction(MainWindow);
        actionSave_As->setObjectName(QStringLiteral("actionSave_As"));
        actionClose = new QAction(MainWindow);
        actionClose->setObjectName(QStringLiteral("actionClose"));
        actionClose_Others = new QAction(MainWindow);
        actionClose_Others->setObjectName(QStringLiteral("actionClose_Others"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionCut = new QAction(MainWindow);
        actionCut->setObjectName(QStringLiteral("actionCut"));
        actionCopy = new QAction(MainWindow);
        actionCopy->setObjectName(QStringLiteral("actionCopy"));
        actionPaste = new QAction(MainWindow);
        actionPaste->setObjectName(QStringLiteral("actionPaste"));
        actionDelete = new QAction(MainWindow);
        actionDelete->setObjectName(QStringLiteral("actionDelete"));
        actionSelect_All = new QAction(MainWindow);
        actionSelect_All->setObjectName(QStringLiteral("actionSelect_All"));
        actionShow_File_Manager = new QAction(MainWindow);
        actionShow_File_Manager->setObjectName(QStringLiteral("actionShow_File_Manager"));
        actionBuild_Run = new QAction(MainWindow);
        actionBuild_Run->setObjectName(QStringLiteral("actionBuild_Run"));
        actionOpen_Resource = new QAction(MainWindow);
        actionOpen_Resource->setObjectName(QStringLiteral("actionOpen_Resource"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 28));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        menuView = new QMenu(menubar);
        menuView->setObjectName(QStringLiteral("menuView"));
        menuProject = new QMenu(menubar);
        menuProject->setObjectName(QStringLiteral("menuProject"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainWindow->setStatusBar(statusbar);
        fileTree = new QDockWidget(MainWindow);
        fileTree->setObjectName(QStringLiteral("fileTree"));
        fileTree->setFeatures(QDockWidget::DockWidgetClosable);
        fileTree->setAllowedAreas(Qt::LeftDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        verticalLayout = new QVBoxLayout(dockWidgetContents);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        fileView = new QTreeView(dockWidgetContents);
        fileView->setObjectName(QStringLiteral("fileView"));

        verticalLayout->addWidget(fileView);

        fileTree->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), fileTree);
        console = new QDockWidget(MainWindow);
        console->setObjectName(QStringLiteral("console"));
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QStringLiteral("dockWidgetContents_3"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents_3);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        consoleWindow = new QTextBrowser(dockWidgetContents_3);
        consoleWindow->setObjectName(QStringLiteral("consoleWindow"));

        verticalLayout_2->addWidget(consoleWindow);

        console->setWidget(dockWidgetContents_3);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(8), console);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuProject->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addSeparator();
        menuFile->addAction(actionRevert_to_Saved);
        menuFile->addAction(actionDiff_to_Saved);
        menuFile->addSeparator();
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_As);
        menuFile->addSeparator();
        menuFile->addAction(actionClose);
        menuFile->addAction(actionClose_Others);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuEdit->addAction(actionDelete);
        menuEdit->addSeparator();
        menuEdit->addAction(actionSelect_All);
        menuView->addAction(actionShow_File_Manager);
        menuProject->addAction(actionOpen_Resource);
        menuProject->addAction(actionBuild_Run);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        actionNew->setText(QApplication::translate("MainWindow", "New", 0));
        actionNew->setShortcut(QApplication::translate("MainWindow", "Ctrl+N", 0));
        actionUndo->setText(QApplication::translate("MainWindow", "Undo", 0));
        actionUndo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Z", 0));
        actionRedo->setText(QApplication::translate("MainWindow", "Redo", 0));
        actionRedo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+Z", 0));
        actionOpen->setText(QApplication::translate("MainWindow", "Open...", 0));
        actionOpen->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", 0));
        actionRevert_to_Saved->setText(QApplication::translate("MainWindow", "Revert to Saved", 0));
        actionDiff_to_Saved->setText(QApplication::translate("MainWindow", "Diff to Saved...", 0));
        actionSave->setText(QApplication::translate("MainWindow", "Save", 0));
        actionSave->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0));
        actionSave_As->setText(QApplication::translate("MainWindow", "Save As...", 0));
        actionSave_As->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+S", 0));
        actionClose->setText(QApplication::translate("MainWindow", "Close", 0));
        actionClose->setShortcut(QApplication::translate("MainWindow", "Ctrl+F4", 0));
        actionClose_Others->setText(QApplication::translate("MainWindow", "Close Others", 0));
        actionClose_Others->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+F4", 0));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", 0));
        actionCut->setText(QApplication::translate("MainWindow", "Cut", 0));
        actionCut->setShortcut(QApplication::translate("MainWindow", "Ctrl+X", 0));
        actionCopy->setText(QApplication::translate("MainWindow", "Copy", 0));
        actionCopy->setShortcut(QApplication::translate("MainWindow", "Ctrl+C", 0));
        actionPaste->setText(QApplication::translate("MainWindow", "Paste", 0));
        actionPaste->setShortcut(QApplication::translate("MainWindow", "Ctrl+V", 0));
        actionDelete->setText(QApplication::translate("MainWindow", "Delete", 0));
        actionDelete->setShortcut(QApplication::translate("MainWindow", "Del", 0));
        actionSelect_All->setText(QApplication::translate("MainWindow", "Select All", 0));
        actionShow_File_Manager->setText(QApplication::translate("MainWindow", "Show File Manager", 0));
        actionShow_File_Manager->setShortcut(QApplication::translate("MainWindow", "F9", 0));
        actionBuild_Run->setText(QApplication::translate("MainWindow", "Build and Run", 0));
        actionBuild_Run->setShortcut(QApplication::translate("MainWindow", "Ctrl+Return", 0));
        actionOpen_Resource->setText(QApplication::translate("MainWindow", "Open Resource...", 0));
        actionOpen_Resource->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", 0));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0));
        menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", 0));
        menuView->setTitle(QApplication::translate("MainWindow", "View", 0));
        menuProject->setTitle(QApplication::translate("MainWindow", "Project", 0));
        fileTree->setWindowTitle(QApplication::translate("MainWindow", "File Browser", 0));
        console->setWindowTitle(QApplication::translate("MainWindow", "Console", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // MAINWINDOW_H
