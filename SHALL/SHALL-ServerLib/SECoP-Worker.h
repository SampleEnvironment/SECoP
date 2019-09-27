/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_SERVERTHREADWORKER_H_7AC18402_2894_A53F_4F2D_1B0D_428DBCEA041979__
#define __SECOP_SERVERTHREADWORKER_H_7AC18402_2894_A53F_4F2D_1B0D_428DBCEA041979__

#include <QThread>
#include <QTimer>
#include <QTcpSocket>
#include <QDebug>
#include <QStringList>
#include <QHash>
#include "SECoP-Variant.h"
#include "SECoP-types.h"

// forward declaration
class QMutex;
class SECoP_S_Command;
class SECoP_S_Module;
class SECoP_S_Node;
class SECoP_S_Parameter;

/**
 * \brief The SECoP_S_Worker class handles a client connection.
 */
class SECoP_S_Worker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SECoP_S_Worker)

public:
    explicit SECoP_S_Worker(QTcpSocket* pSocket, SECoP_S_Node* pNode, QObject *pParent = nullptr);
    ~SECoP_S_Worker();

    void moveToThread(QThread *pThread);
    void quitThreadWhileDelete(bool bQuit);
    void parseData(QString szData);
    void writeData(QByteArray szData);

private slots:
    void newParameterValue(SECoP_S_Parameter* pParameter, quint64 qwRequestId, enum SECoP_S_error iErrorCode,
                           const SECoP_dataPtr pValue, const SECoP_dataPtr pSigma, double dblTimestamp);
    void doneCommand(SECoP_S_Command* pCommand, SECoP_S_error iError, const SECoP_dataPtr pValue, double dblTimestamp);
    void readyRead();
    void disconnected();
    void moveToThreadSlot(QThread *pThread);

private:
    void activate(QString szCommandLine, QString szModule);
    void deactivate(QString szCommandLine, QString szModule);
    bool choiceChange(QString szCommandLine, QString szParameter);
    bool choiceRead(QString szCommandLine, QString szParameter);
    void choiceCommand(QString szCommandLine, QString szData);
    void choicePing(QString szToken);
    void writeData(QString szLine);
    void writeData(QString szPrefix, SECoP_dataPtr pValue, SECoP_dataPtr pSigma, double dblTimestamp);
    void writeError(QString szAction, QString szSpecifier, enum SECoP_S_error iError, QString szCommandLine, QString szDescription);
    virtual void timerEvent(QTimerEvent* pEvent);
    void internalChoiceChange(QString szCommandLine, SECoP_S_Module* pModule, SECoP_S_Parameter* pParameter, QString szParameter);
    void internalChoiceRead(QString szCommandLine, SECoP_S_Module* pModule, SECoP_S_Parameter* pParameter);

    /**
     * \brief The TodoEntry struct stores a request, which could not be fulfilled yet.
     */
    struct TodoEntry
    {
        explicit TodoEntry(QString szLine = QString(), bool bChange = false, SECoP_S_Module* pModule = nullptr,
                           SECoP_S_Parameter* pParameter = nullptr, QString szData = QString());
        explicit TodoEntry(const TodoEntry &src);
        TodoEntry& operator=(const TodoEntry &src);
        /// SECoP command line
        QString            m_szLine;
        /// true: change request, false: read request
        bool               m_bChange;
        /// requested module instance
        SECoP_S_Module*    m_pModule;
        /// requested parameter instance
        SECoP_S_Parameter* m_pParameter;
        /// data for change request
        QString            m_szData;
    };

    /**
     * \brief The RequestEntry struct stores information about a running request.
     */
    struct RequestEntry
    {
        explicit RequestEntry(QString szLine = QString(), bool bChange = false, quint64 qwRequestId = 0U);
        explicit RequestEntry(const RequestEntry &src);
        RequestEntry& operator=(const RequestEntry &src);
        /// SECoP command line
        QString m_szLine;
        /// true: change request, false: read request
        bool    m_bChange;
        /// module request id
        quint64 m_qwRequestId;
    };

    /// exclusive access to internal data
    QMutex*                                 m_pMutex;
    /// the TCP connection to the client
    QTcpSocket*                             m_pSocket;
    /// the associated SEC-node
    SECoP_S_Node*                           m_pNode;
    /// an information string about the client
    QString                                 m_szClientInfo;
    /// the list of activated modules
    QList<SECoP_S_Module*>                  m_apActiveList;
    /// a hash of temporary activated modules for read/change requests
    QHash<SECoP_S_Module*, int>             m_hTempConnection;
    /// a hash of running read/change requests
    QHash<SECoP_S_Parameter*, RequestEntry> m_hRequestList;
    /// a hash of running do requests;
    QHash<SECoP_S_Command*, QString>        m_hDoRequestList;
    /// a to-do list of read/change requests
    QList<TodoEntry>                        m_aTodoList;
    /// should the worker thread end together with client connection
    bool                                    m_bQuitThread;
    /// a retry timer for to-do entries
    int                                     m_iTodoTimer;
};

#endif /*__SECOP_SERVERTHREADWORKER_H_7AC18402_2894_A53F_4F2D_1B0D_428DBCEA041979__*/
