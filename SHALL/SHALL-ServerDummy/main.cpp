/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QApplication>
#include <chrono>
#include <thread>

#include "SECoPModul.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define qUtf8Printable(string) QString(string).toUtf8().constData()
#endif

#define ALLOW_ADD_VALUEERRORS 0
//define ALLOW_ADD_VALUEERRORS 8

#if defined(ALLOW_ADD_VALUEERRORS)
#if ALLOW_ADD_VALUEERRORS <= 1
#undef ALLOW_ADD_VALUEERRORS
#endif
#endif

void Local_GetTemperature(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(piError);
    Q_UNUSED(timestamp);
    *ppSigma = SECoP_V_create("{\"type\":\"double\"}");
    if (SECoPModul::theInstance != nullptr)
        SECoP_V_modifyDouble(*ppData, 0, 0, SECoPModul::theInstance->getActualTemperatur());
    SECoP_V_modifyDouble(*ppSigma, 0, 0, 0.01); // fixed error value
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_GetTarget(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(piError);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    if (SECoPModul::theInstance != nullptr)
        SECoP_V_modifyDouble(*ppData, 0, 0, SECoPModul::theInstance->getTargetTemperatur());
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_SetTarget(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    printf("settarget\n");
    fflush(stdout);
    if (SECoPModul::theInstance != nullptr)
    {
        double dblTarget(std::numeric_limits<double>::quiet_NaN());
        if (SECoP_V_getDouble(*ppData, 0, 0, &dblTarget))
        {
            if (dblTarget < -273.15)
                dblTarget = -273.15;
            else if (dblTarget > 1000.0)
                dblTarget = 1000.0;
            SECoPModul::theInstance->setTargetTemperatur(dblTarget);
            SECoP_V_modifyDouble(*ppData, 0, 0, dblTarget);
        }
        else
            *piError = SECoP_S_ERROR_INVALID_VALUE;
    }
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_GetRamp(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(piError);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    if (SECoPModul::theInstance != nullptr)
        SECoP_V_modifyDouble(*ppData, 0, 0, SECoPModul::theInstance->getRampValue());
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_SetRamp(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    printf("setramp\n");
    fflush(stdout);
    if (SECoPModul::theInstance != nullptr)
    {
        double dblRamp(std::numeric_limits<double>::quiet_NaN());
        if (SECoP_V_getDouble(*ppData, 0, 0, &dblRamp))
        {
            if (dblRamp < 0.01)
                dblRamp = 0.01;
            else if (dblRamp > 100.0)
                dblRamp = 100.0;
            SECoPModul::theInstance->setRampValue(dblRamp);
            SECoP_V_modifyDouble(*ppData, 0, 0, dblRamp);
        }
        else
            *piError = SECoP_S_ERROR_INVALID_VALUE;
    }
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_GetUseRamp(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(piError);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    if (SECoPModul::theInstance != nullptr)
        SECoP_V_modifyInteger(*ppData, 0, 0, SECoPModul::theInstance->getRampBool() ? 1 : 0);
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_SetUseRamp(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    printf("setrampbool\n");
    fflush(stdout);
    if (SECoPModul::theInstance != nullptr)
    {
        long long llUseRamp(0);
        if (SECoP_V_getInteger(*ppData, 0, 0, &llUseRamp))
            SECoPModul::theInstance->setRampBool(llUseRamp != 0);
        else
            *piError = SECoP_S_ERROR_INVALID_VALUE;
    }
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_GetCommit(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(piError);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    if (SECoPModul::theInstance != nullptr)
        SECoP_V_modifyInteger(*ppData, 0, 0, SECoPModul::theInstance->getCommit() ? 1 : 0);
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_SetCommit(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    printf("setcommit\n");
    if (SECoPModul::theInstance != nullptr)
    {
        long long llCommit(0);
        if (SECoP_V_getInteger(*ppData, 0, 0, &llCommit))
            SECoPModul::theInstance->setCommit(llCommit != 0);
        else
            *piError = SECoP_S_ERROR_INVALID_VALUE;
    }
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void Local_GetStatus(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppData, CSECoPbaseType** ppSigma, double* timestamp)
{
    Q_UNUSED(name);
    Q_UNUSED(piError);
    Q_UNUSED(ppSigma);
    Q_UNUSED(timestamp);
    if (SECoPModul::theInstance != nullptr)
    {
        QString szStatus(SECoPModul::theInstance->getStatus());
        qint64 i(0);
        if (szStatus.contains(QString("idle"), Qt::CaseInsensitive))
        {
            szStatus.clear();
            //Counting flowers on the wall, that don't bother me at all Playing solitaire til dawn with a deck of fiftyone Smoking cigarettes and watching Captain Kangaroo Now don't tell me I've
            szStatus.append("IDLE Counting flowers on the wall, that don't bother me at all Playing solitaire til dawn with a deck of fiftyone Smoking cigarettes and watching Captain Kangaroo Now don't tell me I've nothing to do");
            i |= 0x01;
        }
        if (szStatus.contains(QString("pause"), Qt::CaseInsensitive))
            i |= 0x02;
        if (szStatus.contains(QString("start"), Qt::CaseInsensitive))
            i |= 0x04;
        if (szStatus.contains(QString("stop"), Qt::CaseInsensitive))
            i |= 0x08;
        if (szStatus.contains(QString("reset"), Qt::CaseInsensitive))
            i |= 0x10;
        if (szStatus.contains(QString("shutdown"), Qt::CaseInsensitive))
            i |= 0x20;
        if (szStatus.contains(QString("finish"), Qt::CaseInsensitive))
            i |= 0x40;
        switch (i) // only one word should found to map a status
        {
            case 0x01: i = 100; break;
            case 0x02: i = 101; break;
            case 0x04: i = 300; break;
            case 0x08: i = 200; break;
            case 0x10: i = 400; break;
            case 0x20: i = 400; break;
            case 0x40: i = 100; break;
            default: i = -1; break; // unknown, if less or more words are found
        }
        SECoP_V_modifyInteger(*ppData, 1, 0, i);
        SECoP_V_modifyString(*ppData, 2, qUtf8Printable(szStatus), -1);
    }
#if defined(ALLOW_ADD_VALUEERRORS)
    if ((qrand() % ALLOW_ADD_VALUEERRORS) == 0)
        *piError = SECoP_S_ERROR_TIMEOUT;
#endif
}

void funcCall(const char* name, const CSECoPbaseType* pArgument, enum SECoP_S_error* piError, CSECoPbaseType** ppReturn, double* timestamp)
{
    Q_UNUSED(pArgument);
    Q_UNUSED(piError);
    Q_UNUSED(ppReturn);
    Q_UNUSED(timestamp);
    if (SECoPModul::theInstance != nullptr)
    {
        QString szCommand(name);
        if (szCommand.endsWith(":go"))
            SECoPModul::theInstance->start();
        if (szCommand.endsWith(":hold"))
            SECoPModul::theInstance->pause();
        if (szCommand.endsWith(":stop"))
            SECoPModul::theInstance->stop();
        if (szCommand.endsWith(":reset"))
            SECoPModul::theInstance->reset();
        if (szCommand.endsWith(":shutdown"))
            SECoPModul::theInstance->shutdown();
    }
}

#if 0
QString ignorePolicy(QString szData)
{
    //init JSON with NULL
    QString JSON=""; //data
    QString action="";
    QString specifier="";
    int i_firstspace;
    //remove CR if there is one
    if(szData.endsWith("\r\n"))
        szData.remove(szData.lastIndexOf("\r"),1);

    //cut the action part from input before the first space and remove action and frist space from input
    i_firstspace=szData.indexOf(" ");
    if(i_firstspace>0)//there is a space
        {
        action=szData.left(i_firstspace);
        if(i_firstspace+1==szData.length())//one sign more then
            {
            return action;
            }
        else
            szData = szData.right(i_firstspace+1);

        }
    else
        return szData;//there is no space it must be action only

    //now the szData should only contain speciefier and data but malformed request is allowed so we have to check
    //leading space and leading space followed by null should be handled as action only
    if(szData.startsWith(" null") || szData.endsWith(" "))
        return action;

    //cut the specifier part from the reduced input and remove specifier and space from input
    i_firstspace=szData.indexOf(" ");
    if(i_firstspace>0)//there is a space
        {
        specifier=szData.left(i_firstspace);
        szData = szData.right(i_firstspace);
        }
    else//there is no space it must be action with specifier and no data
        return action.append(" "+szData);

    //there may be a right side single JSON null not given as object

    if(szData.startsWith("null\n"))
        return action.append(" "+specifier);

    //check for possible JSON and throw error if JSON is not valid
    if(szData.contains("{" && "}"))
        {
        //cut out JSON for check
        int i_first=szData.indexOf("{");
        int i_last=szData.lastIndexOf("}");
        JSON=szData.mid(i_first,i_last-i_first);
        szData.remove(JSON);
        //JSON valid check
        QJsonDocument doc = QJsonDocument::fromJson(JSON.toUtf8());
        if(!doc.isNull())
            {
            if(!doc.isObject())
                szData="errormsg";

                //check doc.Indented vs doc.Compact not needed it must be the compact version
            }
        else
            {
               szData="errormsg";
            }
        }
    //cant be valid JSON with only one curly bracket
    if (szData.contains("{"))
        {
        szData="errormsg";
        }
    if (szData.contains("}"))
        {
        szData="errormsg";
        }
    return action.append(" "+specifier+" "+JSON);
}
#endif

int main(int argc, char *argv[])
{
    printf("0\n");
    fflush(stdout);

//  printf ("%s\n", ignorePolicy("*IDN?  null").toUtf8().constData());

#if 1
    printf("1\n");
    fflush(stdout);
    QApplication app(argc, argv);
//  QLocale::setDefault(QLocale::C);
//  QGuiApplication::setApplicationDisplayName(SECoPModul::tr("SECoP Test Modul \"Hotplate\""));
    SECoPModul* pSimulator(new SECoPModul());
    pSimulator->show();
    pSimulator->connect(pSimulator, SIGNAL(finished(int)), &app, SLOT(quit()), Qt::QueuedConnection);
    SECoP_S_initLibrary(&app, true, true);
    SECoP_S_setManyThreads(0);
/*
     SECoP_S_createNode("HZB", "", 2055);                                              // Node HZB SECoP_S_WARNING_NO_DESCRIPTION

        SECoP_S_addPropertyString("equipment_id", "test_node");                        // Node equipment_id SECoP_S_ERROR_NAME_ALREADY_USED
        SECoP_S_addPropertyString("description", "example description");               // Node description SECoP_S_ERROR_NAME_ALREADY_USED
        SECoP_S_addPropertyString("descriptIon", "example description");               // Node description SECoP_S_WARNING_CUSTOM_PROPERTY
        SECoP_S_addModule("ts1");
        SECoP_S_addModule("Ts1");                                                      // ts1 Ts1 SECoP_S_ERROR_NAME_ALREADY_USED
            SECoP_S_addPropertyString("description", " module ts 1 something cold");
            SECoP_S_addPropertyString("decription", " module 1 something cold");       // ts1 decription SECoP_S_WARNING_CUSTOM_PROPERTY
            SECoP_S_addPropertyString("description", " module 1 something cold");      // ts1 description SECoP_S_ERROR_NAME_ALREADY_USED
            SECoP_S_addPropertyJSON("interface_class", "[\"Drivable\",\"Writable\",\"Readable\"]");
            SECoP_S_addPropertyString("visibility", "user");
            SECoP_S_addReadableParameter("value", nullptr);                            // ts1 value SECoP_S_ERROR_NO_GETTER
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");    // ts1 datainfo SECoP_S_ERROR_NAME_ALREADY_USED
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("decription", "something with temperature"); // ts1 decription SECoP_S_WARNING_CUSTOM_PROPERTY
            SECoP_S_addWritableParameter("secondValue", nullptr, nullptr);                   // ts1 secondValue SECoP_S_ERROR_NO_SETTER_GETTER
            SECoP_S_addWritableParameter("secondValue", nullptr, nullptr);                   // ts1 secondValue SECoP_S_ERROR_NAME_ALREADY_USED
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("_custom", "test");
                SECoP_S_addPropertyString("_custom", "test");                          // SECoP_S_ERROR_NAME_ALREADY_USED
                SECoP_S_addPropertyString("custom", "test");                           // SECoP_S_WARNING_CUSTOM_PROPERTY

            SECoP_S_addModule("m3");
                SECoP_S_addPropertyString("description", " m3 and alot of description");
                SECoP_S_addPropertyJSON("interface_class", "[\"Readable\"]");
                SECoP_S_addPropertyString("visibility", "expert");
                SECoP_S_addReadableParameter("value", nullptr);                            // m3 value SECoP_S_ERROR_NO_GETTER
                SECoP_S_addReadableParameter("value", nullptr);                            // m3 value SECoP_S_ERROR_NAME_ALREADY_USED
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":""}");

            SECoP_S_addModule("vs2");
                SECoP_S_addPropertyString("description", " vs2 an incredible machine"); //
                SECoP_S_addPropertyString("equipment_id", "test_node");                  // vs2 equipment_id SECoP_S_WARNING_CUSTOM_PROPERTY
                SECoP_S_addReadableParameter("value", nullptr);                            // vs2 value SECoP_S_ERROR_NO_GETTER
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"bool\"}");
                    SECoP_S_addPropertyString("description", "switch from on to off");
                    SECoP_S_addPropertyString("equipment_id", "test_node");              // vs2 equipment_id SECoP_S_WARNING_CUSTOM_PROPERTY
                SECoP_S_addCommand("start", nullptr);
                    SECoP_S_addPropertyString("description", "starts some actions");
                    SECoP_S_addPropertyString("equipment_id", "test_node");              // vs2 equipment_id SECoP_S_WARNING_CUSTOM_PROPERTY
                SECoP_S_addCommand("halt", nullptr);
                    SECoP_S_addPropertyString("description", "stops some actions");
                SECoP_S_addCommand("stop!", nullptr);
                    SECoP_S_addPropertyString("description", "emergency stop caution this will destroy hardware");
        SECoP_S_showErrors();
*/
    printf("%s\n", "hier");
    fflush(stdout);

    SECoP_S_createNode("HZB_Testnode1", "TestNode", 2056);
//      SECoP_S_addPropertyJSON("order","[\"hpdtest\"]");
        SECoP_S_addModule("hpd");
            SECoP_S_addPropertyString("description", "Hotplate drivable");
            SECoP_S_addPropertyJSON("interface_class", "[\"Drivable\",\"Writable\",\"Readable\"]");
            SECoP_S_addPropertyDouble("pollinterval", 10.0);
            SECoP_S_addReadableParameter("value", &Local_GetTemperature);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addPropertyString("description", "actual temperature");
            SECoP_S_addReadableParameter("status", &Local_GetStatus);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"tuple\",\"members\":[{\"type\":\"enum\",\"members\":{\"IDLE\":100,\"WARN\":200,\"BUSY\":300,\"BUSY_Stabilizing\":380,\"ERROR\":400,\"DISABLED\":0}},{\"type\":\"string\"}]}");
                SECoP_S_addPropertyString("description", "machine status");
                SECoP_S_addPropertyDouble("pollinterval", 1.0);
//          SECoP_S_addReadableParameter2("useramp2", &SECoPModul::getRampBool, SECoPModul::theInstance);
            SECoP_S_addWritableParameter("target", &Local_GetTarget, &Local_SetTarget);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("description", "target temperature");
            SECoP_S_addCommand("stop",&funcCall);
                SECoP_S_addPropertyString("description", "stops and settings are not stored no resume");
    SECoP_S_nodeComplete();
/*      SECoP_S_addWritableParameter("ramp", &Local_GetRamp, &Local_SetRamp);
            SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K/s\"}");
            SECoP_S_addPropertyString("description", "ramp rate to target temperature in K/s");
        SECoP_S_addWritableParameter("useramp", &Local_GetUseRamp, &Local_SetUseRamp);
            SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"bool\"}");
            SECoP_S_addPropertyString("description", "if true ramp rate is activated");*/
#if 1

    SECoP_S_createNode("HZB_TestNode2", "TestNode2", 2055);
        SECoP_S_addModule("mod1");
            SECoP_S_addPropertyString("description", "Hotplate drivable");
            SECoP_S_addPropertyJSON("interface_class", "[\"Drivable\",\"Writable\",\"Readable\"]");

            SECoP_S_addPropertyJSON("interface_class", "[\"magnet\",\"Driveable\"]");

            SECoP_S_addPropertyDouble("pollinterval", 10.0);
            SECoP_S_addReadableParameter("value", &Local_GetTemperature);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("description", "actual temperature");
                SECoP_S_addPropertyDouble("pollinterval", 1.0);
            SECoP_S_addCommand("shutdown",&funcCall);
                SECoP_S_addPropertyString("description", "go to defined finish point");
            SECoP_S_addReadableParameter("status", &Local_GetStatus);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"tuple\",\"members\":[{\"type\":\"enum\",\"members\":{\"IDLE\":100,\"WARN\":200,\"BUSY\":300,\"BUSY_Stabilizing\":380,\"ERROR\":400,\"DISABLED\":0}},{\"type\":\"string\"}]}");
                SECoP_S_addPropertyString("description", "machine status");
                SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addPropertyString("group","test1:4");
            SECoP_S_addWritableParameter("target", &Local_GetTarget, &Local_SetTarget);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("description", "target temperature");
            SECoP_S_addWritableParameter("ramp", &Local_GetRamp, &Local_SetRamp);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K/s\"}");
                SECoP_S_addPropertyString("description", "ramp rate to target temperature in K/s");
                SECoP_S_addPropertyString("group","test1:3");
            SECoP_S_addWritableParameter("useramp", &Local_GetUseRamp, &Local_SetUseRamp);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"bool\"}");
                SECoP_S_addPropertyString("description", "if true ramp rate is activated");
                SECoP_S_addPropertyString("group","test2:2");
            SECoP_S_addWritableParameter("_COMMITtestesehrlangennamen", &Local_GetCommit, &Local_SetCommit);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"bool\"}");
                SECoP_S_addPropertyString("description", "if true changes are send");
                SECoP_S_addPropertyString("group","test2:1");
            SECoP_S_addWritableParameter("HelloWorldRW", nullptr, nullptr);
                CSECoPsimpleDouble* pDouble(new CSECoPsimpleDouble());
                pDouble->setValue(42.0001);
                pDouble->additional()["unit"] = "K";
//              SECoP_S_addProperty("datainfo", pDouble); // optional, because of complete "constant"
                SECoP_S_addProperty("constant", pDouble);
                delete pDouble;
                SECoP_S_addPropertyString("description", "a constant writable");
            SECoP_S_addReadableParameter("HelloWorldR", nullptr);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"rubber points\"}");
                SECoP_S_addPropertyString("description", "a constant readable");
                SECoP_S_addPropertyDouble("constant", 42.0002);
            SECoP_S_addCommand("go",&funcCall);
                SECoP_S_addPropertyString("description", "flips the commit bool all buffered values are writen and the module starts work");
//              SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"command\",\"argument\":{\"type\":\"string\"}}");
            SECoP_S_addCommand("hold",&funcCall);
                SECoP_S_addPropertyString("description", "stops and settings are stored for resume");
            SECoP_S_addCommand("stop",&funcCall);
                SECoP_S_addPropertyString("description", "stops and settings are not stored no resume");
            SECoP_S_addCommand("reset",&funcCall);
                SECoP_S_addPropertyString("description", "set back the initial values");


            SECoP_S_addModule("mod2");
                SECoP_S_addPropertyString("group","test");
                SECoP_S_addPropertyString("description", "test writable");
                SECoP_S_addPropertyJSON("interface_class", "[\"Writable\",\"Readable\"]");
                SECoP_S_addPropertyDouble("pollinterval", 10.0);

                SECoP_S_addReadableParameter("value", &Local_GetTemperature);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                    SECoP_S_addPropertyString("description", "actual temperature");
                    SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addReadableParameter("status", &Local_GetStatus);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"tuple\",\"members\":[{\"type\":\"enum\",\"members\":{\"IDLE\":100,\"WARN\":200,\"BUSY\":300,\"BUSY_Stabilizing\":380,\"ERROR\":400,\"DISABLED\":0}},{\"type\":\"string\"}]}");
                    SECoP_S_addPropertyString("description", "machine status");
                    SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addWritableParameter("target", &Local_GetTarget, &Local_SetTarget);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                    SECoP_S_addPropertyString("description", "target temperature");
                SECoP_S_addWritableParameter("ramp", &Local_GetRamp, &Local_SetRamp);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K/s\"}");
                    SECoP_S_addPropertyString("description", "ramp rate to target temperature in K/s");
                    SECoP_S_addPropertyString("group","tst");
                SECoP_S_addWritableParameter("useramp", &Local_GetUseRamp, &Local_SetUseRamp);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"bool\"}");
                    SECoP_S_addPropertyString("description", "if true ramp rate is activated");
                    SECoP_S_addPropertyString("group","tst");
                SECoP_S_addCommand("go",&funcCall);
                    SECoP_S_addPropertyString("description", "flips the commit bool all buffered values are writen and the module starts work");
//                  SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"command\",\"argument\":{\"type\":\"string\"}}");
                SECoP_S_addCommand("hold",&funcCall);
                    SECoP_S_addPropertyString("description", "stops and settings are stored for resume");
                SECoP_S_addCommand("stop",&funcCall);
                    SECoP_S_addPropertyString("description", "stops and settings are not stored no resume");
                SECoP_S_addCommand("reset",&funcCall);
                    SECoP_S_addPropertyString("description", "set back the initial values");
                SECoP_S_addCommand("shutdown",&funcCall);
                    SECoP_S_addPropertyString("description", "go to defined finish point");

            SECoP_S_addModule("mod3");
                SECoP_S_addPropertyString("group","toast");
                SECoP_S_addPropertyString("description", "Hotplate drivable");
                SECoP_S_addPropertyJSON("interface_class", "[\"Drivable\",\"Writable\",\"Readable\"]");
                SECoP_S_addPropertyDouble("pollinterval", 10.0);

                SECoP_S_addReadableParameter("value", &Local_GetTemperature);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                    SECoP_S_addPropertyString("description", "actual temperature");
                    SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addReadableParameter("status", &Local_GetStatus);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"tuple\",\"members\":[{\"type\":\"enum\",\"members\":{\"IDLE\":100,\"WARN\":200,\"BUSY\":300,\"BUSY_Stabilizing\":380,\"ERROR\":400,\"DISABLED\":0}},{\"type\":\"string\"}]}");
                    SECoP_S_addPropertyString("description", "machine status");
                    SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addWritableParameter("target", &Local_GetTarget, &Local_SetTarget);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                    SECoP_S_addPropertyString("description", "target temperature");
                SECoP_S_addCommand("stop",&funcCall);
                    SECoP_S_addPropertyString("description", "stops and settings are not stored no resume");


            SECoP_S_addModule("mod4");
                SECoP_S_addPropertyString("description", "Hotplate drivable");
                SECoP_S_addPropertyJSON("interface_class", "[\"Writable\",\"Readable\"]");
                SECoP_S_addPropertyDouble("pollinterval", 10.0);

                SECoP_S_addReadableParameter("value", &Local_GetTemperature);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                    SECoP_S_addPropertyString("description", "actual temperature");
                    SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addReadableParameter("status", &Local_GetStatus);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"tuple\",\"members\":[{\"type\":\"enum\",\"members\":{\"IDLE\":100,\"WARN\":200,\"BUSY\":300,\"BUSY_Stabilizing\":380,\"ERROR\":400,\"DISABLED\":0}},{\"type\":\"string\"}]}");
                    SECoP_S_addPropertyString("description", "machine status");
                    SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addWritableParameter("target", &Local_GetTarget, &Local_SetTarget);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                    SECoP_S_addPropertyString("description", "target temperature");

            SECoP_S_addModule("mod5");
                SECoP_S_addPropertyString("description", "test readable");
                SECoP_S_addPropertyJSON("interface_class", "[\"Readable\"]");
                SECoP_S_addPropertyDouble("pollinterval", 10.0);

                SECoP_S_addReadableParameter("value", &Local_GetTemperature);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                    SECoP_S_addPropertyString("description", "actual temperature");
                    SECoP_S_addPropertyDouble("pollinterval", 1.0);
                SECoP_S_addReadableParameter("status", &Local_GetStatus);
                    SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"tuple\",\"members\":[{\"type\":\"enum\",\"members\":{\"IDLE\":100,\"WARN\":200,\"BUSY\":300,\"BUSY_Stabilizing\":380,\"ERROR\":400,\"DISABLED\":0}},{\"type\":\"string\"}]}");
                    SECoP_S_addPropertyString("description", "machine status");
                    SECoP_S_addPropertyDouble("pollinterval", 1.0);


    SECoP_S_nodeComplete();

