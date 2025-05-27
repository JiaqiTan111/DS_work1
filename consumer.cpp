#include "consumer.h"
#include "ui_consumer.h" // 自动生成的 UI 头文件
#include "mainwindow.h"
#include <QDateTime>
#include <QLabel>
#include <QWidgetList>
#include <QListWidgetItem>

Consumer::Consumer(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), ui(new Ui::Consumer), mainWindow(mainWindow)
{
    ui->setupUi(this); // 加载 UI 文件

    // 初始化航班信息表格
    ui->tableFlights->setColumnCount(8);
    QStringList headers = {"Flight_Number", "Airline", "Departure_Time", "Arrival_Time", "Departure_City", "Arrival_City", "Available_Seats", "Price"};
    ui->tableFlights->setHorizontalHeaderLabels(headers);



    // 连接按钮的点击事件到槽函数
    connect(ui->pushButtonSearch, &QPushButton::clicked, this, &Consumer::searchFlights);
    connect(ui->pushButtonBook, &QPushButton::clicked, this, &Consumer::bookFlight);
    connect(ui->pushButtonViewBookings, &QPushButton::clicked, this, &Consumer::viewBookings);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &Consumer::cancelBooking);

    // 加载航班信息
    for (const QStringList &flight : mainWindow->getAllFlights()) {
        insertFlightToTable(flight);
    }
}

Consumer::~Consumer()
{
    delete ui;
}

void Consumer::onBroadcastMessage(const QString &message)
{
    ui->broadcast->append(message);
}

void Consumer::insertFlightToTable(const QStringList &flight)
{
    int row = ui->tableFlights->rowCount();
    ui->tableFlights->insertRow(row);
    for (int col = 0; col < flight.size(); ++col) {
        ui->tableFlights->setItem(row, col, new QTableWidgetItem(flight[col]));
    }
}

void Consumer::searchFlights()
{
    // 创建一个搜索对话框
    QDialog searchDialog(this);
    searchDialog.setWindowTitle("Search Flights");

    QFormLayout formLayout(&searchDialog);

    // 添加搜索条件输入框
    QLineEdit lineEditFlightNumber(&searchDialog);
    QLineEdit lineEditAirline(&searchDialog);
    QLineEdit lineEditDepartureCity(&searchDialog);
    QLineEdit lineEditArrivalCity(&searchDialog);

    // 添加排序选项
    QComboBox comboBoxSort(&searchDialog);
    comboBoxSort.addItem("No Sort");
    comboBoxSort.addItem("Sort by Price (Low to High)");
    comboBoxSort.addItem("Sort by Price (High to Low)");
    comboBoxSort.addItem("Sort by Departure Time (Earliest to Latest)");
    comboBoxSort.addItem("Sort by Departure Time (Latest to Earliest)");

    formLayout.addRow("Flight Number:", &lineEditFlightNumber);
    formLayout.addRow("Airline:", &lineEditAirline);
    formLayout.addRow("Departure City:", &lineEditDepartureCity);
    formLayout.addRow("Arrival City:", &lineEditArrivalCity);
    formLayout.addRow("Sort By:", &comboBoxSort);

    QPushButton buttonSearch("Search", &searchDialog);
    QPushButton buttonCancel("Cancel", &searchDialog);

    formLayout.addRow(&buttonSearch, &buttonCancel);

    // 连接按钮的点击事件
    connect(&buttonSearch, &QPushButton::clicked, &searchDialog, &QDialog::accept);
    connect(&buttonCancel, &QPushButton::clicked, &searchDialog, &QDialog::reject);

    // 显示对话框并等待用户输入
    if (searchDialog.exec() == QDialog::Accepted) {
        // 获取用户输入的搜索条件
        QString flightNumber = lineEditFlightNumber.text().trimmed();
        QString airline = lineEditAirline.text().trimmed();
        QString departureCity = lineEditDepartureCity.text().trimmed();
        QString arrivalCity = lineEditArrivalCity.text().trimmed();
        QString sortOption = comboBoxSort.currentText();

        // 清空当前航班信息表格
        ui->tableFlights->setRowCount(0);

        // 遍历所有航班，筛选符合条件的航班
        QList<QStringList> filteredFlights;
        for (const QStringList &flight : mainWindow->getAllFlights()) {
            bool match = true;

            // 检查每个条件是否匹配
            if (!flightNumber.isEmpty() && flight[0] != flightNumber) {
                match = false;
            }
            if (!airline.isEmpty() && flight[1] != airline) {
                match = false;
            }
            if (!departureCity.isEmpty() && flight[4] != departureCity) {
                match = false;
            }
            if (!arrivalCity.isEmpty() && flight[5] != arrivalCity) {
                match = false;
            }

            // 如果所有条件都匹配，则添加到筛选结果中
            if (match) {
                filteredFlights.append(flight);
            }
        }

        // 根据排序选项对筛选结果进行排序
        if (sortOption == "Sort by Price (Low to High)") {
            std::sort(filteredFlights.begin(), filteredFlights.end(), [](const QStringList &a, const QStringList &b) {
                return a[7].toDouble() < b[7].toDouble();
            });
        } else if (sortOption == "Sort by Price (High to Low)") {
            std::sort(filteredFlights.begin(), filteredFlights.end(), [](const QStringList &a, const QStringList &b) {
                return a[7].toDouble() > b[7].toDouble();
            });
        } else if (sortOption == "Sort by Departure Time (Earliest to Latest)") {
            std::sort(filteredFlights.begin(), filteredFlights.end(), [](const QStringList &a, const QStringList &b) {
                QDateTime timeA = QDateTime::fromString(a[2], "yyyy-MM-dd hh:mm");
                QDateTime timeB = QDateTime::fromString(b[2], "yyyy-MM-dd hh:mm");
                return timeA < timeB;
            });
        } else if (sortOption == "Sort by Departure Time (Latest to Earliest)") {
            std::sort(filteredFlights.begin(), filteredFlights.end(), [](const QStringList &a, const QStringList &b) {
                QDateTime timeA = QDateTime::fromString(a[2], "yyyy-MM-dd hh:mm");
                QDateTime timeB = QDateTime::fromString(b[2], "yyyy-MM-dd hh:mm");
                return timeA > timeB;
            });
        }

        // 将筛选和排序后的航班显示在表格中
        for (const QStringList &flight : filteredFlights) {
            insertFlightToTable(flight);
        }

        // 如果没有找到符合条件的航班，显示提示信息
        if (ui->tableFlights->rowCount() == 0) {
            QMessageBox::information(this, "Search Result", "No flights found.");
        }
    }
}

