#include "chatwidget.h"
#include "ui_chatwidget.h"
#include "chatworker.h"
#include <QThread>
#include <QMessageBox>
#include <QHostInfo>
#include <QShortcut>
#include <QDateTime>
#include "util.h"

QChatWidget::QChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::chatWidget)
    , workerThread(new QThread(this))
    , worker(new ChatWorker())
    , checkPortTimer(new QTimer(this))
    , bConnected(false)
{
    ui->setupUi(this);
    ui->connButton->hide();

    QString computerName = QHostInfo::localHostName();
    ui->labelHost->setText(computerName);

    // Create QShortcut for Enter key
    QShortcut *enterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), this);
    QObject::connect(enterShortcut, &QShortcut::activated, ui->sendButton, &QToolButton::click);

    // Create QShortcut for Enter (numpad) key
    QShortcut *numpadEnterShortcut = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    QObject::connect(numpadEnterShortcut, &QShortcut::activated, ui->sendButton, &QToolButton::click);

    // Set the layout of the main widget
    setLayout(ui->vertLayout);

    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &ChatWorker::connected, this, &QChatWidget::onConnected);
    connect(worker, &ChatWorker::disconnected, this, &QChatWidget::onDisconnected);
    connect(worker, &ChatWorker::messageReceived, this, &QChatWidget::onReadyRead);
    connect(worker, &ChatWorker::errorOccurred, this, &QChatWidget::onHandleError);
    connect(worker, &ChatWorker::bytesWritten, this, &QChatWidget::onBytesWritten);
    connect(this,   &QChatWidget::connectToServer, worker, &ChatWorker::connectToServer);
    connect(this,   &QChatWidget::sendMessage, worker, &ChatWorker::sendMessage);

    worker->moveToThread(workerThread);
    workerThread->start();

    tryConnect();

    // Setup and start the timer
    connect(checkPortTimer, &QTimer::timeout, this, &QChatWidget::onCheckPort);
    checkPortTimer->start(5 * 60 * 1000);  // Check every 5 minutes
}

QChatWidget::~QChatWidget()
{
    workerThread->quit();
    workerThread->wait();

    delete ui;
}

void QChatWidget::tryConnect()
{
    emit connectToServer(QString("54.38.157.243"), 3200);
}

void QChatWidget::onConnected()
{
    bConnected = true;
    QString msg = getUTCTimeForChatroom();
    msg += "You are online for chat";
    ui->txtReceive->append(msg);

    setConnectState(true);
}

void QChatWidget::onDisconnected()
{
    bConnected = false;
    QString sendtime = getUTCTimeForChatroom();
    QString formattedText = QString("<font color='red'>%1You are offline for chat</font>")
                                     .arg(sendtime);
    ui->txtReceive->append(formattedText);

    setConnectState(false);
}

void QChatWidget::on_sendButton_clicked()
{
    QString message = ui->txtSend->text();

    if (message.isEmpty() || !bConnected)
        return;

    if (!message.compare(QString("$echo"), Qt::CaseInsensitive))
    {
        QMessageBox::critical(this, tr("Failed to send Chat"), tr("You cannot send reserved words."));
        ui->txtSend->setText("");
        return;
    }

    int length = message.length();
    if (length > 4000)
    {
        QMessageBox::critical(this, tr("Failed to send Chat"), tr("You cannot send more than 4000 characters."));
        return;
    }

    QString sendtime = getUTCTimeForChatroom();
    QString formattedText = QString("<font color='white'>%1%2</font>").arg(sendtime, message);
    ui->txtReceive->append(formattedText);

    emit sendMessage(message);

    ui->txtSend->setText("");
}

void QChatWidget::onReadyRead(const QString &message)
{
    QString sendtime = getUTCTimeForChatroom();
    QString formattedText = QString("<font color='white'>%1%2</font>").arg(sendtime, message);
    ui->txtReceive->append(formattedText);

    bConnected = true;
    setConnectState(true);
}

void QChatWidget::onHandleError(const QString &errorString)
{
    QString sendtime = getUTCTimeForChatroom();
    QString formattedText = QString("<font color='red'>%1%2</font>").arg(sendtime, errorString);
    ui->txtReceive->append(formattedText);

    bConnected = false;
    setConnectState(false);
}

void QChatWidget::on_clearButton_clicked()
{
    QString selectedTxt = ui->txtReceive->textCursor().selectedText();
    if (selectedTxt.isEmpty())
    {
        QMessageBox::StandardButton msgRet;
        msgRet = QMessageBox::question(this, tr("chesscoin-qt"),
            tr("Do you want to clear all chat histories?"), QMessageBox::Yes | QMessageBox::No);

        if (msgRet == QMessageBox::No)
            return;

        ui->txtReceive->clear();
        return;
    }

    ui->txtReceive->textCursor().removeSelectedText();
}

void QChatWidget::onBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);

    //QString logMessage = QString("Bytes written: %1").arg(bytes);
    //ui->txtReceive->append(logMessage);
}

void QChatWidget::on_connButton_clicked()
{
    tryConnect();
}

QString QChatWidget::getUTCTimeForChatroom()
{
    // Convert local date and time to UTC
    QDateTime localDateTime = QDateTime::currentDateTime();
    QDateTime utcDateTime = localDateTime.toUTC();
    return utcDateTime.toString("[hh:mm:ss] ");
}

void QChatWidget::setConnectState(bool connected)
{
    if (connected)
    {
        ui->labelState->setPixmap(QIcon(":/icons/connected").pixmap(14, 14));
        ui->labelState->update();
        ui->labelState->setToolTip(QString("Online"));
        ui->connButton->hide();
    }
    else
    {
        ui->labelState->setPixmap(QIcon(":/icons/disconnected").pixmap(14, 14));
        ui->labelState->update();
        ui->labelState->setToolTip(QString("Offline"));
        ui->connButton->show();
    }
}

void QChatWidget::onCheckPort()
{
    QTcpSocket testSocket;
    testSocket.connectToHost("54.38.157.243", 3200);

    if (testSocket.waitForConnected(5000)) {
        testSocket.disconnectFromHost();
        setConnectState(true);

        if (!bConnected) {
            MilliSleep(250);
            tryConnect();
        }
    }
    else {
        setConnectState(false);
    }
}

