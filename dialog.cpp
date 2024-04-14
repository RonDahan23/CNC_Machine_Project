#include "dialog.h"
#include "ui_dialog.h"
#include <cstring>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <QThread>


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    connect(ui->progressBar, SIGNAL(valueChanged(int)), this, SLOT(on_progressBar_valueChanged(int)));




    arduino = new QSerialPort;
    arduino_is_available = false;
    arduino_port_name = "";


    //view port on your system
    qDebug() << "Number of Ports: " << QSerialPortInfo::availablePorts().length();
    ui->Logs->setText("Number of Ports: " + QString::number(QSerialPortInfo::availablePorts().length()));
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
        {
            qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier();
            qDebug() << "Product ID " << serialPortInfo.productIdentifier();
            ui->Logs->append("Vendor ID: " + QString::number(serialPortInfo.vendorIdentifier()));
            ui->Logs->append("Product ID: " + QString::number(serialPortInfo.productIdentifier()));
        }
    }

    //check which port the arduino is on
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
        {
            if(serialPortInfo.vendorIdentifier() == arduino_uno_vendorID)
            {
                if(serialPortInfo.productIdentifier() == arduino_uno_productID)
                {
                    arduino_port_name = serialPortInfo.portName();
                    arduino_is_available = true;
                    qDebug() << "Port Availble!";
                    ui->Logs->append("Port Available!");
                }
            }
        }
    }


    if (arduino_is_available)
    {
        arduino->setPortName(arduino_port_name);
        if (arduino->open(QSerialPort::ReadWrite))  // Open in ReadWrite mode
        {
            qDebug() << "Serial port opened successfully for reading and writing.";
            ui->Logs->append("Serial port opened successfully for reading and writing.");
            QThread::msleep(100);
        }
        else
        {
            qDebug() << "Error opening serial port for reading and writing: " << arduino->errorString();
            ui->Logs->append("Error opening serial port for reading and writing: " + arduino->errorString());
        }
        arduino->setBaudRate(QSerialPort:: Baud9600);
        arduino->setDataBits(QSerialPort:: Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
    }
    else
    {
        QMessageBox::warning(this, "Port error", "Couldn't find arduino");
        ui->Logs->append("Port Error, Couldn't find arduino");
    }
    connect(arduino, SIGNAL(readyRead()), this, SLOT(ReadFromArduino()));
}

Dialog::~Dialog()
{
    if(arduino->isOpen())
    {
        qDebug() << "closing port";
        ui->Logs->append("closing port");
        arduino->close();
    }
    delete ui;
}

QString Dialog::ReadFromArduino()
{
    QByteArray responseData = arduino->readAll();
    QThread::msleep(100);
    QString response(responseData);

    qDebug() << "Received response from Arduino: " << response;

    arduino->flush();

    return response;
}



void Dialog::WriteToArduino(QString command)
{
    if (arduino->isOpen())
    {
        qint64 bytesWritten = arduino->write(command.toLatin1() + '\n');
        if (bytesWritten == -1)
        {
            qDebug() << "Error writing to serial port: " << arduino->errorString();
        }
        else
        {
            qDebug() << "Bytes written successfully: " << bytesWritten;
        }

        arduino->flush();
    }
    else
    {
        qDebug() << "Serial port is not open!";
        ui->Logs->append("Serial port is not open!");

    }
}


void Dialog::HPGL_to_Stack(const QString& HPGL) {
    std::regex PenUp_Pattern("PU([0-9]+),([0-9]+)");
    std::regex PenDown_Pattern("PD([0-9]+),([0-9]+)");

    QStringList lines = split_to_commands(HPGL);

    for(const QString& line : lines) {
        std::smatch match;
        std::string lineStr = line.toStdString();

        if (std::regex_search(lineStr, match, PenUp_Pattern)) {
            if (match.size() == 3) {
                int x = std::stod(match[1]);
                int y = std::stod(match[2]);
                QString command = "PU," + QString::number(x) + "," + QString::number(y) + '\n';
                commandStack.push(command);
                PrograssBarCounter++;
            }
        } else if (std::regex_search(lineStr, match, PenDown_Pattern)) {
            if (match.size() == 3) {
                int x = std::stod(match[1]);
                int y = std::stod(match[2]);
                QString command = "PD," + QString::number(x) + "," + QString::number(y) + '\n';
                commandStack.push(command);
                PrograssBarCounter++;
            }
        }
    }
    ui->progressBar->setMaximum(PrograssBarCounter);
    commandStack = Reverse_Stack(commandStack);

}

std::stack<QString> Dialog::Reverse_Stack(std::stack<QString> commandStack)
{
    std::stack<QString> reversedStack;

    while (!commandStack.empty())
    {
        reversedStack.push(commandStack.top());
        commandStack.pop();
    }

    return reversedStack;
}

QStringList Dialog::split_to_commands(const QString& str)
{
    std::string str_std = str.toStdString(); // Convert QString to std::string
    std::istringstream ss(str_std); // Use std::string for std::istringstream
    std::string command;
    std::ostringstream result;

    QStringList commandList; // Create QStringList to store commands

    // Split the input string by semicolons
    while (std::getline(ss, command, ';')) {
        // If the command starts with PD (pen down)
        if (command.substr(0, 2) == "PD") {
            std::string args = command.substr(2);
            std::istringstream arg_stream(args);
            std::string arg;
            std::vector<std::string> arg_list;

            // Split the arguments by comma
            while (std::getline(arg_stream, arg, ',')) {
                arg_list.push_back(arg);
            }

            // If there are more than 2 arguments, split them into pairs
            if (arg_list.size() > 2) {
                for (size_t i = 0; i < arg_list.size(); i += 2) {
                    // Form a PD command with 2 arguments
                    result << "PD" << arg_list[i] << "," << arg_list[i + 1] << ";";
                }
            } else {
                // Otherwise, append the original command
                result << command << ";";
            }
        } else {
            // Append other commands as is
            result << command << ";";
        }
    }
    // Convert the formatted HPGL string to a QStringList
    QString formattedCommands = QString::fromStdString(result.str());
    commandList = formattedCommands.split(';');

    return commandList;
}

void Dialog::on_Open_Hpgl_file_clicked()
{

    QString filename = QFileDialog::getOpenFileName(this, "Choose File");
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return;
    QTextStream in(&file);
    fileContent = in.readAll();  // Corrected variable name
    HPGL_to_Stack(fileContent);
    file.close();
}


void Dialog::on_StartSendingData_clicked()
{
    while (!commandStack.empty())
    {
        WriteToArduino(commandStack.top());
        QString check_error = ReadFromArduino();
        if(check_error == "Failed")
        {
            ui->Logs->append("Failed, Couldn't sending Data to Arduino");
            return;
        }
        commandStack.pop();
        int currentValue = ui->progressBar->value();
        ui->progressBar->setValue(currentValue + 1);
    }

}
