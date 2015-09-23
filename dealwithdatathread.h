#ifndef DEALWITHDATATHREAD_H
#define DEALWITHDATATHREAD_H

#include <QByteArray>
#include <QThread>

#include <QNetworkAccessManager>
#include <QUrl>

class DealWithDataThread : public QThread
{
    Q_OBJECT
public:
    DealWithDataThread(QObject *parent = 0)
        : QThread(parent)
    {
      shelf="B2-6-1-1-1-2-";
      flag=false;

      what=0;
    }
    void run();
private:

 //   QNetworkAccessManager network_manager;
    QByteArray data;

     QList<QByteArray> list;

     bool flag;
    static const int num=48;
    static const int site=6;
    static const int column=8;

     int what;


      QString shelf;

    void SendToServer(QString mode, QString content);

    void parseData(QByteArray data);

    void findState(QList<QByteArray> data);

    QList<QByteArray> DealWithError(QList<QByteArray>);

    int findStartAndEndState(QList<int> intList);
    int findContentState(QList<int> intList);

    QList<int> parseArray(QByteArray byteArray);
    QByteArray generateArray(QList<int> boolList);

    QString reverse(QString);

    void debuger(QList<QByteArray>);
    void debuger(QByteArray);
    void debuger(QList<int>);

        QByteArray change01(QByteArray);

signals:
    void sendRequest(QString,QString);

    void TestDataBack(int);

    void AfterDealWithError(QByteArray);

    void AfterFindStateSuccess(QString,QString);


public slots:
    void ReceiveDataSlot(QByteArray s);
};

#endif // DEALWITHDATATHREAD_H
