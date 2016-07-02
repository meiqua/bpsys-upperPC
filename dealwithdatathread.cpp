#include "dealwithdatathread.h"
#include <QNetworkRequest>


#include <QCoreApplication>
#include <QTextStream>
#include <stdio.h>               //for debug


void DealWithDataThread::shutdown()
{
    isShutdownRequested=true;
}

void DealWithDataThread::run()
{
    //int counter=0;
    isShutdownRequested=false;
    while(1)
    {

        if(flag)
        {
               findState(DealWithError(dataList));
               QTextStream cout(stdout);
               cout<<"-------------------------------------"<<endl;
               cout<<"list we have is :"<<endl;
               debuger(DealWithError(dataList));
               cout<<"-------------------------------------"<<endl;
            flag=false;
     //       counter++;
     //      emit TestDataBack(what);
        }
        if(isShutdownRequested)
        {
            break;
        }
    }

}

void DealWithDataThread::SendToServer(QString mode, QString id)
{
//    QString url="http://139.196.25.125:8080/library/struts2/dispatch?key="+mode+"&content="+content;
//    QNetworkRequest req;
//        req.setUrl(QUrl(url));
//        network_manager.get(req);     //it's auto 异步

       emit sendRequest(mode,id,shelf);
}

void DealWithDataThread::parseData(QByteArray data)
{
    //run in main thread
  for(int i=0;i<data.size();i++)
  {
        if(((int)data[i]==-86)&((int)data[i+column+1]==85)) //一组10个数据 开头0xAA 结尾0x55
      // the data received is -86 and 85
        {
//            bool temp[num];
            QByteArray tempArray;
            tempArray.clear();
            for(int j=0;j<column;j++)
            {
               // temp[j]=(data[i+1+j/6]>>(j%6))%2;
                //if book press,state is 0
                   tempArray.append(data[i+j+1]);
            }
                mutex->lock();
                dataList.append(change01(tempArray));//the kaiGuan is closed when unpress it.
                                                 //so need to change 01
                mutex->unlock();
                flag=true;
                rawData.clear();
        }
  }
}

void DealWithDataThread::findState(QList<QByteArray> data)
{
    //run in child thread
    mutex->lock();
    dataList.clear();
    mutex->unlock();

    QString mode; //mode=fetch or return or initial

    QListIterator<QByteArray> i(data);
    while (i.hasNext())
    {
        QByteArray tempArray = i.next();
        QList<int> currentList;
        currentList = parseArray(tempArray);
        QList<QMap<QString,int> > tempStateList = generateMap(currentList);
        QList<QMap<QString,int> > updatedStateList;
        int fetchOne[stateList.size()]={0};

        if(stateList.isEmpty()){
            stateList.append(tempStateList);
            mode="initial";
            for(int i=0;i<stateList.size();i++){
                    QString idString = QString::number(stateList.at(i)["id"],10);
                    SendToServer(mode,idString);
            }
        }else{
            int lastJ=0;
            int tolerance=1;
            for(int i=0;i<tempStateList.size();i++){
                QMap<QString,int> tempMap;
                tempMap = tempStateList.at(i);
                for(int j=lastJ;j<stateList.size();j++){
                    QMap<QString,int> updatedMap;
                    QMap<QString,int> recordMap;
                    recordMap = stateList.at(j);

                    updatedMap["start"]=tempMap["start"];
                    if(((updatedMap["start"]-recordMap["start"])<=tolerance)
                            ||((updatedMap["start"]-recordMap["start"])>=-tolerance)){//continous condition
                        lastJ=j;
                        fetchOne[j]=1;
                        if(tempMap["end"]==tempMap["start"]){//skew book match
                            updatedMap["length"]=tempMap["length"];
                            updatedMap["end"]=tempMap["end"];
                            updatedMap["id"]=updatedStateList.size();
                            updatedStateList.append(updatedMap);
                        }else{//erect book match
                            updatedMap["length"]=recordMap["length"];
                            updatedMap["end"]=updatedMap["start"]+recordMap["length"]-1;
                            updatedMap["id"]=updatedStateList.size();
                            updatedStateList.append(updatedMap);
                            while(updatedMap["end"]+1<tempMap["end"]){
                                updatedMap["start"]=updatedMap["end"]+1;
                                for(int nestedJ=lastJ;nestedJ<stateList.size();nestedJ++){
                                    recordMap = stateList.at(nestedJ);
                                    if(((updatedMap["start"]-recordMap["start"])<=tolerance)
                                            ||((updatedMap["start"]-recordMap["start"])>=-tolerance)){//continous condition
                                        lastJ=nestedJ;
                                        fetchOne[nestedJ]=1;
                                        updatedMap["length"]=recordMap["length"];
                                        updatedMap["end"]=updatedMap["start"]+recordMap["length"]-1;
                                        updatedMap["id"]=updatedStateList.size();
                                        updatedStateList.append(updatedMap);
                                    }else if((recordMap["start"]-updatedMap["start"])>tolerance){
                                        mode="return";
                                        updatedMap["end"]=recordMap["start"]-1;
                                        updatedMap["length"]=updatedMap["end"]-updatedMap["start"]+1;
                                        updatedMap["id"]=updatedStateList.size();
                                        updatedStateList.append(updatedMap);
                                        QString idString = QString::number(updatedMap["id"],10);
                                        SendToServer(mode,idString);
                                    }
                                }
                            }
                        }
                        break;
                    }else if((recordMap["start"]-tempMap["start"])>tolerance){
                        mode="return";
                        if(tempMap["end"]==tempMap["start"]){
                            updatedMap["length"]=tempMap["length"];
                            updatedMap["end"]=tempMap["end"];
                            updatedMap["id"]=updatedStateList.size();
                            updatedStateList.append(updatedMap);
                            QString idString = QString::number(updatedMap["id"],10);
                            SendToServer(mode,idString);
                        }else{
                            updatedMap["end"]=recordMap["start"]-1;
                            updatedMap["length"]=updatedMap["end"]-updatedMap["start"]+1;
                            updatedMap["id"]=updatedStateList.size();
                            updatedStateList.append(updatedMap);
                            QString idString = QString::number(updatedMap["id"],10);
                            SendToServer(mode,idString);
                        }
                        break;
                    }
                }
            }
            for(int i=0;i<stateList.size();i++){
                if(fetchOne[i]==0){
                    mode="fetch";
                    QString idString = QString::number(stateList.at(i)["id"],10);
                    SendToServer(mode,idString);
                }
            }
        }
        stateList.clear();
        stateList.append(updatedStateList);
    }
}

