/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QtWidgets>
#include <QLCDNumber>
#include <QCheckBox>
#include "SECoPModul.h"

SECoPModul* SECoPModul::theInstance = nullptr;

//needs a rework if time is left but on the other hand the demonstrator semms to work like intended and its just for show

SECoPModul::SECoPModul(QWidget *pParent)
    : QDialog(pParent)
    , LCDActualTemperatur(new QLCDNumber(this))         // LCD for temperature
    , LCDTime(new QLCDNumber(this))                     // LCD for systemtime
    , LCDTargetTemperatur(new QLCDNumber(this))         // LCD for temperature wich is set
    , LCDRampRate(new QLCDNumber(this))                 // LCD for ramprate
    , TargetTempLineEdit(new QLineEdit(this))           // set the targettemperature here
    , RampStepLineEdit(new QLineEdit(this))             // if a ramp is used set the rate here
    , UseRampCheckBox(new QCheckBox(this))              // enables the rampusage if not maxRampValue is used
    , RampLabel(new QLabel("true", this))
    , StatusLabel(new QLabel("idle", this))
    , Caption1(new QLabel("act. Temp. [K]", this))
    , Caption2(new QLabel("Target [K]", this))
    , Caption3(new QLabel("Ramprate [K/s]", this))
    , Caption4(new QLabel("Ramp active", this))

