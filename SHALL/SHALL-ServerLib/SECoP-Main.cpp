/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <thread>
#include <QVector>
#include <QMutexLocker>
#include <QDateTime>
#include <QHostAddress>
#include <iostream>
#include "SECoP.h"
#include "SECoP-Command.h"
#include "SECoP-Main.h"
#include "SECoP-Module.h"
#include "SECoP-Node.h"
#include "SECoP-Parameter.h"
#include "SECoP-StatusGui.h"
#include "SECoP-Worker.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define qUtf8Printable(string) QString(string).toUtf8().constData()
#define local_qInfo qDebug
#else
#define local_qInfo qInfo
#endif

/*The SECoP class specifys SECoP. All exported functions, SECoP properties and errorchecking is declared here. */

/* declare data types for Qt's signal/slot system */
Q_DECLARE_METATYPE(bool*)
Q_DECLARE_METATYPE(char*)
Q_DECLARE_METATYPE(int*)
Q_DECLARE_METATYPE(qulonglong*)
Q_DECLARE_METATYPE(SECoP_S_action*)
Q_DECLARE_METATYPE(SECoP_S_callFunction)
Q_DECLARE_METATYPE(SECoP_S_Command*)
Q_DECLARE_METATYPE(SECoP_dataPtr)
Q_DECLARE_METATYPE(SECoP_dataPtr*)
Q_DECLARE_METATYPE(SECoP_S_error)
Q_DECLARE_METATYPE(SECoP_S_error*)
Q_DECLARE_METATYPE(SECoP_S_getsetFunction)
Q_DECLARE_METATYPE(SECoP_S_Module*)
Q_DECLARE_METATYPE(SECoP_S_Node*)
Q_DECLARE_METATYPE(SECoP_S_Parameter*)

/* global variables */
/**
 * \brief flag, if there is an exit handler installed
 * \ingroup intfunc
 */
static bool g_bExitHandlerInstalled = false;

/**
 * \brief pointer to old Qt message logger
 * \ingroup intfunc
 */
static QtMessageHandler g_pOldMessageHandler = nullptr;

/**
 * \brief Qt application pointer, if library had to create it
 * \ingroup intfunc
 */
static QApplication* g_pApplication = nullptr;

/**
 * \brief flag, if the status window should be visible
 * \ingroup intfunc
 */
static bool g_bShowGUI;

/**
 * \brief flag, if we have function pointers or need polling
 * \ingroup intfunc
 */
static bool g_bHasFunctionPointers;

/**
 * \brief the real singleton instance
 * \ingroup intfunc
 */
static SECoP_S_Main* g_pSECoPMain = nullptr;

/**
 * \brief dummy number of arguments for local QApplication instance
 * \ingroup intfunc
 */
static int g_iArgc = 0;

/**
 * \brief dummy arguments for local QApplication instance
 * \ingroup intfunc
 */
static char* g_aszArgv[2] = {nullptr, nullptr};

/**
 * \brief dummy first argument for local QApplication instance
 * \ingroup intfunc
 */
static char* g_szArgv0 = nullptr;

/**
 * \brief helper thread as "first" thread for local QApplication instance
 * \ingroup intfunc
 */
static std::thread* g_pThread = nullptr;

/**
 * \brief global initialization status
 * \ingroup intfunc
 */
static volatile bool g_bInitialized = false;

/* forward declarations */
static void SECoP_S_initLibraryThread(void);
static void SECoP_S_initLibraryHelper(void);
static void SECoP_S_initLibraryExit(void);
static void SECoP_S_initLibraryExitHelper(bool bAtExit);
static void SECoP_S_MessageHandler(QtMsgType iType, const QMessageLogContext &context, const QString &szMessage);

/*
 * \brief SECoP_S_initLibrary is the first function to call. It initializes
 *        the library and prepares internal data. The pApplication pointer
 *        is used, if you already have a QApplication instance in your program.
 *        Provide a nullptr pointer here for all other programs and the library
 *        will instanciate its own QApplication class. If you disable
 *        function pointers, you should provide nullptr pointers to functions
 *        (\ref SECoP_S_addCommand, \ref SECoP_S_addReadableParameter,
 *        \ref SECoP_S_addWritableParameter).
 * \ingroup expfunc
 * \param[in] pApplication pointer to existing Qt application or nullptr
 * \param[in] bGUI   0=false: do not show status window, 1=true: show status window
 * \param[in] bEnableFunctionPointers
 *                   0=false: disable function pointers and force polling,
 *                   1=true: use function pointers
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_initLibrary(QApplication *pApplication, int bGUI, int bEnableFunctionPointers)
{
    if (g_bInitialized || g_pSECoPMain != nullptr || g_pThread != nullptr || g_pOldMessageHandler != nullptr)
    {
        g_pSECoPMain->cleanUp(true);
        return SECoP_S_SUCCESS;
    }
    if (g_pOldMessageHandler == nullptr) // install message logger
        g_pOldMessageHandler = qInstallMessageHandler(&SECoP_S_MessageHandler);
    g_bShowGUI = (bGUI != 0);
    g_bHasFunctionPointers = (bEnableFunctionPointers != 0);
    if (pApplication == nullptr)
    {
        g_pThread = new std::thread(&SECoP_S_initLibraryThread);
        if (g_pThread == nullptr)
            return SECoP_S_ERROR_NOT_INITIALIZED;
        while (!g_bInitialized)
            std::this_thread::yield();
        if (!g_bExitHandlerInstalled)
        {
            g_bExitHandlerInstalled = true;
            atexit(&SECoP_S_initLibraryExit);
        }
    }
    else
        SECoP_S_initLibraryHelper();
    return SECoP_S_SUCCESS;
}

/**
 * \brief This function initializes the Qt to cope with library internal data
 *        types and creates the singleton, which stores all SEC-nodes.
 * \ingroup intfunc
 */
void SECoP_S_initLibraryHelper(void)
{
    qRegisterMetaType<bool*>();
    qRegisterMetaType<char*>();
    qRegisterMetaType<int*>();
    qRegisterMetaType<qulonglong*>();
    qRegisterMetaType<SECoP_S_action*>();
    qRegisterMetaType<SECoP_S_callFunction>();
    qRegisterMetaType<SECoP_S_Command*>();
    qRegisterMetaType<SECoP_dataPtr>();
    qRegisterMetaType<SECoP_dataPtr*>();
    qRegisterMetaType<SECoP_S_error>();
    qRegisterMetaType<SECoP_S_error*>();
    qRegisterMetaType<SECoP_S_getsetFunction>();
    qRegisterMetaType<SECoP_S_Module*>();
    qRegisterMetaType<SECoP_S_Node*>();
    qRegisterMetaType<SECoP_S_Parameter*>();

    g_pSECoPMain = new SECoP_S_Main;
}

/**
 * \brief This function is called by the global exit handler at lib-C exit.
 * \ingroup intfunc
 */
void SECoP_S_initLibraryExit(void)
{
    SECoP_S_initLibraryExitHelper(true);
}

/**
 * \brief This function is called for cleaning up. It tries to revert all,
 *        what has been created or changed since library initialization.
 * \ingroup intfunc
 * \param[in] bAtExit false: user called, true: lib-C called (atexit)
 */
void SECoP_S_initLibraryExitHelper(bool bAtExit)
{
    if (!bAtExit && g_bInitialized && g_bExitHandlerInstalled && g_pSECoPMain != nullptr)
    {
        g_pSECoPMain->cleanUp(true);
        return;
    }
    qInstallMessageHandler(g_pOldMessageHandler);
    g_pOldMessageHandler = nullptr;
    if (g_pSECoPMain != nullptr)
        g_pSECoPMain->cleanUp(false);
    else if (g_pApplication != nullptr)
    {
        g_pApplication->quit();
        while (g_bInitialized)
            std::this_thread::yield();
    }
    else
    {
        qint64 qiStart(QDateTime::currentMSecsSinceEpoch());
        while (g_pSECoPMain->getInstance() != nullptr)
        {
            QApplication::processEvents(QEventLoop::AllEvents, 1000);
            if ((QDateTime::currentMSecsSinceEpoch() - qiStart) >= 5000)
                break;
        }
        if (g_pSECoPMain->getInstance() != nullptr)
            delete g_pSECoPMain;
    }
    if (g_pSECoPMain != nullptr)
        g_pSECoPMain = nullptr;
    if (g_pThread != nullptr)
    {
        g_pThread->join();
        delete g_pThread;
        g_pThread = nullptr;
    }
    if (g_szArgv0 != nullptr)
    {
        free(g_szArgv0);
        g_szArgv0 = nullptr;
        g_aszArgv[0] = nullptr;
        g_aszArgv[1] = nullptr;
    }
}

/**
 * \brief This function is started, if a local QApplication instance is needed
 *        and runs inside its own thread. It creates a QApplication instance,
 *        starts the event loop and wait for its exit. After that, the function
 *        and the thread exit.
 * \ingroup intfunc
 */
static void SECoP_S_initLibraryThread(void)
{
    g_iArgc = 1;
    g_aszArgv[0] = g_szArgv0 = strdup("dummy");
    g_aszArgv[1] = nullptr;
    QApplication app(g_iArgc, g_aszArgv);
    g_pApplication = &app;
    SECoP_S_initLibraryHelper();
    g_bInitialized = true;
    g_pApplication->exec();
    g_bInitialized = false;
    g_pApplication = nullptr;
}

/**
 * \brief This function is called for cleaning up. It cleans up all SEC-nodes
 *        with its modules, parameters, commands, properties and client
 *        connections. If called with bNodeOnly==false, it tries to revert all,
 *        what has been created or changed since library initialization.
 * \ingroup expfunc
 * \param[in] bNodeOnly false: clean up all, true: clean up nodes only
 */
void SHALL_EXPORT SECoP_S_doneLibrary(int bNodeOnly)
{
    SECoP_S_showStatusWindow(0);
    SECoP_S_initLibraryExitHelper(!bNodeOnly);
}

/*
 * \brief SECoP_S_setManyThreads configures thread creation inside the library
 *        0:      minimize thread creation (nevertheless this library is not single threaded but eases debugging)
 *        other:  create a thread for every node, module, client connection (default)
 * \ingroup expfunc
 * \param[in] bManyThreads 0=false: minimize thread creation, other: thread for node/module/connection
 */
extern "C" void SHALL_EXPORT SECoP_S_setManyThreads(int bManyThreads)
{
    SECoP_S_Main::manyThreads(bManyThreads != 0);
}

