#include "signrequest.h"
#include "ui_signrequest.h"

signRequest::signRequest(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::signRequest)
{
    ui->setupUi(this);
}

signRequest::~signRequest()
{
    delete ui;
}

void signRequest::on_find_button_clicked()
{
    QString signature_to_find = ui->sign_to_find->text();
    //qDebug() << signature_to_find;

    emit SignPassed(signature_to_find);
    accept();
}

