#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>
#include <QDateTime>
#include <QHostInfo>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void sendFileToClient(QString fileName);
protected slots:
    void onNewConnection();
    void onReadyRead();
    void onSocketStateChanged(QAbstractSocket::SocketState state);

    void onClientReadyRead();

    void onClientStateChanged(QAbstractSocket::SocketState socketState);
    void onTcpDisconnected();
    void continue_transfer(qint64 size);
    void lookedUp(const QHostInfo &host);
private slots:
    void on_pushButtonBuildServer_clicked();

    void on_pushButtonConnectServer_clicked();

    void on_pushButtonServerSend_clicked();

    void on_pushButtonClientSend_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    QTcpServer *tcpServer;
    QTcpSocket *tcpConnectSocket;

    QTcpSocket  clientSocket;
    int sendStatus = 0;
    QDateTime fileLastModifyTime;

    QFile *file;
    qint64 headerLen;
    qint64 fileSize;
    qint64 restSize;
    qint64 sendSize;

    QFile *recvFile;
    qint64  recvFileSize = 0;
    int  recvFileLen = 0;
    QDateTime recvFileTime;
    QString recvFileName;
    qint64  recvSize = 0;
    int recvStatus = 0;
};

#endif // MAINWINDOW_H
