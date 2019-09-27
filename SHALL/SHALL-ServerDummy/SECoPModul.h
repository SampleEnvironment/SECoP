/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef SECoPModul_H
#define SECoPModul_H

#include <QDialog>
#include <QtWidgets>
#include <QLCDNumber>
#include <QCheckBox>
#include "SECoP.h"

class SECoPModul : public QDialog
{
    Q_OBJECT

public:
    static SECoPModul* theInstance;

    explicit SECoPModul(QWidget *pParent = Q_NULLPTR);
    double getActualTemperatur();
    double getTargetTemperatur();
    void  setTargetTemperatur(double target);
    bool  getRampBool();
    void  setRampBool(bool b);
    double getRampValue();
    void  setRampValue(double rampv);
    void  setCommit(bool b);
    bool  getCommit();
    QString  getStatus();

    static double getActualTemperaturS(void* pParam);
    static double getTargetTemperaturS(void* pParam);
    static void  setTargetTemperaturS(void* pParam, double TargetValue);
    static double getRampValueS(void* pParam);
    static void  setRampValueS(void* pParam, double RampValue);
    static bool  getRampBoolS(void* pParam);
    static void  setRampBoolS(void* pParam, bool b);
    static void  setCommitS(void* pParam, bool b);
    static bool  getCommitS(void* pParam);


    struct
    {
        bool useRampBool;
        double RampValue;
        double TargetTemperatur;
    } buffered, pausebuffer;

private slots:

    void showChanges();
    void setTargetTemperatur();
    void checkTemperaturDifference();
    void useRamp();
    void switchUseRampBool(int bEnabled);
    void setRampValue();

    //additional slots for testing start stop pause and shutdown
public slots:
    void start();
    void pause();
    void stop();
    void reset();
    void shutdown();

private:
    QString status ="idle";
    bool b_commit = true; //only allow changing parameters if it is true TESTING FEATURE atomic
    bool useRampBool = true; // if false use maxramp instead if true use userdefined ramp
    const int maxRampValue = 10;
    bool heatup = false; // since ramprate should be positive a flag for direction was needed
    bool cooldown = false; // since ramprate should be positive a flag for direction was needed
    double ActualTemperatur = 42;
    double TargetTemperatur = 42;
    double RampValue = 2;
    QLCDNumber *LCDActualTemperatur;
    QLCDNumber *LCDTime;
    QLCDNumber *LCDTargetTemperatur;
    QLCDNumber *LCDRampRate;
    QLineEdit *TargetTempLineEdit;
    QLineEdit *RampStepLineEdit;
    QCheckBox *UseRampCheckBox;
    QLabel *RampLabel;
    QLabel *StatusLabel;
    QLabel *Caption1;
    QLabel *Caption2;
    QLabel *Caption3;
    QLabel *Caption4;
};

#endif
