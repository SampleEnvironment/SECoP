/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QDebug>
#include <QTimerEvent>
#include <QMutex>
#include <QMutexLocker>
#include <cmath>
#include "SECoP-defines.h"
#include "SECoP-Command.h"
#include "SECoP-Main.h"
#include "SECoP-Module.h"
#include "SECoP-Node.h"
#include "SECoP-Parameter.h"
#include "SECoP-StatusGui.h"
#include "SECoP-Worker.h"

/**
 * \brief constructor of SECoP_S_Worker is created for every client connection.
 *        It connected to some events of the TCP connection and logs it to
 *        status window.
 * \param[in] pSocket TCP connection to client
 * \param[in] pNode   associated SEC-node
 * \param[in] pParent Qt parent object
 */
SECoP_S_Worker::SECoP_S_Worker(QTcpSocket* pSocket, SECoP_S_Node* pNode, QObject *pParent)
    : QObject(pParent)
    , m_pMutex(nullptr)
    , m_pSocket(pSocket)
    , m_pNode(pNode)
    , m_szClientInfo(QString("%1:%2").arg(pSocket->peerAddress().toString()).arg(pSocket->peerPort()))
    , m_bQuitThread(false)
    , m_iTodoTimer(0)
{
    m_pSocket->setParent(this); // take ownership which allows changing the thread
    m_pMutex = new QMutex(QMutex::Recursive);

    // connect socket and signal
    // note - Qt::DirectConnection is used because it's multithreaded
    //        This makes the slot to be invoked immediately, when the signal is emitted.
    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(readyRead())/*, Qt::DirectConnection*/);
    connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));

    // We'll have multiple clients, we want to know which is which
    SECoP_S_Main::logAddConnection(m_pNode, m_pSocket);
}

/**
 * \brief destructor of SECoP_S_Worker, it destroys the connection and if
 *        enabled, it also triggers the exit of the Qt thread, which hosted
 *        this worker instance.
 */
SECoP_S_Worker::~SECoP_S_Worker()
{
    disconnect(this);
    if (m_pSocket != nullptr)
        delete m_pSocket;
    if (m_bQuitThread)
        thread()->quit();

    if (m_pMutex != nullptr)
    {
        m_pMutex->lock();
        QMutex* pMutex(m_pMutex);
        m_pMutex = nullptr;
        pMutex->unlock();
        delete pMutex;
    }
}

/**
 * \brief This function informs the worker, if it should while deletion quit
 *        the thread, which hosts this worker instance.
 * \param[in] bQuit true: quit thread too, false: do nothing with threads
 */
void SECoP_S_Worker::quitThreadWhileDelete(bool bQuit)
{
    m_bQuitThread = bQuit;
}

/**
 * \brief Overwritten function for moving QThread association around. Normally
 *        you only can "push" the object away into another QThread. This
 *        function also asks the foreign thread to push it back.
 * \param[in] pThread target thread, which should host this object
 */
void SECoP_S_Worker::moveToThread(QThread* pThread)
{
    if (thread() == QThread::currentThread())
        moveToThreadSlot(pThread);
    else
        QMetaObject::invokeMethod(this, "moveToThreadSlot", Qt::BlockingQueuedConnection, Q_ARG(QThread*, pThread));
}

/**
 * \brief Helper function to push the QThread association to another thread.
 * \param[in] pThread target thread, which should host this object
 */
void SECoP_S_Worker::moveToThreadSlot(QThread* pThread)
{
    QObject::moveToThread(pThread);
}

/**
 * \brief This function is called for the SECoP "activate" command. It connects
 *        to the wanted module(s) and prints the cached values of their
 *        parameters.
 * \param[in] szCommandLine SECoP client command line
 * \param[in] szModule      module name to activate
 */
