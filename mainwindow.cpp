/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QtNetwork>
#include <QtWidgets>

#include "dealwithdatathread.h"
#include <QVariant>

#include <QCoreApplication>
#include <QTextStream>
#include <stdio.h>               //for debug

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
//! [0]

    dealWithDataThread=new DealWithDataThread(&mutex,this);

      dealWithDataThread->start();

    ui->setupUi(this);


    console = new Console;
    console->setEnabled(false);
    setCentralWidget(console);


//! [1]
    serial = new QSerialPort(this);
//! [1]
    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    initActionsConnections();

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

//! [2]
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
//! [2]

    //connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));

    connect(this,SIGNAL(DataReceived(QByteArray)),dealWithDataThread,SLOT(ReceiveDataSlot(QByteArray)));

    connect(dealWithDataThread,SIGNAL(TestDataBack(int)),this,SLOT(TestDataReceiver(int)));
    connect(dealWithDataThread,SIGNAL(AfterDealWithError(QByteArray)),this,SLOT(AfterDealReceiver(QByteArray)));
    connect(dealWithDataThread,SIGNAL(sendRequest(QString,QString,QString)),this,SLOT(receiveReqeust(QString,QString,QString)));
            //send received data to thread to deal with it

//    connect(reply, SIGNAL(finished()),
//      this, SLOT(redirectRequest()));
//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
    dealWithDataThread->shutdown();
  //  thread->exit(0);
}

//! [4]
void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {
            console->setEnabled(true);
            console->setLocalEchoEnabled(p.localEchoEnabled);
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
            ui->actionConfigure->setEnabled(false);
            ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        ui->statusBar->showMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    serial->close();
    console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    ui->statusBar->showMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    QByteArray data = serial->readAll();

    for(int i=0;i<data.size();i++)
    {
        if(((int)data[i]!=-86)&&((int)data[i]!=85))
        {
         console->putData(reverse(QString::number((int)data[i],2)));
         console->putData("  ");
        }
    }

    console->putData("\n");
    emit DataReceived(data);

    //send data to a new thread

}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::TestDataReceiver(int test)
{
     console->putData("data has benn put in dealing thread for :");
     console->putData(QString::number(test,10));
     console->putData("\n");
}

void MainWindow::AfterDealReceiver(QByteArray next)
{

    console->putData("after correcting error, data stored in the dealing thread :");
    console->putData("\n");
        for(int i=0;i<next.size();i++)
        {
            if(((int)next[i]!=-86)&&((int)next[i]!=85))
            {
             console->putData(reverse(QString::number((int)next[i],2)));
             console->putData("  ");
            }
        }
        console->putData("\n");
        console->putData("\n");
}

void MainWindow::receiveReqeust(QString mode, QString id,QString shelf)
{
    QString url="http://139.196.25.125:8080/library/struts2/dispatch?key="+mode+"&id="+shelf+id;

//        redirectRequest(req);
          //for safety ,qt can't auto redirect.
        //it's auto 异步
            this->url=QUrl(url);

             startRequest(this->url);
        console->putData("mode: "+mode+"\n");
        console->putData("url: "+url+"\n");
}

QString MainWindow::reverse(QString s)
{
    QString temp;
    for(int i=0;i<s.size();i++)
    {
       temp.append(s.at(s.size()-i-1));
    }
    return temp;
}

//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConfigure, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->actionClear, SIGNAL(triggered()), console, SLOT(clear()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::startRequest(QUrl url)
{

        reply = network_manager.get(QNetworkRequest(url));
        connect(reply, SIGNAL(finished()),
                this, SLOT(redirectRequest()));
}

void MainWindow::redirectRequest()
{
      QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

      if (!redirectionTarget.isNull()) {
              QUrl newUrl = url.resolved(redirectionTarget.toUrl());
              url = newUrl;
              reply->deleteLater();
              startRequest(url);
      }else{
          reply->deleteLater();
      }

}