void Consumer::bookFlight()
{
    int row = ui->tableFlights->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Warning", "Please select a flight to book.");
        return;
    }

    QString flightNumber = ui->tableFlights->item(row, 0)->text();
    QString airline = ui->tableFlights->item(row, 1)->text();
    QString departureTime = ui->tableFlights->item(row, 2)->text();
    QString arrivalTime = ui->tableFlights->item(row, 3)->text();
    QString departureCity = ui->tableFlights->item(row, 4)->text();
    QString arrivalCity = ui->tableFlights->item(row, 5)->text();
    int availableSeats = ui->tableFlights->item(row, 6)->text().toInt();

    // 添加航班号到预订记录
    bookings.append(flightNumber);

    QMessageBox::information(this, "Success", "Flight booked successfully!");
}
void Consumer::viewBookings()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("My Bookings");

    QTableWidget *tableBookings = new QTableWidget(dialog);
    tableBookings->setColumnCount(8);
    QStringList headers = {"Flight_Number", "Airline", "Departure_Time", "Arrival_Time", "Departure_City", "Arrival_City", "Available_Seats", "Price"};
    tableBookings->setHorizontalHeaderLabels(headers);

    for (const QString &booking : bookings) {
        for (const QStringList &flight : mainWindow->getAllFlights()) {
            if (flight[0] == booking) {
                int row = tableBookings->rowCount();
                tableBookings->insertRow(row);
                for (int col = 0; col < flight.size(); ++col) {
                    tableBookings->setItem(row, col, new QTableWidgetItem(flight[col]));
                }
            }
        }
    }

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(tableBookings);
    dialog->exec();
}

void Consumer::cancelBooking()
{
    if (bookings.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No bookings to cancel.");
        return;
    }

    // 如果只有一张预订，直接取消
    if (bookings.size() == 1) {
        bookings.removeFirst();
        QMessageBox::information(this, "Success", "Booking canceled successfully!");
        return;
    }

    // 如果有多张预订，提供选择功能
    QDialog dialog(this);
    dialog.setWindowTitle("Cancel Booking");

    QVBoxLayout layout(&dialog);

    QLabel label("Select a booking to cancel:", &dialog);
    layout.addWidget(&label);

    QListWidget listWidget(&dialog);
    for (const QString &booking : bookings) {
        listWidget.addItem(booking);
    }
    layout.addWidget(&listWidget);

    QPushButton buttonCancel("Cancel", &dialog);
    QPushButton buttonConfirm("Confirm", &dialog);

    QHBoxLayout buttonLayout;
    buttonLayout.addWidget(&buttonCancel);
    buttonLayout.addWidget(&buttonConfirm);
    layout.addLayout(&buttonLayout);

    // 连接按钮的点击事件
    connect(&buttonCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(&buttonConfirm, &QPushButton::clicked, &dialog, &QDialog::accept);

    // 显示对话框并等待用户选择
    if (dialog.exec() == QDialog::Accepted) {
        QListWidgetItem *selectedItem = listWidget.currentItem();
        if (selectedItem) {
            QString flightNumber = selectedItem->text();
            bookings.removeOne(flightNumber);
            QMessageBox::information(this, "Success", "Booking canceled successfully!");
        }
    }
}

