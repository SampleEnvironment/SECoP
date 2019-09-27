/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "exports.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->quitButton, SIGNAL(clicked()), QApplication::instance(), SLOT(quit()), Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_showGUI_stateChanged(int arg1)
{
    SECoP_C_ShowGui(arg1 != 0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    QApplication::instance()->quit();
}


/*
void MainWindow::on_activeCheck_stateChanged()
{
    if(ui->activeCheck->isChecked())
        SECoP_C_startInactive();
}
*/
