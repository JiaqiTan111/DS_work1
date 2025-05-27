#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "consumer.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::importCSV);

    connect(ui->pushButtonAdd, &QPushButton::clicked, this, &MainWindow::addFlight);
    connect(ui->pushButtonEdit, &QPushButton::clicked, this, &MainWindow::editFlight);
    connect(ui->pushButtonDelete, &QPushButton::clicked, this, &MainWindow::deleteFlight);
    connect(ui->pushButtonSearch, &QPushButton::clicked, this, &MainWindow::searchFlight);
    connect(ui->pushButtonReset, &QPushButton::clicked, this, &MainWindow::resetTable);
    connect(ui->pushButtonSave, &QPushButton::clicked, this, &MainWindow::saveToFile);

    connect(ui->pushButtonUpdateStatus, &QPushButton::clicked, this, &MainWindow::updateFlightStatus);
    connect(ui->pushButtonFindTransfer, &QPushButton::clicked, this, &MainWindow::findTransferFlights);

     connect(ui->pushButtonLoadConsumer, &QPushButton::clicked, this, &MainWindow::loadConsumer);

    ui->table_system->setColumnCount(8);
    QStringList headers={"Flight_Number","Airline","Departure_Time","Arrival_Time","Departure_City","Arrival_City","Available_Seats","Price"};
    ui->table_system->setHorizontalHeaderLabels(headers);

    // 创建 Consumer 对象
    Consumer *consumer = new Consumer(this);
    consumer->show();
    addConsumer(consumer);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::importCSV()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open CSV File"), "", tr("CSV Files (*.csv)"));
    if (fileName.isEmpty())
        return;

    currentFilePath = fileName; // 保存当前文件路径

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
        return;
    }

    QTextStream in(&file);
    ui->table_system->setRowCount(0); // Clear the table
    allFlights.clear(); // 清空所有航班信息

    int row = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(',');// 将每行数据按逗号分割为 QStringList

        allFlights.append(fields); // 存储所有航班信息

        ui->table_system->insertRow(row);
        for (int col = 0; col < fields.size(); ++col)
        {
            ui->table_system->setItem(row, col, new QTableWidgetItem(fields[col]));
        }
        ++row;
    }

    file.close();
}

QDialog* MainWindow::createFlightDialog(const QStringList &data)
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Flight Information"));

    QFormLayout *formLayout = new QFormLayout(dialog);

    QLineEdit *lineEditFlightNumber = new QLineEdit(dialog);
    QLineEdit *lineEditAirline = new QLineEdit(dialog);
    QLineEdit *lineEditDepartureTime = new QLineEdit(dialog);
    QLineEdit *lineEditArrivalTime = new QLineEdit(dialog);
    QLineEdit *lineEditDepartureCity = new QLineEdit(dialog);
    QLineEdit *lineEditArrivalCity = new QLineEdit(dialog);
    QSpinBox *spinBoxAvailableSeats = new QSpinBox(dialog);
    QDoubleSpinBox *doubleSpinBoxPrice = new QDoubleSpinBox(dialog);

    // 设置范围
    spinBoxAvailableSeats->setRange(0, 10000); // 可用座位数范围
    doubleSpinBoxPrice->setRange(0, 100000);  // 价格范围

    formLayout->addRow(tr("Flight Number:"), lineEditFlightNumber);
    formLayout->addRow(tr("Airline:"), lineEditAirline);
    formLayout->addRow(tr("Departure Time:"), lineEditDepartureTime);
    formLayout->addRow(tr("Arrival Time:"), lineEditArrivalTime);
    formLayout->addRow(tr("Departure City:"), lineEditDepartureCity);
    formLayout->addRow(tr("Arrival City:"), lineEditArrivalCity);
    formLayout->addRow(tr("Available Seats:"), spinBoxAvailableSeats);
    formLayout->addRow(tr("Price:"), doubleSpinBoxPrice);


    QPushButton *buttonOk = new QPushButton(tr("OK"), dialog);
    QPushButton *buttonCancel = new QPushButton(tr("Cancel"), dialog);

    formLayout->addRow(buttonOk, buttonCancel);

    if (!data.isEmpty()) {
        lineEditFlightNumber->setText(data[0]);
        lineEditAirline->setText(data[1]);
        lineEditDepartureTime->setText(data[2]);
        lineEditArrivalTime->setText(data[3]);
        lineEditDepartureCity->setText(data[4]);
        lineEditArrivalCity->setText(data[5]);
        spinBoxAvailableSeats->setValue(data[6].toInt());
        doubleSpinBoxPrice->setValue(data[7].toDouble());

    }

    connect(buttonOk, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(buttonCancel, &QPushButton::clicked, dialog, &QDialog::reject);

    return dialog;
}

