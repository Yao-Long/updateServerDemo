#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkInterface>

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QtGlobal>
#include <QtEndian>
#include <QDateTime>
#include <QHostInfo>

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

    QIntValidator *intvalidator = new QIntValidator(1000, 65535);
    ui->lineEditServerPort->setValidator(intvalidator);
    ui->lineEditServerPort->setText(tr("7777"));

    ui->lineEditConnectToServerPort->setValidator(intvalidator);
    ui->lineEditConnectToServerPort->setText(tr("42662"));

    ui->lineEditServerName->setText(tr("27v241o078.qicp.vip"));


    ui->textEditClientSend->setText(tr("\\downloadTestFile"));
}

void MainWindow::lookedUp(const QHostInfo &host){
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }

    const auto addresses = host.addresses();
    for (const QHostAddress &address : addresses)
        qDebug() << "Found address:" << address.toString();



    QString addrStr = host.addresses()[0].toString();

    QHostAddress addr(addrStr);
    int port = ui->lineEditConnectToServerPort->text().toInt();
    if(port < 1000 || port > 65535){
        qDebug()<<"客户端连接服端的端口不合法"<<port;
        return;
    }
    clientSocket.connectToHost(addr, port);
    connect(&clientSocket, SIGNAL(readyRead()), this, SLOT(onClientReadyRead()));
    connect(&clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this,
            SLOT(onClientStateChanged(QAbstractSocket::SocketState)));

    QString saveFileName("xxxxxxx.exe");
    recvFile = new QFile(saveFileName);
    bool ret = recvFile->open(QIODevice::WriteOnly);
    qDebug()<<"recvFile->open(QIODevice::WriteOnly) = "<<ret;


    ui->progressBar_2->setRange(0, 100);

#if 0
    QHostAddress addr(addrStr);
    //    quint16 port = 7778;
    int port = ui->lineEditServerPort->text().toInt();
    if(port < 1000 || port > 65536){
        qDebug()<<"服务器端口号不合法"<<port;
        return;
    }
    qDebug()<<"服务器监听 "<<addr<<port;
    bool ret = tcpServer->listen(addr, port);
    if(!ret){
        qDebug()<<"tcpServer->listen error";
        return;
    }
    qDebug()<<tcpServer->errorString();
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
#endif
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
//    connect(tcpConnectSocket, SIGNAL(connected()),
//            this, SLOT(start_transfer()));
    /* 数据已发出 -> 继续发 */
    connect(tcpConnectSocket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(continue_transfer(qint64)));
    /* socket出错 -> 错误处理 */
//    connect(tcpConnectSocket, SIGNAL(error(QAbstractSocket::SocketError)),
//            this, SLOT(show_error(QAbstractSocket::SocketError)));
}

void MainWindow::sendFileToClient(QString fileName){
    file = new QFile(fileName);
//    qDebug()<<file->fileName();
    bool ret = file->open(QFile::ReadOnly);
    if(!ret){
        qDebug()<<"open file failed"<<file->errorString();
        return;
    }
    restSize = fileSize = file->size();
    qDebug()<<"fileSize = "<<fileSize;

//    ui->progressBar->setRange(0, 100);
//    ui->progressBar->setValue(0);
    sendSize = 0;

//    QByteArray buf;
//    QDataStream out(&buf, QIODevice::WriteOnly);
    //    qDebug()<<out.version();
//    out.setVersion(QDataStream::Qt_5_12);
    //    qDebug()<<out.version();

    QFileInfo fileInfo(fileName);
    QString fileNameShort= fileInfo.fileName();
    fileLastModifyTime = fileInfo.fileTime(QFileDevice::FileModificationTime);
    qDebug()<<"短文件名 "<<fileNameShort<<"文件最后修改时间"<<fileLastModifyTime;


    //    out<<fileSize<<fileNameShort.length()<<fileNameShort;
    //    headerLen = sizeof(fileSize) + sizeof(int) + fileNameShort.length();

    //    out<<fileNameShort;
    //    QDataStream fileIn(file);

    //    while (1) {
    //        qint64 readBlockSizeByte = 1024 * 1024 * 10;
    //        QByteArray tmp = file->read(readBlockSizeByte);
    //        if(tmp.isEmpty()){
    //            qDebug()<<"服务端发送文件完成";
    //            break;
    //        }
    //        tcpConnectSocket->write(tmp);
    //    }
//    QByteArray fileNameByteArray = fileNameShort.toUtf8();
//    QByteArray fileNameByteArray = fileNameShort.toLatin1();
//    tcpConnectSocket->write(fileNameByteArray);
    //    tcpConnectSocket->flush();
//    qDebug()<<"发送要传输的文件名"<<fileNameShort<<"utf8:"<<fileNameByteArray;

    QDataStream in(tcpConnectSocket);
    fileNameShort += tr("测试文件");
    in<<fileNameShort;

}