{
    buffered.RampValue = 2;
    buffered.TargetTemperatur = 42;
    buffered.useRampBool = true;
    theInstance = this;
    UseRampCheckBox->setChecked(true);
    TargetTempLineEdit->setText(QString::number(TargetTemperatur));
    TargetTempLineEdit->setValidator(new QDoubleValidator(0, 999, 2, this));
    RampStepLineEdit->setText(QString::number(RampValue));
    RampStepLineEdit->setValidator(new QDoubleValidator(0, 10, 2, this));
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm:ss");
    LCDTime->setDigitCount(8);
    LCDTime->setSegmentStyle(QLCDNumber::Flat);
    LCDTime->display(text);

    LCDActualTemperatur->setDigitCount(8);
    LCDActualTemperatur->setSegmentStyle(QLCDNumber::Flat);
    LCDActualTemperatur->display(ActualTemperatur);

    LCDTargetTemperatur->setDigitCount(8);
    LCDTargetTemperatur->setSegmentStyle(QLCDNumber::Flat);
    LCDTargetTemperatur->display(TargetTemperatur);

    LCDRampRate->setDigitCount(8);
    LCDRampRate->setSegmentStyle(QLCDNumber::Flat);
    LCDRampRate->display(getRampValue());

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QPushButton *startButton = new QPushButton(tr("Go"), this);
    QPushButton *pauseButton = new QPushButton(tr("Hold"), this);
    QPushButton *stopButton = new QPushButton(tr("Stop"), this);
    QPushButton *shutdownButton = new QPushButton(tr("Shutdown"), this);
    QPushButton *resetButton = new QPushButton(tr("Reset"), this);
    QPushButton *quitButton = new QPushButton(tr("Quit"), this);

    quitButton->setAutoDefault(false);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(pauseButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(stopButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(resetButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(shutdownButton, QDialogButtonBox::ActionRole);
    QGridLayout *mainLayout = Q_NULLPTR;

    mainLayout = new QGridLayout(this);
    mainLayout->addWidget(Caption1, 2, 0);
    mainLayout->addWidget(Caption2, 2, 2);
    mainLayout->addWidget(Caption3, 2, 3);
    mainLayout->addWidget(Caption4, 2, 4);
    mainLayout->addWidget(LCDActualTemperatur, 3, 0);
    mainLayout->addWidget(LCDTargetTemperatur, 3, 2);
    mainLayout->addWidget(TargetTempLineEdit, 4, 2);
    mainLayout->addWidget(LCDRampRate,3 ,3);
    mainLayout->addWidget(RampLabel, 3, 4);
    mainLayout->addWidget(RampStepLineEdit, 4, 3);
    mainLayout->addWidget(UseRampCheckBox, 4, 4);

    mainLayout->addWidget(LCDTime, 5, 0);
    mainLayout->addWidget(StatusLabel, 6, 0);
    mainLayout->addWidget(buttonBox, 7, 0,1,5);

    setWindowTitle("Hotplate Testdummy for SECoP");

//------------------SIGNAL-SLOT-CONNECTIONS-------------------------------

    connect(quitButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(startButton, SIGNAL(clicked(bool)), this, SLOT(start()));
    connect(pauseButton, SIGNAL(clicked(bool)), this, SLOT(pause()));
    connect(stopButton, SIGNAL(clicked(bool)), this, SLOT(stop()));
    connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(reset()));
    connect(shutdownButton, SIGNAL(clicked(bool)), this, SLOT(shutdown()));


    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showChanges())); // refresh Time each second
    timer->start(1000);

    QTimer *checktarget = new QTimer(this);
    connect(checktarget, SIGNAL(timeout()), this, SLOT(checkTemperaturDifference())); // check each 100ms for changing temperatur
    checktarget->start(100);

    connect(TargetTempLineEdit, SIGNAL(editingFinished()),
            this, SLOT(setTargetTemperatur()));
    connect(RampStepLineEdit, SIGNAL(editingFinished()),
            this, SLOT(setRampValue()));
    connect(UseRampCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(switchUseRampBool(int)));

}

void SECoPModul::start()
{
    if (status == "paused")
    {
        setRampValue( pausebuffer.RampValue);
        setRampBool( pausebuffer.useRampBool);
        setTargetTemperatur(pausebuffer.TargetTemperatur);
        LCDTargetTemperatur->display(pausebuffer.TargetTemperatur);
        LCDRampRate->display(pausebuffer.RampValue);
        status = "started";
        StatusLabel->setText(status);
        start();
    }
    else
    {
        setCommit(true);
        status="started";
        StatusLabel->setText(status);
    }
}

void SECoPModul::pause()
{
    pausebuffer.RampValue = RampValue;
    pausebuffer.TargetTemperatur = TargetTemperatur;
    pausebuffer.useRampBool = useRampBool;
    UseRampCheckBox->setChecked(true);
    RampStepLineEdit->setText(QString::number(pausebuffer.RampValue));
    TargetTempLineEdit->setText(QString::number(pausebuffer.TargetTemperatur));
    TargetTemperatur = getActualTemperatur();
    RampValue = 0;
    useRampBool = true;

    LCDTargetTemperatur->display(getActualTemperatur());
    LCDRampRate->display(getRampValue());
    status = "paused";
    StatusLabel->setText(status);
}

void SECoPModul::stop()
{
    RampValue = 0;
    useRampBool = true;
    UseRampCheckBox->setChecked(true);
    TargetTemperatur = ActualTemperatur;
    LCDTargetTemperatur->display(TargetTemperatur);
    LCDRampRate->display(RampValue);
    RampStepLineEdit->setText(QString::number(RampValue));
    TargetTempLineEdit->setText(QString::number(TargetTemperatur));
    status = "stopped";
    StatusLabel->setText(status);
    buffered.TargetTemperatur = TargetTemperatur;
    buffered.RampValue = RampValue;
    buffered.useRampBool = useRampBool;
}

void SECoPModul::reset()
{
    stop();
    RampValue = 2;
    setRampBool(true);
    LCDRampRate->display(2);
    RampStepLineEdit->setText("2");
    RampLabel->setText("true");
    buffered.RampValue=2;
    status = "reseted";
    StatusLabel->setText(status);
}

void SECoPModul::shutdown()
{
    setRampBool(true);
    setRampValue(3.1415);
    setTargetTemperatur(23);
    start();
    status = "shutting down";
    StatusLabel->setText(status);
}

void SECoPModul::useRamp()
{
    double RampValue;
    if (useRampBool)
        RampValue = RampStepLineEdit->text().toDouble(); // use ramp with given rate
    else
        RampValue = maxRampValue;                        // use ramp with max rate
    if (fabs(ActualTemperatur - TargetTemperatur) < RampValue)
    {
        ActualTemperatur = TargetTemperatur;
    }
    else
    {
        if (heatup)
            ActualTemperatur = ActualTemperatur + RampValue;
        if (cooldown)
            ActualTemperatur = ActualTemperatur - RampValue;

    }
}

double SECoPModul::getActualTemperatur()
{
    return ActualTemperatur;
}

double SECoPModul::getTargetTemperatur()
{
    return TargetTemperatur;
}

double SECoPModul::getRampValue()
{
    return RampValue;
}

void SECoPModul::setRampValue(double rampv)
{
    b_commit = false;
    buffered.RampValue = rampv;
    RampStepLineEdit->setText(QString::number(rampv));
    LCDRampRate->display(getRampValue());
}

void SECoPModul::setRampValue()
{
    b_commit=false;
    buffered.RampValue = RampStepLineEdit->text().toDouble();
    LCDRampRate->display(getRampValue());
}

void SECoPModul::setTargetTemperatur(double target)
{
    b_commit=false;
    buffered.TargetTemperatur = target;
    TargetTempLineEdit->setText(QString::number(target));
}

void SECoPModul::setTargetTemperatur()
{
    b_commit=false;
    buffered.TargetTemperatur = TargetTempLineEdit->text().toDouble();
    LCDTargetTemperatur->display(TargetTempLineEdit->text());
}

void SECoPModul::checkTemperaturDifference()
{
    if (TargetTemperatur < ActualTemperatur)
    {
        cooldown = true;
        heatup = false;
    }
    else if (TargetTemperatur > ActualTemperatur)
    {
        cooldown = false;
        heatup = true;
    }
    else
    {
        cooldown = false;
        heatup = false;
        if (status=="started")
        {
            status="idle";
        }
        else if (status=="shutting down")
        {
            status="finished";
        }
        StatusLabel->setText(status);
    }
}

void SECoPModul::showChanges()
{
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm:ss");
    if (heatup || cooldown)
        useRamp();
    LCDTime->display(text);
    LCDActualTemperatur->display(ActualTemperatur);
    LCDRampRate->display(RampValue);
    if (UseRampCheckBox->isChecked())
    {
        RampLabel->setText("true");
        RampStepLineEdit->setEnabled(true);
    }
    else
    {
        RampLabel->setText("false");
        LCDRampRate->display(maxRampValue);
        RampStepLineEdit->setEnabled(false);
    }
}

bool SECoPModul::getRampBool()
{
    return useRampBool;
}

void SECoPModul::setCommit(bool b)
{
    if (b)
    {
        LCDTargetTemperatur->display(buffered.TargetTemperatur);
        useRampBool = buffered.useRampBool;
        RampValue = buffered.RampValue;
        TargetTemperatur = buffered.TargetTemperatur;
        b_commit=true;
    }
}

bool SECoPModul::getCommit()
{
    return b_commit;
}

QString SECoPModul::getStatus()
{
    return status;

}

void SECoPModul::setRampBool(bool b)
{
    b_commit = false;
    buffered.useRampBool = b;
    UseRampCheckBox->setChecked(b);
}

void SECoPModul::switchUseRampBool(int bEnabled)
{
    b_commit = false;
    buffered.useRampBool = (bEnabled != 0);
}

double SECoPModul::getActualTemperaturS(void* pParam)
{
    return static_cast<SECoPModul*>(pParam)->getActualTemperatur();
}

double SECoPModul::getTargetTemperaturS(void* pParam)
{
    return static_cast<SECoPModul*>(pParam)->getTargetTemperatur();
}

void SECoPModul::setTargetTemperaturS(void* pParam, double TargetValue)
{
    return static_cast<SECoPModul*>(pParam)->setTargetTemperatur(TargetValue);
}

double SECoPModul::getRampValueS(void* pParam)
{
    return static_cast<SECoPModul*>(pParam)->getRampValue();
}

void SECoPModul::setRampValueS(void* pParam, double RampValue)
{
    return static_cast<SECoPModul*>(pParam)->setRampValue(RampValue);
}

bool SECoPModul::getRampBoolS(void* pParam)
{
    return static_cast<SECoPModul*>(pParam)->getRampBool();
}

void SECoPModul::setRampBoolS(void* pParam, bool b)
{
    return static_cast<SECoPModul*>(pParam)->setRampBool(b);
}

void SECoPModul::setCommitS(void* pParam, bool b)
{
   return static_cast<SECoPModul*>(pParam)->setCommit(b);
}

bool SECoPModul::getCommitS(void* pParam)
{
    return static_cast<SECoPModul*>(pParam)->getCommit();
}
