/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_NODE_H__3BB08092_2314_4A0B_B6CE_4D243DB1723070__
#define __SECOP_NODE_H__3BB08092_2314_4A0B_B6CE_4D243DB1723070__

#include <QVector>
#include <QByteArray>
#include <QHostAddress>
#include "SECoP-Variant.h"
#include "SECoP-types.h"

// forward declarations
class QMutex;
class QTcpServer;
class QTcpSocket;
class SECoP_S_Command;
class SECoP_S_Module;
class SECoP_S_Parameter;
class SECoP_S_Property;
class SECoP_S_Worker;

/**
 * \brief The SECoP_S_Node class stores data about a node, which has modules
 *        parameters, commands and properties. The node opens the TCP port
 *        and creates workers for SECoP client connections.
 */
class SECoP_S_Node : public QObject
{
    friend class SECoP_S_Module;
    Q_OBJECT
    Q_DISABLE_COPY(SECoP_S_Node)

public:
    explicit SECoP_S_Node(QString szNodeID, QString szDesc, QHostAddress address, quint16 wPort, QObject* pParent = nullptr);
    virtual ~SECoP_S_Node();
    bool isValid() const;
    enum SECoP_S_error addCommand(QString szKey, SECoP_S_callFunction ptrToFunc);
    enum SECoP_S_error addReadableParameter(QString szParameterName, SECoP_S_getsetFunction ptrToGet);
    enum SECoP_S_error addWritableParameter(QString szParameterName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet);
    enum SECoP_S_error addModule(QString szModuleName);
    enum SECoP_S_error addProperty(QString szKey, const SECoP_dataPtr pValue);
    enum SECoP_S_error setAddFocus(QString szKey);
    enum SECoP_S_error nodeComplete();
    nlohmann::json getJSON() const;
    QString getErrors() const;
    QString getActiveModuleName() const;
    QString getNodeID() const;
    QString getPrintableActive(bool bWithAccessible) const;
    SECoP_S_Module* getModule(int iModule) const;
    int modulePosition(QString szModuleName) const;
    int propertyPosition(QString szKey) const;
    void moveToThread(QThread *pThread);

private slots:
    void newClient();
    void disconnectedClient(QObject* pObject);
    void moveToThreadSlot(QThread *pThread);
    void addCommand(QString szKey, SECoP_S_callFunction ptrToFunc, SECoP_S_error* piResult);
    void addReadableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_error* piResult);
    void addWritableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet, SECoP_S_error* piResult);
    void addModule(QString szModuleName, SECoP_S_error* piResult);
    void addProperty(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic, SECoP_S_error* piResult);
    void setAddFocus(QString szKey, SECoP_S_error* piResult);
    void nodeComplete(SECoP_S_error* piResult);

private:
    void storeCommand(quint64 qwRequestId, QObject *pTarget, SECoP_S_Module* pModule, enum SECoP_S_action iAction,
                      SECoP_S_Parameter* pParameter, SECoP_S_Command* pCommand, const SECoP_dataPtr pValue);
    static QHash<QString, int> getDefaultProperties(QString szType);
    static QString checkProperties(bool &bError, QString szName, QHash<QString, int> ahProps);
    QString checkProperty(bool& bError, QString szName, QString szKey, SECoP_dataPtr pValue);
    static QString checkDatatype(bool& bError, QString szName, const CSECoPbaseType* pValue,
                                 QHash<QString, int>& ahErrors, bool bAllowCommand);
    static QString checkInterfaceClass(bool& bError, const SECoP_S_Module* pModule, const SECoP_S_Property* pIFC);
    QString checkOrder(bool& bError, const SECoP_S_Module* pModule, const CSECoPbaseType* pOrder);

    /// exclusive access to node internals
    QMutex*                    m_pMutex;
    /// TCP server
    QTcpServer*                m_pServer;
    /// the list of all client connections
    QList<SECoP_S_Worker*>     m_apClients;
    /// the list of properties of the node
    QVector<SECoP_S_Property*> m_apProperties;
    /// the list of modules inside the node
    QVector<SECoP_S_Module*>   m_apModules;
    /// descriptive json for the complete node
    nlohmann::json             m_szDescribingJSON;
    /// the node id
    QString                    m_szNodeID;
    /// node description text
    QString                    m_szDescription;
    /// error and warning text
    QString                    m_szErrorsWarnings;
    /// first true: node can be changed, then false: TCP port is open
    bool                       m_bChangeable;
    /// index of last created module
    int                        m_iModuleFocus;
};

#endif /*__SECOP_NODE_H__3BB08092_2314_4A0B_B6CE_4D243DB1723070__*/