void MainWindow::onReadyRead(){
    QByteArray d = tcpConnectSocket->readAll();
//    QString serverRecvStr = ui->textEditServerReceive->toPlainText();
//    serverRecvStr += "\n";
//    serverRecvStr += tcpConnectSocket->peerAddress().toString();
//    QString portStr = QString::asprintf(":%u:\n", tcpConnectSocket->peerPort());
//    serverRecvStr += portStr;
//    serverRecvStr += d;
//    ui->textEditServerReceive->setText(serverRecvStr);

    QString recvStr(d);
    QDataStream in(tcpConnectSocket);
    qDebug()<<"服务器端收到数据字符串:"<<recvStr;
    if(recvStr == tr("\\downloadTestFile")){
        qDebug()<<"服务器端开始发送数据给客户端";
        sendFileToClient(tr("F:/github_repo/updateServerDemo/main.cpp"));
    }else if(recvStr == tr("\\downloadFileFinished")){
        qDebug()<<"服务器端收到客户端接收文件完成响应";
    }else if (recvStr == tr("\\recvFileNameFinished")) {
        qint64 bigEndianFileSize = qToBigEndian(fileSize);
        in<<bigEndianFileSize;
        qDebug()<<"发送文件总大小:"<<fileSize<<"bigendian:"<<bigEndianFileSize;
    }else if (recvStr == tr("\\recvFileSizeFinished")) {
        in<<fileLastModifyTime;
        qDebug()<<"文件时间:"<<fileLastModifyTime;
    }else if (recvStr == tr("\\recvFileTimeFinished")) {
        qDebug()<<"开始传输文件内容";
        qint64 readBlockSizeByte = 1024 * 1024 * 10;
        QByteArray tmp = file->read(readBlockSizeByte);
        tcpConnectSocket->write(tmp);
        qDebug()<<"发送文件数据大小:"<<tmp.size();
    }else if(recvStr.isEmpty()){
        if(sendStatus == 0){
            qint64 bigEndianFileSize = qToBigEndian(fileSize);
            in<<bigEndianFileSize;
            qDebug()<<"发送文件总大小:"<<fileSize<<"bigendian:"<<bigEndianFileSize;
            sendStatus = 1;
        }else if(sendStatus == 1) {
            //        QByteArray fileTimeByteArray = fileLastModifyTime.toString().toUtf8();
            //        tcpConnectSocket->write(fileTimeByteArray);
            //        qDebug()<<"文件时间:"<<fileTimeByteArray;
            in<<fileLastModifyTime;
            qDebug()<<"文件时间:"<<fileLastModifyTime;
            sendStatus = 2;
        }else if (sendStatus == 2) {
            qDebug()<<"开始传输文件内容";
            qint64 readBlockSizeByte = 1024 * 1024 * 10;
            QByteArray tmp = file->read(readBlockSizeByte);
            tcpConnectSocket->write(tmp);
            qDebug()<<"发送文件数据大小:"<<tmp.size();
            sendStatus = 3;
        }
    }




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
//    QDataStream in(&clientSocket);
//    in.setVersion(QDataStream::Qt_5_12);

//    qint64  recvFileSize = 0;
//    qint64  recvFileLen = 0;
//    QString recvFileName;
//    qint64  recvSize = 0;

//    if(recvFileSize == 0){
//        in >> recvFileSize;
//        qDebug()<<"recvFileSize "<<recvFileSize;
//    }
//    if(recvFileLen == 0){
//        in >> recvFileLen;
//        qDebug()<<"recvFileLen "<<recvFileLen;
//    }
//    if(recvFileName.isEmpty()){
//        in >> recvFileName;
//        qDebug()<<"recvFileName "<<recvFileName;
//    }

//    QFile *file = new QFile(recvFileName);
//    if(!file->open(QFile::WriteOnly)) // 打开失败
//    file->write(clientSocket.readAll());
//    clientSocket.readAll();
//    QByteArray tmp = clientSocket.readAll();
//    if(tmp.isEmpty()){
//        qDebug()<<"clientSocket.readAll() error";
//        return;
//    }
    QDataStream out(&clientSocket);
    QDateTime time;
    QByteArray tmp;
    qint64 len;
    QDataStream in(&clientSocket);
    switch (recvStatus) {
    case 0:
//        tmp = clientSocket.readAll();

//        recvFileName = QString(tmp);
//        QDataStream out(&clientSocket);
        out>>recvFileName;
//        QString xName = QString(tmp);
//        qDebug()<<"客服端收到文件名:"<<recvFileName;
        recvStatus = 1;
        qDebug()<<"客服端收到文件名:"<<recvFileName<<"tmp:"<<tmp;

        in<<tr("\\recvFileNameFinished");

        break;
    case 1:
//        out.setByteOrder(QDataStream::BigEndian);
        out>>len;
        recvFileSize = qFromBigEndian(len);
//        out>>recvFileSize;
        qDebug()<<"客户端收到文件大小:"<<recvFileSize<<"len:"<<len;
        recvStatus = 2;

        in<<tr("\\recvFileSizeFinished");

        break;
    case 2:
//        tmp = clientSocket.readAll();
//        time = QString(tmp);
        out>>time;
        qDebug()<<"客户端收到文件时间:"<<time<<"tmp:"<<tmp;
        recvStatus = 3;
        in<<tr("\\recvFileTimeFinished");
        break;
    case 3:
        qDebug()<<"客户端开始接收文件内容";
        tmp = clientSocket.readAll();
        if(tmp.isEmpty()){
            qDebug()<<"clientSocket.readAll() error";
            return;
        }
        recvSize += tmp.size();
        ui->progressBar_2->setValue(recvSize * 100 / recvFileSize);
        len = recvFile->write(tmp);
        if(len == -1){
            qDebug()<<"recvFile->write(tmp) error";
            return;
        }

        if(recvSize == recvFileSize){
            qDebug()<<"客户端接收文件完成";
//            clientSocket.deleteLater();
            recvFile->close();
            in<<tr("\\downloadFileFinished");
        }

        break;
    default:
        break;
    }
//    QByteArray tmp = clientSocket.readAll();
//    if(tmp.isEmpty()){
//        qDebug()<<"clientSocket.readAll() error";
//        return;
//    }
//    qint64 len = recvFile->write(tmp);
//    if(len == -1){
//        qDebug()<<"recvFile->write(tmp) error";
//        return;
//    }


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
//    if(socketState == QAbstractSocket::ConnectedState){
//        QString saveFileName("xxxxxxx.exe");
//        recvFile = new QFile(saveFileName);
//        bool ret = recvFile->open(QIODevice::WriteOnly);
//        qDebug()<<"recvFile->open(QIODevice::WriteOnly) = "<<ret;
//    }
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
#if 1
    QString addrStr = ui->comboBox->currentText();
#else
    QString addrStr = ui->lineEditServerName->text();
    QHostInfo::lookupHost(addrStr, this, SLOT(lookedUp(QHostInfo)));
#endif
#if 1

    QHostAddress addr(addrStr);
//    quint16 port = 7778;
    int port = ui->lineEditServerPort->text().toInt();
    if(port < 1000 || port > 65536){
        qDebug()<<"服务器端口号不合法"<<port;
        return;
    }
    qDebug()<<"服务器监听 "<<addr<<port;
    bool ret = tcpServer->listen(addr, port);
    if(!ret){
        qDebug()<<"tcpServer->listen error";
        return;
    }
//    qDebug()<<"开始监听"<<addr<<"端口 "<<port;
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
#endif
}