/**
 * \brief convenience operator for debugging purposes
 * \ingroup intfunc
 * \param[in] dbg debug stream for output
 * \param[in] e   SECoP_S_error to print
 * \return a copy of the debug stream
 */
QDebug operator<<(QDebug dbg, enum SECoP_S_error e)
{
    dbg << SECoP_S_Main::getErrorString(e);
    return dbg;
}

/**
 * \brief convenience operator for debugging purposes
 * \ingroup intfunc
 * \param[in] dbg debug stream for output
 * \param[in] a   SECoP_S_action to print
 * \return a copy of the debug stream
 */
QDebug operator<<(QDebug dbg, enum SECoP_S_action a)
{
    switch (a)
    {
        case SECoP_S_ACTION_NONE:   dbg << "none";     break;
        case SECoP_S_ACTION_READ:   dbg << "read";     break;
        case SECoP_S_ACTION_CHANGE: dbg << "change";   break;
        case SECoP_S_ACTION_DO:     dbg << "do";       break;
        default:                    dbg << "?action?"; break;
    }
    return dbg;
}

/**
 * \brief convenience operator for debugging purposes
 * \ingroup intfunc
 * \param[in] dbg debug stream for output
 * \param[in] t   SECoP_V_type to print
 * \return a copy of the debug stream
 */
QDebug operator<<(QDebug dbg, enum SECoP_V_type t)
{
    switch (t)
    {
        case SECoP_VT_NONE:          dbg << "null";             break;
        case SECoP_VT_DOUBLE:        dbg << "double";           break;
        case SECoP_VT_INTEGER:       dbg << "integer";          break;
        case SECoP_VT_BOOL:          dbg << "boolean";          break;
        case SECoP_VT_ENUM:          dbg << "enumeration";      break;
        case SECoP_VT_SCALED:        dbg << "scaled";           break;
        case SECoP_VT_ARRAY:         dbg << "array";            break;
        case SECoP_VT_ARRAY_DOUBLE:  dbg << "arraydouble";      break;
        case SECoP_VT_ARRAY_INTEGER: dbg << "arrayinteger";     break;
        case SECoP_VT_ARRAY_BOOL:    dbg << "arrayboolean";     break;
        case SECoP_VT_ARRAY_ENUM:    dbg << "arrayenumeration"; break;
        case SECoP_VT_ARRAY_SCALED:  dbg << "arrayscaled";      break;
        case SECoP_VT_STRING:        dbg << "string";           break;
        case SECoP_VT_BLOB:          dbg << "blob";             break;
        case SECoP_VT_JSON:          dbg << "json";             break;
        case SECoP_VT_STRUCT:        dbg << "structure";        break;
        case SECoP_VT_TUPLE:         dbg << "tuple";            break;
        default: dbg << qUtf8Printable(QString("?%1?").arg(static_cast<int>(t))); break;
    }
    return dbg;
}

/**
 * \brief convenience operator for debugging purposes
 * \ingroup intfunc
 * \param[in] dbg debug stream for output
 * \param[in] d   SECoP_dataPtr to print
 * \return a copy of the debug stream
 */
QDebug operator<<(QDebug dbg, SECoP_dataPtr d)
{
    dbg << d->exportSECoP();
    return dbg;
}

/**
 * \brief This function is called for every Qt message function like qFatal,
 *        qError, qWarning ... It will print the message to the status window
 *        and then call the previous message handler or print the message to
 *        standard error pipe. The prototype of this function is defined in Qt.
 * \ingroup intfunc
 * \param[in] iType     message type/level
 * \param[in] context   where were the message generated
 * \param[in] szMessage the message text
 */
