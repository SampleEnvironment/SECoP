/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include "main.h"
#include "clientgui.h"
#include <thread>
#include <iostream>
#include <QAbstractSocket>

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define qUtf8Printable(string) QString(string).toUtf8().constData()
#endif

/* declare data types for Qt's signal/slot system */
Q_DECLARE_METATYPE(bool*)
Q_DECLARE_METATYPE(char*)
Q_DECLARE_METATYPE(int*)

/* global variables */
/// true, if there is an exit handler installed
static bool             g_bExitHandlerInstalled = false;
/// pointer to old Qt message loggler
static QtMessageHandler g_pOldMessageHandler = nullptr;
/// Qt application pointer, if library had to create it
static QApplication*    g_pApplication = nullptr;
/// flag, if the status window should be visible
static bool             g_bShowGUI;
/// the real singleton instance
static Client_Main*     g_pCInst = nullptr;
/// dummy number of arguments for local QApplication instance
static int              g_iArgc = 0;
/// dummy arguments for local QApplication instance
static char*            g_aszArgv[2] = {nullptr, nullptr};
/// dummy first argument for local QApplication instance
static char*            g_szArgv0 = nullptr;
/// helper thread as "first" thread for local QApplication instance
static std::thread*     g_pThread = nullptr;
/// global initialization status
static volatile bool    g_bInitialized = false;

/* forward declarations */
static void SECoP_C_initLibraryThread(void);
static void SECoP_C_initLibraryHelper(void);
static void SECoP_C_initLibraryExit(void);
static void SECoP_C_initLibraryExitHelper(bool bAtExit);
static void SECoP_C_MessageHandler(QtMsgType iType, const QMessageLogContext &context, const QString &szMessage);

extern "C" enum SECoP_C_error SHALL_EXPORT SECoP_C_initLibrary(QApplication *pApplication, int bGUI)
{
    if (g_bInitialized || g_pCInst != nullptr || g_pThread != nullptr || g_pOldMessageHandler != nullptr)
    {
        g_pCInst->cleanUp(true);
        return SECOP_EC_SUCCESS;
    }
    if (g_pOldMessageHandler == nullptr) // install message logger
		g_pOldMessageHandler = qInstallMessageHandler(&SECoP_C_MessageHandler);
    g_bShowGUI = (bGUI != 0);
    if (pApplication == nullptr)
    {
        g_pThread = new std::thread(&SECoP_C_initLibraryThread);
        if (g_pThread == nullptr)
            return SECoP_EC_NOT_INITIALIZED;
        while (!g_bInitialized)
            std::this_thread::yield();
        if (!g_bExitHandlerInstalled)
        {
            g_bExitHandlerInstalled = true;
            atexit(&SECoP_C_initLibraryExit);
        }
    }
    else
        SECoP_C_initLibraryHelper();
    return SECOP_EC_SUCCESS;
}

/**
 * \brief This function initializes the Qt to cope with library internal data
 *        types and creates the singleton, which stores all SEC-nodes.
 */
void SECoP_C_initLibraryHelper(void)
{
    qRegisterMetaType<bool*>();
    qRegisterMetaType<char*>();
    qRegisterMetaType<int*>();
    qRegisterMetaType<QAbstractSocket::SocketError>();
#pragma message(CPP_WARNINGPREFIX "TODO: register other data types")

    g_pCInst = new Client_Main;
}

/**
 * \brief This function is called by the global exit handler at lib-C exit.
 */
void SECoP_C_initLibraryExit(void)
{
    SECoP_C_initLibraryExitHelper(true);
}

/**
 * \brief This function is called for cleaning up. It tries to revert all,
 *        what has been created or changed since library initialization.
 * \param[in] bAtExit false: user called, true: lib-C called (atexit)
 */
