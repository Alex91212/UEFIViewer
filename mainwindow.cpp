
#include "mainwindow.h"
#include "signrequest.h"
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
#include <QPushButton>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QDialog>

#include <QStandardItem>
#include <QByteArray>

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
    hexEdit->setHighlightingColor(QColor(0xFFF, 0x00, 0x00, 0x30));
    stacked->addWidget(hexEdit);

    splitter->addWidget(stacked);
    splitter->setSizes({300, 900});

    // Панель инструментов
    QToolBar *toolBar = addToolBar("File");
    toolBar->addAction("Open UEFI", this, &MainWindow::openUefiFile);

    // Кнопка переключения режима просмотра
    viewModeAction = toolBar->addAction("HEX View", this, &MainWindow::toggleViewMode);
    viewModeAction->setCheckable(true);

    // кнопка поиска сигнатур
    toolBar->addAction("Find Signature", this, &MainWindow::findSignature);

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
    if (!process.waitForFinished(60000)) {
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

void MainWindow::setupFileTree(const QString &dumpDir) {
    // Обновляем модель
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

void MainWindow::recvFromUser(const QString &text)
{
    //qDebug() << text;
    sign = text;
    qDebug() << sign;
}

void MainWindow::findSignature()
{
    qDebug() << "findSign";
    signRequest *SignatureFinder = new signRequest(this);
    connect(SignatureFinder, &signRequest::SignPassed, this, &MainWindow::recvFromUser);

    SignatureFinder->exec();

    delete SignatureFinder;

    //qDebug() << "recevied";

    QByteArray signature = ParseHexSignature(sign);

    if (signature.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Неверный формат сигнатуры");
        return;
    }

    QString binPath = dirModel->rootPath().replace(".dump", "");
    if (!QFile::exists(binPath)) {
        QMessageBox::warning(this, "Ошибка", "Исходный UEFI-файл не найден");
        return;
    }

    findSignatureInFile(binPath, signature);
}

QByteArray MainWindow::ParseHexSignature(const QString &signature)
{
    QString s = signature.trimmed();
    QByteArray result;

    if (s.length() % 2 != 0) {
        qWarning() << "Нечётное количество символов в HEX-сигнатуре";
        return QByteArray();
    }

    bool ok;
    for (int i = 0; i < s.length(); i += 2) {
        QString byteString = s.mid(i, 2);
        char byte = static_cast<char>(byteString.toUInt(&ok, 16));
        if (!ok) {
            qWarning() << "Неверный HEX байт:" << byteString;
            return QByteArray();
        }
        result.append(byte);
    }
    return result;
}

void MainWindow::findSignatureInFile(const QString& filePath, const QByteArray& signature) {
    currentSignature = signature; // Сохраняем сигнатуру

    QString dumpDir = filePath + ".dump";
    QDir dir(dumpDir);

    if (!dir.exists()) {
        QMessageBox::warning(this, "Ошибка", "Дамп не найден");
        return;
    }

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Файл", "Смещение", "Путь"});

    // Рекурсивный поиск по всем файлам дампа
    searchInDirectory(dumpDir, signature, model);

    if (model->rowCount() == 0) {
        QMessageBox::information(this, "Результаты", "Сигнатура не найдена");
        delete model;
        return;
    }

    showSearchResults(model);
}

void MainWindow::searchInDirectory(const QString& path, const QByteArray& signature, QStandardItemModel* model) {
    QDir dir(path);

    foreach (const QFileInfo &entry, dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (entry.isDir()) {
            searchInDirectory(entry.absoluteFilePath(), signature, model);
        } else {
            searchInFile(entry.absoluteFilePath(), signature, model);
        }
    }
}

void MainWindow::searchInFile(const QString& filePath, const QByteArray& signature, QStandardItemModel* model) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    int offset = 0;
    while ((offset = data.indexOf(signature, offset)) != -1) {
        QList<QStandardItem*> row;

        // Отображаем "чистое" имя файла (без номера)
        QString displayName = QFileInfo(filePath).fileName().replace(QRegularExpression("^\\d+\\s*"), "");

        row << new QStandardItem(displayName);
        row << new QStandardItem(QString("0x%1").arg(offset, 0, 16));
        row << new QStandardItem(filePath);
        model->appendRow(row);

        offset += signature.size();
    }
}


void MainWindow::jumpToOffset(const QString &path, int offset, const QByteArray &signature) {
    // 1. Проверяем существование файла
    if (!QFile::exists(path)) {
        QMessageBox::warning(this, "Ошибка", QString("Файл не существует:\n%1").arg(path));
        return;
    }

    // 2. Находим и выделяем файл в дереве
    QModelIndex index = dirModel->index(path);
    if (index.isValid()) {
        treeView->setCurrentIndex(index);
        treeView->scrollTo(index);
    }

    // 3. Загружаем данные
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    hexEdit->setData(file.readAll());
    file.close();

    // 4. Устанавливаем позицию (версия для QHexEdit с одним аргументом)
    hexEdit->setCursorPosition(offset * 2);
    hexEdit->ensureVisible();

    // 5. Переключаем в HEX-режим
    if (!viewModeAction->isChecked()) {
        viewModeAction->setChecked(true);
        toggleViewMode();
    }

    statusBar()->showMessage(QString("%1 [0x%2]").arg(path).arg(offset, 0, 16));
}

void MainWindow::showSearchResults(QStandardItemModel* model) {
    QDialog dialog(this);
    dialog.setWindowTitle("Результаты поиска");
    dialog.resize(800, 600);

    QVBoxLayout layout(&dialog);

    QTreeView *view = new QTreeView(&dialog);
    view->setModel(model);
    view->setSortingEnabled(true);
    view->sortByColumn(0, Qt::AscendingOrder);

    QPushButton *btn = new QPushButton("Перейти", &dialog);
    connect(btn, &QPushButton::clicked, [this, &dialog, model, view]() {
        QModelIndex current = view->currentIndex();
        if (current.isValid()) {
            QString filePath = model->item(current.row(), 2)->text();
            bool ok;
            int offset = model->item(current.row(), 1)->text().toInt(&ok, 16);
            if (ok) {
                QByteArray sig = model->item(current.row(), 0)->data(Qt::UserRole).toByteArray();
                this->jumpToOffset(filePath, offset, sig);
                dialog.accept();
            }
        }
    });

    layout.addWidget(view);
    layout.addWidget(btn);
    dialog.exec();
}

QString MainWindow::findContainingFile(const QString& binPath, int offset) {
    QString reportPath = binPath + ".report.txt";
    QFile reportFile(reportPath);
    if (!reportFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return binPath;
    }

    QTextStream in(&reportFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split('|');
        if (parts.size() < 4) continue;

        bool ok;
        int base = parts[2].trimmed().toInt(&ok, 16);
        if (!ok) continue;

        int size = parts[3].trimmed().toInt(&ok, 16);
        if (!ok) continue;

        if (offset >= base && offset < base + size) {
            QString name = parts.last().trimmed().split('|').first();
            QString dumpPath = binPath + ".dump/" + name;
            if (QFile::exists(dumpPath)) {
                return dumpPath;
            }
        }
    }

    return binPath;
}