#endif /**/
    SECoP_S_showErrors();
//  printf("%p", &Local_SetTarget); fflush(stdout);
//  QTimer::singleShot(60000, &app, SLOT(quit()));
    int iResult(app.exec());
    SECoP_S_doneLibrary(false);
    delete pSimulator;
    pSimulator = nullptr;
    return iResult;
#elif 0
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    SECoP_S_initLibrary(nullptr, true, true);
    SECoP_S_createNode("HZB", "TestNode", 2055);
        SECoP_S_addModule("hpd");
            SECoP_S_addPropertyString("description", "Hotplate drivable");
            SECoP_S_addPropertyJSON("interface_class", "[\"Drivable\",\"Writable\",\"Readable\"]");
            SECoP_S_addReadableParameter("temp", &Local_GetTemperature);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("description", "actual temperature");
//          SECoP_S_addReadableParameter2("useramp2", &SECoPModul::getRampBool, SECoPModul::theInstance);
            SECoP_S_addWritableParameter("target", &Local_GetTarget, &Local_SetTarget);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("description", "target temperature");
            SECoP_S_addWritableParameter("ramp", &Local_GetRamp, &Local_SetRamp);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K/s\"}");
                SECoP_S_addPropertyString("description", "ramp rate to target temperature in K/s");
            SECoP_S_addWritableParameter("useramp", &Local_GetUseRamp, &Local_SetUseRamp);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"bool\"}");
                SECoP_S_addPropertyString("description", "if true ramp rate is activated");
    SECoP_S_nodeComplete();
    printf("1\n");
    fflush(stdout);
    std::this_thread::sleep_for(std::chrono::seconds(30));
    printf("2\n");
    fflush(stdout);

    SECoP_S_doneLibrary(false);
    return 0;
