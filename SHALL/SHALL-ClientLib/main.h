/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef MAIN_H
#define MAIN_H
#include <QHostAddress>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include "exports.h"

// forward declarations
class ClientGui;

/**
 * \brief The SECoP_Main class is a singleton which stores all SEC-nodes with
 *        its modules, parameters, commands and properties. It is used for
 *        communication purposes and for polling too.
 */
class Client_Main : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Client_Main)

    friend class Client_Gui;


public:
    Client_Main();
    ~Client_Main();
    enum SECoP_C_error addNode(QHostAddress IP, int Port);
    QString getNodeNameByIP(QHostAddress IP, int Port);
    enum SECoP_C_error removeNode(QHostAddress IP, int Port);
    int getNumberOfNodes();
    int getNumberOfModules(QString node);
    std::list<std::string> getNodeNamesList();
    std::list<std::string> getModuleNamesList(std::string Node);
    std::list<std::string> getAccNameList(std::string Node, std::string Module);

    std::list<std::string> getNodePropertiesKeyList(std::string Node);
    std::list<std::string> getModulePropertiesKeyList(std::string Node, std::string Module);
    std::list<std::string> getAccPropertiesKeyList(std::string Node, std::string Module, std::string Acc);

    std::string getProperty(std::string Node, std::string Module, std::string Acc, std::string Key);
    std::string getPropertyHelper(const nlohmann::json &props, const std::string &key);
    std::string  readParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal);
    std::string writeParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr Value);

    std::string forcedReadParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp,SECoP_dataPtr &ErrorVal);    
    std::string testRead(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal, std::string &SECoPtripel, qint64 &respTime, qint64 maxTime);
    //SECoP_C_error ClientGui::readParam(QString Node, QString Module, QString Param)
    static Client_Main* getInstance();
//  static const char* getErrorString(enum SECoP_C_error iError);
    static double getCurrentTime();

    static void log(QString szData);
    void cleanUp(bool bHideOnly);
    static void cleanUpSlot(bool bHideOnly);
    static void showGUI(bool bShowGUI);
    void startInactive();
    void callActivate(std::string Node);

private:
    static Client_Main* m_pInstance;
    ClientGui*         m_pGui;
    QMutex*            m_pMutex;
    bool               m_bShowGUI;

};

#endif // MAIN_H
