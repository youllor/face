#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTime>
#include <QTimer>

#include <QPainter>
#include <QPen>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_tabWidget_tabBarDoubleClicked(int index);
    void picRecv();
    void paintFace(QImage *image, float &x, float &y, float &h, float &w, char *id);

    void dataRecv();

    void on_btn_start_clicked();

    void on_btn_stop_clicked();

    void on_btn_input_clicked();

    void on_btn_cancle_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer, *timer2;


    QTcpSocket *picSocket, *dataSocket;

    int status, pic_len, timeCount;
    float x, y, w, h;
    char pic_buf[1024*1024], user_id[12];
    static int tabRow;

    QString ip;
    int port, port1;
};

#endif // MAINWINDOW_H