#else
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    SECoP_S_initLibrary(nullptr, true, false);
    SECoP_S_setManyThreads(0);
    SECoP_S_createNode("HZB", "TestNode", 2055);
        SECoP_S_addModule("hpd");
            SECoP_S_addPropertyJSON("interface_class", "[\"Writable\",\"Readable\"]");
            SECoP_S_addPropertyDouble("pollinterval", 0.0);
            SECoP_S_addPropertyString("description", "simulate");
            SECoP_S_addReadableParameter("value", nullptr);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("description", "actual temperature");
            SECoP_S_addWritableParameter("target", nullptr, nullptr);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"double\",\"unit\":\"K\"}");
                SECoP_S_addPropertyString("description", "target temperature");
            SECoP_S_addReadableParameter("status", nullptr);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"tuple\",\"members\":[{\"type\":\"enum\",\"members\":{\"IDLE\":100,\"WARN\":200,\"BUSY\":300,\"BUSY_Stabilizing\":380,\"ERROR\":400,\"DISABLED\":0}},{\"type\":\"string\"}]}");
                SECoP_S_addPropertyString("description", "actual temperature");
            SECoP_S_addWritableParameter("devid", nullptr, nullptr);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"string\"}");
                SECoP_S_addPropertyString("description", "string test param");
            SECoP_S_addWritableParameter("pid", nullptr, nullptr);
                SECoP_S_addPropertyJSON("datainfo", "{\"type\":\"array\",\"members\":{\"type\":\"double\",\"unit\":\"none\"},\"minlen\":3,\"maxlen\":3}");
                SECoP_S_addPropertyString("description", "controller params");
    SECoP_S_error iResult(SECoP_S_nodeComplete());
    printf("node complete %d\n", iResult);
    fflush(stdout);
    for (;;)
    {
        unsigned long long llId;
        SECoP_S_action iAction;
        char szParameter[64], szValue[1024];
        int iParameterSize(sizeof(szParameter)), iValueSize(sizeof(szValue));
        if (SECoP_S_getStoredCommand2(&llId, &iAction, szParameter, &iParameterSize, szValue, &iValueSize) >= 0)
        {
            szParameter[sizeof(szParameter) - 1] = '\0';
            szValue[sizeof(szValue) - 1] = '\0';
            printf("%s  %u  %s  ", szParameter, iAction, szValue);
            if (QString(szParameter).indexOf(":devid", Qt::CaseInsensitive) > 0)
                strcpy(szValue, "\"hello world\"");
            else if (QString(szParameter).indexOf(":pid", Qt::CaseInsensitive) > 0)
                strcpy(szValue, "[10,20,30]");
            else
                snprintf(szValue, sizeof(szValue), "%d", qrand());
            szParameter[0] = '\0';
            szValue[sizeof(szValue) - 1] = '\0';
            printf("%s\n",szValue);
            fflush(stdout);
            SECoP_S_putCommandAnswer2(llId, SECoP_S_SUCCESS, szValue, -1, szParameter, -1,
                                      std::numeric_limits<double>::quiet_NaN());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    SECoP_S_doneLibrary(false);
    return 0;
#endif
}
