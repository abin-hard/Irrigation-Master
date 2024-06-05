#include "widget.h"
#include "ui_widget.h"
#include <QString>
#include <QDebug>
#include <QMessageBox>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromLocal8Bit("��ȴ�ʦ"));
    sock = new QTcpSocket(this);
    flag = false;
    autoMode = false;

    timer = new QTimer(this);
    timer->setInterval(1000);

    connect(timer, &QTimer::timeout, this, &Widget::timer_trigger);  //��ʱ��
    connect(sock, &QTcpSocket::connected, this, &Widget::tcp_connect_success);
    //connect(sock, SIGNAL(connected()), this, SLOT(tcp_connect_failed())); //����Ϊʲô���� ������
    connect(sock, SIGNAL(QTcpSocket::disconnected), this, SLOT(tcp_connect_failed));

    //����
    ui->lcdNumber->setDigitCount(4);
    ui->lcdNumber->setMode(QLCDNumber::Dec);
    ui->lcdNumber_2->setDigitCount(4);
    ui->lcdNumber_2->setMode(QLCDNumber::Dec);
    ui->lcdNumber_3->setDigitCount(4);
    ui->lcdNumber_3->setMode(QLCDNumber::Dec);
    ui->lcdNumber_4->setDigitCount(5);
    ui->lcdNumber_4->setMode(QLCDNumber::Dec);

    window_init();
    setWindowIcon(QIcon(":/photo/resource/sun.png"));
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
}


//
//                tcp����ͨ�Ÿ�ʽ---�ɼ�

// 1byte:  *      //Ĭ�ϵ�һ���ֽ���*�ַ���������֤�Ƿ�����Ҫ������

// 1 byte  0 | 1 | 2  //0��ʾ��Ч��1�����ã�2�ǲɼ�
// 1byte:  0 | 1  //1��ʾ�ɼ�����ǿ�ȣ�0��ʾ���ɼ�
// 1byte:  0 | 1  //1��ʾ�ɼ��¶Ⱥ�ʪ�ȣ� 0��ʾ���ɼ�
//-----------------------------------------------------------------------//
//                tcp����ͨ�Ÿ�ʽ---����

// 1byte:  *      //Ĭ�ϵ�һ���ֽ���*�ַ���������֤�Ƿ�����Ҫ������

// 1 byte  0 | 1 | 2  //0��ʾ��Ч��1�����ã�2�ǲɼ�

//

Widget::~Widget()
{
    delete ui;
}




void Widget::on_pushButton_clicked()
{
    if(!flag) {
        tcp_connect();
    } else {
        sock->abort();
        flag = false;
        ui->pushButton->setText(QString::fromLocal8Bit("����"));
        QMessageBox::information(this, QString::fromLocal8Bit("����"),QString::fromLocal8Bit("���Ѿ��Ͽ��ɹ����ڴ��´��������������"));
    }
}

//tcp���ӳɹ��Ĳۺ���
void Widget::tcp_connect_success() {
    qDebug() << "right";
    flag = true;
    ui->pushButton->setText(QString::fromLocal8Bit("�Ͽ�"));
    ui->wendu->show();
    QMessageBox::information(this, QString::fromLocal8Bit("����"),QString::fromLocal8Bit("���Ѿ����ӳɹ�����ȥʹ�ðɣ�����"));

}
//tcp����ʧ�ܵĲۺ���
void Widget::tcp_connect_failed() {
    QMessageBox::information(this, QString::fromLocal8Bit("����"),QString::fromLocal8Bit("���Ѿ��Ͽ��ɹ����ڴ��´��������������"));
}

void Widget::timer_trigger() {
    if(autoMode && flag) {
        char buffer[24] = "*20"; //dht11 scada
        send_data(buffer);
        recv_data();

        buffer[2] = '1';         //bh1750 scada
        send_data(buffer);
        recv_data();

        buffer[2] = '2';
        send_data(buffer);
        recv_data();
    }
}

//���Ӻ���
void Widget::tcp_connect() {
    QString ip = ui->ipedit->text();
    quint16 port = ui->portedit->text().toShort();
    qDebug() << ip << " -----" << port;
    sock->connectToHost(ip, port);
}

void Widget::send_data(char* buffer) {
        sock->write(buffer);
}

 int check_data(char* buffer, QString& temp, QString& humd, QString& light, QString& moisture) {
    if(!buffer || buffer[0] != '*' || buffer[1] != '2') {
        return -1;
    }

    QString str(buffer);
    if(buffer[2] == '0') { //dht11
        int idx = str.indexOf("t:");
        int end_idx = str.indexOf("&");
        if(idx < 0) {
            return -1;
        } else {
            temp = str.mid(idx + 2, end_idx - (idx + 2));
        }
        idx = str.indexOf("h:");
        end_idx = str.indexOf("$");
        if(idx < 0) {
            return -1;
        } else {
            humd = str.mid(idx + 2, end_idx - (idx + 2));
        }
        return 0;
    } else if(buffer[2] == '1') {  //bh1750
        int idx = str.indexOf("b:");
        int end_idx = str.indexOf("$");
        if(idx < 0)
            return -1;
        light = str.mid(idx + 2, end_idx - (idx + 2));
        return 1;
    } else if(buffer[2] == '2') {
        int idx = str.indexOf("m:");
        int end_idx = str.indexOf("$");
        if(idx < 0)
            return -1;
        moisture = str.mid(idx + 2, end_idx - (idx + 2));
        return 2;
    }

    return -1;
}

