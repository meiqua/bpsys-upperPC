#include "dealwithdatathread.h"
#include <QNetworkRequest>


#include <QCoreApplication>
#include <QTextStream>
#include <stdio.h>               //for debug

void DealWithDataThread::run()
{
    //int counter=0;
    while(1)
    {

        if(flag)
        {
            findState(DealWithError(list));
               QTextStream cout(stdout);
               cout<<"-------------------------------------"<<endl;
               cout<<"list we have is :"<<endl;
            debuger(DealWithError(list));
               cout<<"-------------------------------------"<<endl;
            flag=false;
     //       counter++;
     //      emit TestDataBack(what);





        }
    }

}

void DealWithDataThread::SendToServer(QString mode, QString content)
{
//    QString url="http://139.196.25.125:8080/library/struts2/dispatch?key="+mode+"&content="+content;
//    QNetworkRequest req;
//        req.setUrl(QUrl(url));
//        network_manager.get(req);     //it's auto 异步

       emit sendRequest(mode,content);
}

void DealWithDataThread::parseData(QByteArray data)
{
   list.clear();
  for(int i=0;i<data.size();i++)
  {

      //attention
        if(((int)data[i]==-86)&((int)data[i+column+1]==85)) //一组10个数据 开头0xAA 结尾0x55
 //I don't konw why,but the data received is -86 and 85
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


                list.append(change01(tempArray));//the kaiGuan is closed when unpress it.
                                                 //so need to change 01

        }
  }
}

void DealWithDataThread::findState(QList<QByteArray> data)
{
    QString mode; //mode=fetch or return
    QString content;
    bool sendReady=false;

    int startState=0;
    int endState=0;
    int length=0;

    int contentStart=1000;
    int contentEnd;

    //this method is to find special states  when someone fetch or return a book
   // QString s = QString::number(a, 10);
    QListIterator<QByteArray> i(data);
    int counter=-1;
    int endStateIndex=0;
    while (i.hasNext())                      //find start and end state
    {
        counter++;
        QList<int> intList;
        intList=parseArray(i.next());

        if(findStartAndEndState(intList)>0)  //find state of 00001111111
        {
            if(startState==0)
            {
                startState=findStartAndEndState(intList);
            }else
            {
                endState=findStartAndEndState(intList);

                endStateIndex=counter; //find endStateIndex ,prepare for nexrt movement
            }
        }

    }

    length=endState-startState;

    if(startState>0 && endState>0)   //fetch or return movement is finished
    {
        if(length>0)
        {
            mode="return";
            sendReady=true;
        }
        else if (length<0)
        {
            mode="fetch";
            length=0-length;
            sendReady=true;
        }

        i.toFront();
        while (i.hasNext())                     //find content
        {
            QList<int> intList;
            intList=parseArray(i.next());
            if(findContentState(intList)<contentStart)
            {
                contentStart=findContentState(intList); //find first 1 index
            }
        }

        contentEnd=contentStart+length;
        content=shelf+QString::number(contentStart, 10)+"to"+QString::number(contentEnd, 10);


        //once states are all found ,byteArray can be clear
        this->data.remove(0,(endStateIndex)*(column+2));  //留下一个endstate的数据
                                                           //prepare for next movement
        //can't be data!!!!!!!!!!!!!!!
        //much time on this...

   //     this->data.append(data.last());

//        i.toFront();
//        counter=-1;
//        while (i.hasNext())                     //prepare for next movement
//        {
//            counter++;

//            if(counter>=endStateIndex)
//            {
//                QByteArray next=i.next();

//                this->data.append(next);
//                QTextStream cout(stdout);
//              cout<<"_______________________"<<endl;
//              cout<<"data will append :"<<endl;
//              debuger(next);
//              cout<<"_______________________"<<endl;

//            }else
//            {
//                i.next();
//            }

//        }
        if(sendReady)
        {
            SendToServer(mode,content);
            sendReady=false;
        }

        startState=0;
        endState=0;
    }
}

