#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <qtcpsocket.h>
#include <QTimer>
#include <QMenu>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();
    //tcp���ӳɹ��Ĳۺ���
    void tcp_connect_success();
    //tcp����ʧ�ܵĲۺ���
    void tcp_connect_failed();

    void on_config_button_clicked();

    void on_scada_button_clicked();

    void timer_trigger();

    void on_auto_water_button_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket* sock;
    QTimer* timer;
    bool autoMode;
    bool flag;
private:
    void tcp_connect();

    void send_data(char* buffer);
    void recv_data();

private:
    //��ʾ�ɼ������ݣ��¶ȡ�ʪ��
    void show_collect_data(QString, QString);
    //��ʾ����ǿ��
    void show_collect_illumination(QString);

    void show_collect_moisture(QString);

private:
    void window_init();
    QMenu* file_menu;
    QMenu* setting_menu;
    QMenu* help_menu;

};
#endif // WIDGET_H