void SECoP_S_MessageHandler(QtMsgType iType, const QMessageLogContext &context, const QString &szMessage)
{
    QString szMsgType("unknown");
    switch (iType)
    {
        case QtDebugMsg:    szMsgType="debug";    break;
        case QtWarningMsg:  szMsgType="warning";  break;
        case QtCriticalMsg: szMsgType="critical"; break;
        case QtFatalMsg:    szMsgType="fatal";    break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        case QtInfoMsg:     szMsgType="info";     break;
#endif
    }

    bool bMultiLine(false);
    QString szOutput(szMessage);
    szOutput.replace(QString("\r\n"), QString("\n"));
    szOutput.replace(QChar('\r'), QChar('\n'));
    while (szOutput.endsWith(QChar('\n')))
        szOutput.chop(1);
    for (int iStartIndex = 0; ;)
    {
        int iIndex(szOutput.indexOf(QChar('\n'), iStartIndex));
        if (iIndex < 0)
            break;
        szOutput.insert(++iIndex, QString("  "));
        iStartIndex = iIndex + 2;
        if (!bMultiLine)
        {
            bMultiLine = true;
            szOutput.insert(0, QString("\n  "));
        }
    }

    if (!bMultiLine)
        szOutput.prepend(QChar(' '));
    szOutput.prepend(QString("%1(%2)/%3:").arg(context.file).arg(context.line).arg(szMsgType));
    if (g_pOldMessageHandler == nullptr) // if we never had a previous message logger or Windows
        std::cerr << qUtf8Printable(QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss.zzz "))
                  << qUtf8Printable(szOutput) << std::endl;
    SECoP_S_Main::log(nullptr, szOutput, false);
    if (g_pOldMessageHandler != nullptr) // call previous message logger
        (*g_pOldMessageHandler)(iType, context, szMessage);
}

/**
 * \brief the singleton instance of SECoP_S_Main, \ref g_pSECoPMain
 */
SECoP_S_Main* SECoP_S_Main::m_pInstance = nullptr;

/**
 * \brief The constructor of SECoP_S_Main creates an invisible status window and
 *        initializes the singleton.
 */
SECoP_S_Main::SECoP_S_Main()
    : QObject()
    , m_pGui(nullptr)
    , m_pLastNode(nullptr)
    , m_bManyThreads(true)
{
    m_pInstance = this;
    m_pGui = new SECoP_S_StatusGui(g_bShowGUI);
    m_pMutex = new QMutex(QMutex::Recursive);
    m_pSessionCleanUpTimer = new QTimer;
    connect(m_pSessionCleanUpTimer, SIGNAL(timeout()), this, SLOT(sessionCleanUpTimer()), Qt::QueuedConnection);
    m_pSessionCleanUpTimer->start(1000);
}

/**
 * \brief The destructor deletes all SEC-nodes with modules, parameters,
 *        commands, properties and client connections. It destroys the status
 *        window and triggers the exit of the local QApplication instance,
 *        which stops the associated thread too.
 */
SECoP_S_Main::~SECoP_S_Main()
{
    delete m_pSessionCleanUpTimer;
    QThread* pMySelfThread(QThread::currentThread());
    for (auto it = m_apNodes.begin(); it != m_apNodes.end(); ++it)
    {
        SECoP_S_Node* pNode(*it);
        if (pNode == m_pLastNode)
            m_pLastNode = nullptr;
        if (pNode != nullptr)
        {
            QThread* pNodeThread(pNode->thread());
            if (pNodeThread != pMySelfThread)
            {
                pNode->disconnect(SIGNAL(destroyed()), pNodeThread, SLOT(quit()));
                pNode->moveToThread(pMySelfThread);
                pNodeThread->quit();
            }
            delete pNode;
        }
    }
    if (m_pLastNode != nullptr)
        delete m_pLastNode;
    if (m_pGui != nullptr)
    {
        m_pGui->allowClose();
        delete m_pGui;
    }
    m_pInstance = nullptr;
    if (m_pMutex != nullptr)
    {
        m_pMutex->lock();
        QMutex* pMutex(m_pMutex);
        m_pMutex = nullptr;
        pMutex->unlock();
        delete pMutex;
    }
    if (g_pApplication != nullptr)
        QTimer::singleShot(0, g_pApplication, SLOT(quit()));
}

/**
 * \brief This function transfers a reference to a new SECoP node to the
 *        status window.
 * \param[in] pNode new SECoP node instance
 */
void SECoP_S_Main::logAddNode(SECoP_S_Node* pNode)
{
    if (g_pSECoPMain != nullptr && g_pSECoPMain->m_pGui != nullptr)
        g_pSECoPMain->m_pGui->addNode(pNode);
}

/**
 * \brief This function informs the status window about the deletion of an
 *        existing SECoP node.
 * \param[in] pNode removed SECoP node instance
 */
void SECoP_S_Main::logRemoveNode(SECoP_S_Node* pNode)
{
    if (g_pSECoPMain != nullptr && g_pSECoPMain->m_pGui != nullptr)
        g_pSECoPMain->m_pGui->removeNode(pNode);
}

/**
 * \brief This function transfers a reference to a new SECoP client connection
 *        to the status window.
 * \param[in] pNode   SECoP node, which has got a new client
 * \param[in] pClient new client instance
 */
void SECoP_S_Main::logAddConnection(SECoP_S_Node* pNode, QTcpSocket* pClient)
{
    if (g_pSECoPMain != nullptr && g_pSECoPMain->m_pGui != nullptr)
        g_pSECoPMain->m_pGui->addConnection(pNode, pClient);
}

/**
 * \brief This function informs the status window about the deletion of an
 *        existing SECoP client connection.
 * \param[in] pClient client instance
 */
void SECoP_S_Main::logRemoveConnection(QTcpSocket* pClient)
{
    if (g_pSECoPMain != nullptr && g_pSECoPMain->m_pGui != nullptr)
        g_pSECoPMain->m_pGui->removeConnection(pClient);
}

/**
 * \brief generic logging function to put some information to the status window
 * \param[in] pNode     SECoP node, which should get the logging text or nullptr
 * \param[in] szData    the text to put to status window
 * \param[in] bNodeOnly false: put text to normal log and node,
 *                      true: put text to node tab only
 */
void SECoP_S_Main::log(SECoP_S_Node* pNode, QString szData, bool bNodeOnly)
{
    if (g_pSECoPMain != nullptr && g_pSECoPMain->m_pGui != nullptr)
        g_pSECoPMain->m_pGui->log(pNode, szData, bNodeOnly);
}

/**
 * \brief This function calls the implementation of cleaning up, which depends
 *        on the calling thread and argument.
 * \param[in] bNodeOnly false: try to clean up everything, true: delete SEC-nodes only
 */
void SECoP_S_Main::cleanUp(bool bNodeOnly)
{
    if (g_pApplication != nullptr)
    {
        if (QThread::currentThread() == thread())
            cleanUpSlot(bNodeOnly);
        else if (!bNodeOnly)
            QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
        else
            QMetaObject::invokeMethod(this, "cleanUpSlot", Qt::BlockingQueuedConnection, Q_ARG(bool, bNodeOnly));
    }
    else
        delete g_pSECoPMain;
}

/**
 * \brief This function calls cleans up the SEC-nodes only or triggers later
 *        self deletion, depending on argument.
 * \param[in] bNodeOnly false: trigger later deletion, true: delete SEC-nodes only
 */
void SECoP_S_Main::cleanUpSlot(bool bNodeOnly)
{
    if (bNodeOnly && g_bInitialized)
    {
        for (int iPos = 0; iPos < m_apNodes.size(); ++iPos)
        {
            SECoP_S_Node* pNode(m_apNodes.takeAt(iPos));
            if (pNode != nullptr)
            {
                QMutexLocker locker(m_pMutex);
                for (int i = 0; i < m_aStoredCommands.size(); ++i)
                {
                    ActionEntry& entry = m_aStoredCommands[i];
                    if (entry.m_pNode == pNode)
                        m_aStoredCommands.removeAt(i--);
                }
                for (int i = 0; i < m_aExecutedCommands.size(); ++i)
                {
                    ActionEntry& entry = m_aExecutedCommands[i];
                    if (entry.m_pNode == pNode)
                        m_aExecutedCommands.removeAt(i--);
                }
                if (pNode == m_pLastNode)
                    m_pLastNode = nullptr;
                QThread* pNodeThread(pNode->thread());
                QThread* pMySelfThread(QThread::currentThread());
                if (pNodeThread != pMySelfThread)
                {
                    pNode->disconnect(SIGNAL(destroyed()), pNodeThread, SLOT(quit()));
                    pNode->moveToThread(pMySelfThread);
                    pNodeThread->quit();
                }
                delete pNode;
            }
        }
        if (m_pGui != nullptr)
        {
            if (m_pGui->isVisible())
                m_pGui->hide();
            m_pGui->clearLog();
        }
    }
    else
        deleteLater();
}

/**
 * \brief This function translates a SECoP error or warning into a human
 *        readable text.
 * \param[in] iError SECoP_S_error to translate
 * \return the description
 */
const char* SECoP_S_Main::getErrorString(enum SECoP_S_error iError)
{
    const char* szResult = nullptr;
    switch (iError)
    {
#define SWITCHCASE(x, y) case x: szResult = #x ": " y; break
        SWITCHCASE(SECoP_S_SUCCESS, "success");
        SWITCHCASE(SECoP_S_WARNING_NO_DESCRIPTION,   "no description was given");
        SWITCHCASE(SECoP_S_WARNING_CUSTOM_PROPERTY,  "custom name of property should be prefixed with an underscore");
        SWITCHCASE(SECoP_S_WARNING_BUFFER_TOO_SMALL, "input buffer is too small, some information is missing (call again with greater buffer)");
        SWITCHCASE(SECoP_S_WARNING_MISS_PROPERTIES,  "missing optional properties");
        SWITCHCASE(SECoP_S_ERROR_UNKNOWN_COMMAND,    "invalid SECoP command");
        SWITCHCASE(SECoP_S_ERROR_INVALID_NAME,       "invalid SECoP name");
        SWITCHCASE(SECoP_S_ERROR_INVALID_NODE,       "invalid node reference");
        SWITCHCASE(SECoP_S_ERROR_INVALID_MODULE,     "invalid module reference");
        SWITCHCASE(SECoP_S_ERROR_INVALID_PARAMETER,  "invalid parameter reference");
        SWITCHCASE(SECoP_S_ERROR_INVALID_PROPERTY,   "invalid property reference");
        SWITCHCASE(SECoP_S_ERROR_INVALID_COMMAND,    "invalid command (reference)");
        SWITCHCASE(SECoP_S_ERROR_NOT_IMPLEMENTED,    "function call is not implemented");
        SWITCHCASE(SECoP_S_ERROR_READONLY,           "element is read only");
        SWITCHCASE(SECoP_S_ERROR_NO_DATA,            "function call cannot return any data");
        SWITCHCASE(SECoP_S_ERROR_NO_MEMORY,          "memory allocation problem");
        SWITCHCASE(SECoP_S_ERROR_NOT_INITIALIZED,    "call SECoP_S_initLibrary() first");
        SWITCHCASE(SECoP_S_ERROR_INVALID_VALUE,      "datainfo or value was invalid");
        SWITCHCASE(SECoP_S_ERROR_MISSING_MANDATORY,  "an expected property was empty");
        SWITCHCASE(SECoP_S_ERROR_NO_SETTER_GETTER,   "missing a pointer to getter and setter function");
        SWITCHCASE(SECoP_S_ERROR_NO_SETTER,          "missing a pointer to setter function");
        SWITCHCASE(SECoP_S_ERROR_NO_GETTER,          "missing a pointer to getter function");
        SWITCHCASE(SECoP_S_ERROR_NAME_ALREADY_USED,  "the name is used before");
        SWITCHCASE(SECoP_S_ERROR_TIMEOUT,            "parameter or command timeout");
        SWITCHCASE(SECoP_S_ERROR_WRONG_BITNESS,      "unsupported platform");
        SWITCHCASE(SECoP_S_ERROR_COMMAND_FAILED,     "SECoP error \"CommandFailed\"");
        SWITCHCASE(SECoP_S_ERROR_COMMAND_RUNNING,    "SECoP error \"CommandRunning\"");
        SWITCHCASE(SECoP_S_ERROR_COMM_FAILED,        "SECoP error \"CommunicationFailed\"");
        SWITCHCASE(SECoP_S_ERROR_IS_BUSY,            "SECoP error \"IsBusy\"");
        SWITCHCASE(SECoP_S_ERROR_IS_ERROR,           "SECoP error \"IsError\"");
        SWITCHCASE(SECoP_S_ERROR_DISABLED,           "SECoP error \"Disabled\"");
        SWITCHCASE(SECoP_S_ERROR_SYNTAX,             "SECoP error \"SyntaxError\"");
        SWITCHCASE(SECoP_S_ERROR_INTERNAL,           "SECoP error \"InternalError\"");
#undef SWITCHCASE
        default:
            if (iError >= 0)
                szResult = "INVALID: this is an unknown SECoP warning";
            else
                szResult = "INVALID: this is an unknown SECoP error";
            break;
    }
    return szResult;
}

/// \returns the current time as fractional unix time
double SECoP_S_Main::getCurrentTime()
{
    return static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;
}

/// \return the singeton instance
SECoP_S_Main* SECoP_S_Main::getInstance()
{
    return m_pInstance;
}

/// \return the Subversion revision as string
QByteArray SECoP_S_Main::getGitVersion()
{
    if (m_pInstance->m_szGitVersion.isNull())
    {
        QByteArray szGitVersion;
#ifdef GITVERSION
        // try using global Git version
        szGitVersion = GITVERSION;
#endif
        if (szGitVersion.isEmpty())
            szGitVersion = "unknown"; // use warning
        m_pInstance->m_szGitVersion = szGitVersion.simplified().trimmed();
    }
    return m_pInstance->m_szGitVersion;
}

/// \return if the library was initialized using function pointers
bool SECoP_S_Main::hasFunctionPointers()
{
    return g_bHasFunctionPointers;
}

/*
 * \brief Call SECoP_S_createNode to create an empty SECoP node. You need at
 *        least one for a working SEC-node. Every node exists independent of
 *        each other, but it is up to the user, how they act.
 * \ingroup expfunc
 * \param[in] szID   unique name of the SECoP node
 * \param[in] szDesc a mandatory description text
 * \param[in] wPort  TCP port to listen to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_createNode(const char* szID, const char* szDesc, unsigned short wPort)
{
    local_qInfo("szID=%p %s\nszDesc=%p %s", static_cast<const void*>(szID), szID,
                static_cast<const void*>(szDesc), szDesc);
    return SECoP_S_Main::createNode(szID, szDesc, QString(), wPort);
}

/*
 * \brief Call SECoP_S_createNode2 to create an empty SECoP node and bind it to
 *        a specific interface and port. You need at least one for a working
 *        SEC-node. Every node exists independent of each other, but it is up
 *        to the user, how they act. If you disable function pointers, you
 *        should provide nullptr pointers to functions (\ref SECoP_S_addCommand,
 *        \ref SECoP_S_addReadableParameter, \ref SECoP_S_addWritableParameter).
 * \ingroup expfunc
 * \param[in] szID        unique name of the SECoP node
 * \param[in] szDesc      a mandatory description text
 * \param[in] szInterface the TCP interface to listen to
 * \param[in] wPort       TCP port to listen to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_createNode2(const char* szID, const char* szDesc,
                                                                   const char* szInterface, unsigned short wPort)
{
    local_qInfo("szID=%p %s\nszDesc=%p %s", static_cast<const void*>(szID), szID,
                static_cast<const void*>(szDesc), szDesc);
    return SECoP_S_Main::createNode(szID, szDesc, szInterface, wPort);
}

/**
 * \brief This function calls the implementation of createNode.
 * \param[in] szID        unique name of the SECoP node
 * \param[in] szDesc      a mandatory description text
 * \param[in] szInterface TCP interface to listen to
 * \param[in] wPort       TCP port to listen to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::createNode(QString szID, QString szDesc, QString szInterface, quint16 wPort)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->createNode(szID, szDesc, szInterface, wPort, &iResult);
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "createNode", Qt::BlockingQueuedConnection, Q_ARG(QString, szID),
                                  Q_ARG(QString, szDesc), Q_ARG(QString, szInterface), Q_ARG(quint16, wPort),
                                  Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates an empty SECoP node. You need at least one for
 *        a working SEC-node. Every node exists independent of each other, but
 *        it is up to the user, how they act.
 * \param[in]  szID        unique name of the SECoP node
 * \param[in]  szDesc      a mandatory description text
 * \param[in]  szInterface TCP interface to listen to
 * \param[in]  wPort       TCP port to listen to
 * \param[out] piResult    on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Main::createNode(QString szID, QString szDesc, QString szInterface, quint16 wPort, SECoP_S_error* piResult)
{
    if (szID.isEmpty() || !isValidName(szID))
        *piResult = SECoP_S_ERROR_MISSING_MANDATORY; // SECoPError no ID but ID is mandatory
    else
    {
        QHostAddress addr;
        if (szDesc.isEmpty())
            *piResult = SECoP_S_WARNING_NO_DESCRIPTION; // SECoP Warning no Description because description is recommended
        if (szInterface.isEmpty())
            addr = QHostAddress::Any;
        else
        {
            addr = QHostAddress(szInterface);
            if (addr.isNull())
                addr = QHostAddress::Any;
        }

        m_pLastNode = new SECoP_S_Node(szID, szDesc, addr, wPort);
        if (m_pLastNode->isValid())
        {
            if (m_bManyThreads)
            {
                QThread* pNodeThread = new QThread;
                m_pLastNode->moveToThread(pNodeThread);
                pNodeThread->connect(m_pLastNode, SIGNAL(destroyed()), SLOT(quit()), Qt::DirectConnection);
                pNodeThread->connect(pNodeThread, SIGNAL(finished()), SLOT(deleteLater()));
                pNodeThread->start();
            }

            if (*piResult != 0)
            {
                QString szData("node " + szID + " " + getErrorString(*piResult));
                if (*piResult >= 0)
                    m_aszWarningList.append(szData);
                else
                    m_aszErrorList.append(szData);
            }
            m_apNodes.append(m_pLastNode);
        }
        else
            *piResult = SECoP_S_ERROR_INVALID_NODE;
    }
}

/*
 * \brief Delete an existing SECoP node with SECoP_S_deleteNode.
 * \ingroup expfunc
 * \param[in] szID name of the SECoP node to delete
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_deleteNode(const char* szID)
{
    return SECoP_S_Main::deleteNode(szID);
}

/**
 * \brief This function calls the implementation of deleteNode.
 * \param[in] szID name of the SECoP node to delete
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::deleteNode(QString szID)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->deleteNode(szID, &iResult);
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "deleteNode", Qt::BlockingQueuedConnection,
                                  Q_ARG(QString, szID),Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief Delete an existing SECoP node with SECoP_S_deleteNode.
 * \param[in]  szID     name of the SECoP node to delete
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Main::deleteNode(QString szID, SECoP_S_error* piResult)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    int iPos(-1);
    SECoP_S_Node* pNode(nullptr);
    if (!szID.isEmpty())
        iPos = nodePosition(szID);
    if (iPos >= 0)
    {
        QMutexLocker locker(m_pMutex);
        pNode = m_apNodes.takeAt(iPos);
        for (int i = 0; i < m_aStoredCommands.size(); ++i)
        {
            ActionEntry& entry = m_aStoredCommands[i];
            if (entry.m_pNode == pNode)
                m_aStoredCommands.removeAt(i--);
        }
        for (int i = 0; i < m_aExecutedCommands.size(); ++i)
        {
            ActionEntry& entry = m_aExecutedCommands[i];
            if (entry.m_pNode == pNode)
                m_aExecutedCommands.removeAt(i--);
        }
        if (pNode == m_pLastNode)
            m_pLastNode = nullptr;
    }
    else
        iResult = SECoP_S_ERROR_INVALID_NODE;
    if (pNode != nullptr)
    {
        QThread* pNodeThread(pNode->thread());
        QThread* pMySelfThread(QThread::currentThread());
        if (pNodeThread != pMySelfThread)
        {
            pNode->disconnect(SIGNAL(destroyed()), pNodeThread, SLOT(quit()));
            pNode->moveToThread(pMySelfThread);
            pNodeThread->quit();
        }
        delete pNode;
    }

    if (piResult != nullptr)
        *piResult = iResult;
}

/**
 * \brief This function searches for the index into node list for a node name.
 * \param[in] szNode name to search for
 * \return negative: SECoP_S_error, else index into list
 */
int SECoP_S_Main::nodePosition(QString szNode) const
{
    for (int i = 0; i < m_apNodes.size(); ++i)
    {
        SECoP_S_Node* pNode(m_apNodes[i]);
        if (pNode != nullptr && szNode.compare(pNode->getNodeID(), Qt::CaseInsensitive) == 0)
            return i;
    }
    return SECoP_S_ERROR_INVALID_NODE;;
}

/*
 * \brief Call SECoP_S_addProperty to append a SECoP property to the last SECoP
 *        node, module, command or parameter. You need this function or some of
 *        the convenience functions SECoP_S_addProperty... , because there are
 *        mandatory and some optional properties for everything.
 * \ingroup expfunc
 * \param[in] szKey  name of the property, which is unique inside its context.
 *                   You may prepend an unterscore for own properties
 * \param[in] pValue value of the property; note: use the recommended data type
 *                   for the standard SECoP properties
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addProperty(const char* szKey, const CSECoPbaseType* pValue)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    return g_pSECoPMain->addProperty(szKey, SECoP_dataPtr(pValue->duplicate()));
}

/*
 * \brief Convenience function for \ref SECoP_S_addProperty with a boolean value.
 * \ingroup expfunc
 * \param[in] szKey  name of the property, which is unique inside its context.
 * \param[in] bValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyBoolean(const char* szKey, long long bValue)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    return g_pSECoPMain->addProperty(szKey, CSECoPbaseType::simpleBool(bValue != 0));
}

/*
 * \brief Convenience function for \ref SECoP_S_addProperty with an integer value.
 * \ingroup expfunc
 * \param[in] szKey   name of the property, which is unique inside its context.
 * \param[in] llValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyInteger(const char* szKey, long long llValue)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    return g_pSECoPMain->addProperty(szKey, CSECoPbaseType::simpleInteger(llValue));
}

/*
 * \brief Convenience function for \ref SECoP_S_addProperty with a floating point value.
 * \ingroup expfunc
 * \param[in] szKey    name of the property, which is unique inside its context.
 * \param[in] dblValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyDouble(const char* szKey, double dblValue)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    return g_pSECoPMain->addProperty(szKey, CSECoPbaseType::simpleDouble(dblValue));
}

/*
 * \brief Convenience function for \ref SECoP_S_addProperty with a string value.
 * \ingroup expfunc
 * \param[in] szKey   name of the property, which is unique inside its context.
 * \param[in] szValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyString(const char* szKey, const char* szValue)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    return g_pSECoPMain->addProperty(szKey, CSECoPbaseType::simpleString(szValue));
}

/*
 * \brief Convenience function for \ref SECoP_S_addProperty with a JSON value.
 * \ingroup expfunc
 * \param[in] szKey   name of the property, which is unique inside its context.
 * \param[in] szValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyJSON(const char* szKey, const char* szValue)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    SECoP_dataPtr pValue(CSECoPbaseType::simpleJSON(szValue));
    CSECoPstring* pJson(dynamic_cast<CSECoPstring*>(pValue.get()));
    if (pJson == nullptr || !pJson->isValid())
        return SECoP_S_ERROR_INVALID_VALUE;
    return g_pSECoPMain->addProperty(szKey, pValue);
}

/**
 * \brief This function calls the implementation of addProperty inside the last
 *        created node/module/command/parameter.
 * \param[in] szKey  name of the property, which is unique inside its context.
 * \param[in] pValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::addProperty(QString szKey, const SECoP_dataPtr pValue)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->m_pLastNode == nullptr)
        return SECoP_S_ERROR_INVALID_NODE;
    enum SECoP_S_error iResult(g_pSECoPMain->m_pLastNode->addProperty(szKey, pValue));
    if (iResult != 0)
    {
        QString szData(QString("%1 property \"%2\": %3").arg(g_pSECoPMain->m_pLastNode->getPrintableActive(true)).arg(szKey).arg(getErrorString(iResult)));
        QMutexLocker locker(g_pSECoPMain->m_pMutex);
        if (iResult >= 0)
            g_pSECoPMain->m_aszWarningList.append(szData);
        else
            g_pSECoPMain->m_aszErrorList.append(szData);
    }
    return iResult;
}

/*
 * \brief Call SECoP_S_addModule to create a SECoP module inside the last created SECoP node.
 * \ingroup expfunc
 * \param[in] szName name of the module, which is unique inside the SECoP node
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addModule(const char* szName)
{
    return SECoP_S_Main::addModule(szName);
}

/**
 * \brief This function calls the implementation of addModule inside the last
 *        created node.
 * \param[in] szName name of the module, which is unique inside the SECoP node
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::addModule(QString szName)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->m_pLastNode == nullptr)
        return SECoP_S_ERROR_INVALID_NODE;
    enum SECoP_S_error iResult(g_pSECoPMain->m_pLastNode->addModule(szName));
    if (iResult != 0)
    {
        QString szData(QString("node \"%1\" module \"%2\": %3").arg(g_pSECoPMain->m_pLastNode->getNodeID()).arg(szName).arg(getErrorString(iResult)));
        QMutexLocker locker(g_pSECoPMain->m_pMutex);
        if (iResult >= 0)
            g_pSECoPMain->m_aszWarningList.append(szData);
        else
            g_pSECoPMain->m_aszErrorList.append(szData);
    }
    return iResult;
}

/*
 * \brief Call SECoP_S_addCommand to create a SECoP command inside the last created SECoP module.
 * \ingroup expfunc
 * \param[in] szKey     name of the command, which is unique inside the SECoP module
 * \param[in] ptrToFunc function, which is called when a client invokes the command
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addCommand(const char* szKey, SECoP_S_callFunction ptrToFunc)
{
    return SECoP_S_Main::addCommand(szKey, ptrToFunc);
}

/**
 * \brief This function calls the implementation of addCommand inside the last
 *        created module.
 * \param[in] szKey     name of the command, which is unique inside the SECoP module
 * \param[in] ptrToFunc function, which is called when a client invokes the command
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::addCommand(QString szKey, SECoP_S_callFunction ptrToFunc)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->m_pLastNode == nullptr)
        return SECoP_S_ERROR_INVALID_NODE;
    enum SECoP_S_error iResult(g_pSECoPMain->m_pLastNode->addCommand(szKey, ptrToFunc));
    if (iResult != 0)
    {
        QString szData(QString("%1 command \"%2\": %3").arg(g_pSECoPMain->m_pLastNode->getPrintableActive(false)).arg(szKey).arg(getErrorString(iResult)));
        QMutexLocker locker(g_pSECoPMain->m_pMutex);
        if (iResult >= 0)
            g_pSECoPMain->m_aszWarningList.append(szData);
        else
            g_pSECoPMain->m_aszErrorList.append(szData);
    }
    return iResult;
}

/*
 * \brief Call SECoP_S_addReadableParameter to create a read only SECoP parameter
 *        inside the last created SECoP module.
 * \ingroup expfunc
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addReadableParameter(const char* szName, SECoP_S_getsetFunction ptrToGet)
{
    return SECoP_S_Main::addReadableParameter(szName, ptrToGet);
}

/**
 * \brief This function calls the implementation of addReadableParameter inside
 *        the last created module.
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::addReadableParameter(QString szName, SECoP_S_getsetFunction ptrToGet)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->m_pLastNode == nullptr)
        return SECoP_S_ERROR_INVALID_NODE;
    enum SECoP_S_error iResult(g_pSECoPMain->m_pLastNode->addReadableParameter(szName, ptrToGet));
    if (iResult != 0)
    {
        QString szData(QString("%1 parameter \"%2\": %3").arg(g_pSECoPMain->m_pLastNode->getPrintableActive(false)).arg(szName).arg(getErrorString(iResult)));
        QMutexLocker locker(g_pSECoPMain->m_pMutex);
        if (iResult >= 0)
            g_pSECoPMain->m_aszWarningList.append(szData);
        else
            g_pSECoPMain->m_aszErrorList.append(szData);
    }
    return iResult;
}

/*
 * \brief Call SECoP_S_addWritableParameter to create a read- and writable SECoP
 *        parameter inside the last created SECoP module.
 * \ingroup expfunc
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[in] ptrToSet function, which is called when a client wants to set a new value
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_addWritableParameter(const char* szName, SECoP_S_getsetFunction ptrToGet,
                                                                            SECoP_S_getsetFunction ptrToSet)
{
    return SECoP_S_Main::addWritableParameter(szName, ptrToGet, ptrToSet);
}

/**
 * \brief This function calls the implementation of addWritableParameter inside
 *        the last created module.
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[in] ptrToSet function, which is called when a client wants to set a new value
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::addWritableParameter(QString szName, SECoP_S_getsetFunction ptrToGet,
                                                      SECoP_S_getsetFunction ptrToSet)
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->m_pLastNode == nullptr)
        return SECoP_S_ERROR_INVALID_NODE;
    enum SECoP_S_error iResult(g_pSECoPMain->m_pLastNode->addWritableParameter(szName, ptrToGet, ptrToSet));
    if (iResult != 0)
    {
        QString szData(QString("%1 parameter \"%2\": %3").arg(g_pSECoPMain->m_pLastNode->getPrintableActive(false)).arg(szName).arg(getErrorString(iResult)));
        QMutexLocker locker(g_pSECoPMain->m_pMutex);
        if (iResult >= 0)
            g_pSECoPMain->m_aszWarningList.append(szData);
        else
            g_pSECoPMain->m_aszErrorList.append(szData);
    }
    return iResult;
}

/*
 * \brief Call SECoP_S_setAddFocus to change the focus while SECoP node creation.
 *        If your creation is not able to guarantee the order of SECoP_add...
 *        functions, you could use this function. Normally you should not need
 *        this. Use a colon (':') separated name for selecting the current item.
 *        Selectable items are nodes, modules, command and parameters.
 * \ingroup expfunc
 * \param[in] szKey name of the SECoP item to point to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_setAddFocus(const char* szKey)
{
    return SECoP_S_Main::setAddFocus(szKey);
}

/**
 * \brief This function calls the implementation of setAddFocus inside the last
 *        created node/module/command/parameter (\ref SECoP_S_setAddFocus).
 * \param[in] szKey name of the SECoP item to point to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::setAddFocus(QString szKey)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->setAddFocus(szKey, &iResult);
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "setAddFocus", Qt::BlockingQueuedConnection, Q_ARG(QString, szKey),
                                  Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief Call SECoP_S_setAddFocus to change the focus while SECoP node creation.
 *        If your creation is not able to guarantee the order of SECoP_add...
 *        functions, you could use this function. Normally you should not need
 *        this. Use a colon (':') separated name for selecting the current item.
 *        Selectable items are nodes, modules, command and parameters.
 * \param[in]  szKey    name of the SECoP item to point to
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Main::setAddFocus(QString szKey, SECoP_S_error* piResult)
{
    SECoP_S_Node* pNode(nullptr);
    int iPos, iIndex;

    *piResult = SECoP_S_ERROR_INVALID_NAME;
    if (szKey.isEmpty())
        return;

    *piResult = SECoP_S_ERROR_INVALID_NODE;
    iPos = szKey.indexOf(':');
    if (iPos < 0)
        return;
    iIndex = nodePosition(szKey.left(iPos));
    if (iIndex < 0)
        return;
    szKey.remove(0, iPos + 1);
    pNode = m_apNodes[iIndex];
    if (pNode == nullptr)
        return;

    *piResult = pNode->setAddFocus(szKey);
    if (*piResult >= 0)
        m_pLastNode = pNode;
}

/*
 * \brief Inform the SECoP node that it is complete. This function makes some
 *        checks and on success, it opens the TCP port and marks the node that
 *        you cannot change its structure.
 * \ingroup expfunc
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_nodeComplete()
{
    return SECoP_S_Main::nodeComplete();
}

/**
 * \brief This function calls the implementation of nodeComplete inside
 *        the last created node.
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::nodeComplete()
{
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->m_pLastNode == nullptr)
        return SECoP_S_ERROR_INVALID_NODE;
    return g_pSECoPMain->m_pLastNode->nodeComplete();
}

/*
 * \brief Call SECoP_S_updateParameter, if you have a new value for an existing
 *        parameter (read- or writable). All clients which subscribed to this
 *        module should be informed about the new value.
 * \ingroup expfunc
 * \param[in] szParameterName name of the parameter (<node>:<module>:<parameter>)
 * \param[in] pData           new value of the parameter
 * \param[in] pSigma          error of value of the parameter or nullptr
 * \param[in] dblTimestamp    SECoP timestamp of the value or NaN
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_updateParameter(const char* szParameterName, const CSECoPbaseType* pData,
                                                                       const CSECoPbaseType* pSigma, double dblTimestamp)
{
    return SECoP_S_Main::updateParameter(szParameterName, SECoP_dataPtr(pData->duplicate()),
                                         SECoP_dataPtr(pSigma->duplicate()), dblTimestamp);
}

/**
 * \brief This function calls the implementation of updateParameter inside
 *        SECoP_S_Main.
 * \param[in] szParameterName name of the parameter (<node>:<module>:<parameter>)
 * \param[in] pData           new value of the parameter
 * \param[in] pSigma          error of value of the parameter or nullptr
 * \param[in] dblTimestamp    SECoP timestamp of the value or NaN
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::updateParameter(QString szParameterName, const SECoP_dataPtr pData,
                                                 const SECoP_dataPtr pSigma, double dblTimestamp)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->updateParameter(szParameterName, pData, pSigma, dblTimestamp, &iResult);
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "updateParameter", Qt::BlockingQueuedConnection, Q_ARG(QString, szParameterName),
                                  Q_ARG(const SECoP_dataPtr, pData), Q_ARG(const SECoP_dataPtr, pSigma),
                                  Q_ARG(double, dblTimestamp), Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief Call SECoP_S_updateParameter, if you have a new value for an existing
 *        parameter (read- or writable). All clients which subscribed to this
 *        module should be informed about the new value.
 * \param[in] szParameterName name of the parameter (<node>:<module>:<parameter>)
 * \param[in] pData           new value of the parameter
 * \param[in] pSigma          error of value of the parameter or nullptr
 * \param[in] dblTimestamp    SECoP timestamp of the value or NaN
 * \param[out] piResult       on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Main::updateParameter(QString szParameterName, const SECoP_dataPtr pData, const SECoP_dataPtr pSigma,
                                   double dblTimestamp, SECoP_S_error* piResult)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    SECoP_S_Node* pNode(nullptr);
    SECoP_S_Module* pModule(nullptr);
    SECoP_S_Parameter* pParameter(nullptr);
    int iPos, iIndex;

    if (szParameterName.isEmpty())
    {
        iResult = SECoP_S_ERROR_INVALID_NAME;
        goto finish;
    }

    iResult = SECoP_S_ERROR_INVALID_NODE;
    iPos = szParameterName.indexOf(':');
    if (iPos < 0)
        goto finish;
    iIndex = nodePosition(szParameterName.left(iPos));
    if (iIndex < 0)
        goto finish;

    szParameterName.remove(0, iPos + 1);
    pNode = m_apNodes[iIndex];
    if (pNode == nullptr)
        goto finish;

    iResult = SECoP_S_ERROR_INVALID_MODULE;
    iPos = szParameterName.indexOf(':');
    if (iPos < 0)
        goto finish;
    pModule = pNode->getModule(pNode->modulePosition(szParameterName.left(iPos)));
    if (pModule == nullptr)
        goto finish;

    iResult = SECoP_S_ERROR_INVALID_PARAMETER;
    szParameterName.remove(0, iPos + 1);
    iPos = pModule->parameterPosition(szParameterName);
    if (iPos < 0)
        goto finish;

    pParameter = pModule->getParameter(iPos);
    if (pParameter->isConstant())
        iResult = SECoP_S_ERROR_INVALID_PARAMETER;
    else
    {
        pModule->updateParameter(iPos, pData, pSigma, dblTimestamp);
        iResult = SECoP_S_SUCCESS;
    }
finish:
    if (piResult != nullptr)
        *piResult = iResult;
}

/*
 * \brief Call SECoP_S_updateParameter, if you have a new value for an existing
 *        parameter (read- or writable). All clients which subscribed to this
 *        module should be informed about this.
 * \ingroup expfunc
 * \param[in] szParameterName name of the parameter (<node>:<module>:<parameter>)
 * \param[in] szValue         new JSON value of the parameter
 * \param[in] iValueSize      size of the JSON value
 * \param[in] szSigma         error of value of the parameter
 * \param[in] iSigmaSize      size of the JSON sigma value or 0 (no sigma value)
 * \param[in] dblTimestamp    SECoP timestamp of the value or NaN
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_updateParameter2(const char* szParameterName, const char* szValue,
                                                                        int iValueSize, const char* szSigma, int iSigmaSize,
                                                                        double dblTimestamp)
{
    return SECoP_S_Main::updateParameter2(szParameterName, QByteArray(szValue, iValueSize), QByteArray(szSigma, iSigmaSize), dblTimestamp);
}

/**
 * \brief This function calls the implementation of updateParameter2 inside
 *        SECoP_S_Main.
 * \param[in] szParameterName name of the parameter (<node>:<module>:<parameter>)
 * \param[in] szData          new JSON value of the parameter
 * \param[in] szSigma         error of value of the parameter
 * \param[in] dblTimestamp    SECoP timestamp of the value or NaN
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::updateParameter2(QString szParameterName, QByteArray szData, QByteArray szSigma,
                                                  double dblTimestamp)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->updateParameter2(szParameterName, szData, szSigma, dblTimestamp, &iResult);
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "updateParameter2", Qt::BlockingQueuedConnection, Q_ARG(QString, szParameterName),
                                  Q_ARG(QByteArray, szData), Q_ARG(QByteArray, szSigma), Q_ARG(double, dblTimestamp),
                                  Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief Call SECoP_S_updateParameter2, if you have a new value for an existing
 *        parameter (read- or writable). All clients which subscribed to this
 *        module should be informed about the new value.
 * \param[in]  szParameterName name of the parameter (<node>:<module>:<parameter>)
 * \param[in]  szData          new value of the parameter
 * \param[in]  szSigma         error of value of the parameter
 * \param[in]  dblTimestamp    SECoP timestamp of the value or NaN
 * \param[out] piResult        on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Main::updateParameter2(QString szParameterName, QByteArray szData, QByteArray szSigma,
                                    double dblTimestamp, SECoP_S_error* piResult)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    SECoP_S_Node* pNode(nullptr);
    SECoP_S_Module* pModule(nullptr);
    SECoP_S_Parameter* pParameter(nullptr);
    SECoP_dataPtr pValue;
    SECoP_dataPtr pSigma;
    int iPos, iIndex;

    if (szParameterName.isEmpty())
    {
        iResult = SECoP_S_ERROR_INVALID_NAME;
        goto finish;
    }

    iResult = SECoP_S_ERROR_INVALID_NODE;
    iPos = szParameterName.indexOf(':');
    if (iPos < 0)
        goto finish;
    iIndex = nodePosition(szParameterName.left(iPos));
    if (iIndex < 0)
        goto finish;

    szParameterName.remove(0, iPos + 1);
    pNode = m_apNodes[iIndex];
    if (pNode == nullptr)
        goto finish;

    iResult = SECoP_S_ERROR_INVALID_MODULE;
    iPos = szParameterName.indexOf(':');
    if (iPos < 0)
        goto finish;
    pModule = pNode->getModule(pNode->modulePosition(szParameterName.left(iPos)));
    if (pModule == nullptr)
        goto finish;

    iResult = SECoP_S_ERROR_INVALID_PARAMETER;
    szParameterName.remove(0, iPos + 1);
    iPos = pModule->parameterPosition(szParameterName);
    if (iPos < 0)
        goto finish;

    pParameter = pModule->getParameter(iPos);
    if (pParameter == nullptr || pParameter->isConstant())
        goto finish;

    iResult = SECoP_S_ERROR_INVALID_VALUE;
    pValue = pParameter->value();
    if (!pValue->importSECoP(szData.constData()))
        goto finish;

    if (!szSigma.isEmpty())
    {
        pSigma = SECoP_dataPtr(pValue->duplicate());
        if (!pSigma->importSECoP(szSigma.constData()))
            goto finish;
    }

    pModule->updateParameter(iPos, pValue, pSigma, dblTimestamp);
    iResult = SECoP_S_SUCCESS;
finish:
    if (piResult != nullptr)
        *piResult = iResult;
}

/**
 * \brief This function stores a todo action for the polling interface.
 * \param[in] qwRequestId module request id
 * \param[in] pTarget     target which will invoked for answer
 * \param[in] pNode       corresponding node
 * \param[in] pModule     corresponding module
 * \param[in] iAction     action, what to do
 * \param[in] pParameter  for "read" and "change" actions: parameter
 * \param[in] pCommand    for "do" action: command
 * \param[in] pValue      for "change": write value and "do": function call argument
 */
void SECoP_S_Main::storeCommand(quint64 qwRequestId, QObject* pTarget, SECoP_S_Node* pNode, SECoP_S_Module* pModule,
                                enum SECoP_S_action iAction, SECoP_S_Parameter* pParameter, SECoP_S_Command* pCommand,
                                const SECoP_dataPtr pValue)
{
    if (g_pSECoPMain != nullptr)
    {
        ActionEntry entry;
        entry.m_qwRequestId = qwRequestId;
        entry.m_pNode       = pNode;
        entry.m_pModule     = pModule;
        entry.m_pParameter  = pParameter;
        entry.m_pCommand    = pCommand;
        entry.m_pTarget     = pTarget;
        entry.m_iAction     = iAction;
        entry.m_pValue      = pValue;
        QMutexLocker locker(m_pMutex);
        if (iAction == SECoP_S_ACTION_READ)
        {
            // check queue for already inserted read requests
            QList<ActionEntry>* pList = &m_aStoredCommands;
            for (int i = 0; i < 2; ++i)
            {
                for (auto it = pList->constBegin(); it != pList->constEnd(); ++it)
                {
                    const ActionEntry &tmp(*it);
                    if (tmp.m_iAction == iAction && tmp.m_pNode == pNode &&
                        tmp.m_pModule == pModule && tmp.m_pParameter == pParameter)
                        return; // do not read unnecessary doubled values
                }
                pList = &m_aExecutedCommands;
            }
        }
        m_aStoredCommands.append(entry);
    }
}

/**
 * \brief On SECoP client disconnection, forget all actions depending on
 *        this target.
 * \param[in] pTarget SECoP client instance, which was disconnected
 */
void SECoP_S_Main::forgetStoredCommands(QObject* pTarget)
{
    if (g_pSECoPMain != nullptr)
    {
        QMutexLocker locker(g_pSECoPMain->m_pMutex);
        for (int i = 0; i < g_pSECoPMain->m_aStoredCommands.size(); ++i)
            if (g_pSECoPMain->m_aStoredCommands[i].m_pTarget == pTarget)
                g_pSECoPMain->m_aStoredCommands.removeAt(i--);
        for (int i = 0; i < g_pSECoPMain->m_aExecutedCommands.size(); ++i)
            if (g_pSECoPMain->m_aExecutedCommands[i].m_pTarget == pTarget)
                g_pSECoPMain->m_aExecutedCommands.removeAt(i--);
    }
}

/*
 * \brief When polling is enabled (\ref SECoP_S_createNode) call SECoP_S_getStoredCommand
 *        regulary to check, if there is something to do. If yes, it will return
 *        successfully with an id, what to do, a parameter/command name and new set
 *        value or argument. The function will never write more characters to the
 *        parameter/command name, which are provided inside piParameterSize.
 * \ingroup expfunc
 * \param[out]    pllId           id to use with \ref SECoP_S_putCommandAnswer
 * \param[out]    piAction        what is to do: read or change parameter, do command invokation
 * \param[out]    szParameter     buffer which gets the parameter/command name
 * \param[in,out] piParameterSize input: maximum buffer size, out: needed buffer size
 * \param[out]    ppValue         on "change": set value and "do": function call argument
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SHALL_EXPORT SECoP_S_getStoredCommand(unsigned long long* pllId, enum SECoP_S_action *piAction,
                                                                        char* szParameter, int* piParameterSize,
                                                                        CSECoPbaseType** ppValue)
{
    SECoP_dataPtr pTmp;
    SECoP_S_error iResult(SECoP_S_Main::getStoredCommand(pllId, piAction, szParameter, piParameterSize, &pTmp));
    if (ppValue != nullptr)
    {
        if (pTmp == nullptr || pTmp.get() == nullptr)
            *ppValue = nullptr;
        else
            *ppValue = pTmp->duplicate();
    }
    return iResult;
}

/*
 * \brief When polling is enabled (\ref SECoP_S_createNode) call SECoP_S_getStoredCommand2
 *        regulary to check, if there is something to do. If yes, it will return
 *        successfully with an id, what to do, a parameter/command name and new set
 *        value or argument. The function will never write more characters to the
 *        parameter/command name, which are provided inside piParameterSize.
 * \ingroup expfunc
 * \param[out]    pllId           id to use with \ref SECoP_S_putCommandAnswer2
 * \param[out]    piAction        what is to do: read or change parameter, do command invokation
 * \param[out]    szParameter     buffer which gets the parameter/command name
 * \param[in,out] piParameterSize input: maximum name buffer size, out: needed name buffer size
 * \param[out]    szValue         buffer which gets the set value or command argument as json
 * \param[in,out] piValueSize     input: maximum value buffer size, out: needed value buffer size
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
extern "C" enum SECoP_S_error SECoP_S_getStoredCommand2(unsigned long long* pllId, SECoP_S_action* piAction, char* szParameter,
                                                        int* piParameterSize, char* szValue, int* piValueSize)
{
    SECoP_dataPtr pTmp;
    enum SECoP_S_error iResult(SECoP_S_Main::getStoredCommand(pllId, piAction, szParameter, piParameterSize, &pTmp));
    if (iResult >= 0)
    {
        QByteArray szBuffer;
        if (pTmp != nullptr)
            szBuffer = pTmp->exportSECoP(false);
        int i(szBuffer.size());
        int j(*piValueSize - 1);
        if (j < i && iResult == SECoP_S_SUCCESS)
            iResult = SECoP_S_WARNING_BUFFER_TOO_SMALL;
        if (j > i)
            j = i;
        if (j < 0)
            j = 0;
        else if (j > 0)
            memmove(szValue, szBuffer.constData(), static_cast<size_t>(j));
        szValue[j] = '\0';
        *piValueSize = i;
    }
    return iResult;
}

/**
 * \brief This function calls the implementation of getStoredCommand inside
 *        SECoP_S_Main.
 * \param[out]    pllId           id to use with \ref SECoP_S_putCommandAnswer
 * \param[out]    piAction        what is to do: read or change parameter, do command invokation
 * \param[out]    szParameter     buffer which gets the parameter/command name
 * \param[in,out] piParameterSize input: maximum buffer size, out: needed buffer size
 * \param[out]    ppValue         on "change": set value, on "do": function call argument
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Main::getStoredCommand(qulonglong* pllId, SECoP_S_action* piAction, char* szParameter,
                                                  int* piParameterSize, SECoP_dataPtr* ppValue)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (g_pSECoPMain == nullptr)
        return SECoP_S_ERROR_NOT_INITIALIZED;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->getStoredCommand(pllId, piAction, szParameter, piParameterSize, ppValue, &iResult);
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "getStoredCommand", Qt::BlockingQueuedConnection, Q_ARG(qulonglong*, pllId),
                                  Q_ARG(SECoP_S_action*, piAction), Q_ARG(char*, szParameter), Q_ARG(int*, piParameterSize),
                                  Q_ARG(SECoP_dataPtr*, ppValue), Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief When polling is enabled (\ref SECoP_S_createNode) call SECoP_S_getStoredCommand
 *        regulary to check, if there is something to do. If yes, it will return
 *        successfully with an id, what to do, a parameter/command name and new set
 *        value or argument. The function will never write more characters to the
 *        parameter/command name, which are provided inside piParameterSize.
 * \param[out]    pllId           id to use with \ref SECoP_S_putCommandAnswer
 * \param[out]    piAction        what is to do: read or change parameter, do command invokation
 * \param[out]    szParameter     buffer which gets the parameter/command name
 * \param[in,out] piParameterSize input: maximum buffer size, out: needed buffer size
 * \param[out]    ppValue         on "change": set value, on "do" function call argument
 * \param[out]    piResult        on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Main::getStoredCommand(qulonglong* pllId, SECoP_S_action* piAction, char* szParameter, int* piParameterSize,
                                    SECoP_dataPtr *ppValue, SECoP_S_error* piResult)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    QMutexLocker locker(m_pMutex);
    if (!m_aStoredCommands.isEmpty())
    {
        ActionEntry entry(m_aStoredCommands.takeFirst());
        entry.m_llCreateTime = QDateTime::currentMSecsSinceEpoch(); // update timestamp
        m_aExecutedCommands.append(entry);
        locker.unlock();
        *pllId = entry.m_qwId;
        *piAction = entry.m_iAction;

        QByteArray szBuffer(entry.m_pNode->getNodeID().toUtf8() + ':' + entry.m_pModule->getModuleID().toUtf8() + ':');
        if (entry.m_pParameter != nullptr)
            szBuffer.append(entry.m_pParameter->getParameterID().toUtf8());
        else if (entry.m_pCommand != nullptr)
            szBuffer.append(entry.m_pCommand->getCommandID().toUtf8());
        int i = szBuffer.size();
        int j = *piParameterSize - 1;
        if (j < i)
            iResult = SECoP_S_WARNING_BUFFER_TOO_SMALL;
        if (j > i)
            j = i;
        if (j < 0)
            j = 0;
        else if (j > 0)
            memmove(szParameter, szBuffer.constData(), static_cast<size_t>(j));
        szParameter[j] = '\0';
        *piParameterSize = i;

        if (entry.m_pValue != nullptr && entry.m_pValue.get() != nullptr)
            *ppValue = entry.m_pValue;
        else
            *ppValue = nullptr;
    }
    else
        iResult = SECoP_S_ERROR_NO_DATA;

    if (piResult != nullptr)
        *piResult = iResult;
}

/*
 * \brief Call SECoP_S_putCommandAnswer, when \ref SECoP_S_getStoredCommand provided
 *        a to do action. You have to provide the read value, value after change
 *        or command call result. All clients which subscribed to this module
 *        should be informed about parameter values.
 * \ingroup expfunc
 * \param[in] llId         id to use with \ref SECoP_S_putCommandAnswer2
 * \param[in] iErrorCode   SECoP_S_SUCCESS or SECoP_S_error for this action.
 * \param[in] pValue       new value of the parameter or command result
 * \param[in] pSigma       error of value of the parameter or nullptr
 * \param[in] dblTimestamp SECoP timestamp of the value or NaN
 */
extern "C" void SHALL_EXPORT SECoP_S_putCommandAnswer(unsigned long long llId, enum SECoP_S_error iErrorCode,
                                                          const CSECoPbaseType *pValue, const CSECoPbaseType *pSigma,
                                                          double dblTimestamp)
{
    SECoP_S_Main::putCommandAnswer(llId, iErrorCode, SECoP_dataPtr(pValue->duplicate()), SECoP_dataPtr(pSigma->duplicate()),
                                   dblTimestamp);
}

/**
 * \brief This function calls the implementation of putCommandAnswer inside
 *        SECoP_S_Main.
 * \param[in] llId         id to use with \ref SECoP_S_putCommandAnswer2
 * \param[in] iErrorCode   SECoP_S_SUCCESS or SECoP_S_error for this action.
 * \param[in] pValue       new value of the parameter or command result
 * \param[in] dblTimestamp SECoP timestamp of the value or NaN
 */
void SECoP_S_Main::putCommandAnswer(qulonglong llId, enum SECoP_S_error iErrorCode, const SECoP_dataPtr pValue,
                                    const SECoP_dataPtr pSigma, double dblTimestamp)
{
    if (g_pSECoPMain == nullptr)
        return;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->putCommandAnswerSlot(llId, iErrorCode, pValue, pSigma, dblTimestamp);
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "putCommandAnswerSlot", Qt::BlockingQueuedConnection, Q_ARG(qulonglong, llId),
                                  Q_ARG(SECoP_S_error, iErrorCode), Q_ARG(const SECoP_dataPtr, pValue),
                                  Q_ARG(const SECoP_dataPtr, pSigma), Q_ARG(double, dblTimestamp));
}

