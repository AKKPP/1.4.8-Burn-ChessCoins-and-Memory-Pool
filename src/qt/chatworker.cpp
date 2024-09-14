// chatworker.cpp
#include "chatworker.h"

ChatWorker::ChatWorker(QObject *parent)
    : QObject(parent), socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::connected, this, &ChatWorker::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &ChatWorker::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &ChatWorker::onReadyRead);
    connect(socket, &QTcpSocket::bytesWritten, this, &ChatWorker::onBytesWritten);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onHandleError(QAbstractSocket::SocketError)));
}

void ChatWorker::connectToServer(const QString &serverIp, quint16 port)
{
    socket->abort();
    socket->connectToHost(serverIp, port);
}

void ChatWorker::sendMessage(const QString &message)
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState)
    {
        socket->write(message.toUtf8());
        socket->flush();
    }
}

void ChatWorker::onConnected()
{
    emit connected();
}

void ChatWorker::onDisconnected()
{
    emit disconnected();
}

void ChatWorker::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString receivedMessage = QString::fromUtf8(data);
    emit messageReceived(receivedMessage);
}

void ChatWorker::onHandleError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(socket->errorString());
}

void ChatWorker::onBytesWritten(qint64 bytes)
{
    emit bytesWritten(bytes);
}