void MainWindow::addFlight()
{
    QDialog *dialog = createFlightDialog();
    if (dialog->exec() == QDialog::Accepted) {
        QFormLayout *formLayout = qobject_cast<QFormLayout*>(dialog->layout());
        QStringList data;
        for (int i = 0; i < formLayout->rowCount() - 1; ++i) {
            QLineEdit *lineEdit = qobject_cast<QLineEdit*>(formLayout->itemAt(i, QFormLayout::FieldRole)->widget());
            if (lineEdit) {
                data << lineEdit->text();
            } else {
                QSpinBox *spinBox = qobject_cast<QSpinBox*>(formLayout->itemAt(i, QFormLayout::FieldRole)->widget());
                if (spinBox) {
                    data << QString::number(spinBox->value());
                } else {
                    QDoubleSpinBox *doubleSpinBox = qobject_cast<QDoubleSpinBox*>(formLayout->itemAt(i, QFormLayout::FieldRole)->widget());
                    if (doubleSpinBox) {
                        data << QString::number(doubleSpinBox->value());
                    }
                }
            }
        }

        allFlights.append(data); // 添加到所有航班信息中

        int row = ui->table_system->rowCount();
        ui->table_system->insertRow(row);
        for (int col = 0; col < data.size(); ++col) {
            ui->table_system->setItem(row, col, new QTableWidgetItem(data[col]));
        }
    }
    delete dialog;
}

void MainWindow::editFlight()
{
    int row = ui->table_system->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a flight to edit."));
        return;
    }

    QStringList data;
    for (int col = 0; col < ui->table_system->columnCount(); ++col) {
        data << ui->table_system->item(row, col)->text();
    }

    QDialog *dialog = createFlightDialog(data);
    if (dialog->exec() == QDialog::Accepted) {
        QFormLayout *formLayout = qobject_cast<QFormLayout*>(dialog->layout());
        QStringList newData;
        for (int i = 0; i < formLayout->rowCount() - 1; ++i) {
            QLineEdit *lineEdit = qobject_cast<QLineEdit*>(formLayout->itemAt(i, QFormLayout::FieldRole)->widget());
            if (lineEdit) {
                newData << lineEdit->text();
            } else {
                QSpinBox *spinBox = qobject_cast<QSpinBox*>(formLayout->itemAt(i, QFormLayout::FieldRole)->widget());
                if (spinBox) {
                    newData << QString::number(spinBox->value());
                } else {
                    QDoubleSpinBox *doubleSpinBox = qobject_cast<QDoubleSpinBox*>(formLayout->itemAt(i, QFormLayout::FieldRole)->widget());
                    if (doubleSpinBox) {
                        newData << QString::number(doubleSpinBox->value());
                    }
                }
            }
        }

        // 更新所有航班信息
        allFlights[row] = newData;

        // 更新表格显示
        for (int col = 0; col < newData.size(); ++col) {
            ui->table_system->setItem(row, col, new QTableWidgetItem(newData[col]));
        }
    }
    delete dialog;
}

void MainWindow::deleteFlight()
{
    int row = ui->table_system->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a flight to delete."));
        return;
    }
    allFlights.removeAt(row); // 从所有航班信息中移除
    ui->table_system->removeRow(row);
}