/**
 * \brief Call SECoP_S_putCommandAnswer, when \ref SECoP_S_getStoredCommand provided
 *        a to do action. You have to provide the read value, value after change
 *        or command call result. All clients which subscribed to this module
 *        should be informed about parameter values.
 * \param[in] llId         id to use with \ref SECoP_S_putCommandAnswer2
 * \param[in] iErrorCode   SECoP_S_SUCCESS or SECoP_S_error for this action.
 * \param[in] pValue       new value of the parameter or command result
 * \param[in] pSigma       error of value of the parameter
 * \param[in] dblTimestamp SECoP timestamp of the value or NaN
 */
void SECoP_S_Main::putCommandAnswerSlot(qulonglong llId, enum SECoP_S_error iErrorCode, const SECoP_dataPtr pValue,
                                        const SECoP_dataPtr pSigma, double dblTimestamp)
{
    QMutexLocker locker(m_pMutex);
    for (int i = 0; i < m_aExecutedCommands.size(); ++i)
    {
        if (m_aExecutedCommands[i].m_qwId != llId)
            continue;

        ActionEntry entry(m_aExecutedCommands.takeAt(i));
        if (dblTimestamp <= 0.0)
            dblTimestamp = std::numeric_limits<double>::quiet_NaN();
        switch (entry.m_iAction)
        {
            case SECoP_S_ACTION_READ:
            case SECoP_S_ACTION_CHANGE:
                entry.m_pModule->doParameterResult(iErrorCode, entry.m_qwRequestId, entry.m_pParameter, pValue, pSigma, dblTimestamp);
                break;
            case SECoP_S_ACTION_DO:
                entry.m_pModule->doCommandResult(entry.m_pTarget, iErrorCode, entry.m_pCommand, pValue, dblTimestamp);
                break;
            default:
                break;
        }
        break;
    }
}