void SECoP_S_Worker::activate(QString szCommandLine, QString szModule)
{
    QList<SECoP_S_Module*> apPrintList;
    bool bFound;
    szModule = szModule.simplified();
    int i(szModule.indexOf(' '));
    if (i >= 0)
        szModule.resize(i);
    i = szModule.indexOf(':');
    if (i >= 0)
        szModule.resize(i);
    bFound = szModule.isEmpty();
    if (!bFound && !SECoP_S_Main::isValidName(szModule))
    {
        writeError("activate", szModule, SECoP_S_ERROR_INVALID_NAME, szCommandLine, nullptr);
        return;
    }
    for (SECoP_S_Module* pModule(m_pNode->getModule(i = 0)); pModule != nullptr; pModule = m_pNode->getModule(++i))
    {
        if (!szModule.isEmpty() && pModule->getModuleID().compare(szModule, Qt::CaseInsensitive) != 0)
            continue;
        bFound = true;
        if (!m_apActiveList.contains(pModule))
        {
            // interested to signals of this module
            connect(pModule, SIGNAL(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                    this, SLOT(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                    Qt::QueuedConnection);
            m_apActiveList.append(pModule);
        }
        apPrintList.append(pModule);
    }
    for (auto itm = apPrintList.constBegin(); itm != apPrintList.constEnd(); ++itm)
    {
        const SECoP_S_Module* pModule(*itm);
        QString szModuleID;
        for (const SECoP_S_Parameter* pParameter(pModule->getParameter(i = 0)); pParameter != nullptr; pParameter = pModule->getParameter(++i))
        {
            if (szModuleID.isNull())
                szModuleID = pModule->getModuleID();
            if (pParameter->isConstant())
                continue;
            if (pParameter->hasValue())
                writeData(QString("update %1:%2").arg(szModuleID).arg(pParameter->getParameterID()), pParameter->value(),
                          pParameter->sigma(), pParameter->timestamp());
            else
                writeData(QString("update %1:%2").arg(szModuleID).arg(pParameter->getParameterID()), SECoP_dataPtr(),
                          SECoP_dataPtr(), pParameter->timestamp());
        }
    }
    if (bFound)
    {
        if (!szModule.isEmpty())
            szModule.prepend(' ');
        szModule.prepend("active");
        writeData(szModule);
    }
    else
        writeError("activate", szModule, SECoP_S_ERROR_INVALID_MODULE, szCommandLine, nullptr);
}

/**
 * \brief This function is called for the SECoP "deactivate" command. It
 *        disconnects from wanted module(s).
 * \param[in] szCommandLine SECoP client command line
 * \param[in] szModule      module name to deactivate
 */
void SECoP_S_Worker::deactivate(QString szCommandLine, QString szModule)
{
    int i(szModule.indexOf(' '));
    bool bFound;
    szModule = szModule.simplified();
    if (i >= 0)
        szModule.resize(i);
    i = szModule.indexOf(':');
    if (i >= 0)
        szModule.resize(i);
    bFound = szModule.isEmpty();
    if (!bFound && !SECoP_S_Main::isValidName(szModule))
    {
        writeError("activate", szModule, SECoP_S_ERROR_INVALID_NAME, szCommandLine, nullptr);
        return;
    }
    for (SECoP_S_Module* pModule(m_pNode->getModule(i = 0)); pModule != nullptr; pModule = m_pNode->getModule(++i))
    {
        if (!szModule.isEmpty() && pModule->getModuleID().compare(szModule, Qt::CaseInsensitive) != 0)
            continue;
        bFound = true;
        if (m_apActiveList.contains(pModule))
        {
            // not interested in signals from this module
            disconnect(pModule, SIGNAL(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                       this, SLOT(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)));
            m_apActiveList.removeAll(pModule);
        }
    }
    if (bFound)
    {
        if (!szModule.isEmpty())
            szModule.prepend(' ');
        szModule.prepend("inactive");
        writeData(szModule);
    }
    else
        writeError("deactivate", szModule, SECoP_S_ERROR_INVALID_MODULE, szCommandLine, nullptr);
}

/**
 * \brief This Qt event handler is called when a new SECoP command comes in
 *        from the client. It reads a complete line and calls \ref SECoP_S_Worker::parseData.
 */
void SECoP_S_Worker::readyRead()
{
    while (m_pSocket->canReadLine())
    {
        // get the information
        QByteArray szData = m_pSocket->readLine();
        while (szData.endsWith('\r') || szData.endsWith('\n'))
            szData.chop(1);

        // will write on server side window
        SECoP_S_Worker::parseData(szData);
    }
}

/**
 * \brief This function is used for answering the client. It appends a line
 *        break, if necessary.
 * \param[in] szData text to send back
 */
void SECoP_S_Worker::writeData(QByteArray szData)
{
    while (szData.endsWith('\r') || szData.endsWith('\n'))
        szData.chop(1);
    szData.append(10);
    SECoP_S_Main::log(m_pNode, QString("[send %1] %2").arg(m_szClientInfo).arg(QString(szData)), true);
    m_pSocket->write(szData);
}

/**
 * \brief This function is used for answering the client. It appends a line
 *        break, if necessary.
 * \param[in] szData text to send back
 */
void SECoP_S_Worker::writeData(QString szData)
{
    SECoP_S_Worker::writeData(szData.toUtf8());
}

/**
 * \brief This function is used for answering the client. It converts a variant
 *        into a json, appends the SECoP timestamp and appends a line break.
 * \param[in] szPrefix     line prefix to send, e.g. SECoP answer
 * \param[in] pValue       variant to convert and send
 * \param[in] pSigma       error of value of the parameter
 * \param[in] dblTimestamp timestamp to append
 */
void SECoP_S_Worker::writeData(QString szPrefix, SECoP_dataPtr pValue, SECoP_dataPtr pSigma, double dblTimestamp)
{
    using std::isnan;
    using std::isinf;
    nlohmann::json v;
    nlohmann::json o(nlohmann::json::object());
    if (!isnan(dblTimestamp) && !isinf(dblTimestamp))
        o["t"] = dblTimestamp;
    if (pSigma != nullptr && pSigma.get() != nullptr)
    {
        nlohmann::json s(pSigma->exportSECoPjson());
        if (!s.is_null())
            o["e"] = s;
    }
    if (pValue != nullptr && pValue.get() != nullptr)
        v.push_back(pValue->exportSECoPjson());
    else
        v.push_back(nlohmann::json());
    v.push_back(o);
    szPrefix.append(' ');
    szPrefix.append(QString::fromStdString(v.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace)));
    writeData(szPrefix.toUtf8());
}

/**
 * \brief This function is called for the SECoP "change" command. It looks for
 *        the module and parameter and starts the request.
 * \param[in] szCommandLine SECoP client command line
 * \param[in] szParameter   module and parameter with new value
 * \return true: request could be handled, false: retry via to-do timer
 */
bool SECoP_S_Worker::choiceChange(QString szCommandLine, QString szParameter)
{
    int iPos(szParameter.indexOf(' '));
    if (iPos < 0)
    {
        writeError("change", szParameter, SECoP_S_ERROR_INVALID_VALUE, szCommandLine, QString());
        return true;
    }
    QString szArgument(szParameter);
    szArgument.remove(0, iPos + 1);
    szParameter.resize(iPos);
    iPos = szParameter.indexOf(':');
    if (iPos < 0)
    {
        writeError("change", szParameter, SECoP_S_ERROR_INVALID_NAME, szCommandLine, QString());
        return true;
    }
    SECoP_S_Module* pModule(m_pNode->getModule(m_pNode->modulePosition(szParameter.left(iPos))));
    if (pModule == nullptr)
    {
        writeError("change", szParameter, SECoP_S_ERROR_INVALID_MODULE, szCommandLine, QString());
        return true;
    }
    SECoP_S_Parameter* pParameter(pModule->getParameter(pModule->parameterPosition(szParameter.mid(iPos + 1))));
    if (pParameter == nullptr || pParameter->isConstant())
    {
        writeError("change", szParameter, SECoP_S_ERROR_INVALID_PARAMETER, szCommandLine, QString());
        return true;
    }
    if (!pParameter->isWritable())
    {
        writeError("change", szParameter, SECoP_S_ERROR_READONLY, szCommandLine, QString());
        return true;
    }

    QMutexLocker locker(m_pMutex);
    if (m_hRequestList.contains(pParameter))
    {
        m_aTodoList.append(TodoEntry(szCommandLine, true, pModule, pParameter, szArgument));
        return false;
    }

    internalChoiceChange(szCommandLine, pModule, pParameter, szArgument);
    return true;
}

/**
 * \brief This function is called for the SECoP "change" command. It converts
 *        the new value and starts the request.
 * \param[in] szCommandLine SECoP client command line
 * \param[in] pModule       module to change
 * \param[in] pParameter    parameter to change
 * \param[in] szValue       new value
 */
void SECoP_S_Worker::internalChoiceChange(QString szCommandLine, SECoP_S_Module* pModule,
                                          SECoP_S_Parameter* pParameter, QString szValue)
{
    SECoP_dataPtr pValue(pParameter->value());
    if (pValue->importSECoP(szValue.toUtf8().constData(), true))
    {
        bool bConnected(false);
        quint64 qwRequestId(0);
        if (!m_apActiveList.contains(pModule))
        {
            bool bFound(false);
            for (auto it = m_hRequestList.constBegin(); it != m_hRequestList.constEnd(); ++it)
            {
                if (it.key()->getParentModule() == pModule)
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound) // interested to signals of this module
                bConnected =
                    connect(pModule, SIGNAL(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                    this, SLOT(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                    Qt::QueuedConnection);
        }
        enum SECoP_S_error iErrorCode(pParameter->getParentModule()->changeParameter(pParameter, pValue, &qwRequestId));
        if (iErrorCode < 0)
        {
            if (bConnected)
                disconnect(pModule, SIGNAL(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                this, SLOT(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)));
            writeError("change", QString("%1:%2").arg(pModule->getModuleID()).arg(pParameter->getParameterID()), iErrorCode,
                       szCommandLine, QString());
        }
        else
            m_hRequestList.insert(pParameter, SECoP_S_Worker::RequestEntry(szCommandLine, true, qwRequestId));
    }
    else
        writeError("change", QString("%1:%2").arg(pModule->getModuleID()).arg(pParameter->getParameterID()),
                   SECoP_S_ERROR_INVALID_VALUE, szCommandLine, szValue);
}

/**
 * \brief This function is called for the SECoP "read" command. It looks for
 *        the module and parameter and starts the request.
 * \param[in] szCommandLine SECoP client command line
 * \param[in] szParameter   module and parameter
 * \return true: request could be handled, false: retry via to-do timer
 */
bool SECoP_S_Worker::choiceRead(QString szCommandLine, QString szParameter)
{
    int iPos(szParameter.indexOf(' '));
    if (iPos >= 0)
        szParameter.resize(iPos);
    iPos = szParameter.indexOf(':');
    if (iPos < 0)
    {
        writeError("read", szParameter, SECoP_S_ERROR_INVALID_NAME, szCommandLine, QString());
        return true;
    }
    SECoP_S_Module* pModule(m_pNode->getModule(m_pNode->modulePosition(szParameter.left(iPos))));
    if (pModule == nullptr)
    {
        writeError("read", szParameter, SECoP_S_ERROR_INVALID_MODULE, szCommandLine, QString());
        return true;
    }
    szParameter.remove(0, iPos + 1);
    SECoP_S_Parameter* pParameter(pModule->getParameter(pModule->parameterPosition(szParameter)));
    if (pParameter == nullptr || pParameter->isConstant())
    {
        iPos = szParameter.indexOf(' ');
        if (iPos < 0)
            iPos = 0;
        writeError("read", QString("%1:%2").arg(pModule->getModuleID()).arg(szParameter.left(iPos)),
                   SECoP_S_ERROR_INVALID_PARAMETER, szCommandLine, QString());
        return true;
    }

    QMutexLocker locker(m_pMutex);
    if (m_hRequestList.contains(pParameter))
    {
        m_aTodoList.append(TodoEntry(szCommandLine, false, pModule, pParameter, QString()));
        return false;
    }

    internalChoiceRead(szCommandLine, pModule, pParameter);
    return true;
}

/**
 * \brief This function is called for the SECoP "read" command.
 *        It starts the request.
 * \param[in] szCommandLine SECoP client command line
 * \param[in] pModule       module to change
 * \param[in] pParameter    parameter to change
 */
void SECoP_S_Worker::internalChoiceRead(QString szCommandLine, SECoP_S_Module* pModule, SECoP_S_Parameter* pParameter)
{
    quint64 qwRequestId(0);
    bool bConnected(false);
    if (!m_apActiveList.contains(pModule))
    {
        bool bFound(false);
        for (auto it = m_hRequestList.constBegin(); it != m_hRequestList.constEnd(); ++it)
        {
            if (it.key()->getParentModule() == pModule)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound) // interested to signals of this module
            bConnected =
                connect(pModule, SIGNAL(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                        this, SLOT(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                        Qt::QueuedConnection);
    }
    enum SECoP_S_error iErrorCode(pParameter->getParentModule()->readParameter(pParameter, &qwRequestId));
    if (iErrorCode < 0)
    {
        if (bConnected)
            disconnect(pModule, SIGNAL(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
            this, SLOT(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)));
        writeError("read", QString("%1:%2").arg(pModule->getModuleID()).arg(pParameter->getParameterID()),
                   iErrorCode, szCommandLine, QString());
    }
    else
        m_hRequestList.insert(pParameter, SECoP_S_Worker::RequestEntry(szCommandLine, false, qwRequestId));
}

/**
 * \brief This function is called via Qt event system by the module, which
 *        finished the read or change request.
 * \param[in] pParameter    parameter, which was requested
 * \param[in] qwRequestId   request id of the module
 * \param[in] iErrorCode    result of request as SECoP_S_error code
 * \param[in] pValue        read or updated value of parameter
 * \param[in] pSigma        error of value of the parameter
 * \param[in] dblTimestamp  timestamp of request
 */
void SECoP_S_Worker::newParameterValue(SECoP_S_Parameter* pParameter, quint64 qwRequestId, enum SECoP_S_error iErrorCode,
                                       const SECoP_dataPtr pValue, const SECoP_dataPtr pSigma, double dblTimestamp)
{
    QMutexLocker locker(m_pMutex);
    SECoP_S_Module* pModule(pParameter->getParentModule());
    SECoP_S_Worker::RequestEntry entry(QString(), false, 0);
    bool bSubscribed(m_apActiveList.contains(pModule));
    bool bReply(false);

    // a parameter value was read, updated or has changed
    if (m_hRequestList.contains(pParameter))
    {
        entry = m_hRequestList[pParameter];
        m_hRequestList.remove(pParameter);
        bReply = true;
    }
    else if (!bSubscribed) // not subscribed data should be ignored
        goto trydisconnect;
    if (!pParameter->isConstant())
    {
        if (iErrorCode >= 0)
        {
            // valid data
            QString szReply(entry.m_bChange ? "changed" : bReply ? "reply" : "update");
            szReply.append(' ');
            SECoP_S_Module* pModule(dynamic_cast<SECoP_S_Module*>(pParameter->parent()));
            Q_ASSERT(pModule != nullptr);
            szReply.append(pModule->getModuleID());
            szReply.append(':');
            szReply.append(pParameter->getParameterID());
            writeData(szReply, pValue, pSigma, dblTimestamp);
        }
        else if (qwRequestId > 0 && entry.m_qwRequestId == qwRequestId) // this request was generated here
            writeError(entry.m_bChange ? "change" : "read", QString("%1:%2").arg(pParameter->getParentModule()->getModuleID()).
                       arg(pParameter->getParameterID()), iErrorCode, entry.m_szLine, QString());
    }
    if (!bSubscribed)
    {
trydisconnect:
        bool bFound(false);
        for (auto it = m_hRequestList.constBegin(); it != m_hRequestList.constEnd(); ++it)
        {
            if (it.key()->getParentModule() == pModule)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound) // not interested in signals from this module
            disconnect(pModule, SIGNAL(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)),
                       this, SLOT(newParameterValue(SECoP_S_Parameter*, quint64, SECoP_S_error, const SECoP_dataPtr, const SECoP_dataPtr, double)));
    }
}

/**
 * \brief This function is called for the SECoP "do" command. It looks for
 *        the module and command, and starts the request.
 * \param[in] szCommandLine SECoP client command line
 * \param[in] szCommand     module and command with argument
 */
void SECoP_S_Worker::choiceCommand(QString szCommandLine, QString szCommand)
{
    QString szArgument;
    // split argument from module:command
    int iPos(szCommand.indexOf(' '));
    if (iPos >= 0)
    {
        szArgument = szCommand.mid(iPos + 1);
        szCommand.resize(iPos);
    }
    // split module from command
    iPos = szCommand.indexOf(':');
    if (iPos < 0)
    {
        writeError("do", szCommand, SECoP_S_ERROR_INVALID_NAME, szCommandLine, QString());
        return;
    }
    SECoP_S_Module* pModule(m_pNode->getModule(m_pNode->modulePosition(szCommand.left(iPos))));
    if (pModule == nullptr)
    {
        writeError("do", szCommand, SECoP_S_ERROR_INVALID_MODULE, szCommandLine, QString());
        return;
    }
    SECoP_S_Command* pCommand(pModule->getCommand(pModule->commandPosition(szCommand.mid(iPos + 1))));
    if (pCommand == nullptr)
    {
        writeError("do", szCommand, SECoP_S_ERROR_INVALID_COMMAND, szCommandLine, QString());
        return;
    }

    // get argument
    SECoP_dataPtr pArgument(pCommand->getArgument()->duplicate());
    if (pArgument.get() != nullptr)
    {
        if (!pArgument->importSECoP(szArgument.toUtf8().constData(), true))
        {
            writeError("do", szCommand, SECoP_S_ERROR_INVALID_VALUE, szCommandLine, QString());
            return;
        }
    }
    if (m_hDoRequestList.contains(pCommand))
    {
        writeError("do", szCommand, SECoP_S_ERROR_COMMAND_RUNNING, szCommandLine, QString());
        return;
    }
    m_hDoRequestList[pCommand] = szCommandLine;

    // do call inside module
    QMetaObject::invokeMethod(pModule, "doCommand", Qt::QueuedConnection, Q_ARG(SECoP_S_Command*, pCommand),
                              Q_ARG(SECoP_dataPtr, pArgument), Q_ARG(QObject*, this));
}

/**
 * \brief This function is triggered by a module for a completed command
 *        invocation.
 * \param[in] pCommand      invoked command instance
 * \param[in] iError        result of invocation as SECoP_S_error
 * \param[in] pValue        result value of invocation
 * \param[in] dblTimestamp  timestamp of command invocation
 */
void SECoP_S_Worker::doneCommand(SECoP_S_Command* pCommand, SECoP_S_error iError, const SECoP_dataPtr pValue, double dblTimestamp)
{
    // return from function call inside module
    QString szCommand(QString("%1:%2").arg(pCommand->getParentModule()->getModuleID()).arg(pCommand->getCommandID()));
    QString szCommandLine;
    if (m_hDoRequestList.contains(pCommand))
    {
        szCommandLine = m_hDoRequestList[pCommand];
        m_hDoRequestList.remove(pCommand);
    }
    if (iError == SECoP_S_SUCCESS)
        writeData(szCommand.prepend("done "), pValue, SECoP_dataPtr(), dblTimestamp);
    else
        writeError("do", szCommand, iError, szCommandLine, QString());
}

/**
 * \brief This function is triggered by a heartbeat command invocation.
 * \param[in] szToken token of the SECoP client
 */
void SECoP_S_Worker::choicePing(QString szToken)
{
    szToken = szToken.simplified();
    int iPos(szToken.indexOf(' '));
    if (iPos >= 0)
        szToken.resize(iPos);
    szToken.prepend("pong ");
    writeData(szToken, nullptr, nullptr, SECoP_S_Main::getCurrentTime());
}

/**
 * \brief The function parseData is called for every SECoP client command line. It
 *        parses it, handles simple commands directly or calls helper functions.
 *        Parsing the comma separated SECoP triple pattern with action, specifier and data (JSON part).
 *        Following a ignore strategy as described in SECoP ISSUE 30 [] the first call is ignoreSplit.
 *
 * \param[in] szData command line of the SECoP client
 */
void SECoP_S_Worker::parseData(QString szData)
{
    // split on first space to split instruction from data
    SECoP_S_Main::log(m_pNode, QString("[recv %1] %2").arg(m_szClientInfo).arg(szData), true);
    QString szSECoPCmd, szCommandLine(szData);
    int iPos(szData.indexOf(' '));
    if (iPos >= 0)
    {
        szSECoPCmd = szData.left(iPos);
        szData.remove(0, iPos + 1);
    }
    else
    {
        szSECoPCmd = szData;
        szData.clear();
    }

    if (szSECoPCmd.compare("describe", Qt::CaseInsensitive) == 0)
    {
        // describe this SECoP node
        writeData(QString("describing . %1").arg(m_pNode->getJSON().dump(-1, ' ', false,
                  nlohmann::json::error_handler_t::replace).c_str()).toUtf8());
    }
    else if (szSECoPCmd.compare("change", Qt::CaseInsensitive) == 0)
    {
        // set a parameter
        if (!choiceChange(szCommandLine, szData))
            if (m_iTodoTimer <= 0)
                m_iTodoTimer = startTimer(50);
    }
    else if (szSECoPCmd.compare("read", Qt::CaseInsensitive) == 0)
    {
        // get a parameter
        if (!choiceRead(szCommandLine, szData))
            if (m_iTodoTimer <= 0)
                m_iTodoTimer = startTimer(50);
    }
    else if (szSECoPCmd.compare("do", Qt::CaseInsensitive) == 0)
    {
        // call a function (command)
        choiceCommand(szCommandLine, szData);
    }
    else if (szSECoPCmd.compare("help", Qt::CaseInsensitive) == 0)
    {
        // interactive help with Git hash
        QByteArray szGitVersion(SECoP_S_Main::getGitVersion());
        szData = "commands are: *IDN?, ping, describe, change, read, do, activate, deactivate and help";
        if (!szGitVersion.isEmpty())
        {
            szData.append(". Git ");
            szData.append(szGitVersion);
        }
        writeData(szData);
    }
    else if (szSECoPCmd.compare("*IDN?", Qt::CaseSensitive) == 0)
    {
        // identify SECoP node
        writeData(QByteArray("ISSE&SINE2020,SECoP,V2019-09-16,v1.0"));
    }
    else if (szSECoPCmd.compare("ping", Qt::CaseInsensitive) == 0)
    {
        // connection check
        choicePing(szData);
    }
    else if (szSECoPCmd.compare("activate", Qt::CaseInsensitive) == 0)
    {
        // activate asynchronous mode
        activate(szCommandLine, szData);
    }
    else if (szSECoPCmd.compare("deactivate", Qt::CaseInsensitive) == 0)
    {
        // activate synchronous mode
        deactivate(szCommandLine, szData);
    }
    else
    {
        // fall through with wrong/unknown SECoP command
        writeError(szSECoPCmd, QString(), SECoP_S_ERROR_SYNTAX, szCommandLine,
                   QString("%1 is unknown, type help to get a list of commands").arg(szSECoPCmd));
    }
}

/**
 * \brief This function generates an error message answer to the SECoP client.
 * \param[in] szAction      SECoP action for error message
 * \param[in] szSpecifier   SECoP specifier for error message
 * \param[in] iError        SECoP_S_error to convert and send
 * \param[in] szCommandLine SECoP client command line
 * \param[in] szDescription additional description text for user
 */
void SECoP_S_Worker::writeError(QString szAction, QString szSpecifier, enum SECoP_S_error iError, QString szCommandLine, QString szDescription)
{
    QByteArray szData("error");
    nlohmann::json a;
    if (!szAction.isEmpty())
    {
        szData.append('_');
        szData.append(szAction.toUtf8());
    }
    szData.append(' ');
    szData.append(szSpecifier.toUtf8());
    switch (iError)
    {
        case SECoP_S_ERROR_INVALID_MODULE:
            a.push_back(nlohmann::json("NoSuchModule"));
            break;
        case SECoP_S_ERROR_INVALID_PARAMETER:
            a.push_back(nlohmann::json("NoSuchParameter"));
            break;
        case SECoP_S_ERROR_INVALID_COMMAND:
            a.push_back(nlohmann::json("NoSuchCommand"));
            break;
        case SECoP_S_ERROR_NOT_IMPLEMENTED:
            a.push_back(nlohmann::json("NotImplemented"));
            break;
        case SECoP_S_ERROR_COMMAND_FAILED:
            a.push_back(nlohmann::json("CommunicationFailed"));
            break;
        case SECoP_S_ERROR_COMMAND_RUNNING:
            a.push_back(nlohmann::json("CommandRunning"));
            break;
        case SECoP_S_ERROR_READONLY:
            a.push_back(nlohmann::json("ReadOnly"));
            break;
        case SECoP_S_ERROR_INVALID_VALUE:
            a.push_back(nlohmann::json("BadValue"));
            break;
        case SECoP_S_ERROR_TIMEOUT:
        case SECoP_S_ERROR_COMM_FAILED:
            a.push_back(nlohmann::json("CommunicationFailed"));
            break;
        case SECoP_S_ERROR_IS_BUSY:
            a.push_back(nlohmann::json("IsBusy"));
            break;
        case SECoP_S_ERROR_IS_ERROR:
            a.push_back(nlohmann::json("IsError"));
            break;
        case SECoP_S_ERROR_DISABLED:
            a.push_back(nlohmann::json("Disabled"));
            break;
        case SECoP_S_ERROR_UNKNOWN_COMMAND:
        case SECoP_S_ERROR_INVALID_NAME:
        case SECoP_S_ERROR_INVALID_NODE:
        case SECoP_S_ERROR_SYNTAX:
            a.push_back(nlohmann::json("ProtocolError"));
            break;
        case SECoP_S_ERROR_INTERNAL:
        default:
            a.push_back(nlohmann::json("InternalError"));
            break;
    }
    if (szDescription.isEmpty())
        a.push_back(nlohmann::json(szCommandLine.toStdString()));
    else
        a.push_back(nlohmann::json(szDescription.toStdString()));
    a.push_back(nlohmann::json::object_t());
    szData.append(' ');
    szData.append(QString::fromStdString(a.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace)));
    writeData(szData);
}

/**
 * \brief This Qt event handler is called for to-do entries, which could not be
 *        started earlier. It retries until the to-do list is empty.
 * \param[in] pEvent
 */
void SECoP_S_Worker::timerEvent(QTimerEvent* pEvent)
{
    if (pEvent == nullptr)
        return;
    int iTimerId(pEvent->timerId());
    if (iTimerId < 1)
        return;
    if (m_iTodoTimer > 0 && iTimerId == m_iTodoTimer)
    {
        QMutexLocker locker(m_pMutex);
        for (;;)
        {
            if (m_aTodoList.isEmpty())
            {
                killTimer(m_iTodoTimer);
                m_iTodoTimer = 0;
                break;
            }
            struct TodoEntry entry(m_aTodoList.first());
            if (m_hRequestList.contains(entry.m_pParameter))
                break;

            m_aTodoList.removeFirst();
            if (entry.m_bChange)
                internalChoiceChange(entry.m_szLine, entry.m_pModule, entry.m_pParameter, entry.m_szData);
            else
                internalChoiceRead(entry.m_szLine, entry.m_pModule, entry.m_pParameter);
        }
    }
}

/**
 * \brief Called by the Qt event system, when the client connection is closed.
 */
void SECoP_S_Worker::disconnected()
{
    SECoP_S_Main::forgetStoredCommands(this);
    SECoP_S_Main::logRemoveConnection(m_pSocket);
    deleteLater();
}

/**
 * \brief default constructor of SECoP_S_Worker::TodoEntry
 * \param[in] szLine     SECoP client command line
 * \param[in] bChange    false: read request, true: change request
 * \param[in] pModule    associated module instance
 * \param[in] pParameter associated parameter instance
 * \param[in] szData     for change request new value
 */
SECoP_S_Worker::TodoEntry::TodoEntry(QString szLine, bool bChange, SECoP_S_Module* pModule, SECoP_S_Parameter* pParameter, QString szData)
    : m_szLine(szLine)
    , m_bChange(bChange)
    , m_pModule(pModule)
    , m_pParameter(pParameter)
    , m_szData(szData)
{
}

/**
 * \brief copy constructor of SECoP_S_Worker::TodoEntry
 * \param[in] src source to copy
 */
SECoP_S_Worker::TodoEntry::TodoEntry(const SECoP_S_Worker::TodoEntry &src)
    : m_szLine(src.m_szLine)
    , m_bChange(src.m_bChange)
    , m_pModule(src.m_pModule)
    , m_pParameter(src.m_pParameter)
    , m_szData(src.m_szData)
{
}

/**
 * \brief copy operator of SECoP_S_Worker::TodoEntry
 * \param[in] src source to copy
 * \return the reference of this instance
 */
SECoP_S_Worker::TodoEntry& SECoP_S_Worker::TodoEntry::operator=(const SECoP_S_Worker::TodoEntry &src)
{
    m_szLine     = src.m_szLine;
    m_bChange    = src.m_bChange;
    m_pModule    = src.m_pModule;
    m_pParameter = src.m_pParameter;
    m_szData     = src.m_szData;
    return *this;
}

/**
 * \brief default constructor of SECoP_S_Worker::RequestEntry
 * \param[in] szLine      SECoP client command line
 * \param[in] iType       request type
 * \param[in] qwRequestId module request id
 */
SECoP_S_Worker::RequestEntry::RequestEntry(QString szLine, bool bChange, quint64 qwRequestId)
    : m_szLine(szLine)
    , m_bChange(bChange)
    , m_qwRequestId(qwRequestId)
{
}

/**
 * \brief copy constructor of SECoP_S_Worker::RequestEntry
 * \param[in] src source to copy
 */
SECoP_S_Worker::RequestEntry::RequestEntry(const SECoP_S_Worker::RequestEntry &src)
    : m_szLine(src.m_szLine)
    , m_bChange(src.m_bChange)
    , m_qwRequestId(src.m_qwRequestId)
{
}

/**
 * \brief copy operator of SECoP_S_Worker::RequestEntry
 * \param[in] src source to copy
 * \return the reference of this instance
 */
SECoP_S_Worker::RequestEntry& SECoP_S_Worker::RequestEntry::operator=(const SECoP_S_Worker::RequestEntry &src)
{
    m_szLine      = src.m_szLine;
    m_bChange     = src.m_bChange;
    m_qwRequestId = src.m_qwRequestId;
    return *this;
}
