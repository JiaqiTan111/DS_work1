#include "customer.h"
#include "ui_customer.h"

customer::customer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::customer)
{
    ui->setupUi(this);
}

customer::~customer()
{
    delete ui;
}
