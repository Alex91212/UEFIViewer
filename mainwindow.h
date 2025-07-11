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
    void findSignature();
    void recvFromUser(const QString &text);
    void jumpToOffset(const QString &filePath, int offset, const QByteArray &signature = QByteArray());


private:
    void runUefiExtract(const QString &filePath);
    void setupFileTree(const QString &dumpDir);
    bool isTextFile(const QString &filePath);
    QByteArray ParseHexSignature(const QString &signature);
    void findSignatureInFile(const QString& filePath, const QByteArray& signature);
    QString findContainingFile(const QString& binPath, int offset);
    void showSearchResults(QStandardItemModel* model);
    void searchInDirectory(const QString& path, const QByteArray& signature, QStandardItemModel* model);
    void searchInFile(const QString& filePath, const QByteArray& signature, QStandardItemModel* model);


    QFileSystemModel *dirModel;
    QTreeView *treeView;
    QTextEdit *textEdit;
    QHexEdit *hexEdit;        // HEX-редактор
    QStackedWidget *stacked;  // Переключатель между текстом и HEX
    QSplitter *splitter;
    QAction *viewModeAction;  // Действие для переключения режима

    QString sign;

    QByteArray currentSignature;

};

#endif // MAINWINDOW_H
