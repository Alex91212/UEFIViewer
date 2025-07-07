
#include "mainwindow.h"
#include <QToolBar>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QTreeView>
#include <QSplitter>
#include <QStackedWidget>
#include <QMimeDatabase>
#include <QAction>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("UEFI Dump Viewer");
    resize(1200, 800);

    splitter = new QSplitter(this);
    setCentralWidget(splitter);

    // Дерево файлов
    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

    treeView = new QTreeView(this);
    treeView->setModel(dirModel);
    treeView->hideColumn(1);
    treeView->hideColumn(2);
    treeView->hideColumn(3);
    splitter->addWidget(treeView);

    // Создаем stacked widget для переключения
    stacked = new QStackedWidget(this);

    // Текстовый редактор
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    stacked->addWidget(textEdit);

    // HEX-редактор
    hexEdit = new QHexEdit(this);
    hexEdit->setOverwriteMode(false);
    hexEdit->setReadOnly(true);
    stacked->addWidget(hexEdit);

    splitter->addWidget(stacked);
    splitter->setSizes({300, 900});

    // Панель инструментов
    QToolBar *toolBar = addToolBar("File");
    toolBar->addAction("Open UEFI", this, &MainWindow::openUefiFile);

    // Кнопка переключения режима просмотра
    viewModeAction = toolBar->addAction("HEX View", this, &MainWindow::toggleViewMode);
    viewModeAction->setCheckable(true);

    connect(treeView, &QTreeView::clicked, this, &MainWindow::onFileSelected);
}

void MainWindow::openUefiFile()
{
    QString binPath = QFileDialog::getOpenFileName(
        this,
        "Select UEFI File",
        "",
        "UEFI Files (*.bin *.rom);;All Files (*)"
        );

    if (!binPath.isEmpty()) {
        runUefiExtract(binPath);
    }
}

void MainWindow::runUefiExtract(const QString &filePath)
{
    QProcess process;
    statusBar()->showMessage("Extracting UEFI...");

    
    // QString uefiExtractPath = "/home/student/projects/UEFIExtract";
    // Универсальный путь к UEFIExtract
    QString uefiExtractPath_build = QCoreApplication::applicationDirPath();
    QDir projectDir(uefiExtractPath_build);
    projectDir.cdUp();
    
    QString uefiExtractPath = projectDir.absolutePath() + 
    #ifdef Q_OS_WIN
                              "/tools/UEFIExtract/UEFIExtract.exe";
    #else
                              "/tools/UEFIExtract/UEFIExtract";
    #endif





    process.start(uefiExtractPath, {filePath});
    if (!process.waitForFinished(30000)) {
        QMessageBox::warning(this, "Error", "Failed to run uefiextract");
        return;
    }

    QString dumpDir = filePath + ".dump";
    if (QDir(dumpDir).exists()) {
        setupFileTree(dumpDir);
        statusBar()->showMessage("Ready. Dump: " + dumpDir);
    } else {
        QMessageBox::warning(this, "Error", "No dump directory created");
    }
}

void MainWindow::setupFileTree(const QString &dumpDir)
{
    dirModel->setRootPath(dumpDir);
    treeView->setRootIndex(dirModel->index(dumpDir));
    treeView->expandAll();
}

void MainWindow::onFileSelected(const QModelIndex &index)
{
    QString filePath = dirModel->filePath(index);

    if (QFileInfo(filePath).isFile()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();

            if (viewModeAction->isChecked()) {
                // HEX-режим
                hexEdit->setData(fileData);
            } else {
                // Текстовый режим
                if (isTextFile(filePath)) {
                    QTextStream in(&fileData);
                    textEdit->setPlainText(in.readAll());
                } else {
                    textEdit->setPlainText(tr("<Binary file - switch to HEX view>"));
                }
            }

            statusBar()->showMessage(filePath);
            file.close();
        }
    }
}

bool MainWindow::isTextFile(const QString &filePath)
{
    QMimeDatabase mimeDatabase;
    return mimeDatabase.mimeTypeForFile(filePath).name().startsWith("text/");
}

void MainWindow::toggleViewMode()
{
    if (viewModeAction->isChecked()) {
        viewModeAction->setText("Text View");
        stacked->setCurrentIndex(1); // HEX-вид
    } else {
        viewModeAction->setText("HEX View");
        stacked->setCurrentIndex(0); // Текстовый вид
    }
}