void MainWindow::searchFlight()
{
    QDialog *searchDialog = new QDialog(this);
    searchDialog->setWindowTitle(tr("Search Flight"));

    QFormLayout *formLayout = new QFormLayout(searchDialog);

    // 添加多个查询条件输入框
    QLineEdit *lineEditFlightNumber = new QLineEdit(searchDialog);
    QLineEdit *lineEditAirline = new QLineEdit(searchDialog);
    QLineEdit *lineEditDepartureTime = new QLineEdit(searchDialog);
    QLineEdit *lineEditArrivalTime = new QLineEdit(searchDialog);
    QLineEdit *lineEditDepartureCity = new QLineEdit(searchDialog);
    QLineEdit *lineEditArrivalCity = new QLineEdit(searchDialog);

    formLayout->addRow(tr("Flight Number:"), lineEditFlightNumber);
    formLayout->addRow(tr("Airline:"), lineEditAirline);
    formLayout->addRow(tr("Departure Time:"), lineEditDepartureTime);
    formLayout->addRow(tr("Arrival Time:"), lineEditArrivalTime);
    formLayout->addRow(tr("Departure City:"), lineEditDepartureCity);
    formLayout->addRow(tr("Arrival City:"), lineEditArrivalCity);

    QPushButton *buttonSearch = new QPushButton(tr("Search"), searchDialog);
    QPushButton *buttonCancel = new QPushButton(tr("Cancel"), searchDialog);

    formLayout->addRow(buttonSearch, buttonCancel);

    connect(buttonSearch, &QPushButton::clicked, searchDialog, &QDialog::accept);
    connect(buttonCancel, &QPushButton::clicked, searchDialog, &QDialog::reject);

    if (searchDialog->exec() == QDialog::Accepted) {
        // 获取用户输入的查询条件
        QString flightNumber = lineEditFlightNumber->text().trimmed();
        QString airline = lineEditAirline->text().trimmed();
        QString departureTime = lineEditDepartureTime->text().trimmed();
        QString arrivalTime = lineEditArrivalTime->text().trimmed();
        QString departureCity = lineEditDepartureCity->text().trimmed();
        QString arrivalCity = lineEditArrivalCity->text().trimmed();

        ui->table_system->setRowCount(0); // 清空表格

        // 遍历所有航班，筛选符合条件的航班
        for (const QStringList &flight : allFlights) {
            bool match = true;

            // 检查每个条件是否匹配
            if (!flightNumber.isEmpty() && flight[0] != flightNumber) {
                match = false;
            }
            if (!airline.isEmpty() && flight[1] != airline) {
                match = false;
            }
            if (!departureTime.isEmpty() && flight[2] != departureTime) {
                match = false;
            }
            if (!arrivalTime.isEmpty() && flight[3] != arrivalTime) {
                match = false;
            }
            if (!departureCity.isEmpty() && flight[4] != departureCity) {
                match = false;
            }
            if (!arrivalCity.isEmpty() && flight[5] != arrivalCity) {
                match = false;
            }

            // 如果所有条件都匹配，则显示该航班
            if (match) {
                int row = ui->table_system->rowCount();
                ui->table_system->insertRow(row);
                for (int col = 0; col < flight.size(); ++col) {
                    ui->table_system->setItem(row, col, new QTableWidgetItem(flight[col]));
                }
            }
        }

        // 如果没有找到符合条件的航班，显示提示信息
        if (ui->table_system->rowCount() == 0) {
            QMessageBox::information(this, tr("Search Result"), tr("No flights found."));
        }
    }
    delete searchDialog;
}

void MainWindow::resetTable()
{
    ui->table_system->setRowCount(0); // 清空表格
    for (const QStringList &flight : allFlights) {
        int row = ui->table_system->rowCount();
        ui->table_system->insertRow(row);
        for (int col = 0; col < flight.size(); ++col) {
            ui->table_system->setItem(row, col, new QTableWidgetItem(flight[col]));
        }
    }
}

void MainWindow::saveToFile()
{
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No file is opened."));
        return;
    }

    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for saving."));
        return;
    }

    QTextStream out(&file);
    for (const QStringList &flight : allFlights) {
        out << flight.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("File saved successfully."));
}