void Widget::recv_data() {
    if(!flag) return;

    char buffer[24];
    memset(buffer, 0, sizeof(buffer));

    int total = 0;
    while(total < 24) {
        int ret = sock->read(buffer + total, sizeof(buffer) - total);
        if(ret == -1) {
            QMessageBox::information(this, QString::fromLocal8Bit("���ӶϿ�"), QString::fromLocal8Bit("�Է��Ͽ����ӣ�bye bye"));
            flag = false;
            return;
        } else if(ret > 0) {
            total += ret;
        } else {
            if(!sock->waitForReadyRead(200)) {
                QMessageBox::information(this, QString::fromLocal8Bit("���ӶϿ�"), QString::fromLocal8Bit("�Է��Ͽ����ӣ�bye bye"));
                flag = false;
                return;
            }
        }
    }

    QString temp, humd, light, moisture;
    int re = check_data(buffer, temp, humd, light, moisture);
    if(re == 0) {
        show_collect_data(temp, humd);
    } else if(re == 1) {
        show_collect_illumination(light);
    } else if(re == 2) {
        show_collect_moisture(moisture);
    } else {
        QMessageBox::information(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ݸ�ʽ����"));
    }
}



void Widget::show_collect_data(QString temp, QString humd) {
    ui->lcdNumber->display(temp);
    ui->lcdNumber_2->display(humd);
}

void Widget::show_collect_illumination(QString data) {
//    ui->illumination_brow->setText(data);
    ui->lcdNumber_4->display(data);
}

void Widget::show_collect_moisture(QString moisture) {
    ui->lcdNumber_3->display(moisture);
}



void Widget::on_config_button_clicked()
{
    char buffer[24] = {0};
//    int temp = ui->config_temp->text().toInt();
//    int humd = ui->config_humd->text().toInt();
//    sprintf(buffer, "*10t:%d&h:%d$", temp, humd);                  //*10 is config dht11

    int moisture = ui->config_moisture->text().toInt();
//    if(moisture > 99 || moisture < 0) {
//        moisture = 0;
//        ui->config_moisture->setText(QString::number(moisture));
//    }
    sprintf(buffer, "*11m:%02d$", moisture);                         //*11 is config ����ʪ�ȴ���������ֵ

    sock->write(buffer,sizeof(buffer));

    char recv_buffer[24] = {0};
    int total = 0;
    while(total < 24) {
        int ret = sock->read(recv_buffer + total, sizeof(recv_buffer) - total);
        if(ret == -1) {
            QMessageBox::information(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�һ�û�����ӣ������ӣ�"));
            flag = false;
            return;
        } else if(ret > 0) {
            total += ret;
        } else {
            if(!sock->waitForReadyRead(200)) {
                QMessageBox::information(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�һ�û�����ӣ������ӣ�"));
                flag = false;
                return;
            }
        }
    }

    if(recv_buffer[0] == '*' && recv_buffer[1] == '*') {
        QMessageBox::information(this, QString::fromLocal8Bit("����"),QString::fromLocal8Bit("���óɹ�����Ҫ��ʼ������������"));
    } else {
        QMessageBox::information(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����ʧ�ܣ�������������ˣ�"));
    }

}


//����1����֪������ǿ�ȵĳ���






void Widget::on_scada_button_clicked()
{
    if(flag) {
        char buffer[24] = "*20"; //dht11 scada
        send_data(buffer);
        recv_data();

        buffer[2] = '1';         //bh1750 scada
        send_data(buffer);
        recv_data();

        buffer[2] = '2';
        send_data(buffer);
        recv_data();
    }
}


void Widget::on_auto_water_button_clicked()
{
    if(autoMode) {
        autoMode = false;
        timer->stop();
        ui->scada_button->setEnabled(true);
        ui->auto_water_button->setText(QString::fromLocal8Bit("�ֶ�"));
    } else {
        autoMode = true;
        timer->start();
        ui->scada_button->setEnabled(false);
        ui->auto_water_button->setText(QString::fromLocal8Bit("�Զ�"));
    }
}


void Widget::window_init() {
    file_menu = new QMenu(this);
    file_menu->addAction(QString::fromLocal8Bit("���ļ�"));
    file_menu->addAction(QString::fromLocal8Bit("�ر��ļ�"));

    setting_menu = new QMenu(this);
    setting_menu->addAction(QString::fromLocal8Bit("�˳�"));

    help_menu = new QMenu(this);
    help_menu->addAction(QString::fromLocal8Bit("ʹ���ĵ�"));
    help_menu->addAction(QString::fromLocal8Bit("�汾"));

    ui->file_button->setMenu(file_menu);
    ui->setting_button->setMenu(setting_menu);
    ui->help_button->setMenu(help_menu);
}