QList<QByteArray> DealWithDataThread::DealWithError(QList<QByteArray> data)
{
    //make 101 to 111   010 to 000


    //below may seem complicated
    //I use QMutableListIterator at the beginning
    //but don't konw why I get a wrong data in a positon
    //after debug for a long time for below codes


//    QList<int> intList;
//    intList.append(1);
//    intList.append(1);
//    intList.append(1);
//    intList.append(0);
//    intList.append(1);
//    intList.append(1);
//    QMutableListIterator<int> j(intList);
//    int previous=1;//it will decide the first site is 0 or 1; =1 make it 1;
//    int next;
//    cout<<"debuging..."<<endl;
//    while(j.hasNext())
//    {
//        j.next();
//        if(j.hasPrevious())
//        {
//            previous=j.previous();
//            j.next();
//        }
//        if(j.hasNext())
//        {
//            next=j.next();
//            j.previous();
//        }
//        if(previous==next) //like 010 or 101
//        {
//            j.setValue(previous);
//        }
//    }


//    cout<<"now intList is ";

//    j.toFront();
//    while(j.hasNext())
//    {
//        cout<<j.next();
//    }
//    cout<<endl;


    //I think it may be a bug of QList iterator
    //so at last I just use array....

    QList<QByteArray> temp;
    temp.clear();

    QListIterator<QByteArray> i(data);
    while (i.hasNext())
    {
        //qDebug() << i.next();
        QList<int> intList;
        intList=parseArray(i.next());

        QListIterator<int> j(intList);
        int temArray[1000];          //I think 1000 is enough
        int counter=-1;
        while(j.hasNext())
        {
            counter++;
            temArray[counter]=j.next();
        }
          for(int iter=intList.size()-2;iter>=0;iter--)
          {
              if(iter==0)
              {
                  if(temArray[iter+1]==1)
                  {
                      temArray[iter]=1;
                  }
              }else
              {
                  if(temArray[iter-1]==temArray[iter+1])
                  {
                      temArray[iter]=temArray[iter-1];
                  }
              }
          }

          QList<int> intListed;
          for(int iter=0;iter<intList.size();iter++)
          {
              intListed.append(temArray[iter]);
          }

        temp.append(generateArray(intListed));

        //below is for debug

//            QTextStream cout(stdout);

//            QListIterator<int> jj(intListed);
//            while(jj.hasNext())
//            {
//                cout<<jj.next();
//            }
//            cout<<endl;


//            for(int i=0;i<array.size();i++)
//            {
//                if(((int)array[i]!=-86)&&((int)array[i]!=85))
//                {
//                 cout<<reverse(QString::number((int)array[i],2))<<"  ";
//                }
//            }


  //     emit AfterDealWithError(generateArray(intList));
    }



    return temp;
}

int DealWithDataThread::findStartAndEndState(QList<int> intList)
{
    //find this kind of state : 0000111111111   find number of 0

    int temp=0;

    int counter=0;
    int numOf01=0;

    QListIterator<int> i(intList);
    int temArray[1000];
    int cc=-1;
//    int next;
    while (i.hasNext())
    {
        cc++;
       temArray[cc]=i.next();

//        if(next==0)
//        {
//            counter++;
//        }

//        if(previous==0&&next==1)
//        {
//            numOf01++;
//        }

//        if(numOf01==1)
//        {
//            temp=counter;
//        }

    }

    for(int j=0;j<intList.size()-1;j++)
    {
        if(temArray[j]==0)
        {
            counter++;
        }

        if((temArray[j]==0&&temArray[j+1]==1)||(temArray[j]==1&&temArray[j+1]==0))
        {
            numOf01++;  //make sure 01 or 10 just has one
        }
    }

    if(numOf01==1)
    {
        temp=counter;
    }

    return temp;
}

int DealWithDataThread::findContentState(QList<int> intList)
{
    int temp=0;
    QListIterator<int> i(intList);
    int next;
    int counter=-1;
    while (i.hasNext())
    {

        counter++;
        next=i.next();

        if(next==1)
        {
            temp=counter;   //find first 1 in the list
            i.toBack();
        }
    }
    return temp;
}

QList<int> DealWithDataThread::parseArray(QByteArray byteArray)
{
    QList<int> intList;
    for(int i=0;i<byteArray.size();i++)
    {
        for(int j=0;j<site;j++)
        {
            intList.append((byteArray[i]>>(j%site))%2);
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

    //once data is changed, execute the main codes in run method

//    while(flag) //run is calling  can't change flag at this time
//    {
//        ;       //or main codes in run may not be called
//    }
    data.append(s);
    parseData(data);
        flag=true;
}


