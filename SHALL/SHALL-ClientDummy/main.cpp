/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QTimer>
#include <QDebug>
#include <math.h>
#include "main.h"
#include "mainwindow.h"

// define and set this to 1, to test the SEC node
#if !defined (TEST_LOCAL_SECoP_NODE)
#define TEST_LOCAL_SECoP_NODE 0
#endif

//testing the client dll
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    QApplication app(argc, argv);
    int iResult(0);

    MainWindow* pMainWindow(new MainWindow());
    pMainWindow->show();
   // pMainWindow->connect(pMainWindow, SIGNAL(finished(int)), &app, SLOT(quit()), Qt::QueuedConnection);
    pMainWindow->connect(pMainWindow, SIGNAL(destroyed()), &app, SLOT(quit()), Qt::DirectConnection);

    qDebug("initializing");
    iResult = SECoP_C_initLibrary(&app, 0);
    qDebug("returncode of initLibrary %d", iResult);
    //SECoP_C_startInactive();

/*  QTimer::singleShot(30000, &app, SLOT(quit()));*/

#if defined (TEST_LOCAL_SECoP_NODE) && TEST_LOCAL_SECoP_NODE > 0
    qDebug("addNode");
    std::string node;
#if 1
   //SECoP_C_startInactive();
    SECoP_C_addNode("134.30.137.168",2055,node);
   // std::string nname=SECoP_C_getNodeNameByIP("127.0.0.1",2055);
   // SECoP_C_activateNode(node);
    qDebug("nodename is: %s",node.c_str());
    const char* ccpNode="HZB-TestNode2";
    int nodes = SECoP_C_getNumberOfNodes();
    qDebug("connected Nodes %d", nodes);
    int modules=SECoP_C_getNumberOfModules(ccpNode);
    qDebug("modules inside: %d",modules);
    std::string Node2;
    //iResult = SECoP_C_addNode("127.0.0.1",2056,Node2);
    nodes = SECoP_C_getNumberOfNodes();
    qDebug("connected Nodes %d", nodes);
    SECoP_C_removeNode("127.0.0.1",2056);
#else
    addNode("192.168.1.140", 1883, *pNode);
#endif
   // qDebug("returncode of addNode %d", iResult);

    nodes = SECoP_C_getNumberOfNodes();
    qDebug("connected Nodes %d", nodes);

    std::list<std::string> Nodelist =getNodeNamesList();
    foreach(std::string strNode, Nodelist)        
    {
        qDebug("            ------->Node: %s",strNode.c_str());
        std::list<std::string> NodePropList = getNodePropertiesKeyList(strNode);
        foreach (std::string strPropKey, NodePropList)
        {
             qDebug("Node Property key: %s",strPropKey.c_str());
             qDebug("Node Property val: %s",getNodePropertyAsString(strNode,strPropKey).c_str());
        }
        std::list<std::string> Modlist =getModuleNamesList(strNode);
        foreach(std::string strMod, Modlist)
        {
            qDebug("    Module: %s",strMod.c_str());
            std::list<std::string> ModulePropList = getModulePropertiesKeyList(strNode,strMod);
            foreach (std::string strPropKey, ModulePropList)
            {
                 qDebug("Module Property key: %s",strPropKey.c_str());
                 qDebug("Module Property val: %s",getModulePropertyAsString(strNode,strMod,strPropKey).c_str());
            }
            std::list<std::string> Acclist =getAccNameList(strNode,strMod);
            foreach(std::string strAcc, Acclist)
            {                
                qDebug("        Acc: %s",strAcc.c_str());
                std::list<std::string> AccPropList = getAccPropertiesKeyList(strNode,strMod,strAcc);
                foreach (std::string strPropKey, AccPropList)
                {
                    if(strPropKey=="readonly")//only parameters have the readonly property
                    {
                       SECoP_dataPtr pSecVal;
                       double dTimSta(std::numeric_limits<double>::quiet_NaN());
                       SECoP_dataPtr pSecErr;
                       std::string SECoPTripel;
                       qint64 resp;
                       std::string value=testRead(strNode,strMod,strAcc,pSecVal,dTimSta,pSecErr,SECoPTripel,resp,5000);
                       qDebug ("Value of parameter : %s",value.c_str());
                       qDebug("Answer needed : %g ms", static_cast<double>(resp));
                       std::list<std::string>sectriple;
                       std::string tmp;
                       sectriple=splitSECoPTriple(SECoPTripel);
                       tmp=sectriple.front();
                       qDebug ("Split Value1 : %s", tmp.c_str());
                       sectriple.pop_front();
                       tmp=sectriple.front();
                       qDebug ("Split Value2 : %s", tmp.c_str());
                       sectriple.pop_front();
                       tmp=sectriple.front();
                       qDebug ("Split Value3 : %s", tmp.c_str());
                       qDebug ("pSecVal=%s  pTimSta=%g  pSecErr=%s",
                               pSecVal->toStdString().c_str(), dTimSta,
                               pSecErr->toStdString().c_str());
                    }

                    qDebug("Acc Property key: %s",strPropKey.c_str());
                    qDebug("Acc Property val: %s",getAccPropertyAsString(strNode,strMod,strAcc,strPropKey).c_str());
                }
            }
        }
    }
    SECoP_C_ShowGui(true);
/*  for()
    {
        getNodeName();
        getNodePropertiesKeyList();
        getNodeProperties();
        getNumberOfModules();
        getModuleNamesList();
        for()
        {
            getModuleName();
            getModulePropertiesKeyList();
            getModuleProperties();
            getNumberOfAcc();
            getAccNameList();


        }
    }
*/

   //removeNode("127.0.0.1",2055);

    qDebug("Hello World" );
    SECoP_dataPtr pSecVal;
    double pTimSta;
    SECoP_dataPtr pSecErr;
    qDebug ("Value of parameter : %s",readParam("HZB-TestNode2","mod1","value",pSecVal,pTimSta,pSecErr).c_str());
#endif
    iResult = app.exec();
    SECoP_C_doneLibrary(0);
    return iResult;
}
