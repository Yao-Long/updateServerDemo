#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkInterface>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tcpServer = new QTcpServer();
    if(tcpServer == nullptr){
        qDebug()<<"no memory"<<__LINE__;
        return;
    }
//    QHostAddress addr("127.0.0.1");
//    quint16 port = 8080;
//    bool ret = tcpServer->listen(addr, port);
//    qDebug()<<_socket.errorString();
//    if(!ret){
//        qDebug()<<"tcpServer->listen error";
//        return;
//    }
//    if(tcpServer->isListening()){
//        qDebug()<<"tcpServer->isListening() return true";
//    }
//    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (int i = 0; i < ipAddressesList.size(); i++) {
        if(ipAddressesList[i].protocol() == QAbstractSocket::IPv4Protocol){
            ui->comboBox->addItem(ipAddressesList[i].toString());
            ui->comboBoxClient->addItem(ipAddressesList[i].toString());
        }
    }
    tcpConnectSocket = nullptr;
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(tcpServer != nullptr){
        delete tcpServer;
        tcpServer = nullptr;
    }
}

void MainWindow::onTcpDisconnected(){
    delete tcpConnectSocket;
    tcpConnectSocket = nullptr;
    qDebug()<<"释放连接资源";
}

void MainWindow::onNewConnection(){
    tcpConnectSocket = tcpServer->nextPendingConnection();
    qDebug()<<"服务器接收新连接:"<<tcpConnectSocket->peerAddress().toString()<<tcpConnectSocket->peerPort();
    connect(tcpConnectSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(tcpConnectSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    connect(tcpConnectSocket, SIGNAL(disconnected()),this, SLOT(onTcpDisconnected()));


    /* 连接已建立 -> 开始发数据 */
    connect(tcpConnectSocket, SIGNAL(connected()),
            this, SLOT(start_transfer()));
    /* 数据已发出 -> 继续发 */
    connect(tcpConnectSocket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(continue_transfer(qint64)));
    /* socket出错 -> 错误处理 */
    connect(tcpConnectSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(show_error(QAbstractSocket::SocketError)));
}

void MainWindow::onReadyRead(){
    QByteArray d = tcpConnectSocket->readAll();
    QString serverRecvStr = ui->textEditServerReceive->toPlainText();
    serverRecvStr += "\n";
    serverRecvStr += tcpConnectSocket->peerAddress().toString();
    QString portStr = QString::asprintf(":%u:\n", tcpConnectSocket->peerPort());
    serverRecvStr += portStr;
    serverRecvStr += d;
    ui->textEditServerReceive->setText(serverRecvStr);
}


void MainWindow::onSocketStateChanged(QAbstractSocket::SocketState state){
    qDebug()<<"新连接的状态改变为："<<state;
}


void MainWindow::onClientReadyRead(){
#if 0
//    qDebug()<<"onClientReadyRead";
    QByteArray array = clientSocket.readAll();
    QString clientRecvStr = ui->textEditClientReceive->toPlainText();
//    if(!clientRecvStr.isEmpty()){
//        clientRecvStr += "\n";
//    }
    clientRecvStr += "\n";
    qDebug()<<clientSocket.peerAddress().toString();
    qDebug()<<clientSocket.peerName();
    qDebug()<<clientSocket.peerPort();

    clientRecvStr += clientSocket.peerAddress().toString();
//    clientRecvStr += clientSocket.peerName();
    QString portStr = QString::asprintf(":%u:\n", clientSocket.peerPort());
    clientRecvStr += portStr;

//    qDebug()<<portStr;
//    clientRecvStr.asprintf("%u:\n", clientSocket.peerPort());
//    qDebug()<<clientRecvStr;

    clientRecvStr += array;
    ui->textEditClientReceive->setText(clientRecvStr);
#else
    QDataStream in(&clientSocket);
    in.setVersion(QDataStream::Qt_5_12);

//    qint64  recvFileSize = 0;
//    qint64  recvFileLen = 0;
//    QString recvFileName;
//    qint64  recvSize = 0;

    if(recvFileSize == 0){
        in >> recvFileSize;
        qDebug()<<"recvFileSize "<<recvFileSize;
    }
    if(recvFileLen == 0){
        in >> recvFileLen;
        qDebug()<<"recvFileLen "<<recvFileLen;
    }
    if(recvFileName.isEmpty()){
        in >> recvFileName;
        qDebug()<<"recvFileName "<<recvFileName;
    }

//    QFile *file = new QFile(recvFileName);
//    if(!file->open(QFile::WriteOnly)) // 打开失败
//    file->write(clientSocket.readAll());
    clientSocket.readAll();

#if 0

    /* 首部未接收/未接收完 */
    if(gotBytes <= 2 * sizeof(qint64))
    {
        if(!nameSize) // 前两个长度字段未接收
        {
            if(receive->bytesAvailable() >= 2 * sizeof(qint64))
            {
                in >> fileBytes >> nameSize;
                gotBytes += 2 * sizeof(qint64);
                ui->recvProg->setMaximum(fileBytes);
                ui->recvProg->setValue(gotBytes);
            }
            else // 数据不足，等下次
               return;
        }
        else if(receive->bytesAvailable() >= nameSize)
        {
            in >> fileName;
            gotBytes += nameSize;
            ui->recvProg->setValue(gotBytes);
            std::cout << "--- File Name: "
                      << fileName.toStdString() << std::endl;
        }
        else // 数据不足文件名长度，等下次
            return;
    }

    /* 已读文件名、文件未打开 -> 尝试打开文件 */
    if(!fileName.isEmpty() && file == Q_NULLPTR)
    {
        file = new QFile(fileName);
        if(!file->open(QFile::WriteOnly)) // 打开失败
        {
            std::cerr << "*** File Open Failed ***" << std::endl;
            delete file;
            file = Q_NULLPTR;
            return;
        }
        ui->stLabel->setText(QString("Open %1 Successfully!").arg(fileName));
    }
    if(file == Q_NULLPTR) // 文件未打开，不能进行后续操作
        return;

    if(gotBytes < fileBytes) // 文件未接收完
    {
        gotBytes += receive->bytesAvailable();
        ui->recvProg->setValue(gotBytes);
        file->write(receive->readAll());
    }
    if(gotBytes == fileBytes) // 文件接收完
    {
        receive->close(); // 关socket
        file->close(); // 关文件
        delete file;
        ui->stLabel->setText(QString("Finish receiving %1").arg(fileName));
        ui->listenBtn->setEnabled(true);
    }
#endif
#endif
}

void MainWindow::onClientStateChanged(QAbstractSocket::SocketState socketState){
    qDebug()<<"客服端状态改变为："<<socketState;
}

void MainWindow::on_pushButtonBuildServer_clicked()
{
    if(tcpServer == nullptr){
        qDebug()<<"tcpServer == null";
        return;
    }

    if(tcpServer->isListening()){
        tcpServer->close();
    }

    QString addrStr = ui->comboBox->currentText();
    QHostAddress addr(addrStr);
    quint16 port = 8080;
    qDebug()<<"服务器监听 "<<addr<<port;
    bool ret = tcpServer->listen(addr, port);
    if(!ret){
        qDebug()<<"tcpServer->listen error";
        return;
    }
    qDebug()<<tcpServer->errorString();
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

void MainWindow::on_pushButtonConnectServer_clicked()
{
    if(clientSocket.isOpen()){
         clientSocket.close();
    }
//    if(clientSocket.state() != QAbstractSocket::UnconnectedState){
//        clientSocket.close();
//    }
    QHostAddress addr(ui->comboBoxClient->currentText());
    clientSocket.connectToHost(addr, 8080);
    connect(&clientSocket, SIGNAL(readyRead()), this, SLOT(onClientReadyRead()));
    connect(&clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this,
            SLOT(onClientStateChanged(QAbstractSocket::SocketState)));
}

void MainWindow::on_pushButtonServerSend_clicked()
{
    if(tcpConnectSocket == nullptr){
        qDebug()<<"tcpConnectSocket == null";
        return;
    }
    QString str = ui->textEditServerSend->toPlainText();
    QByteArray b;
    b.append(str);
    tcpConnectSocket->write(b);
}

void MainWindow::on_pushButtonClientSend_clicked()
{
    QString str = ui->textEditClientSend->toPlainText();
    QByteArray b;
    b.append(str);
    clientSocket.write(b);
}

void MainWindow::on_pushButton_clicked()
{
    QString fileName = "C:\\Users\\yaooolon\\Documents\\updateServerDemo\\VirtualBox-5.1.26-117224-Win.exe";
    file = new QFile(fileName);
//    QFile file(fileName);
    qDebug()<<file->fileName();
    bool ret = file->open(QFile::ReadOnly);
    if(!ret){
        qDebug()<<"open file failed";
        return;
    }
    restSize = fileSize = file->size();
    qDebug()<<"fileSize = "<<fileSize;

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    sendSize = 0;

    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
//    qDebug()<<out.version();
    out.setVersion(QDataStream::Qt_5_12);
//    qDebug()<<out.version();

    QFileInfo fileInfo(fileName);
    QString fileNameShort= fileInfo.fileName();
    qDebug()<<fileNameShort;


    out<<fileSize<<fileNameShort.length()<<fileNameShort;
    headerLen = sizeof(fileSize) + sizeof(int) + fileNameShort.length();

    tcpConnectSocket->write(buf);
}


void MainWindow::continue_transfer(qint64 size){
    sendSize += size;
    qint64 sendFileSize = sendSize - headerLen;
    if(sendFileSize > 0){
        ui->progressBar->setValue(sendFileSize * 100 / fileSize);
    }else{
        ui->progressBar->setValue(0);
    }
    /* 还有数据要发 */
    if(restSize > 0)
    {
        qint64 blockSize = 10 * 1024 * 1024;
        qint64 readLen = restSize >= blockSize ? blockSize : restSize;
        /* 从文件读数据 */
        QByteArray buf = file->read(readLen);
        /* 发送 */
        qint64 sendLen = tcpConnectSocket->write(buf);
        restSize -= sendLen;
    }
    else{
        file->close();
    }
    /* 全部发送完 */
    if(sendFileSize == fileSize)
    {
//        send->close(); // 关socket
//        fileName.clear(); // 清空文件名
//        ui->stLabel->setText(QString("Finish sending!"));
        qDebug()<<"文件发送完成";
        tcpConnectSocket->close();
    }


}