/*
 * \brief Call SECoP_S_putCommandAnswer2, when \ref SECoP_S_getStoredCommand2 provided
 *        a to do action. You have to provide the read value, value after change
 *        or command call result. All clients which subscribed to this module
 *        should be informed about parameter values.
 * \ingroup expfunc
 * \param[in] llId         id to use with \ref SECoP_S_putCommandAnswer2
 * \param[in] iErrorCode   SECoP_S_SUCCESS or SECoP_S_error for this action.
 * \param[in] szValue      new JSON value of the parameter or command result
 * \param[in] iValueSize   size of the JSON value
 * \param[in] szSigma      error of value of the parameter
 * \param[in] iSigmaSize   size of the JSON sigma value or 0 (no sigma value)
 * \param[in] dblTimestamp SECoP timestamp of the value or NaN
 */
extern "C" void SHALL_EXPORT SECoP_S_putCommandAnswer2(unsigned long long llId, enum SECoP_S_error iErrorCode,
                                                           const char* szValue, int iValueSize, const char *szSigma,
                                                           int iSigmaSize, double dblTimestamp)
{
    SECoP_S_Main::putCommandAnswer2(llId, iErrorCode, QByteArray(szValue, iValueSize), QByteArray(szSigma, iSigmaSize), dblTimestamp);
}