void MainWindow::on_pushButtonConnectServer_clicked()
{
    if(clientSocket.isOpen()){
         clientSocket.close();
    }
//    if(clientSocket.state() != QAbstractSocket::UnconnectedState){
//        clientSocket.close();
//    }
    QString addrStr = ui->lineEditServerName->text();
    QHostInfo::lookupHost(addrStr, this, SLOT(lookedUp(QHostInfo)));

#if 0
    QHostAddress addr(ui->comboBoxClient->currentText());
    clientSocket.connectToHost(addr, 7778);
    connect(&clientSocket, SIGNAL(readyRead()), this, SLOT(onClientReadyRead()));
    connect(&clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this,
            SLOT(onClientStateChanged(QAbstractSocket::SocketState)));

    QString saveFileName("xxxxxxx.exe");
    recvFile = new QFile(saveFileName);
    bool ret = recvFile->open(QIODevice::WriteOnly);
    qDebug()<<"recvFile->open(QIODevice::WriteOnly) = "<<ret;


    ui->progressBar_2->setRange(0, 100);
#endif
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
    QString fileName = QFileDialog::getOpenFileName(this);
    qDebug()<<fileName;
//    return;"F:/github_repo/updateServerDemo/mainwindow.cpp"

//    QString fileName = "C:\\Users\\yaooolon\\Documents\\updateServerDemo\\VirtualBox-5.1.26-117224-Win.exe";
    file = new QFile(fileName);
//    QFile file(fileName);
    qDebug()<<file->fileName();
    bool ret = file->open(QFile::ReadOnly);
    if(!ret){
        qDebug()<<"open file failed"<<file->errorString();
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
    fileLastModifyTime = fileInfo.fileTime(QFileDevice::FileModificationTime);
    qDebug()<<fileNameShort<<fileLastModifyTime;


//    out<<fileSize<<fileNameShort.length()<<fileNameShort;
//    headerLen = sizeof(fileSize) + sizeof(int) + fileNameShort.length();

//    out<<fileNameShort;
//    QDataStream fileIn(file);

//    while (1) {
//        qint64 readBlockSizeByte = 1024 * 1024 * 10;
//        QByteArray tmp = file->read(readBlockSizeByte);
//        if(tmp.isEmpty()){
//            qDebug()<<"服务端发送文件完成";
//            break;
//        }
//        tcpConnectSocket->write(tmp);
//    }
    QByteArray fileNameByteArray = fileNameShort.toUtf8();
    tcpConnectSocket->write(fileNameByteArray);
//    tcpConnectSocket->flush();
    qDebug()<<"发送要传输的文件名"<<fileNameShort;

//    qint64 tmpFileSize = qToBigEndian(fileSize);
//    tcpConnectSocket->write((char *)&tmpFileSize, sizeof (tmpFileSize));
//    tcpConnectSocket->flush();

//    QByteArray fileTimeByteArray = fileLastModifyTime.toString().toUtf8();
//    tcpConnectSocket->write(fileTimeByteArray);
//    tcpConnectSocket->flush();

//    qint64 readBlockSizeByte = 1024 * 1024 * 10;
//    QByteArray tmp = file->read(readBlockSizeByte);
//    tcpConnectSocket->write(tmp);


//    QDataStream tcpOut(tcpConnectSocket);
//    tcpOut<<fileNameShort;

//    tcpConnectSocket->write(buf);
//    tcpConnectSocket->flush();
}


void MainWindow::continue_transfer(qint64 size){
//    return;
    if(size == 0){
        return;
    }
    QDataStream in(tcpConnectSocket);

    if (sendStatus == 3) {
        sendSize += size;
        ui->progressBar->setValue(sendSize * 100 / fileSize);
        if(sendSize == fileSize){
            qDebug()<<"文件发送完成";
            //            tcpConnectSocket->close();
            tcpConnectSocket->flush();

            file->close();
            return;
        }


        qint64 blockSize = 10 * 1024 * 1024;
        //    qint64 readLen = restSize >= blockSize ? blockSize : restSize;
        /* 从文件读数据 */
        QByteArray buf = file->read(blockSize);
        /* 发送 */
        tcpConnectSocket->write(buf);
    }
    return;


    if(sendStatus == 0){
//        in.setByteOrder(QDataStream::BigEndian);
        qint64 bigEndianFileSize = qToBigEndian(fileSize);

        in<<bigEndianFileSize;
//        qint64 tmpFileSize = qToBigEndian(fileSize);
//        tcpConnectSocket->write((char *)&tmpFileSize, sizeof (tmpFileSize));
        qDebug()<<"发送文件总大小:"<<fileSize<<"bigendian:"<<bigEndianFileSize;
        sendStatus = 1;
    }else if(sendStatus == 1) {
//        QByteArray fileTimeByteArray = fileLastModifyTime.toString().toUtf8();
//        tcpConnectSocket->write(fileTimeByteArray);
//        qDebug()<<"文件时间:"<<fileTimeByteArray;
        in<<fileLastModifyTime;
        qDebug()<<"文件时间:"<<fileLastModifyTime;
        sendStatus = 2;
    }else if (sendStatus == 2) {
        qDebug()<<"开始传输文件内容";
        qint64 readBlockSizeByte = 1024 * 1024 * 10;
        QByteArray tmp = file->read(readBlockSizeByte);
        tcpConnectSocket->write(tmp);
        qDebug()<<"发送文件数据大小:"<<tmp.size();
        sendStatus = 3;
    }else if (sendStatus == 3) {
        sendSize += size;
        ui->progressBar->setValue(sendSize * 100 / fileSize);
        if(sendSize == fileSize){
            qDebug()<<"文件发送完成";
//            tcpConnectSocket->close();
            tcpConnectSocket->flush();

            file->close();
            return;
        }


        qint64 blockSize = 10 * 1024 * 1024;
    //    qint64 readLen = restSize >= blockSize ? blockSize : restSize;
        /* 从文件读数据 */
        QByteArray buf = file->read(blockSize);
        /* 发送 */
        tcpConnectSocket->write(buf);
    }





   #if 0

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

#endif
}
