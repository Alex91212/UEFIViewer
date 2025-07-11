#ifndef SIGNREQUEST_H
#define SIGNREQUEST_H

#include <QDialog>

namespace Ui {
class signRequest;
}

class signRequest : public QDialog
{
    Q_OBJECT

public:
    explicit signRequest(QWidget *parent = nullptr);
    ~signRequest();

signals:
    void SignPassed(const QString &sign);

private slots:
    void on_find_button_clicked();

private:
    Ui::signRequest *ui;
};

#endif // SIGNREQUEST_H