void SECoP_C_initLibraryExitHelper(bool bAtExit)
{
    if (!bAtExit && g_bInitialized && g_bExitHandlerInstalled && g_pCInst != nullptr)
    {
        g_pCInst->cleanUp(true);
        return;
    }
    qInstallMessageHandler(g_pOldMessageHandler);
    g_pOldMessageHandler = nullptr;
    if (g_pCInst != nullptr)
        g_pCInst->cleanUp(false);
    else if (g_pApplication != nullptr)
    {
        g_pApplication->quit();
        while (g_bInitialized)
            std::this_thread::yield();
    }
    else
    {
        qint64 qiStart(QDateTime::currentMSecsSinceEpoch());
        while (g_pCInst->getInstance() != nullptr)
        {
            QApplication::processEvents(QEventLoop::AllEvents, 1000);
            if ((QDateTime::currentMSecsSinceEpoch() - qiStart) >= 5000)
                break;
        }
        if (g_pCInst->getInstance() != nullptr)
            delete g_pCInst;
    }
    if (g_pCInst != nullptr)
        g_pCInst = nullptr;
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
 */
void SECoP_C_initLibraryThread(void)
{
    g_iArgc = 1;
    g_aszArgv[0] = g_szArgv0 = strdup("dummy");
    g_aszArgv[1] = nullptr;
    QApplication app(g_iArgc, g_aszArgv);
    g_pApplication = &app;
    SECoP_C_initLibraryHelper();
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
 * \param[in] bNodeOnly false: clean up all, true: clean up nodes only
 */
extern "C" void SHALL_EXPORT SECoP_C_doneLibrary(int bNodeOnly)
{
    SECoP_C_initLibraryExitHelper(!bNodeOnly);
}

/**
 * \brief This function is called for every Qt message function like qFatal,
 *        qError, qWarning ... It will print the message to the status window
 *        and then call the previous message handler or print the message to
 *        standard error pipe. The prototype of this function is defined in Qt.
 * \param[in] iType     message type/level
 * \param[in] context   where were the message generated
 * \param[in] szMessage the message text
 */
void SECoP_C_MessageHandler(QtMsgType iType, const QMessageLogContext &context, const QString &szMessage)
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
    Client_Main::log(szOutput);
    if (g_pOldMessageHandler != nullptr) // call previous message logger
        (*g_pOldMessageHandler)(iType, context, szMessage);
}

/// the singleton instance of SECoP_Main, \ref g_pSECoPMain
Client_Main* Client_Main::m_pInstance = nullptr;

/**
 * \brief The constructor of SECoP_Main creates an invisible status window and
 *        initializes the singleton.
 */
Client_Main::Client_Main()
    : QObject()
    , m_pGui(nullptr)
    //, m_pLastNode(nullptr)
{
    m_pInstance = this;
    m_pGui = new ClientGui(g_bShowGUI);
    m_pMutex = new QMutex(QMutex::Recursive);
}

/**
 * \brief The destructor deletes all SEC-nodes with modules, parameters,
 *        commands, properties and client connections. It destroys the status
 *        window and triggers the exit of the local QApplication instance,
 *        which stops the associated thread too.
 */
Client_Main::~Client_Main()
{
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

void Client_Main::log(QString szData)
{
    if (g_pCInst != nullptr && g_pCInst->m_pGui != nullptr)
        g_pCInst->m_pGui->log(szData);
}

/**
 * \brief This function calls the implementation of cleaning up, which depends
 *        on the calling thread and argument.
 * \param[in] bHideOnly false: try to clean up everything, true: hide GUI only
 */
void Client_Main::cleanUp(bool bHideOnly)
{
    if (g_pApplication != nullptr)
    {
        if (QThread::currentThread() == thread())
            cleanUpSlot(bHideOnly);
        else if (!bHideOnly)
            QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
        else
            QMetaObject::invokeMethod(this, "cleanUpSlot", Qt::BlockingQueuedConnection, Q_ARG(bool, bHideOnly));
    }
    else
        delete g_pCInst;
}

/**
 * \brief This function hides the GUI or triggers later
 *        self deletion, depending on argument.
 * \param[in] bHideOnly false: trigger later deletion, true: hide GUI only
 */
void Client_Main::cleanUpSlot(bool bHideOnly)
{
    if (bHideOnly && g_bInitialized)
    {
    }
    else
    {
    }
        //deleteLater();
}

#if 0
/**
 * \brief This function translates a SECoP error or warning into a human
 *        readable text.
 * \param[in] iError SECoP_C_error to translate
 * \return the description
 */
const char* Client_Main::getErrorString(enum SECoP_C_error iError)
{
    const char* szResult = nullptr;
    switch (iError)
    {
#define SWITCHCASE(x, y) case x: szResult = #x ": " y; break
        SWITCHCASE(SECOP_C_SUCCESS, "success");
/*        SWITCHCASE(SECoP_C_WARNING_NO_DESCRIPTION,   "no description was given");
        SWITCHCASE(SECoP_C_WARNING_CUSTOM_PROPERTY,  "custom name of property should be prefixed with an underscore");
        SWITCHCASE(SECoP_C_WARNING_BUFFER_TOO_SMALL, "input buffer is too small, some information is missing (call again with greater buffer)");
        SWITCHCASE(SECoP_C_WARNING_MISS_PROPERTIES,  "missing optional properties");
        SWITCHCASE(SECoP_C_ERROR_UNKNOWN_COMMAND,    "invalid SECoP command");
        SWITCHCASE(SECoP_C_ERROR_INVALID_NAME,       "invalid SECoP name");
        SWITCHCASE(SECoP_C_ERROR_INVALID_NODE,       "invalid node reference");
        SWITCHCASE(SECoP_C_ERROR_INVALID_MODULE,     "invalid module reference");
        SWITCHCASE(SECoP_C_ERROR_INVALID_PARAMETER,  "invalid parameter reference");
        SWITCHCASE(SECoP_C_ERROR_INVALID_PROPERTY,   "invalid property reference");
        SWITCHCASE(SECoP_C_ERROR_INVALID_COMMAND,    "invalid command (reference)");
        SWITCHCASE(SECoP_C_ERROR_NOT_IMPLEMENTED,    "function call is not implemented");
        SWITCHCASE(SECoP_C_ERROR_READONLY,           "element is read only");
        SWITCHCASE(SECoP_C_ERROR_NO_DATA,            "function call cannot return any data");
        SWITCHCASE(SECoP_C_ERROR_NO_MEMORY,          "memory allocation problem");*/
        SWITCHCASE(SECoP_C_NOT_INITIALIZED,          "call SECoP_S_initLibrary() first");
/*      SWITCHCASE(SECoP_C_ERROR_INVALID_VALUE,      "datainfo or value was invalid");
        SWITCHCASE(SECoP_C_ERROR_MISSING_MANDATORY,  "an expected property was empty");
        SWITCHCASE(SECoP_C_ERROR_NO_SETTER_GETTER,   "missing a pointer to getter and setter function");
        SWITCHCASE(SECoP_C_ERROR_NO_SETTER,          "missing a pointer to setter function");
        SWITCHCASE(SECoP_C_ERROR_NO_GETTER,          "missing a pointer to getter function");
        SWITCHCASE(SECoP_C_ERROR_NAME_ALREADY_USED,  "the name is used before");
        SWITCHCASE(SECoP_C_ERROR_TIMEOUT,            "parameter or command timeout");*/
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
#endif

/// \returns the current time as fractional unix time
double Client_Main::getCurrentTime()
{
    return static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;
}

/// \return the singleton instance
Client_Main* Client_Main::getInstance()
{
    return m_pInstance;
}

enum SECoP_C_error Client_Main::removeNode(QHostAddress IP, int Port)
{
     enum SECoP_C_error secerror=SECOP_EC_SUCCESS;
     QString Node=m_pGui->getNodeNameByIP(IP,Port);
     if(m_pGui->ConnectedNodesHash.contains(Node))
        m_pGui->removeNode(IP,Port);
     else
        secerror=SECoP_EC_ItemNotFound;
     return secerror;
}
QString Client_Main::getNodeNameByIP(QHostAddress addr,int Port)
{
    return m_pGui->getNodeNameByIP(addr,Port);
}

enum SECoP_C_error Client_Main::addNode(QHostAddress IP, int Port)
{
    QString ip = IP.toString();
   // printf("kdfs "%s,ip);
    if (m_pGui->addNode(IP,Port))
    {
        qDebug().nospace() << "added node: " << ip;
        return SECOP_EC_SUCCESS;
    }
    else
        qDebug().nospace() << "could not add node: " << ip;
    return SECOP_EC_FAILED;
}

std::list<std::string> Client_Main::getNodeNamesList()
{
    QStringList NodesList;
    NodesList = m_pGui->getNodeNamesList();
    std::list<std::string> stdlststdstr;
    foreach (QString str, NodesList)
    {
        stdlststdstr.push_back(str.toStdString());
    }
    return stdlststdstr;
}

std::list<std::string> Client_Main::getModuleNamesList(std::string Node)
{
    QStringList ModuleNames;
    QString Nod =QString::fromStdString(Node);
    ModuleNames = m_pGui->getModuleNamesList(Nod);
    std::list<std::string> stdlststdstr;
    foreach (QString str, ModuleNames)
    {
        stdlststdstr.push_back(str.toStdString());
    }
    return stdlststdstr;
}

std::list<std::string> Client_Main::getAccNameList(std::string Node, std::string Module)
{
    QStringList AccNames;
    QString Nod =QString::fromStdString(Node);
    QString Mod =QString::fromStdString(Module);
    AccNames= m_pGui->getAccNameList(Nod,Mod);
    std::list<std::string> stdlststdstr;
    foreach (QString str, AccNames)
    {
        stdlststdstr.push_back(str.toStdString());
    }
    return stdlststdstr;
}

std::list<std::string> Client_Main::getNodePropertiesKeyList(std::string Node)
{
    std::list<std::string> keylist;
    QString Nod(QString::fromStdString(Node));
    nlohmann::json nodprops(m_pGui->getNodeProperties(Nod));
    if (nodprops.is_object())
        for (auto &it : nodprops.items())
            keylist.push_back(it.key());
    return keylist;
}

std::list<std::string> Client_Main::getModulePropertiesKeyList(std::string Node, std::string Module)
{
    std::list<std::string> keylist;
    QString Nod(QString::fromStdString(Node));
    QString Mod(QString::fromStdString(Module));
    nlohmann::json modprops(m_pGui->getModuleProperties(Nod, Mod));
    if (modprops.is_object())
        for (auto &it : modprops.items())
            keylist.push_back(it.key());
    return keylist;
}

std::list<std::string> Client_Main::getAccPropertiesKeyList(std::string Node, std::string Module, std::string Acce)
{
    std::list<std::string> keylist;
    QString Nod(QString::fromStdString(Node));
    QString Mod(QString::fromStdString(Module));
    QString Acc(QString::fromStdString(Acce));
    nlohmann::json accprops(m_pGui->getAccProperties(Nod, Mod, Acc));
    if (accprops.is_object())
        for (auto &it : accprops.items())
            keylist.push_back(it.key());
    return keylist;
}

/*
*For a first shot the implementation and cast to std::string looks good.
*Since there is no counterpart i can  not guess what may be needed.
*I can imagine that this code needs a rewrite if we want for each property
*its own return type. candidates may property datainfo, property readonly,
*property interface class and everything that has a jsonarray as value
*datainfo returns std::string jsonvalue
*{"type":"tuple","members":[{"type":"enum","members":{"BUSY":300,"ERROR":400,"IDLE":100,"NOTHING":0,"PAUSE":101,"UNKNOWN":-1,"UNSTABLE":250,"WARN":200}},{"type":"string"}]}
*/

std::string Client_Main::getProperty(std::string Node, std::string Module, std::string Acc, std::string Key)
{
    std::string retval;
    QString Nod(QString::fromStdString(Node));
    QString Mod(QString::fromStdString(Module));
    QString Ac (QString::fromStdString(Acc));
    nlohmann::json props;
    if(!Acc.empty()) //it is a accessible
        props = m_pGui->getAccProperties(Nod, Mod, Ac);
    else if(!Module.empty()) //it is a module
        props = m_pGui->getModuleProperties(Nod, Mod);
    else //it is a node
        props = m_pGui->getNodeProperties(Nod);
    if (props.contains(Key))
        retval = getPropertyHelper(props, Key);
    return retval;
}

std::string Client_Main::getPropertyHelper(const nlohmann::json &props, const std::string &key)
{
    std::string Prop;
    if(key!="datainfo")
    {
        if(props.contains(key))
        {
            switch (props[key].type())
            {
                case nlohmann::detail::value_t::null:
                    break;
                case nlohmann::detail::value_t::object:
                    Prop = "The value for key " + key +
                           " is an json object. This may be an error, if not the implementation details where missed. Contact the developers of this library.";
                    break;
                case nlohmann::detail::value_t::discarded:
                    Prop="SECoPError JSON property is undefined";
                    break;
                default:
                    Prop = props[key].dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
                    break;
            }
        }
    }
    else
    { //datainfo is defined with json so we return the json value but !!always as json object and never as json string!!
      //{"type":"tuple","members":[{"type":"enum","members":{"BUSY":300,"ERROR":400,"IDLE":100,"NOTHING":0,"PAUSE":101,"UNKNOWN":-1,"UNSTABLE":250,"WARN":200}},{"type":"string"}]}
      //{"type":"double"}
        if(props[key].is_object())
            Prop = props[key].dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
        if(props[key].is_string())
        {
            //Prop=props[key].get<std::string>();
            Prop="[\""+props[key].get<std::string>()+"\"]";
        }
    }
    return Prop;
}

std::string Client_Main::readParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal)
{
    QString Nod =QString::fromStdString(Node);
    QString Mod =QString::fromStdString(Module);
    QString Acc =QString::fromStdString(ParamName);
    bool forced = false;
    return m_pGui->readParameter(Nod,Mod,Acc,Value,dTimestamp,ErrorVal,forced);
}

std::string Client_Main::forcedReadParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp,SECoP_dataPtr &ErrorVal)
{
    QString Nod =QString::fromStdString(Node);
    QString Mod =QString::fromStdString(Module);
    QString Acc =QString::fromStdString(ParamName);
    bool forced = true;
    return m_pGui->readParameter(Nod,Mod,Acc,Value,dTimestamp,ErrorVal,forced);
}

std::string Client_Main::testRead(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal, std::string &SECoPtripel, qint64 &respTime, qint64 maxTime)
{
    QString Nod =QString::fromStdString(Node);
    QString Mod =QString::fromStdString(Module);
    QString Acc =QString::fromStdString(ParamName);
    return m_pGui->testRead(Nod,Mod,Acc,Value,dTimestamp,ErrorVal,SECoPtripel,respTime, maxTime);
}

std::string Client_Main::writeParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr Value)
{
    QString Nod =QString::fromStdString(Node);
    QString Mod =QString::fromStdString(Module);
    QString Acc =QString::fromStdString(ParamName);
    return m_pGui->writeParam(Nod,Mod,Acc,Value);
}

int Client_Main::getNumberOfNodes()
{
    return m_pGui->getNumberOfNodes();
}

int Client_Main::getNumberOfModules(QString node)
{
    return m_pGui->getNumberOfModules(node);
}

/*
*    ---------------------->       exports
*/

/*
 * \brief shows or hides the status window
 * \param[in] bShowGUI true: show status window, false: hide it
 */
extern "C" void SHALL_EXPORT SECoP_C_showStatusWindow(int bShowGUI)
{
    Client_Main::showGUI(bShowGUI != 0);
}

/**
 * \brief shows or hides the status window
 * \param[in] bShowGUI true: show status window, false: hide it
 */
void Client_Main::showGUI(bool bShowGUI)
{
    if (m_pInstance != nullptr && m_pInstance->m_pGui != nullptr)
        m_pInstance->m_pGui->showGUI(bShowGUI != 0);
}

void Client_Main::startInactive()
{
    m_pGui->setInactiveBool();
}

void Client_Main::callActivate(std::string Node)
{
    QString node;
    node=QString::fromStdString(Node);
    QByteArray SECoPCommand;
    SECoPCommand.clear();
    SECoPCommand.append("activate");
    SECoPCommand.append(10);
    m_pGui->writeToNode(node,SECoPCommand);
}

enum SECoP_C_error SHALL_EXPORT SECoP_C_addNode(std::string IP, int Port, std::string &Node)
{
    QHostAddress addr;
    QString address=QString::fromStdString(IP);
    addr = QHostAddress(address);   
    SECoP_C_error error =g_pCInst->addNode(addr,Port);
    if(error==SECOP_EC_SUCCESS)
    {
        QString name = g_pCInst->getNodeNameByIP(addr,Port);
        Node = name.toStdString();
    }
    return error;
}

std::string SHALL_EXPORT SECoP_C_getNodeNameByIP(std::string IP, int Port)
{
    QHostAddress addr;
    QString address=QString::fromStdString(IP);
    addr = QHostAddress(address);
    QString name = g_pCInst->getNodeNameByIP(addr,Port);
    std::string nodename = name.toStdString();
    return nodename;
}

enum SECoP_C_error SHALL_EXPORT SECoP_C_removeNode(const char* IP,  unsigned short wPort)
{
    QHostAddress addr;
    QString address=QString::fromStdString(IP);
    addr = QHostAddress(address);
    return g_pCInst->removeNode(addr,wPort);
}

int SHALL_EXPORT SECoP_C_getNumberOfNodes()
{
    return g_pCInst->getNumberOfNodes();
}

std::list<std::string> SHALL_EXPORT getNodeNamesList()
{
    return g_pCInst->getNodeNamesList();
}

std::list<std::string> SHALL_EXPORT getModuleNamesList(std::string Node)
{
    return g_pCInst->getModuleNamesList(Node);
}

std::list<std::string> SHALL_EXPORT getAccNameList(std::string Node, std::string Module)
{
    return g_pCInst->getAccNameList(Node,Module);
}

int SHALL_EXPORT SECoP_C_getNumberOfModules(const char* szNode)
{
    if (szNode!=nullptr)
    {
        QString string = QString::fromLocal8Bit(szNode);
        //QString qs= QString::fromStdString(s);
        return g_pCInst->getNumberOfModules(string);
    }
    else
        return -1;
}

std::list<std::string> SHALL_EXPORT getNodePropertiesKeyList(std::string Node)
{
    return g_pCInst->getNodePropertiesKeyList(Node);
}

std::list<std::string> SHALL_EXPORT getModulePropertiesKeyList(std::string Node, std::string Module)
{
    return g_pCInst->getModulePropertiesKeyList(Node,Module);
}

std::list<std::string> SHALL_EXPORT getAccPropertiesKeyList(std::string Node, std::string Module, std::string Acc)
{
    return g_pCInst->getAccPropertiesKeyList(Node,Module,Acc);
}

std::string SHALL_EXPORT getNodePropertyAsString(std::string Node, std::string Key)
{
    std::string dummy="";
    return g_pCInst->getProperty(Node,dummy,dummy,Key);
}

std::string SHALL_EXPORT getModulePropertyAsString(std::string Node, std::string Module, std::string Key)
{
    std::string dummy="";
    return g_pCInst->getProperty(Node,Module,dummy,Key);
}

std::string SHALL_EXPORT getAccPropertyAsString(std::string Node, std::string Module, std::string Acc, std::string Key)
{
    return g_pCInst->getProperty(Node,Module,Acc,Key);
}

std::string SHALL_EXPORT readParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal)
{
    return g_pCInst->readParam(Node,Module,ParamName,Value,dTimestamp,ErrorVal);
}
void SHALL_EXPORT SECoP_C_ShowGui(bool bShowGui)
{
    g_pCInst->showGUI(bShowGui);
}
void SHALL_EXPORT SECoP_C_startInactive()
{
    g_pCInst->startInactive();
}

