#ifndef THREADSENDDATA_H
#define THREADSENDDATA_H

#include <QThread>
#include <QStack>
#include <QDebug>

// Forward declaration of Dialog class to resolve incomplete type issue
class Dialog;

// Include the header containing the Ui namespace
#include "ui_dialog.h"

class ThreadSendData : public QThread
{
    Q_OBJECT

public:
    explicit ThreadSendData(Ui::Dialog *ui, QStack<QString> *commandStack, Dialog *dialog, QObject *parent = nullptr);
    void run() override;
    bool stop = false;

public slots:


signals:
    void SendCommandsToArduino(QString command);

private slots:
    void on_Open_Hpgl_file_clicked();

private:
    Ui::Dialog *ui;
    QStack<QString> *commandStack;
    Dialog *dialog;

};

#endif // THREADSENDDATA_H
