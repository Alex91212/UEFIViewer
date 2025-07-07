#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QStackedWidget>
#include "qhexedit.h"  // Добавляем заголовок QHexEdit

class QTreeView;
class QSplitter;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void openUefiFile();
    void onFileSelected(const QModelIndex &index);
    void toggleViewMode();  // Новый слот для переключения режима

private:
    void runUefiExtract(const QString &filePath);
    void setupFileTree(const QString &dumpDir);
    bool isTextFile(const QString &filePath);

    QFileSystemModel *dirModel;
    QTreeView *treeView;
    QTextEdit *textEdit;
    QHexEdit *hexEdit;        // HEX-редактор
    QStackedWidget *stacked;  // Переключатель между текстом и HEX
    QSplitter *splitter;
    QAction *viewModeAction;  // Действие для переключения режима
};

#endif // MAINWINDOW_H