QList<QMap<QString, int> > DealWithDataThread::generateMap(QList<int> currentList)
{
    QList<QMap<QString,int> > tempStateList;
    int id=-1;
    for(int i=-1;i<currentList.size()-2;i++){
        bool tempFlag = false;
        if(i==-1){
            tempFlag = (currentList.at(i+1)==1);
        }else{
            tempFlag = ((currentList.at(i)==0)&&(currentList.at(i+1)==1));
        }
        if(tempFlag){
            id++;
            for(int j=i+1;j<currentList.size()-1;j++){
                if(currentList.at(j)==1&&currentList.at(j+1)==0){
                    QMap<QString,int> tempMap;
                    tempMap["start"]=i+1;
                    tempMap["end"]=j;
                    tempMap["length"]=j-i;//actual occupied length
                    if(tempMap["length"]==1){
                        int k;
                        for(k=j+1;k<currentList.size();k++){
                            if(currentList.at(k)==1){
                                break;
                            }
                        }
                        tempMap["length"]=k-j;//estimated length，>= book width
                    }
                    tempMap["id"]=id;
                    tempStateList.append(tempMap);
                    break;
                }
            }
        }
    }
    return tempStateList;
}

QList<QByteArray> DealWithDataThread::DealWithError(QList<QByteArray> data)
{
    return data;
}

QList<int> DealWithDataThread::parseArray(QByteArray byteArray)
{
    QList<int> intList;
    if (!byteArray.isEmpty())
    {
        for(int i=0;i<byteArray.size();i++)
        {
            for(int j=0;j<site;j++)
            {
                intList.append((byteArray[i]>>(j%site))%2);
            }

        }
    }
    return intList;

}

QByteArray DealWithDataThread::generateArray(QList<int> intList)
{
    QByteArray byteArray;
    int counter=0;
    int temp[site];
    QListIterator<int> i(intList);
    while (i.hasNext()) {
        //qDebug() << i.next();
       temp[counter]= i.next()<<(counter%site);
       counter++;
       if(counter%site==0)
       {
         counter=0;
         int total=0;
         for(int j=0;j<site;j++)
         {
             total=temp[j]+total;
         }
         byteArray.append((char)total);
       }
    }
    return byteArray;
}

QString DealWithDataThread::reverse(QString s)
{
    QString temp;
    for(int i=0;i<s.size();i++)
    {
       temp.append(s.at(s.size()-i-1));
    }
    return temp;
}

void DealWithDataThread::debuger(QList<QByteArray> list)
{

    QTextStream cout(stdout);
                QListIterator<QByteArray> jj(list);
                while(jj.hasNext())
                {
                    debuger(jj.next());
                    cout<<endl;
                }

}

void DealWithDataThread::debuger(QByteArray s)
{
    QTextStream cout(stdout);
          for(int i=0;i<s.size();i++)
          {
              if(((int)s[i]!=-86)&&((int)s[i]!=85))
              {
               cout<<(reverse(QString::number((int)s[i],2)))<<"  ";
              }
              cout<<endl;
          }
}

void DealWithDataThread::debuger(QList<int> list)
{
        QTextStream cout(stdout);
                    QListIterator<int> jj(list);
                    while(jj.hasNext())
                    {
                        cout<<jj.next();
                    }
                    cout<<endl;
}

QByteArray DealWithDataThread::change01(QByteArray s)
{
    QByteArray temp;
    QList<int> myList;
    QList<int> tempList=parseArray(s);
    QListIterator<int> i(tempList);
    int next;
    while(i.hasNext())
    {
        next=i.next();
        if(next>0)
        {
           next=0;
        }
        else if(next==0)
        {
          next=1;
        }
        myList.append(next);
    }
    temp=generateArray(myList);
    return temp;
}

void DealWithDataThread::ReceiveDataSlot(QByteArray s)
{

    //run in main thread
    rawData.append(s);
    parseData(rawData);

}