/**
 * \brief This function calls the implementation of putCommandAnswer2 inside
 *        SECoP_S_Main.
 * \param[in] llId         id to use with \ref SECoP_S_putCommandAnswer2
 * \param[in] iErrorCode   SECoP_S_SUCCESS or SECoP_S_error for this action.
 * \param[in] szValue      new JSON value of the parameter or command result
 * \param[in] szSigma      error of value of the parameter
 * \param[in] dblTimestamp SECoP timestamp of the value or NaN
 */
void SECoP_S_Main::putCommandAnswer2(qulonglong llId, enum SECoP_S_error iErrorCode, QByteArray szValue, QByteArray szSigma, double dblTimestamp)
{
    if (g_pSECoPMain == nullptr)
        return;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->putCommandAnswer2Slot(llId, iErrorCode, szValue, szSigma, dblTimestamp);
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "putCommandAnswer2Slot", Qt::BlockingQueuedConnection, Q_ARG(qulonglong, llId),
                                  Q_ARG(SECoP_S_error, iErrorCode), Q_ARG(QByteArray, szValue), Q_ARG(QByteArray, szSigma),
                                  Q_ARG(double, dblTimestamp));
}

/**
 * \brief Call SECoP_S_putCommandAnswer2, when \ref SECoP_S_getStoredCommand2 provided
 *        a to do action. You have to provide the read value, value after change
 *        or command call result. All clients which subscribed to this module
 *        should be informed about parameter values.
 * \param[in] llId         id to use with \ref SECoP_S_putCommandAnswer2
 * \param[in] iErrorCode   SECoP_S_SUCCESS or SECoP_S_error for this action.
 * \param[in] szValue      new JSON value of the parameter or command result
 * \param[in] szSigma      error of value of the parameter
 * \param[in] dblTimestamp SECoP timestamp of the value or NaN
 */
