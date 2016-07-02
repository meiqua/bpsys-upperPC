#ifndef DEALWITHDATATHREAD_H
#define DEALWITHDATATHREAD_H

#include <QByteArray>
#include <QThread>

#include <QNetworkAccessManager>
#include <QUrl>
#include <QMutex>

class DealWithDataThread : public QThread
{
    Q_OBJECT
public:
    DealWithDataThread(QMutex* m,QObject *parent = 0)
        : QThread(parent)
    {
      shelf="B2-6-1-1-1-2-";
      flag=false;
      mutex=m;
      what=0;
    }

    void shutdown();
    void run();
private:
    QMutex* mutex;
    bool isShutdownRequested;
 //   QNetworkAccessManager network_manager;
    QByteArray rawData;

    QList<QByteArray> dataList;
    QList<QMap<QString,int> > stateList; // start-end-length

     bool flag;
     static const int site=5;
     static const int column=8;
     int what;
     QString shelf;

    void SendToServer(QString mode, QString id);
    void parseData(QByteArray data);
    void findState(QList<QByteArray> data);
    QList<QMap<QString,int> > generateMap(QList<int> currentList);

    QList<QByteArray> DealWithError(QList<QByteArray>);
    QList<int> parseArray(QByteArray byteArray);
    QByteArray generateArray(QList<int> boolList);
    QString reverse(QString);
    void debuger(QList<QByteArray>);
    void debuger(QByteArray);
    void debuger(QList<int>);
    QByteArray change01(QByteArray);

signals:
    void sendRequest(QString,QString,QString);

    void TestDataBack(int);

    void AfterDealWithError(QByteArray);

    void AfterFindStateSuccess(QString,QString);


public slots:
    void ReceiveDataSlot(QByteArray s);
};

#endif // DEALWITHDATATHREAD_H
