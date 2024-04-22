#include "ThreadSendData.h"
#include "dialog.h" // Include Dialog header file

ThreadSendData::ThreadSendData(Ui::Dialog *ui, QStack<QString> *commandStack, Dialog *dialog, QObject *parent) :
    QThread(parent), ui(ui), commandStack(commandStack), dialog(dialog), stop(false)
{
}

void ThreadSendData::run()
{
    while (!stop && !commandStack->isEmpty())
    {
        SendCommandsToArduino(commandStack->pop());
    }
}

void Dialog::on_StartSendingData_clicked() {
    while (!commandStack.empty()) {
        WriteToArduino(commandStack.top());
        QString check_error = ReadFromArduino();
        if(check_error == "Failed") {
            ui->Logs->append("Failed, Couldn't sending Data to Arduino");
            return;
        }
        qDebug() << commandStack.top();
        commandStack.pop();
        int currentValue = ui->progressBar->value();
        ui->progressBar->setValue(currentValue + 1);
    }
}