void SHALL_EXPORT SECoP_C_activateNode(std::string Node)
{
    g_pCInst->callActivate(Node);
}

std::list<std::string> SHALL_EXPORT splitSECoPTriple(std::string Triple) //Triple looks like [42,{\"t\":1533122805.354,\"e\":0.01}]\n"
//an array with 2 entries first the value of an SECoPDatatype second an JSONobject with t: timestamp and e:error
{
    for (;;)
    {
        std::string::size_type n(Triple.find("\n"));
        if (n == std::string::npos)
            break;
        Triple.erase(n, 1);
    }
    std::list<std::string> retlst;
    std::string tmp;
    nlohmann::json doc;
    try
    {
        doc = nlohmann::json::parse(Triple);
    }
    catch (nlohmann::json::exception&)
    {
    }
    catch (...)
    {
    }
    if (doc.is_array() && !doc.empty())//it has to be an array and it should not be empty
    {
        if (doc[0].is_string())
            tmp = doc[0].get<std::string>();
        retlst.push_back(tmp);
        if (doc.size() >= 2 && doc[1].is_object())
        {
            const nlohmann::json &qjo(doc[1]);
            const char* aItems[2] = { "t", "e" };
            for (int i = 0; i < 2; ++i)
            {
                tmp.clear();
                if (qjo.contains(aItems[i]))
                {
                    const nlohmann::json &v(qjo[aItems[i]]);
                    if (v.is_string())
                        tmp = v.get<std::string>();
                }
                retlst.push_back(tmp);
            }
        }
    }
    else
    {
        retlst.push_back("if you see this text your json was invalid");
        retlst.push_back("");
        retlst.push_back("");
    }
    return retlst;
}

std::string SHALL_EXPORT writeParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr Value)
{
    return g_pCInst->writeParam(Node,Module,ParamName,Value);
}

std::string SHALL_EXPORT forcedReadParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp,SECoP_dataPtr  &ErrorVal)
{
    return g_pCInst->forcedReadParam(Node,Module,ParamName,Value,dTimestamp,ErrorVal);
}

std::string SHALL_EXPORT testRead(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal, std::string &SECoPtripel, qint64 &respTime, qint64 maxTime)
{
    return g_pCInst->testRead(Node,Module,ParamName,Value,dTimestamp,ErrorVal,SECoPtripel,respTime,maxTime);
}
