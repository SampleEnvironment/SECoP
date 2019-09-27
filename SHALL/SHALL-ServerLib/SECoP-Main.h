/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_MAIN_H__3BB08092_2314_4A0B_B6CE_4D173DB4123080__
#define __SECOP_MAIN_H__3BB08092_2314_4A0B_B6CE_4D173DB4123080__

#include <QObject>
#include <QDebug>
#include "SECoP-defines.h"
#include "SECoP-Variant.h"
#include "SECoP-types.h"

// forward declarations
class SECoP_S_Command;
class SECoP_S_Module;
class SECoP_S_Node;
class SECoP_S_Parameter;
class SECoP_S_StatusGui;
class SECoP_S_Worker;
class QMutex;
class QTimer;
class QTcpSocket;

/**
 * \brief The SECoP_S_Main class is a singleton which stores all SEC-nodes with
 *        its modules, parameters, commands and properties. It is used for
 *        communication purposes for polling too.
 */
class SECoP_S_Main : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SECoP_S_Main)

public:
    SECoP_S_Main();
    ~SECoP_S_Main();
    static SECoP_S_Main* getInstance();
    static const char* getErrorString(enum SECoP_S_error iError);
    static double getCurrentTime();
    static QByteArray getGitVersion();
    static bool hasFunctionPointers();
    static enum SECoP_S_error createNode(QString szID, QString szDesc, QString szInterface, quint16 wPort);
    static enum SECoP_S_error deleteNode(QString szID);
    static enum SECoP_S_error addProperty(QString szKey, const SECoP_dataPtr pValue);
    static enum SECoP_S_error addCommand(QString szKey, SECoP_S_callFunction ptrToFunc);
    static enum SECoP_S_error addModule(QString szName);
    static enum SECoP_S_error addReadableParameter(QString szName, SECoP_S_getsetFunction ptrToGet);
    static enum SECoP_S_error addWritableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet);
    static enum SECoP_S_error setAddFocus(QString szKey);
    static enum SECoP_S_error nodeComplete();
    static enum SECoP_S_error updateParameter(QString szParameterName, const SECoP_dataPtr pData, const SECoP_dataPtr pSigma, double dblTimestamp);
    static enum SECoP_S_error updateParameter2(QString szParameterName, QByteArray szData, QByteArray szSigma, double dblTimestamp);
    static void forgetStoredCommands(QObject* pTarget);
    static enum SECoP_S_error getStoredCommand(qulonglong* pllId, SECoP_S_action* piAction, char* szParameter, int* piParameterSize,
                                               SECoP_dataPtr* ppValue);
    static void putCommandAnswer(qulonglong llId, enum SECoP_S_error iErrorCode, const SECoP_dataPtr pValue,
                                 const SECoP_dataPtr pSigma, double dblTimestamp);
    static void putCommandAnswer2(qulonglong llId, enum SECoP_S_error iErrorCode, QByteArray szValue,
                                  QByteArray szSigma, double dblTimestamp);
    static void showErrors();
    static void logAddConnection(SECoP_S_Node* pNode, QTcpSocket* pClient);
    static void logRemoveConnection(QTcpSocket* pClient);
    static void log(SECoP_S_Node* pNode, QString szData, bool bNodeOnly);
    void cleanUp(bool bNodeOnly);
    static void showGUI(bool bShowGUI);
    static bool manyThreads();
    static void manyThreads(bool bManyThreads);
    static bool isValidName(QString szName);

