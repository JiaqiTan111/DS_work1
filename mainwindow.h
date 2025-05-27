#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QComboBox>
#include <QTableWidget>
#include <QList>

class Consumer; // 前向声明

namespace Ui { class MainWindow; }

struct FlightRecord {
    QString flightNumber;      // 航班号
    QString airline;          // 航空公司
    QString departureTime;    // 起飞时间
    QString arrivalTime;      // 到达时间
    QString departureCity;    // 出发城市
    QString arrivalCity;      // 到达城市
    int availableSeats;      // 可用座位数
    double price;             // 价格
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QList<QStringList> getAllFlights() const; // 返回所有航班信息
    void addConsumer(Consumer *consumer); // 添加 Consumer 对象
    void loadConsumer();

signals:
    void broadcastMessage(const QString &message); // 广播消息信号

private slots:
    void importCSV(); // 槽函数，用于导入CSV文件
    void addFlight(); // 添加航班
    void editFlight(); // 编辑航班
    void deleteFlight(); // 删除航班
    //void searchFlight(); // 查找航班
    void resetTable(); // 重置表格，显示所有航班信息
    void saveToFile(); // 保存到文件

    //void updateFlightStatus(); // 更新航班状态
    //void findTransferFlights(); // 查找转机方案


public slots:
    void searchFlight(); // 查找航班
    void updateFlightStatus(); // 更新航班状态
    void findTransferFlights(); // 查找转机方案


private:
    Ui::MainWindow *ui;
    QDialog* createFlightDialog(const QStringList &data = QStringList()); // 创建临时窗口
    QList<QStringList> allFlights; // 存储所有航班信息
    QString currentFilePath; // 当前文件路径
    QMap<QString, QString> flightStatusMap; // 存储航班状态
    QList<Consumer*> consumers; // 存储所有 Consumer 对象

    QList<FlightRecord*> flightRecords; // 存储所有航班记录

};
#endif // MAINWINDOW_H
