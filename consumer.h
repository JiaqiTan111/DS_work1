#ifndef CONSUMER_H
#define CONSUMER_H

#include <QWidget>
#include <QTableWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

namespace Ui {
class Consumer;
}

class MainWindow; // 前向声明

class Consumer : public QWidget
{
    Q_OBJECT

public:
    explicit Consumer(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~Consumer();

public slots:
    void onBroadcastMessage(const QString &message); // 处理广播消息

private slots:
    void searchFlights(); // 搜索航班
    void bookFlight(); // 订票
    void viewBookings(); // 查看预订记录
    void cancelBooking(); // 取消预订



private:
    Ui::Consumer *ui; // UI 指针
    MainWindow *mainWindow; // 指向主窗口的指针
    QList<QString> bookings; // 用户的预订记录

    void insertFlightToTable(const QStringList &flight); // 将航班插入表格
};

#endif // CONSUMER_H
