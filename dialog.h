#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <stack>
#include <QTimer>


QT_BEGIN_NAMESPACE
namespace Ui {
class CNC_Machine;
}
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
    static QStringList split_to_commands(const QString& str);
    int PrograssBarCounter = 0;


private slots:
    void HPGL_to_Stack(const QString& HPGL);
    void on_Open_Hpgl_file_clicked();
    std::stack<QString>Reverse_Stack(std::stack<QString> commandStack);
    void on_progressBar_valueChanged(int value);
    void ReadFromArduino();
    void on_StartSendData_BT_clicked();

private:
    Ui::CNC_Machine *ui;
    QString fileContent;
    QString Number;
    QSerialPort *arduino;
    static const quint16 arduino_uno_vendorID = 9025;
    static const quint16 arduino_uno_productID = 67;
    QString arduino_port_name;
    bool arduino_is_available;
    QString Data_From_SerialPort;
    bool IS_Data_Recevied = false;
    std::stack<QString> commandStack;
    QTimer* Timer;
    QString check_S_or_F = "";
    bool Stop = false;


signals:
    void sendDataFailed();
    void updateProgressBar();
    void sendDataCompleted();
};

#endif // DIALOG_H