void MainWindow::updateFlightStatus()
{
    int row = ui->table_system->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a flight to update status."));
        return;
    }

    QString flightNumber = ui->table_system->item(row, 0)->text();
    QStringList statusOptions = {"On Time", "Delayed", "Cancelled"};
    QString status = QInputDialog::getItem(this, tr("Update Flight Status"), tr("Select Status:"), statusOptions, 0, false);

    if (!status.isEmpty()) {
        flightStatusMap[flightNumber] = status;

        // 广播航班状态变化
        QString message = QString("Flight %1 status updated to: %2").arg(flightNumber).arg(status);
        emit broadcastMessage(message);

        // 如果航班状态是 Delayed 或 Cancelled，推荐替代方案
        if (status == "Cancelled") {
            QString departureCity = ui->table_system->item(row, 4)->text();
            QString arrivalCity = ui->table_system->item(row, 5)->text();

            // 查找直达航班
            QStringList directFlight;
            for (const QStringList &flight : allFlights) {
                if (flight[4] == departureCity && flight[5] == arrivalCity && flightStatusMap[flight[0]] != "Cancelled") {
                    directFlight = flight;
                    break;
                }
            }

            // 如果找到直达航班，直接推荐
            if (!directFlight.isEmpty()) {
                QString directMessage = "Here is the best direct flight option:\n"
                                       "Flight Number: " + directFlight[0] + "\n"
                                       "Airline: " + directFlight[1] + "\n"
                                       "Departure Time: " + directFlight[2] + "\n"
                                       "Arrival Time: " + directFlight[3] + "\n"
                                       "Price: " + directFlight[7] + "\n";
                emit broadcastMessage(directMessage);
            } else {
                // 如果没有直达航班，查找转机方案
                QList<QPair<QStringList, QStringList>> transferOptions;

                for (const QStringList &flight1 : allFlights) {
                    QString flightNumber1 = flight1[0];
                    if (flight1[4] == departureCity && flightStatusMap[flightNumber1] != "Cancelled") {
                        for (const QStringList &flight2 : allFlights) {
                            QString flightNumber2 = flight2[0];
                            if (flight2[4] == flight1[5] && flight2[5] == arrivalCity && flightStatusMap[flightNumber2] != "Cancelled") {
                                // 检查时间：第二趟航班的起飞时间不能早于第一趟航班的到达时间
                                QDateTime flight1ArrivalTime = QDateTime::fromString(flight1[3], "yyyy-MM-dd hh:mm");
                                QDateTime flight2DepartureTime = QDateTime::fromString(flight2[2], "yyyy-MM-dd hh:mm");
                                if (flight2DepartureTime >= flight1ArrivalTime) {
                                    transferOptions.append(qMakePair(flight1, flight2));
                                }
                            }
                        }
                    }
                }

                if (transferOptions.isEmpty()) {
                    emit broadcastMessage("No alternative flights found.");
                } else {
                    // 找到最优转机方案（价格最低和时间最短）
                    QPair<QStringList, QStringList> minPriceOption;
                    QPair<QStringList, QStringList> minTimeOption;
                    double minPrice = std::numeric_limits<double>::max();
                    qint64 minTime = std::numeric_limits<qint64>::max();

                    for (const auto &option : transferOptions) {
                        double totalPrice = option.first[7].toDouble() + option.second[7].toDouble();
                        QDateTime flight1DepartureTime = QDateTime::fromString(option.first[2], "yyyy-MM-dd hh:mm");
                        QDateTime flight2ArrivalTime = QDateTime::fromString(option.second[3], "yyyy-MM-dd hh:mm");
                        qint64 totalTime = flight1DepartureTime.secsTo(flight2ArrivalTime);

                        if (totalPrice < minPrice) {
                            minPrice = totalPrice;
                            minPriceOption = option;
                        }
                        if (totalTime < minTime) {
                            minTime = totalTime;
                            minTimeOption = option;
                        }
                    }

                    // 构建推荐方案信息
                    QString alternativeMessage = "Here are the best alternative options:\n";

                    // 价格最优的方案
                    alternativeMessage += "Cheapest Option:\n"
                                          "First Flight: " + minPriceOption.first[0] + " (" + minPriceOption.first[4] + " -> " + minPriceOption.first[5] + ")\n"
                                          "Departure Time: " + minPriceOption.first[2] + "\n"
                                          "Arrival Time: " + minPriceOption.first[3] + "\n"
                                          "Price: " + minPriceOption.first[7] + "\n"
                                          "Second Flight: " + minPriceOption.second[0] + " (" + minPriceOption.second[4] + " -> " + minPriceOption.second[5] + ")\n"
                                          "Departure Time: " + minPriceOption.second[2] + "\n"
                                          "Arrival Time: " + minPriceOption.second[3] + "\n"
                                          "Price: " + minPriceOption.second[7] + "\n"
                                          "Total Price: " + QString::number(minPrice) + "\n\n";

                    // 时间最优的方案
                    alternativeMessage += "Fastest Option:\n"
                                          "First Flight: " + minTimeOption.first[0] + " (" + minTimeOption.first[4] + " -> " + minTimeOption.first[5] + ")\n"
                                          "Departure Time: " + minTimeOption.first[2] + "\n"
                                          "Arrival Time: " + minTimeOption.first[3] + "\n"
                                          "Second Flight: " + minTimeOption.second[0] + " (" + minTimeOption.second[4] + " -> " + minTimeOption.second[5] + ")\n"
                                          "Departure Time: " + minTimeOption.second[2] + "\n"
                                          "Arrival Time: " + minTimeOption.second[3] + "\n"
                                          "Total Time: " + QString::number(minTime / 60) + " minutes\n";

                    // 广播推荐方案
                    emit broadcastMessage(alternativeMessage);
                }
            }
        }
    }
}