void SECoP_S_Main::putCommandAnswer2Slot(qulonglong llId, SECoP_S_error iErrorCode, QByteArray szValue,
                                         QByteArray szSigma, double dblTimestamp)
{
    QMutexLocker locker(m_pMutex);
    for (int i = 0; i < m_aExecutedCommands.size(); ++i)
    {
        if (m_aExecutedCommands[i].m_qwId != llId)
            continue;

        ActionEntry entry(m_aExecutedCommands.takeAt(i));
        SECoP_dataPtr pValue;
        SECoP_dataPtr pSigma;
        if (entry.m_pParameter == nullptr)
        {
            CSECoPbaseType* p(CSECoPbaseType::importSECoP(szValue.constData()));
            if (p == nullptr)
            {
                iErrorCode = SECoP_S_ERROR_INVALID_VALUE;
                return;
            }
            pValue = SECoP_dataPtr(p);
            if (!szSigma.isEmpty())
            {
                p = CSECoPbaseType::importSECoP(szValue.constData());
                if (p == nullptr)
                {
                    iErrorCode = SECoP_S_ERROR_INVALID_VALUE;
                    return;
                }
                pSigma = SECoP_dataPtr(p);
            }
        }
        else
        {
            CSECoPbaseType* pHint(entry.m_pParameter->value().get());
            pValue = SECoP_dataPtr(pHint->duplicate());
            if (!pValue->importSECoP(szValue.constData(), true))
            {
                iErrorCode = SECoP_S_ERROR_INVALID_VALUE;
                return;
            }
            if (!szSigma.isEmpty())
            {
                pSigma = SECoP_dataPtr(pHint->duplicate());
                if (!pSigma->importSECoP(szSigma.constData(), true))
                {
                    iErrorCode = SECoP_S_ERROR_INVALID_VALUE;
                    return;
                }
            }
        }
        if (dblTimestamp <= 0.0)
            dblTimestamp = std::numeric_limits<double>::quiet_NaN();
        switch (entry.m_iAction)
        {
            case SECoP_S_ACTION_READ:
            case SECoP_S_ACTION_CHANGE:
                entry.m_pModule->doParameterResult(iErrorCode, entry.m_qwRequestId, entry.m_pParameter, pValue, pSigma, dblTimestamp);
                break;
            case SECoP_S_ACTION_DO:
                entry.m_pModule->doCommandResult(entry.m_pTarget, iErrorCode, entry.m_pCommand, pValue, dblTimestamp);
                break;
            default:
                break;
        }
        break;
    }
}

