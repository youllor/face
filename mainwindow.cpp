#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    picSocket = NULL;
    dataSocket = NULL;
    timeCount = 0;

    ui->btn_start->setEnabled(false);
    ui->btn_stop->setEnabled(false);
    ui->btn_input->setEnabled(false);
    ui->btn_cancle->setEnabled(false);

    ui->lab_time->setStyleSheet("background:transparent; color: rgb(255, 0, 0); font: 75 18pt '楷体'");//设置lable控件的透明

    ui->label->setStyleSheet("background:transparent; color: rgb(0, 0, 0); font: 75 18pt '楷体'");//设置lable控件的透明

    ui->lab_time_2->setStyleSheet("background:transparent; color: rgb(255, 0, 0); font: 75 48pt '楷体'");//设置lable控件的透明

    timer = new QTimer;
    connect(timer, QTimer::timeout, [=](){
        QTime qtimeObj = QTime::currentTime();
        QString strTime = qtimeObj.toString("hh:mm:ss ap");
        QDate qdateObj = QDate::currentDate();
        QString strDate = qdateObj.toString("  MMM dd  yyyy  dddd    "); //星期、月份、天、年
//        strTime.prepend(".");
        strDate.append(strTime);
        ui->lab_time->setText(strDate);
    });

    timer2 = new QTimer;
    connect(timer2, QTimer::timeout, [=](){
        timeCount++;
        QString data = QString("%1 : %2 : %3").arg(timeCount/60/60).arg(timeCount/60).arg(timeCount);
        ui->lab_time_2->setText(data);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

#include <QDebug>

int MainWindow::tabRow = 1;
void MainWindow::on_tabWidget_tabBarDoubleClicked(int index)
{
	//填空， 服务器的IP地址
    ip = "192.168.32.128";
    port = 8080; //picSocket
    port1 = 8888; //dataSocket

    if (picSocket == NULL){
        picSocket = new QTcpSocket;
		//填空， picSocket客户端连接主机（服务器）
        picSocket->connectToHost(ip,port);
		
		//填空， picSocket产生readyread信号，执行picRecv()槽函数
        connect(picSocket, SIGNAL(readyRead()),this,SLOT(picRecv()));
		
        connect(picSocket, QTcpSocket::connected, [=](){
        });

        connect(picSocket, QTcpSocket::disconnected, [=](){
            picSocket->close();
            picSocket = NULL;
        });
    }

    if (dataSocket == NULL){
        dataSocket = new QTcpSocket;
		//填空， dataSocket客户端连接主机（服务器）
        dataSocket->connectToHost(ip,port1);
		
		//填空， dataSocket产生readyread信号，执行dataRecv()槽函数
        connect(dataSocket,SIGNAL(readyRead()),this,SLOT(dataRecv()));
		
		
        connect(dataSocket, QTcpSocket::connected, [=](){
            ui->btn_start->setEnabled(true);
            ui->btn_input->setEnabled(true);
            ui->btn_cancle->setEnabled(true);
        });
        connect(dataSocket, QTcpSocket::disconnected, [=](){
            dataSocket->close();
            dataSocket = NULL;

            timer2->stop();
            timeCount = 0;
            ui->lab_time_2->setText("00 : 00 : 00");

            ui->btn_stop->setEnabled(false);
            ui->btn_start->setEnabled(false);
            ui->btn_input->setEnabled(false);
            ui->btn_cancle->setEnabled(false);
        });
    }

    qDebug() << "index = " << index;
    if (index == 0){
        status = 1;
        timer->start(1000);
    } else if (index == 2){
        if(dataSocket->state() == QAbstractSocket::ConnectedState){
            tabRow = 1;
            dataSocket->write("RecordDate", 32);
            dataSocket->flush();
        }
    }
}
//开始上课
void MainWindow::on_btn_start_clicked()
{
    timer2->start(1000);
    dataSocket->write("start", 32);
    dataSocket->flush();
    ui->btn_stop->setEnabled(true);
    ui->btn_start->setEnabled(false);
}
//下课
void MainWindow::on_btn_stop_clicked()
{
    timer2->stop();
    timeCount = 0;
    ui->lab_time_2->setText("00 : 00 : 00");
	//填空， 发送数据
    dataSocket->write("stop", 32);
    dataSocket->flush();
    ui->btn_stop->setEnabled(false);
    ui->btn_start->setEnabled(true);
}
//录入信息
void MainWindow::on_btn_input_clicked()
{
    QString num = ui->edt_num->text();
    QString name = ui->edt_name->text();
    char response[32] = {0};
    sprintf(response, "Input:%s", num.toStdString().data());
    sprintf(response+20, "%s", name.toStdString().data());

	//填空， 发送数据
    dataSocket->write(response, 32);
    dataSocket->flush();
}
//删除信息
void MainWindow::on_btn_cancle_clicked()
{
    QString num = ui->edt_num->text();
    QString name = ui->edt_name->text();
    char response[32] = {0};
    sprintf(response, "Delet:%s", num.toStdString().data());
    sprintf(response+20, "%s", name.toStdString().data());

	//填空， 发送数据
    dataSocket->write(response, 32);
    dataSocket->flush();
}


void MainWindow::picRecv()
{
    int ret , count = 0;
    char response[64] = {0};
    switch (status) {
    case 1:
        if (picSocket->bytesAvailable() < sizeof(response))
            return;
        if ( picSocket->read(response, sizeof(response)) < sizeof(response))
            return;
        sscanf(response, "l:%d x:%f y:%f w:%f h:%f uid:%s", &pic_len, &x, &y, &w, &h, user_id);
        status = 2;
        break;
    case 2:
        if (picSocket->bytesAvailable() < pic_len)
            return ;
        memset(pic_buf, 0, sizeof(pic_buf));

        while(count < pic_len){
            ret = picSocket->read(pic_buf + count, pic_len - count);
            if (ret < 0){
                return;
            }
            count += ret;
        }
        QImage image;
        image.loadFromData((const uchar *)pic_buf, pic_len);
        paintFace(&image, x, y, w, h, user_id);
        ui->label->setScaledContents(true);
		
		//填空， 显示图片
        ui->label->setPixmap(QPixmap::fromImage(image));

        status = 1;
        break;
    }
}

void MainWindow::paintFace(QImage *image, float &x, float &y, float &h, float &w, char *id)
{
    QPainter paint1(image); //为这个QImage构造一个QPainter
    paint1.setCompositionMode(QPainter::CompositionMode_SourceIn);
    //设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
    QPen p(QColor("green"));
    p.setWidth(2);
    paint1.setPen(p);
    paint1.drawRect(x, y, w, h);
    p.setColor("red");
    p.setWidth(6);
    paint1.setPen(p);
    paint1.drawText(x, y - 15, 15, 15, Qt::AlignCenter, id);
}


void MainWindow::dataRecv()
{
    QByteArray data = dataSocket->readAll();
    qDebug() << data.toStdString().data();
    int id, lateNum, leavEarlyNum, truancyNum;
    char name[12] = {0};
    char StuNum[20] = {0};

    sscanf(data.toStdString().data(), "%d %s %s %d %d %d", &id, StuNum, name, &lateNum, &leavEarlyNum, &truancyNum);

    QTableWidgetItem * item2 = new QTableWidgetItem(StuNum);
    QTableWidgetItem * item3 = new QTableWidgetItem(name);
    QTableWidgetItem * item4 = new QTableWidgetItem(QString::number(lateNum));
    QTableWidgetItem * item5 = new QTableWidgetItem(QString::number(leavEarlyNum));
    QTableWidgetItem * item6 = new QTableWidgetItem(QString::number(truancyNum));
    ui->tableWidget->setItem(tabRow, 0, item2);
    ui->tableWidget->setItem(tabRow, 1, item3);
    ui->tableWidget->setItem(tabRow, 2, item4);
    ui->tableWidget->setItem(tabRow, 3, item5);
    ui->tableWidget->setItem(tabRow, 4, item6);
    tabRow++;
}
