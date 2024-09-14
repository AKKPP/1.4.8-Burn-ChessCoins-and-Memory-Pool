// chatworker.h
#ifndef CHATWORKER_H
#define CHATWORKER_H

#include <QObject>
#include <QTcpSocket>

class ChatWorker : public QObject
{
    Q_OBJECT
public:
    explicit ChatWorker(QObject *parent = nullptr);
    void connectToServer(const QString &serverIp, quint16 port);

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &message);
    void errorOccurred(const QString &errorString);
    void bytesWritten(qint64 bytes);

public slots:
    void sendMessage(const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onHandleError(QAbstractSocket::SocketError socketError);
    void onBytesWritten(qint64 bytes);

private:
    QTcpSocket *socket;
};

#endif // CHATWORKER_H