/*
 * \brief The function SECoP_S_showErrors prints all errors and warnings regarding
 *        node creation to the status window and/or terminal output.
 * \ingroup expfunc
 */
extern "C" void SHALL_EXPORT SECoP_S_showErrors()
{
    SECoP_S_Main::showErrors();
}

/**
 * \brief The function SECoP_S_showErrors prints all errors and warnings regarding
 *        node creation to the status window and/or terminal output.
 */
void SECoP_S_Main::showErrors()
{
    if (g_pSECoPMain == nullptr)
        return;
    if (g_pSECoPMain->thread() == QThread::currentThread())
        g_pSECoPMain->showErrorsSlot();
    else
        QMetaObject::invokeMethod(g_pSECoPMain, "showErrorsSlot", Qt::BlockingQueuedConnection);
}

/**
 * \brief The function SECoP_S_showErrors prints all errors and warnings regarding
 *        node creation to the status window and/or terminal output.
 */
void SECoP_S_Main::showErrorsSlot()
{
    QByteArray szList(printErrorList());
    QByteArray szListW(printWarningList());
    if (!szListW.isEmpty())
    {
        if (!szList.isEmpty())
            szList.append('\n');
        szList.append(szListW);
    }
    for (auto it = m_apNodes.constBegin(); it != m_apNodes.constEnd(); ++it)
    {
        QString szNodeMessages((*it)->getErrors());
        if (!szNodeMessages.isEmpty())
        {
            szList.append("\nnode \"");
            szList.append((*it)->getNodeID().toUtf8());
            szList.append("\" messages:\n");
            szList.append(szNodeMessages.toUtf8());
        }
    }
    local_qInfo("%s", szList.constData());
}

/*
 * \brief Call SECoP_S_getErrorString to convert a SECoP_S_error value into a human
 *        readable text, which describes the error.
 * \ingroup expfunc
 * \param[in] iError SECoP_S_error to convert
 * \return a constant static string pointer to the description
 */
extern "C" const char SHALL_EXPORT * SECoP_S_getErrorString(enum SECoP_S_error iError)
{
    return SECoP_S_Main::getErrorString(iError);
}

/*
 * \brief shows or hides the status window
 * \ingroup expfunc
 * \param[in] bShowGUI true: show status window, false: hide it
 */
extern "C" void SHALL_EXPORT SECoP_S_showStatusWindow(int bShowGUI)
{
    SECoP_S_Main::showGUI(bShowGUI != 0);
}

/**
 * \brief shows or hides the status window
 * \param[in] bShowGUI true: show status window, false: hide it
 */
void SECoP_S_Main::showGUI(bool bShowGUI)
{
    if (m_pInstance != nullptr && m_pInstance->m_pGui != nullptr)
        m_pInstance->m_pGui->showGUI(bShowGUI != 0);
}

/**
 * \returns thread creation configuration inside the library
 */
bool SECoP_S_Main::manyThreads()
{
    return (m_pInstance != nullptr) ? m_pInstance->m_bManyThreads : true;
}

/**
 * \brief configures thread creation inside the library
 *        false: minimize thread creation (nevertheless this library is not single threaded but eases debugging)
 *        true:  create a thread for every node, module, client connection (default)
 * \param[in] bManyThreads 0=false: minimize thread creation, other: thread for node/module/connection
 */
void SECoP_S_Main::manyThreads(bool bManyThreads)
{
    if (m_pInstance != nullptr)
        m_pInstance->m_bManyThreads = bManyThreads;
}

/**
 * \brief SECoP_S_Main::isValidName
 * \param[in] szName name to be used as SECoP module/accessible/property
 * \return true: name if valid, false: invalid name
 */
bool SECoP_S_Main::isValidName(QString szName)
{
    if (szName.isEmpty() || szName.size() > 63)
        return false;
    return QRegExp("_?[A-Za-z][0-9A-Za-z_]*", Qt::CaseSensitive, QRegExp::RegExp2).exactMatch(szName);
}

/// \returns a error description text while SEC-node creation
QByteArray SECoP_S_Main::printErrorList()
{
    QByteArray szResult("Errors:\n");
    szResult.append(g_pSECoPMain->m_aszErrorList.join(QChar('\n')).toUtf8());
    return szResult;
}

/// \returns a warning description text while SEC-node creation
QByteArray SECoP_S_Main::printWarningList()
{
    QByteArray szResult("Warnings:\n");
    szResult.append(g_pSECoPMain->m_aszWarningList.join(QChar('\n')).toUtf8());
    return szResult;
}

/**
 * \brief Regularly called function to check polling interface timeouts.
 */
void SECoP_S_Main::sessionCleanUpTimer()
{
    // clean up old stored actions or started, but not finished actions
    QMutexLocker locker(m_pMutex);
    qint64 llNow(QDateTime::currentMSecsSinceEpoch());
    QList<ActionEntry>* pList(&m_aStoredCommands);
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < pList->size(); ++j)
        {
            ActionEntry &entry = (*pList)[j];
            if ((llNow - entry.m_llCreateTime) > SECOP_POLLING_TIMEOUT)
            {
                QString szParameter, szCommand;
                if (entry.m_pParameter != nullptr)
                    szParameter = entry.m_pParameter->getParameterID();
                if (entry.m_pCommand != nullptr)
                    szCommand = entry.m_pCommand->getCommandID();
                qWarning().nospace() << "clearing action because of timeout: id=" << entry.m_qwId << " action=" << entry.m_iAction
                    << " node=" << entry.m_pNode->getNodeID() << " module=" << entry.m_pModule->getModuleID()
                    << " parameter=" << szParameter << " command=" << szCommand << " value=" << entry.m_pValue;
                switch (entry.m_iAction)
                {
                    case SECoP_S_ACTION_READ:
                    case SECoP_S_ACTION_CHANGE:
                        entry.m_pModule->doParameterResult(SECoP_S_ERROR_TIMEOUT, entry.m_qwRequestId, entry.m_pParameter,
                                                           SECoP_dataPtr(), SECoP_dataPtr(), std::numeric_limits<double>::quiet_NaN());
                        break;
                    case SECoP_S_ACTION_DO:
                        entry.m_pModule->doCommandResult(entry.m_pTarget, SECoP_S_ERROR_TIMEOUT, entry.m_pCommand, SECoP_dataPtr(),
                                                         std::numeric_limits<double>::quiet_NaN());
                        break;
                    default:
                        break;
                }
                pList->removeAt(j--);
            }
        }
        pList = &m_aExecutedCommands;
    }
}

/// \brief global storages for next unique id
quint64 SECoP_S_Main::ActionEntry::m_qwNextId(0);

/**
 * \brief constructor of SECoP_S_Main::ActionEntry creates a unique id with
 *        current time
 */
SECoP_S_Main::ActionEntry::ActionEntry()
    : m_qwRequestId(0)
    , m_pNode(nullptr)
    , m_pModule(nullptr)
    , m_pParameter(nullptr)
    , m_pCommand(nullptr)
    , m_pTarget(nullptr)
    , m_iAction(SECoP_S_ACTION_NONE)
{
    QMutexLocker locker(SECoP_S_Main::m_pInstance->m_pMutex);
    m_qwId = m_qwNextId++;
    m_llCreateTime = QDateTime::currentMSecsSinceEpoch();
}

/**
 * \brief copy constructor of SECoP_S_Main::ActionEntry
 * \param[in] src existing item to copy
 */
SECoP_S_Main::ActionEntry::ActionEntry(const ActionEntry &src)
{
    *this = src;
}

/**
 * \brief copy operator of SECoP_S_Main::ActionEntry
 * \param[in] src existing item to copy
 * \return reference to this instance
 */
SECoP_S_Main::ActionEntry& SECoP_S_Main::ActionEntry::operator=(const SECoP_S_Main::ActionEntry &src)
{
    m_qwId         = src.m_qwId;
    m_qwRequestId  = src.m_qwRequestId;
    m_pNode        = src.m_pNode;
    m_pModule      = src.m_pModule;
    m_pParameter   = src.m_pParameter;
    m_pCommand     = src.m_pCommand;
    m_pTarget      = src.m_pTarget;
    m_iAction      = src.m_iAction;
    m_pValue       = src.m_pValue;
    m_llCreateTime = src.m_llCreateTime;
    return *this;
}
