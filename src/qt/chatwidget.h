// chatwidget.h
#ifndef QCHATWIDGET_H
#define QCHATWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include "chatworker.h"

namespace Ui {
class chatWidget;
}

class QChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QChatWidget(QWidget *parent = nullptr);
    ~QChatWidget();

signals:
    void connectToServer(const QString &serverIp, quint16 port);
    void sendMessage(const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead(const QString &message);
    void onHandleError(const QString &errorString);
    void onBytesWritten(qint64 bytes);
    void onCheckPort();

    void on_sendButton_clicked();
    void on_clearButton_clicked();
    void on_connButton_clicked();

private:
    QString getUTCTimeForChatroom();

    void setConnectState(bool connected);
    void tryConnect();

    Ui::chatWidget *ui;
    QThread *workerThread;
    ChatWorker *worker;
    QTimer *checkPortTimer;
    bool bConnected;
};

#endif // QCHATWIDGET_H
