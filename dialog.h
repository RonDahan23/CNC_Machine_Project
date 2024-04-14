#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <stack>



QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
    static QStringList split_to_commands(const QString& str);


private slots:
    void WriteToArduino(QString command);
    void ReadFromArduino();
    void HPGL_to_Stack(const QString& HPGL);
    void on_BtHomePoint_clicked();
    void on_Open_Hpgl_file_clicked();
    std::stack<QString>Reverse_Stack(std::stack<QString> commandStack);

private:
    Ui::Dialog *ui;
    QString fileContent;
    float lastX = 0;
    float lastY = 0;
    QString Number;
    QSerialPort *arduino;
    static const quint16 arduino_uno_vendorID = 9025;
    static const quint16 arduino_uno_productID = 67;
    QString arduino_port_name;
    bool arduino_is_available;
    QString Data_From_SerialPort;
    bool IS_Data_Recevied = false;
    std::stack<QString> commandStack;

};

#endif // DIALOG_H