private slots:
    void showErrorsSlot();
    void cleanUpSlot(bool bNodeOnly);
    void sessionCleanUpTimer();
    void createNode(QString szID, QString szDesc, QString szInterface, quint16 wPort, SECoP_S_error* piResult);
    void deleteNode(QString szID, SECoP_S_error* piResult);
    void setAddFocus(QString szID, SECoP_S_error* piResult);
    void updateParameter(QString szParameterName, const SECoP_dataPtr pData, const SECoP_dataPtr pSigma,
                         double dblTimestamp, SECoP_S_error* piResult);
    void updateParameter2(QString szParameterName, QByteArray szData, QByteArray szSigma,
                          double dblTimestamp, SECoP_S_error* piResult);
    void getStoredCommand(qulonglong* pllId, SECoP_S_action* piAction, char* szParameter, int* piParameterSize,
                          SECoP_dataPtr* ppValue, SECoP_S_error* piResult);
    void putCommandAnswerSlot(qulonglong llId, SECoP_S_error iErrorCode, const SECoP_dataPtr pValue,
                              const SECoP_dataPtr pSigma, double dblTimestamp);
    void putCommandAnswer2Slot(qulonglong llId, SECoP_S_error iErrorCode, QByteArray szValue, QByteArray szSigma,
                               double dblTimestamp);

private:
    void storeCommand(quint64 qwRequestId, QObject *pTarget, SECoP_S_Node* pNode, SECoP_S_Module* pModule, SECoP_S_action iAction,
                      SECoP_S_Parameter* pParameter, SECoP_S_Command* pCommand, const SECoP_dataPtr pValue);
    int nodePosition(QString szNode) const;
    static void logAddNode(SECoP_S_Node* pNode);
    static void logRemoveNode(SECoP_S_Node* pNode);
    static QByteArray printErrorList();
    static QByteArray printWarningList();

    /**
     * \brief The ActionEntry class stores one action for the polling interface
     */
    class ActionEntry
    {
    public:
        ActionEntry();
        ActionEntry(const ActionEntry &src);
        ActionEntry& operator=(const ActionEntry &src);
        /// a unique id to this action
        quint64             m_qwId;
        /// an id used by the module in action
        quint64             m_qwRequestId;
        /// the appropriate SECoP node, which contains the module
        SECoP_S_Node*       m_pNode;
        /// the SECoP module
        SECoP_S_Module*     m_pModule;
        /// if action means a parameter, the SECoP parameter or nullptr
        SECoP_S_Parameter*  m_pParameter;
        /// if action means a command, the SECoP command or nullptr
        SECoP_S_Command*    m_pCommand;
        /// Qt object, which will get invokes by answers to this action
        QObject*            m_pTarget;
        /// what is to do
        enum SECoP_S_action m_iAction;
        /// "change" value or "do" argument
        SECoP_dataPtr       m_pValue;
        /// creation timestamp for timeout handler
        qint64              m_llCreateTime;
    private:
        /// global storages for next unique id
        static quint64      m_qwNextId;
    };

    /// the status window
    SECoP_S_StatusGui*   m_pGui;
    /// the last created SECoP node for next "add...." function
    SECoP_S_Node*        m_pLastNode;
    /// a list of errors while SEC-node creation
    QStringList          m_aszErrorList;
    /// a list of warnings while SEC-node creation
    QStringList          m_aszWarningList;
    /// the globals list of existing SEC-nodes
    QList<SECoP_S_Node*> m_apNodes;
    /// for polling interface: stored commands, not executed yet
    QList<ActionEntry>   m_aStoredCommands;
    /// for polling interface: commands currently in execution
    QList<ActionEntry>   m_aExecutedCommands;
    /// a mutex for exclusive access to data
    QMutex*              m_pMutex;
    /// a timer which checks stored and executed actions for timeouts
    QTimer*              m_pSessionCleanUpTimer;
    /// the Git hash of this library
    QByteArray           m_szGitVersion;
    /// flag, if library creates many new threads
    bool                 m_bManyThreads;
    /// the singleton instance
    static SECoP_S_Main* m_pInstance;

    friend class SECoP_S_Node;
    friend class SECoP_S_Main::ActionEntry;
};

#endif /*__SECOP_MAIN_H__3BB08092_2314_4A0B_B6CE_4D173DB4123080__*/