//转机方案
void MainWindow::findTransferFlights()
{
    QString departureCity = QInputDialog::getText(this, tr("Departure City"), tr("Enter Departure City:"));
    QString arrivalCity = QInputDialog::getText(this, tr("Arrival City"), tr("Enter Arrival City:"));

    if (departureCity.isEmpty() || arrivalCity.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter both departure and arrival cities."));
        return;
    }

    // 查找转机方案
    QList<QPair<QStringList, QStringList>> transferOptions; // 存储转机方案（第一段航班，第二段航班）

    for (const QStringList &flight1 : allFlights) {
        QString flightNumber1 = flight1[0];
        if (flight1[4] == departureCity && flightStatusMap[flightNumber1] != "Cancelled") { // 第一段航班：出发城市匹配且状态正常
            for (const QStringList &flight2 : allFlights) {
                QString flightNumber2 = flight2[0];
                if (flight2[4] == flight1[5] && flight2[5] == arrivalCity && flightStatusMap[flightNumber2] != "Cancelled") { // 第二段航班：出发城市匹配第一段航班的目的地，且目的地匹配目标城市，状态正常
                    // 检查时间：第二趟航班的起飞时间不能早于第一趟航班的到达时间
                    QDateTime flight1ArrivalTime = QDateTime::fromString(flight1[3], "yyyy-MM-dd hh:mm");
                    QDateTime flight2DepartureTime = QDateTime::fromString(flight2[2], "yyyy-MM-dd hh:mm");
                    if (flight2DepartureTime >= flight1ArrivalTime) {
                        transferOptions.append(qMakePair(flight1, flight2));
                    }
                }
            }
        }
    }

    if (transferOptions.isEmpty()) {
        emit broadcastMessage("No transfer flights found from " + departureCity + " to " + arrivalCity + ".");
        return;
    }

    // 找到最优转机方案（价格最低和时间最短）
    QPair<QStringList, QStringList> minPriceOption;
    QPair<QStringList, QStringList> minTimeOption;
    double minPrice = std::numeric_limits<double>::max();
    qint64 minTime = std::numeric_limits<qint64>::max();

    for (const auto &option : transferOptions) {
        double totalPrice = option.first[7].toDouble() + option.second[7].toDouble();
        QDateTime flight1DepartureTime = QDateTime::fromString(option.first[2], "yyyy-MM-dd hh:mm");
        QDateTime flight2ArrivalTime = QDateTime::fromString(option.second[3], "yyyy-MM-dd hh:mm");
        qint64 totalTime = flight1DepartureTime.secsTo(flight2ArrivalTime);

        if (totalPrice < minPrice) {
            minPrice = totalPrice;
            minPriceOption = option;
        }
        if (totalTime < minTime) {
            minTime = totalTime;
            minTimeOption = option;
        }
    }

    // 构建推荐方案信息
    QString alternativeMessage = "Here are the best transfer options from " + departureCity + " to " + arrivalCity + ":\n";

    // 价格最优的方案
    alternativeMessage += "Cheapest Option:\n"
                          "First Flight: " + minPriceOption.first[0] + " (" + minPriceOption.first[4] + " -> " + minPriceOption.first[5] + ")\n"
                          "Departure Time: " + minPriceOption.first[2] + "\n"
                          "Arrival Time: " + minPriceOption.first[3] + "\n"
                          "Price: " + minPriceOption.first[7] + "\n"
                          "Second Flight: " + minPriceOption.second[0] + " (" + minPriceOption.second[4] + " -> " + minPriceOption.second[5] + ")\n"
                          "Departure Time: " + minPriceOption.second[2] + "\n"
                          "Arrival Time: " + minPriceOption.second[3] + "\n"
                          "Price: " + minPriceOption.second[7] + "\n"
                          "Total Price: " + QString::number(minPrice) + "\n\n";

    // 时间最优的方案
    alternativeMessage += "Fastest Option:\n"
                          "First Flight: " + minTimeOption.first[0] + " (" + minTimeOption.first[4] + " -> " + minTimeOption.first[5] + ")\n"
                          "Departure Time: " + minTimeOption.first[2] + "\n"
                          "Arrival Time: " + minTimeOption.first[3] + "\n"
                          "Second Flight: " + minTimeOption.second[0] + " (" + minTimeOption.second[4] + " -> " + minTimeOption.second[5] + ")\n"
                          "Departure Time: " + minTimeOption.second[2] + "\n"
                          "Arrival Time: " + minTimeOption.second[3] + "\n"
                          "Total Time: " + QString::number(minTime / 60) + " minutes\n";

    // 广播推荐方案
    emit broadcastMessage(alternativeMessage);
}

void MainWindow::loadConsumer()
{
    Consumer *consumer = new Consumer(this);
    consumer->show();
    addConsumer(consumer);
}

void MainWindow::addConsumer(Consumer *consumer)
{
    consumers.append(consumer);
    connect(this, &MainWindow::broadcastMessage, consumer, &Consumer::onBroadcastMessage);
}
QList<QStringList> MainWindow::getAllFlights() const
{
    return allFlights;
}

