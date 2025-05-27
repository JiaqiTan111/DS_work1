#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QWidget>

namespace Ui {
class customer;
}

class customer : public QWidget
{
    Q_OBJECT

public:
    explicit customer(QWidget *parent = nullptr);
    ~customer();

private:
    Ui::customer *ui;
};

#endif // CUSTOMER_H
