/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include "SECoP-Command.h"
#include "SECoP-Main.h"
#include "SECoP-Module.h"
#include "SECoP-Node.h"
#include "SECoP-Parameter.h"
#include "SECoP-Property.h"
#include "SECoP-StatusGui.h"
#include "SECoP-Worker.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define qUtf8Printable(string) QString(string).toUtf8().constData()
#define local_qInfo qDebug
#else
#define local_qInfo qInfo
#endif

/**
 * \brief The constructor of SECoP_S_Node creates a SEC-node, opens the TCP port
 *        and creates a tab in the status window.
 * \param[in] szNodeID name of the node
 * \param[in] szDesc   description of the node
 * \param[in] address  TCP interface or address to listen to
 * \param[in] wPort    TCP port to listen to
 * \param[in] pParent  Qt parent object
 */
SECoP_S_Node::SECoP_S_Node(QString szNodeID, QString szDesc, QHostAddress address, quint16 wPort, QObject* pParent)
    : QObject(pParent)
    , m_pMutex(nullptr)
    , m_pServer(nullptr)
    , m_szNodeID(szNodeID)
    , m_szDescription(szDesc)
    , m_bChangeable(true)
    , m_iModuleFocus(-1)
{
    SECoP_S_Main::logAddNode(this);
    m_pMutex = new QMutex;
    m_pServer = new QTcpServer(this);
    if (m_pServer != nullptr)
    {
        if (m_pServer->listen(address, wPort))
        {
            connect(m_pServer, SIGNAL(newConnection()), this, SLOT(newClient()), Qt::QueuedConnection);
            SECoP_S_Main::log(this, QString("listening to %1:%2").arg(address.toString()).arg(wPort), true);
        }
        else
        {
            delete m_pServer;
            m_pServer = nullptr;
        }
    }
    enum SECoP_S_error iError(SECoP_S_SUCCESS);
    addProperty(QString("equipment_id"), CSECoPbaseType::simpleString(szNodeID.toUtf8()), false, &iError);
    addProperty(QString("description"), CSECoPbaseType::simpleString(szDesc.toUtf8()), false, &iError);
    addProperty(QString("firmware"), CSECoPbaseType::simpleString(QString("SHALL server library (Git %1)").
                arg(SECoP_S_Main::getGitVersion().constData()).toUtf8()), true, &iError);
}

/**
 * \brief destructor of SECoP_S_Node, it destroys all child modules and properties.
 *        It also destroys existing client connections, closes the TCP port and
 *        removes the tab in the status window.
 */
SECoP_S_Node::~SECoP_S_Node()
{
    QThread* pMySelfThread(QThread::currentThread());
    if (m_pServer != nullptr)
        delete m_pServer;
    for (;;)
    {
        QMutexLocker locker(m_pMutex);
        if (m_apClients.isEmpty())
            break;
        SECoP_S_Worker* pWorker(m_apClients.first());
        QThread* pWorkerThread(pWorker->thread());
        if (pWorkerThread != pMySelfThread)
        {
            pWorker->disconnect(SIGNAL(destroyed()), pWorkerThread, SLOT(quit()));
            pWorker->moveToThread(pMySelfThread);
            pWorkerThread->quit();
            pWorker->quitThreadWhileDelete(false);
        }
        locker.unlock();
        delete pWorker;
    }
    for (int i = 0; i < m_apModules.size(); ++i)
    {
        SECoP_S_Module* pModule(m_apModules[i]);
        QThread* pModuleThread(pModule->thread());
        pModule->deleteLater();
        if (pModuleThread != pMySelfThread)
            pModuleThread->quit();
    }
    for (auto it = m_apProperties.begin(); it != m_apProperties.end(); ++it)
        delete (*it);
    SECoP_S_Main::logRemoveNode(this);
    if (m_pMutex != nullptr)
    {
        m_pMutex->lock();
        QMutex* pMutex(m_pMutex);
        m_pMutex = nullptr;
        pMutex->unlock();
        delete pMutex;
    }
}

/// \returns if this SEC-node is usable, because it could open the TCP server
bool SECoP_S_Node::isValid() const
{
    return m_pServer != nullptr && m_pServer->isListening();
}

/**
 * \brief Overwritten function for moving QThread association around. Normally
 *        you only can "push" the object away into another QThread. This
 *        function also asks the foreign thread to push it back.
 * \param[in] pThread target thread, which should host this object
 */
void SECoP_S_Node::moveToThread(QThread* pThread)
{
    if (QThread::currentThread() == thread())
        moveToThreadSlot(pThread);
    else
        QMetaObject::invokeMethod(this, "moveToThreadSlot", Qt::BlockingQueuedConnection, Q_ARG(QThread*, pThread));
}

/**
 * \brief Helper function to push the QThread association to another thread.
 * \param[in] pThread target thread, which should host this object
 */
void SECoP_S_Node::moveToThreadSlot(QThread* pThread)
{
    QObject::moveToThread(pThread);
}

/**
 * \brief This function creates a command for the last created module.
 * \param[in] szKey     name of the command, which is unique inside the SECoP module
 * \param[in] ptrToFunc function, which is called when a client invokes the command
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Node::addCommand(QString szKey, SECoP_S_callFunction ptrToFunc)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (!m_bChangeable)
        return SECoP_S_ERROR_READONLY;
    if (QThread::currentThread() == thread())
        addCommand(szKey, ptrToFunc, &iResult);
    else
        QMetaObject::invokeMethod(this, "addCommand", Qt::BlockingQueuedConnection, Q_ARG(QString, szKey),
                                  Q_ARG(SECoP_S_callFunction, ptrToFunc), Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a command for the last created module.
 * \param[in]  szKey     name of the command, which is unique inside the SECoP module
 * \param[in]  ptrToFunc function, which is called when a client invokes the command
 * \param[out] piResult  on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Node::addCommand(QString szKey, SECoP_S_callFunction ptrToFunc, SECoP_S_error* piResult)
{
    if (!m_bChangeable)
    {
        *piResult = SECoP_S_ERROR_READONLY;
        return;
    }
    if (!SECoP_S_Main::hasFunctionPointers())
        ptrToFunc = nullptr;
    if (m_iModuleFocus >= 0 && m_iModuleFocus < m_apModules.size())
        *piResult = m_apModules[m_iModuleFocus]->addCommand(szKey, ptrToFunc);
    else
        *piResult = SECoP_S_ERROR_INVALID_MODULE;
}

/**
 * \brief This function creates a read only SECoP parameter inside the last created module.
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Node::addReadableParameter(QString szName, SECoP_S_getsetFunction ptrToGet)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (!m_bChangeable)
        return SECoP_S_ERROR_READONLY;
    if (QThread::currentThread() == thread())
        addReadableParameter(szName, ptrToGet, &iResult);
    else
        QMetaObject::invokeMethod(this, "addReadableParameter", Qt::BlockingQueuedConnection, Q_ARG(QString, szName),
                                  Q_ARG(SECoP_S_getsetFunction, ptrToGet), Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a read only SECoP parameter inside the last created module.
 * \param[in]  szName   name of the parameter, which is unique inside the SECoP module
 * \param[in]  ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Node::addReadableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_error* piResult)
{
    if (!m_bChangeable)
    {
        *piResult = SECoP_S_ERROR_READONLY;
        return;
    }
    if (!SECoP_S_Main::hasFunctionPointers())
        ptrToGet = nullptr;
    if (m_iModuleFocus < 0 || m_iModuleFocus >= m_apModules.size())
        *piResult = SECoP_S_ERROR_INVALID_MODULE;
    else
        *piResult = m_apModules[m_iModuleFocus]->addReadableParameter(szName, ptrToGet);
}

/**
 * \brief This function creates a read- and writable SECoP parameter inside the last created module.
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[in] ptrToSet function, which is called when a client wants to set a new value
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Node::addWritableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (!m_bChangeable)
        return SECoP_S_ERROR_READONLY;
    if (QThread::currentThread() == thread())
        addWritableParameter(szName, ptrToGet, ptrToSet, &iResult);
    else
        QMetaObject::invokeMethod(this, "addWritableParameter", Qt::BlockingQueuedConnection, Q_ARG(QString, szName),
                                  Q_ARG(SECoP_S_getsetFunction, ptrToGet), Q_ARG(SECoP_S_getsetFunction, ptrToSet),
                                  Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a read- and writable SECoP parameter inside the last created module.
 * \param[in]  szName   name of the parameter, which is unique inside the SECoP module
 * \param[in]  ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[in]  ptrToSet function, which is called when a client wants to set a new value
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Node::addWritableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet, SECoP_S_error* piResult)
{
    if (!m_bChangeable)
    {
        *piResult = SECoP_S_ERROR_READONLY;
        return;
    }
    if (!SECoP_S_Main::hasFunctionPointers())
        ptrToGet = ptrToSet = nullptr;
    if (m_iModuleFocus < 0 || m_iModuleFocus >= m_apModules.size())
        *piResult = SECoP_S_ERROR_INVALID_MODULE;
    else
        *piResult = m_apModules[m_iModuleFocus]->addWritableParameter(szName, ptrToGet, ptrToSet);
}

/**
 * \brief This function creates a module inside this node.
 * \param[in] szName name of the module, which is unique inside the SECoP node
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Node::addModule(QString szName)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (!m_bChangeable)
        return SECoP_S_ERROR_READONLY;
    if (QThread::currentThread() == thread())
        addModule(szName, &iResult);
    else
        QMetaObject::invokeMethod(this, "addModule", Qt::BlockingQueuedConnection, Q_ARG(QString, szName),
                                  Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a module inside this node.
 * \param[in]  szName   name of the module, which is unique inside the SECoP node
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Node::addModule(QString szName, SECoP_S_error* piResult)
{
    if (!m_bChangeable)
    {
        *piResult = SECoP_S_ERROR_READONLY;
        return;
    }
    if (!SECoP_S_Main::isValidName(szName))
    {
        *piResult = SECoP_S_ERROR_INVALID_NAME;
        return;
    }
    if (modulePosition(szName) >= 0)
    {
        *piResult = SECoP_S_ERROR_NAME_ALREADY_USED;
        return;
    }

    m_iModuleFocus = m_apModules.size();

    SECoP_S_Module* pModule = new SECoP_S_Module(szName, this);
    if (pModule == nullptr)
    {
        *piResult = SECoP_S_ERROR_NO_MEMORY;
        return;
    }
    m_apModules.append(pModule);

    if (SECoP_S_Main::manyThreads())
    {
        QThread* pModuleThread = new QThread;
        pModule->moveToThread(pModuleThread);
        if (pModule->thread() == pModuleThread)
        {
            pModule->quitThreadWhileDelete(true);
            pModuleThread->connect(pModule, SIGNAL(destroyed()), SLOT(quit()), Qt::DirectConnection);
            pModuleThread->connect(pModuleThread, SIGNAL(finished()), SLOT(deleteLater()));
            pModuleThread->start();
        }
        else
            delete pModuleThread;
    }
    *piResult = SECoP_S_SUCCESS;
}

/**
 * \brief This function creates a SECoP property to this node or last created
 *        module, command or parameter.
 * \param[in] szKey  name of the property, which is unique inside its context.
 *                   You may prepend an unterscore for own properties
 * \param[in] pValue value of the property; note: use the recommended data type
 *                   for the standard SECoP properties
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Node::addProperty(QString szKey, const SECoP_dataPtr pValue)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (!m_bChangeable)
        return SECoP_S_ERROR_READONLY;
    if (QThread::currentThread() == thread())
        addProperty(szKey, pValue, false, &iResult);
    else
        QMetaObject::invokeMethod(this, "addProperty", Qt::BlockingQueuedConnection, Q_ARG(QString, szKey),
                                  Q_ARG(const SECoP_dataPtr, pValue), Q_ARG(bool, false), Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a SECoP property to this node or last created
 *        module, command or parameter.
 * \param[in]  szKey      name of the property, which is unique inside its context.
 *                        You may prepend an unterscore for own properties
 * \param[in]  pValue     value of the property; note: use the recommended data type
 *                        for the standard SECoP properties
 * \param[in]  bAutomatic false: finally create property, true: create overwritable property
 * \param[out] piResult   on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Node::addProperty(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic, SECoP_S_error* piResult)
{
    if (!m_bChangeable)
    {
        *piResult = SECoP_S_ERROR_READONLY;
        return;
    }
    if (!SECoP_S_Main::isValidName(szKey))
    {
        *piResult = SECoP_S_ERROR_INVALID_NAME;
        return;
    }
    local_qInfo("SECoP_S_Node::addProperty(szKey=%s szValue=%s), module=%d/%s",
                qUtf8Printable(szKey), qUtf8Printable(pValue->exportSECoP()),
                m_iModuleFocus, m_iModuleFocus >= 0 ? qUtf8Printable(m_apModules[m_iModuleFocus]->getModuleID()) : "?");

    if (szKey.compare("description", Qt::CaseInsensitive) == 0)
    {
        // description has to be always a string
        const CSECoPstring* pString(nullptr);
        if (pValue != nullptr)
            pString = dynamic_cast<const CSECoPstring*>(pValue.get());
        if (pString == nullptr || pString->getSize() < 1)
        {
            *piResult = SECoP_S_ERROR_INVALID_VALUE;
            return;
        }
    }

    if (m_iModuleFocus >= 0)
    {
        *piResult = m_apModules[m_iModuleFocus]->addProperty(szKey, pValue);
        return;
    }
    if (szKey.compare("modules", Qt::CaseInsensitive) == 0)
    {
        *piResult = SECoP_S_ERROR_INVALID_PROPERTY;
        return;
    }
    int iPos(propertyPosition(szKey));
    if (iPos >= 0)
    {
        if (!m_apProperties[iPos]->isAuto())
        {
            *piResult = SECoP_S_ERROR_NAME_ALREADY_USED;
            return;
        }
    }
    *piResult = SECoP_S_SUCCESS;
    if (!szKey.startsWith('_'))
    {
        const CSECoPbaseType* pTmp(nullptr);
        if (pValue != nullptr)
            pTmp = pValue.get();
        if (pTmp == nullptr || pTmp->getType() == SECoP_VT_NONE)
            *piResult = SECoP_S_WARNING_NO_DESCRIPTION;
        if (SECoP_S_Property::SECoPPropertyListHas(szKey, "Node") < 0)
            *piResult = SECoP_S_WARNING_CUSTOM_PROPERTY;
    }
    if (iPos >= 0)
    {
        if (!m_apProperties[iPos]->setValue(pValue))
            *piResult = SECoP_S_ERROR_NAME_ALREADY_USED;
    }
    else
    {
        SECoP_S_Property* pProperty(new SECoP_S_Property(szKey, pValue, bAutomatic));
        if (pProperty != nullptr)
            m_apProperties.append(pProperty);
        else
            *piResult = SECoP_S_ERROR_NO_MEMORY;
    }
}

/**
 * \brief This function changes the focus while SECoP node creation.
 *        If your creation is not able to guarantee the order of SECoP_add...
 *        functions, you could use this function. Normally you should not need
 *        this. Use a colon (':') separated name for selecting the current item.
 *        Selectable items are nodes, modules, command and parameters.
 * \param[in] szKey   name of the SECoP item to point to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Node::setAddFocus(QString szKey)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (!m_bChangeable)
        return SECoP_S_ERROR_READONLY;
    if (QThread::currentThread() == thread())
        setAddFocus(szKey, &iResult);
    else
        QMetaObject::invokeMethod(this, "setAddFocus", Qt::BlockingQueuedConnection, Q_ARG(QString, szKey),
                                  Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function changes the focus while SECoP node creation.
 *        If your creation is not able to guarantee the order of SECoP_add...
 *        functions, you could use this function. Normally you should not need
 *        this. Use a colon (':') separated name for selecting the current item.
 *        Selectable items are nodes, modules, command and parameters.
 * \param[in]  szKey    name of the SECoP item to point to
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Node::setAddFocus(QString szKey, SECoP_S_error* piResult)
{
    int iPos, iIndex;
    if (szKey.isEmpty())
    {
        m_iModuleFocus = -1;
        *piResult = SECoP_S_SUCCESS;
        return;
    }

    *piResult = SECoP_S_ERROR_INVALID_MODULE;
    iPos = szKey.indexOf(':');
    if (iPos < 0)
    {
        iIndex = modulePosition(szKey);
        szKey.clear();
    }
    else
    {
        iIndex = modulePosition(szKey.left(iPos));
        szKey.remove(0, iPos + 1);
    }
    if (iIndex < 0 || iIndex >= m_apModules.size())
        return;
    SECoP_S_Module* pModule(m_apModules[iIndex]);
    if (pModule == nullptr)
        return;

    *piResult = pModule->setAddFocus(szKey);
    if (*piResult >= 0)
        m_iModuleFocus = iIndex;
}

/**
 * \brief Call this function, if the SECoP node is complete. This triggers some
 *        checks and finally allows clients to connect to this SEC node. It
 *        creates the descriptive json and all modules are informed to start
 *        their polling timers.
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Node::nodeComplete()
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (!m_bChangeable)
        return SECoP_S_ERROR_READONLY;
    if (QThread::currentThread() == thread())
        nodeComplete(&iResult);
    else
        QMetaObject::invokeMethod(this, "nodeComplete", Qt::BlockingQueuedConnection, Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief Call this function, if the SECoP node is complete. This triggers some
 *        checks and finally allows clients to connect to this SEC node. It
 *        creates the descriptive json and all modules are informed to start
 *        their polling timers.
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Node::nodeComplete(SECoP_S_error* piResult)
{
    bool bAppendModOrder(true); //append the module order
    bool bAppendAccOrder(true); //append the accessibles order
    nlohmann::json qja_Module, qjo_NODE;

    QHash<QString, int> ahNodeProps(getDefaultProperties("Node"));
    QHash<QString, int> ahModuleProps(getDefaultProperties("Module"));
    QHash<QString, int> ahParamProps(getDefaultProperties("Parameter"));
    QHash<QString, int> ahCmdProps(getDefaultProperties("Command"));
    QHash<QString, int> ahDatatypeProps(getDefaultProperties("datainfo"));
    QString szMessages;
    bool bError(false);

    // insert node properties
    for (int i = 0; i < m_apProperties.size(); i++)
    {
        const SECoP_S_Property* pProperty(m_apProperties[i]);
        QString szKey(pProperty->getKey());
        const SECoP_dataPtr pValue(pProperty->getValue());
        local_qInfo("nodeprop key=%s val=%s", qUtf8Printable(szKey), qUtf8Printable(pValue->exportSECoP()));
        szMessages.append(checkProperty(bError, "node", szKey, pValue));
        qjo_NODE[szKey.toStdString()] = pValue->exportSECoPjson();
        ahNodeProps.remove(szKey);
        if (!m_apProperties[i]->getKey().compare("order", Qt::CaseInsensitive))//check if modorder exists
        {
            bAppendModOrder = false;//do not append moduleorder
            szMessages.append(checkOrder(bError, nullptr, (pValue != nullptr) ? pValue.get() : nullptr));
        }
    }
    if (bAppendModOrder)
    {
        nlohmann::json orderarray;
        for (int i = 0; i < m_apModules.size(); i++)
            orderarray.push_back(m_apModules[i]->getModuleID().toStdString());
        qjo_NODE["order"] = orderarray;
    }
    szMessages.append(checkProperties(bError, "node", ahNodeProps));

    // insert modules
    for (int i = 0; i < m_apModules.size(); i++)
    {
        SECoP_S_Module* pModule(m_apModules[i]);
        QString szModuleName(pModule->getModuleID()); // JSON STRING
        nlohmann::json qja_module;
        local_qInfo("module=%s", qUtf8Printable(szModuleName));
        qja_module.push_back(szModuleName.toStdString());//
        // insert module properties
        nlohmann::json qjo_moduledata;
        QHash<QString, int> ahProps(ahModuleProps);
        for (int j = 0; j < pModule->getNumberOfProperties(); j++)
        {
            const SECoP_S_Property* pProperty(pModule->getProperty(j));
            QString szKey(pProperty->getKey());
            const SECoP_dataPtr pValue(pProperty->getValue());
            local_qInfo("  moduleprop key=%s val=%s", qUtf8Printable(szKey), qUtf8Printable(pValue->exportSECoP()));

            szMessages.append(checkProperty(bError, QString("module \"%1\"").arg(pModule->getModuleID()), szKey, pValue));
            qjo_moduledata[szKey.toStdString()] = pValue->exportSECoPjson();
            if (pProperty->isAuto())
                ahProps[szKey] = -abs(ahProps[szKey]);
            else
                ahProps.remove(szKey);
            if(!szKey.compare("order", Qt::CaseInsensitive))//check if accorder exists
            {
                bAppendAccOrder = false;//do not append moduleorder
                szMessages.append(checkOrder(bError, pModule, (pValue != nullptr) ? pValue.get() : nullptr));
            }
            else if (!szKey.compare("interface_class", Qt::CaseInsensitive))
                szMessages.append(checkInterfaceClass(bError, pModule, pProperty));
            else if (!szKey.compare("visibility", Qt::CaseInsensitive))
            {
                nlohmann::json v(pValue->exportSECoPjson());
                if (v.is_string())
                {
                    QStringList aszOptions;
                    aszOptions << "expert";
                    aszOptions << "advanced";
                    aszOptions << "user";
                    if (!aszOptions.contains(QString::fromStdString(v.get<std::string>()), Qt::CaseInsensitive))
                        szMessages.append(QString("\nproperty \"visibility\" of node \"%1\" module \"%2\" contains an unknown value\n")
                                          .arg(m_szNodeID).arg(szModuleName));
                }
            }
        } //for (int j=0;j<pModule->getNumberOfProperties();j++)
        if (bAppendAccOrder)
        {
            struct SECoP_S_Module::s_Accessibles entry;
            nlohmann::json orderarray;

            for (int j=0; j < pModule->getNumberOfAccessibles(); j++)
            {
                entry = *(pModule->getAccessibles(j));
                if (entry.command)//case it is a command
                {
                    SECoP_S_Command* pCommand(pModule->getCommand(entry.position));
                    orderarray.push_back(pCommand->getCommandID().toStdString());
                }
                else //case it is a parameter
                {
                    const SECoP_S_Parameter* pParameter(pModule->getParameter(entry.position));
                    orderarray.push_back(pParameter->getParameterID().toStdString());
                }
            }
            qjo_moduledata["order"] = orderarray;
        }
        szMessages.append(checkProperties(bError, QString("module \"%1\"").arg(pModule->getModuleID()), ahProps));

        //insert modules accessibles which is a collection of parameters and commands
        struct SECoP_S_Module::s_Accessibles entry;
        nlohmann::json accessibleobject;
        for (int j=0; j < pModule->getNumberOfAccessibles(); j++)
        {
            entry = *(pModule->getAccessibles(j));
            if (entry.command)//case it is a command
            {
                SECoP_S_Command* pCommand(pModule->getCommand(entry.position));
                QString szCommandName(pCommand->getCommandID());
                local_qInfo("  cmdname=%s", qUtf8Printable(szCommandName));

                // insert command properties
                nlohmann::json qjo_commandproperties;
                ahProps = ahCmdProps;
                for (int k = 0; k < pCommand->getNumberOfProperties(); k++)
                {
                    const SECoP_S_Property* pProperty(pCommand->getProperty(k));
                    QString szKey(pProperty->getKey());
                    const SECoP_dataPtr pValue(pProperty->getValue());
                    local_qInfo("    cmdprop key=%s val=%s", qUtf8Printable(szKey), qUtf8Printable(pValue->exportSECoP()));
                    szMessages.append(checkProperty(bError, QString("command \"%1:%2\"").arg(pModule->getModuleID()).
                                                    arg(pCommand->getCommandID()), szKey, pValue));
                    qjo_commandproperties[szKey.toStdString()] = pValue->exportSECoPjson();
                    if (pProperty->isAuto())
                        ahProps[szKey] = -abs(ahProps[szKey]);
                    else
                        ahProps.remove(szKey);
                }
                szMessages.append(checkProperties(bError, QString("command \"%1:%2\"").arg(pModule->getModuleID()).
                                                  arg(pCommand->getCommandID()), ahProps));
                const SECoP_S_Property* pDatatype(pCommand->getProperty(pCommand->propertyPosition("datainfo")));
                if (pDatatype != nullptr)
                {
                    QHash<QString, int> ahErrors;
                    const SECoP_dataPtr value(pDatatype->getValue());
                    szMessages.append(checkDatatype(bError, szCommandName, (value != nullptr) ? value.get() : nullptr,
                                                    ahErrors, true));
                }
                accessibleobject[szCommandName.toStdString()]=qjo_commandproperties;
            }
            else //case it is a parameter
            {
                const SECoP_S_Parameter* pParameter(pModule->getParameter(entry.position));
                QString szParameterName(pParameter->getParameterID());
                local_qInfo("  parametername=%s", qUtf8Printable(szParameterName));
                QString accName = szParameterName;
                szParameterName.prepend(":");
                szParameterName.prepend(pModule->getModuleID());
                szParameterName.prepend("parameter \"");
                szParameterName.append("\"");
                SECoP_dataPtr pValue(pParameter->value());
                if (pValue.get() == nullptr)
                {
                    bError = true;
                    szMessages.append(QString("%1 has an invalid data type").arg(szParameterName));
                }
                // insert parameter properties
                bool bConstant(false);
                nlohmann::json qjo_parameterproperties;
                ahProps = ahParamProps;
                for (int k = 0; k < pParameter->getNumberOfProperties(); k++)
                {
                    const SECoP_S_Property* pProperty(pParameter->getProperty(k));
                    QString szKey(pProperty->getKey());
                    const SECoP_dataPtr pValue(pProperty->getValue());
                    local_qInfo("    paramprop key=%s val=%s", qUtf8Printable(szKey), qUtf8Printable(pValue->exportSECoP()));
                    if (szKey.compare("constant", Qt::CaseInsensitive) == 0)
                        bConstant = true;
                    szMessages.append(checkProperty(bError, szParameterName, szKey, pValue));
                    qjo_parameterproperties[szKey.toStdString()] = pValue->exportSECoPjson();
                    if (pProperty->isAuto())
                        ahProps[szKey] = -abs(ahProps[szKey]);
                    else
                        ahProps.remove(szKey);
                }
                szMessages.append(checkProperties(bError, szParameterName, ahProps));
                if (!bConstant && SECoP_S_Main::hasFunctionPointers())
                {
                    bool bHasGetter(pParameter->getter() != nullptr);
                    bool bHasSetter(!pParameter->isWritable() || pParameter->setter() != nullptr);
                    QString szLine;
                    if (!bHasGetter && !bHasSetter)
                        szLine = QString("getter, setter");
                    else if (!bHasGetter && bHasSetter)
                        szLine = QString("getter");
                    else if (bHasGetter && !bHasSetter)
                        szLine = QString("setter");
                    if (!szLine.isEmpty())
                    {
                        szMessages.append(QString("\nmissing %1 or \"constant\" property for %2").arg(szLine).arg(szParameterName));
                        bError = true;
                    }
                }
                const SECoP_S_Property* pDatatype(pParameter->getProperty(pParameter->propertyPosition("datainfo")));
                if (pDatatype != nullptr)
                {
                    const SECoP_dataPtr value(pParameter->value());
                    if (!value->isValid())
                    {
                        szMessages.append(QString("\ninvalid datainfo of %1").arg(szParameterName));
                        bError = true;
                    }
                    QHash<QString, int> ahErrors;
                    szMessages.append(checkDatatype(bError, szParameterName, (value != nullptr) ? value.get() : nullptr,
                                                    ahErrors, false));
                }
                accessibleobject[accName.toStdString()]=qjo_parameterproperties;
            }
        }
        qjo_moduledata["accessibles"] = accessibleobject;
        //qja_module.push_back(qjo_moduledata);
        qja_Module[szModuleName.toStdString()] = qjo_moduledata;
        //qja_Module.push_back(qja_module);
    } //for (int i=0;i<m_apModules.size();i++)
    qjo_NODE["modules"] = qja_Module;

    m_szDescribingJSON = qjo_NODE;
    m_bChangeable = false;
    SECoP_S_Main::log(this, QString::fromStdString(m_szDescribingJSON.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace)), true);
    if (!szMessages.isEmpty())
    {
        szMessages.remove(0, 1);
        m_szErrorsWarnings = szMessages;
        SECoP_S_Main::log(this, szMessages, true);
        *piResult = bError ? SECoP_S_ERROR_INVALID_PROPERTY : SECoP_S_WARNING_MISS_PROPERTIES;
    }
    else
    {
        m_szErrorsWarnings.clear();
        *piResult = SECoP_S_SUCCESS;
    }
    if (*piResult >= 0)
        SECoP_S_Main::log(this, QString("opened the node, allowing clients\n"), false);

    // inform modules: start their polling timers
    for (auto it = m_apModules.constBegin(); it != m_apModules.constEnd(); ++it)
        (*it)->nodeIsComplete();
}

/// \returns the name of the actual module
QString SECoP_S_Node::getActiveModuleName() const
{
    if (m_iModuleFocus >= 0 && m_iModuleFocus < m_apModules.size())
        return m_apModules[m_iModuleFocus]->getModuleID();
    return QString();
}

/// \returns the printable active element
QString SECoP_S_Node::getPrintableActive(bool bWithAccessible) const
{
    QString szResult(QString("node \"%1\"").arg(m_szNodeID));
    if (m_iModuleFocus >= 0)
    {
        szResult.append(" ");
        szResult.append(m_apModules[m_iModuleFocus]->getPrintableActive(bWithAccessible));
    }
    return szResult;
}

/**
 * \brief This helper function is used for checking properties while generation
 *        of descriptive json.
 * \param[in] szType requested scope: Node, Module, Parameter, Command
 * \return a hash with mandatory, recommended and optional properties
 */
QHash<QString, int> SECoP_S_Node::getDefaultProperties(QString szType)
{
    QStringList asz(SECoP_S_Property::SECoPPropertyNeeds(szType));
    QHash<QString, int> ahResult;
    for (int i = 0; i < asz.size(); ++i)
    {
        QString szKey(asz[i]);
        ahResult[szKey] = SECoP_S_Property::SECoPPropertyListHas(szKey, szType);
    }
    return ahResult;
}

/**
 * \brief The helper function checks the remaining properties, which are not
 *        part of descriptive json yet. It generates a warning or error text
 *        for use in the status window.
 * \param[in] bError  reference the global error flag
 * \param[in] szName  name of the module, parameter or command
 * \param[in] ahProps remaining properties to test
 * \return empty: success, non-empty: warning or error text
 */
QString SECoP_S_Node::checkProperties(bool& bError, QString szName, QHash<QString, int> ahProps)
{
    int iCount(0);
    bool bMandatoryMissing(false);
    for (auto it = ahProps.constBegin(); it != ahProps.constEnd(); ++it)
    {
        int iMandatoryLevel(it.value());
        if (iMandatoryLevel == 0)
            continue;
        ++iCount;
        if (iMandatoryLevel > 1)
            bMandatoryMissing = bError = true;
    }
    if (iCount < 1)
        return QString();
    QString szMessage(QString("\n%1 missing propert%2 for %3").arg(bMandatoryMissing ? "error" : "warning").
                      arg(ahProps.count() == 1 ? "y" : "ies").arg(szName));
    iCount = 0;
    for (auto it = ahProps.constBegin(); it != ahProps.constEnd(); ++it, ++iCount)
    {
        szMessage += iCount > 0 ? ", ": ": ";
        szMessage += abs(it.value()) > 1 ? "mandatory" : "optional";
        szMessage += " \"";
        szMessage += it.key();
        szMessage += "\"";
    }
    return szMessage;
}

/**
 * \brief The helper function checks the a property for correct data type. It
 *        generates a warning or error text for use in the status window.
 * \param[in] bError  reference the global error flag
 * \param[in] szName  type and name of the node, module, parameter or command
 * \param[in] szKey   name of the property to check
 * \param[in] pValue  current value of the property to check
 * \return empty: success, non-empty: error text
 */
QString SECoP_S_Node::checkProperty(bool& bError, QString szName, QString szKey, SECoP_dataPtr pValue)
{
    bool bSetError(false), bSetWarning(false);
    QString szType(szName);
    int i(szType.indexOf(' '));
    if (i >= 0)
        szType.resize(i);
    QList<enum SECoP_V_type> aiWantedTypes(SECoP_S_Property::SECoPPropertyTypes(szKey, szType));
    if (aiWantedTypes.isEmpty())
        return QString();
    const CSECoPbaseType* pData(pValue.get());
    enum SECoP_V_type iHasType(SECoP_VT_NONE);
    if (pData == nullptr)
    {
        bSetError = true;
        goto message;
    }
    else
        iHasType = pData->getType();
    for (i = 0; i < aiWantedTypes.size(); ++i)
    {
        if (aiWantedTypes[i] == SECoP_VT_JSON)
            break;
        if (aiWantedTypes[i] != iHasType)
            continue;
        switch (iHasType)
        {
            case SECoP_VT_DOUBLE:
            case SECoP_VT_INTEGER:
            case SECoP_VT_BOOL:
            case SECoP_VT_JSON:
            case SECoP_VT_STRING:
            case SECoP_VT_BLOB:
                break;
            default:
                bSetError = true;
                goto message;
        }
        if (i > 0)
            bSetWarning = true;
        break;
    }
    if (i >= aiWantedTypes.size())
        bSetError = true;
message:
    QString szMessage;
    if (!bSetError && !bSetWarning)
        return szMessage;

    if (bSetError)
        bError = true;
    szMessage = QString("\n%1 invalid data type of %2 property %3").arg(bSetError ? "error": "warning").arg(szName).arg(szKey);
    for (i = 0; i <= aiWantedTypes.size(); ++i)
    {
        enum SECoP_V_type iType(iHasType);
        if (i < aiWantedTypes.size())
        {
            iType = aiWantedTypes[i];
            if (!i)
                szMessage += "; wanted type ";
            else
                szMessage += '/';
        }
        else
            szMessage += " != current type ";
        switch (iType)
        {
            case SECoP_VT_ARRAY_DOUBLE:
            case SECoP_VT_DOUBLE:  szMessage += "double";      break;
            case SECoP_VT_ARRAY_INTEGER:
            case SECoP_VT_INTEGER: szMessage += "integer";     break;
            case SECoP_VT_ARRAY_BOOL:
            case SECoP_VT_BOOL:    szMessage += "boolean";     break;
            case SECoP_VT_ARRAY_ENUM:
            case SECoP_VT_ENUM:    szMessage += "enumeration"; break;
            case SECoP_VT_ARRAY_SCALED:
            case SECoP_VT_SCALED:  szMessage += "scaled";      break;
            case SECoP_VT_STRING:  szMessage += "string";      break;
            case SECoP_VT_BLOB:    szMessage += "blob";        break;
            case SECoP_VT_JSON:    szMessage += "json";        break;
            case SECoP_VT_STRUCT:  szMessage += "struct";      break;
            case SECoP_VT_TUPLE:   szMessage += "tuple";       break;
            case SECoP_VT_ARRAY:   szMessage += "array";       break;
            default:               szMessage += "???";         break;
        }
    }
    return szMessage;
}

/**
 * \brief This function checks a accessible property for correct data type.
 * \param[in,out] bError   flag, if there is/was an error
 * \param[in]     szName   property name
 * \param[in]     pValue   property value
 * \param[in,out] ahErrors hash of already triggered errors
 * \return a human readable error message
 */
QString SECoP_S_Node::checkDatatype(bool& bError, QString szName, const CSECoPbaseType *pValue,
                                    QHash<QString, int>& ahErrors, bool bAllowCommand)
{
    const CSECoPsimpleBool*   pSBool(nullptr);
    const CSECoParrayBool*    pABool(nullptr);
    const CSECoPsimpleDouble* pSDbl(nullptr);
    const CSECoParrayDouble*  pADbl(nullptr);
    const CSECoPsimpleInt*    pSInt(nullptr);
    const CSECoParrayInt*     pAInt(nullptr);
    const CSECoPsimpleScaled* pSScl(nullptr);
    const CSECoParrayScaled*  pAScl(nullptr);
    const CSECoPsimpleEnum*   pSEnm(nullptr);
    const CSECoParrayEnum*    pAEnm(nullptr);
    const CSECoPstring*       pString(nullptr);
    const CSECoPstruct*       pStruct(nullptr);
    const CSECoPtuple*        pTuple(nullptr);
    const CSECoParray*        pArray(nullptr);
    const CSECoPcommand*      pCommand(nullptr);
    QString szResult;
    do
    {
        pSScl = dynamic_cast<const CSECoPsimpleScaled*>(pValue);
        if (pSScl != nullptr)
            break;
        pAScl = dynamic_cast<const CSECoParrayScaled*>(pValue);
        if (pAScl != nullptr)
            break;
        pSEnm = dynamic_cast<const CSECoPsimpleEnum*>(pValue);
        if (pSEnm != nullptr)
            break;
        pAEnm = dynamic_cast<const CSECoParrayEnum*>(pValue);
        if (pAEnm != nullptr)
            break;
        pSBool = dynamic_cast<const CSECoPsimpleBool*>(pValue);
        if (pSBool != nullptr)
            break;
        pABool = dynamic_cast<const CSECoParrayBool*>(pValue);
        if (pABool != nullptr)
            break;
        pSDbl = dynamic_cast<const CSECoPsimpleDouble*>(pValue);
        if (pSDbl != nullptr)
            break;
        pADbl = dynamic_cast<const CSECoParrayDouble*>(pValue);
        if (pADbl != nullptr)
            break;
        pSInt = dynamic_cast<const CSECoPsimpleInt*>(pValue);
        if (pSInt != nullptr)
            break;
        pAInt = dynamic_cast<const CSECoParrayInt*>(pValue);
        if (pAInt != nullptr)
            break;
        pString = dynamic_cast<const CSECoPstring*>(pValue);
        if (pString != nullptr)
            break;
        pStruct = dynamic_cast<const CSECoPstruct*>(pValue);
        if (pStruct != nullptr)
            break;
        pTuple = dynamic_cast<const CSECoPtuple*>(pValue);
        if (pTuple != nullptr)
            break;
        pArray = dynamic_cast<const CSECoParray*>(pValue);
        if (pArray != nullptr)
            break;
        if (bAllowCommand)
        {
            pCommand = dynamic_cast<const CSECoPcommand*>(pValue);
            if (pCommand != nullptr)
                break;
        }
        if (ahErrors.contains(""))
            return QString();
        ahErrors[""] = 0;
        return QString("\ninvalid datainfo of %1").arg(szName);
    } while (0);
    nlohmann::json o(pValue->additional());
    if (pSScl != nullptr || pAScl != nullptr || pSDbl != nullptr || pADbl != nullptr)
    {
        // check property "unit", "fmtstr", "absolute_resolution" and "relative_resolution"
        // for types "scaled" and "double"
        struct item
        {
            const char* szName;
            const char* szRegExp;
            bool        bWarnMissing;
            bool        bWarnValue;
        } aItems[] = { { "unit", ".*", true, false }, { "fmtstr", "%\\.[0-9]+[feg]", false, true },
                       { "absolute_resolution", nullptr, false, false },
                       { "relative_resolution", nullptr, false, false } };
        for (unsigned int i = 0; i < sizeof(aItems) / sizeof(aItems[0]); ++i)
        {
            struct item* p(&aItems[i]);
            if (o.contains(p->szName))
            {
                const nlohmann::json &v(o[p->szName]);
                if ((p->szRegExp != nullptr && !v.is_string()) ||
                    (p->szRegExp == nullptr && !v.is_number()))
                {
                    bError = true;
                    if (!ahErrors.contains(p->szName))
                    {
                        ahErrors[p->szName] = 0;
                        szResult.append(QString("\nwrong \"%1\" type in \"datainfo\" property of %2").arg(p->szName).arg(szName));
                    }
                }
                else
                {
                    if (p->bWarnValue)
                    {
                        QString szTmp(QString("%1_warn").arg(p->szName));
                        if (p->szRegExp != nullptr && !ahErrors.contains(szTmp) &&
                            !QRegExp(p->szRegExp).exactMatch(QString::fromStdString(v.get<std::string>())))
                        {
                            ahErrors[szTmp] = 0;
                            szResult.append(QString("\ninvalid \"%1\" in \"datainfo\" property of %2").arg(p->szName).arg(szName));
                        }
                    }
                }
                o.erase(p->szName);
            }
            else if (p->bWarnMissing)
                szResult.append(QString("\nmissing \"%1\" in \"datainfo\" property of %2").arg(p->szName).arg(szName));
        }
    }
    if (pStruct != nullptr)
    {
        // check property "optional" for type "struct"
        if (o.contains("optional"))
        {
            bool bTmpErr(false);
            const nlohmann::json &vO(o["optional"]);
            if (vO.is_array())
            {
                std::map<std::string, int> ahOpts;
                for (unsigned int i = 0; i < vO.size(); ++i)
                {
                    const nlohmann::json &v(vO[i]);
                    if (v.is_string())
                    {
                        std::string szOpt(v.get<std::string>());
                        unsigned int uIndex(0);
                        if (!pStruct->findItem(szOpt.c_str(), uIndex))
                        {
                            bTmpErr = true;
                            break;
                        }
                        ahOpts[szOpt] = 0;
                    }
                    else
                    {
                        bTmpErr = true;
                        break;
                    }
                }
            }
            else
                bTmpErr = true;
            if (bTmpErr)
            {
                bError = true;
                if (!ahErrors.contains("optional"))
                {
                    ahErrors["optional"] = 0;
                    szResult.append(QString("\nwrong \"optional\" type or data in \"datainfo\" property of %1").arg(szName));
                }
            }
            o.erase("optional");
        }
    }
    for (auto &it : o.items())
    {
        QString szKey(QString::fromStdString(it.key()));
        bError = true;
        if (!ahErrors.contains(szKey))
        {
            ahErrors[szKey] = 0;
            szResult.append(QString("\nunknown \"%1\" in \"datainfo\" property of %2").arg(szKey).arg(szName));
        }
    }
    if (pArray != nullptr || pCommand != nullptr || pStruct != nullptr || pTuple != nullptr)
    {
        // check sub types of types "array", "command", "struct" and "tuple"
        unsigned int uMaxSize(0);
        if (pArray != nullptr)
            uMaxSize = pArray->getSize();
        else if (pCommand != nullptr)
            uMaxSize = 2;
        else if (pStruct != nullptr)
            uMaxSize = pStruct->getItemCount();
        else if (pTuple != nullptr)
            uMaxSize = pTuple->getSize();
        for (unsigned int i = 0; i < uMaxSize; ++i)
        {
            QHash<QString, int> ahTmpErrors;
            CSECoPbaseType* pMember(nullptr);
            if (pArray != nullptr)
                pMember = pArray->getValue(i);
            else if (pCommand != nullptr)
            {
                if (!i)
                    pMember = pCommand->getArgument();
                else
                    pMember = pCommand->getResult();
            }
            else if (pStruct != nullptr)
                pMember = pStruct->getItem(i);
            else if (pTuple != nullptr)
                pMember = pTuple->getValue(i);
            if (pMember != nullptr)
                szResult.append(checkDatatype(bError, szName, pMember, ahTmpErrors, false));
        }
    }
    return szResult;
}

/**
 * \brief This function checks the property "interface_class". It understands the following:
 *        - readable (has to contain parameters "value" and "status" with at least "idle")
 *        - writable (is a readable plus "target" parameter)
 *        - drivable (is a writable plus "stop" command and status with at least "busy")
 * \param[in,out] bError  flag, if there is/was any error
 * \param[in]     pModule module to check
 * \param[in]     pIFC    "interface_class" property
 * \return a human readable error message
 */
QString SECoP_S_Node::checkInterfaceClass(bool& bError, const SECoP_S_Module* pModule, const SECoP_S_Property* pIFC)
{
    // check interface_class
    const SECoP_dataPtr pValue(pIFC->getValue());
    const CSECoPstring* pString(nullptr);
    nlohmann::json vIFC;
    bool bCommunicator(false);
    int iType(0);

    if (pValue != nullptr)
        pString = dynamic_cast<const CSECoPstring*>(pValue.get());
    if (pString == nullptr)
        goto wrongIFCtype;
    vIFC = pString->exportSECoPjson();
    if (!vIFC.is_array())
        goto wrongIFCtype;

    for (unsigned int k = 0; k < vIFC.size(); ++k)
    {
        nlohmann::json v(vIFC[k]);
        if (!v.is_string())
            goto wrongIFCtype;

        QString szType(QString::fromStdString(v.get<std::string>()));
        if (!szType.compare("readable", Qt::CaseInsensitive))
        {
            if (iType < 1)
                iType = 1;
        }
        else if (!szType.compare("writable", Qt::CaseInsensitive))
        {
            if (iType < 2)
                iType = 2;
        }
        else if (!szType.compare("drivable", Qt::CaseInsensitive))
        {
            if (iType < 3)
                iType = 3;
        }
        else if (!szType.compare("communicator", Qt::CaseInsensitive))
            bCommunicator = true;
    }
    if (iType > 0)
    {
        // check for "value" and "status" parameters
        if (pModule->parameterPosition("value") < 0)
        {
            bError = true;
            return QString("\nmodule \"%1\" is missing a parameter \"value\"").arg(pModule->getModuleID());
        }
        const SECoP_S_Parameter* pStatusParam(pModule->getParameter(pModule->parameterPosition("status")));
        if (pStatusParam == nullptr)
        {
            bError = true;
            return QString("\nmodule \"%1\" is missing a parameter \"status\"").arg(pModule->getModuleID());
        }
        else
        {
            const SECoP_dataPtr pStatusValue(pStatusParam->value());
            const CSECoPtuple* pStatus(nullptr);
            bool bIdle(false), bBusy(false);
            if (pStatusValue != nullptr)
                pStatus = dynamic_cast<const CSECoPtuple*>(pStatusValue.get());
            if (pStatus == nullptr || pStatus->getSize() < 2)
                goto wrongStatus;
            const CSECoPsimpleEnum* pEnum(dynamic_cast<const CSECoPsimpleEnum*>(pStatus->getValue(0)));
            const CSECoPstring* pString(dynamic_cast<const CSECoPstring*>(pStatus->getValue(1)));
            if (pEnum == nullptr || pString == nullptr)
                goto wrongStatus;
            for (unsigned int i = 0; i < pEnum->getItemCount(); ++i)
            {
                const char* szItemName(pEnum->getItemName(i));
                if (szItemName == nullptr || szItemName[0] == '\0')
                    goto wrongStatusEnum;
                if (QRegExp("idle(_.*)?", Qt::CaseInsensitive, QRegExp::RegExp2).exactMatch(szItemName))
                {
                    bIdle = true;
                    continue;
                }
                if (QRegExp("busy(_.*)?", Qt::CaseInsensitive, QRegExp::RegExp2).exactMatch(szItemName))
                {
                    bBusy = true;
                    continue;
                }
                long long llValue(pEnum->getItemValue(i));
                if (llValue >= 100 && llValue < 200)
                    bIdle = true;
                else if (llValue >= 300 && llValue < 400)
                    bBusy = true;
            }
            // every status has to contain an "IDLE" and every drivable a "BUSY"
            if (!bIdle || (iType > 2 && !bBusy))
                goto wrongStatus;
        }
        if (iType > 1)
        {
            // check for "target" parameter
            if (pModule->parameterPosition("target") < 0)
            {
                bError = true;
                return QString("\nmodule \"%1\" is missing a parameter \"target\"").arg(pModule->getModuleID());
            }
        }
        if (iType > 2)
        {
            // check for "stop" command
            if (pModule->commandPosition("stop") < 0)
            {
                bError = true;
                return QString("\nmodule \"%1\" is missing a command \"stop\"").arg(pModule->getModuleID());
            }
        }
    }
    else if (iType < 0 || !bCommunicator)
    {
        bError = true;
        return QString("\nmodule \"%1\" has an invalid property \"interface_class\"");
    }
    return QString();

wrongIFCtype:
    bError = true;
    return QString("\nproperty \"interface_class\" of module \"%1\" is not a JSON array of strings").arg(pModule->getModuleID());

wrongStatus:
    bError = true;
    return QString("\nparameter \"status\" of module \"%1\" is not a tuple of enum and string").arg(pModule->getModuleID());

wrongStatusEnum:
    bError = true;
    return QString("\nparameter \"status\" of module \"%1\" has a wrong enumeration item").arg(pModule->getModuleID());
}

/**
 * \brief This function checks the property "order" of a node or module.
 * \param[in,out] bError  flag, if there is/was any error
 * \param[in]     pModule module to check or nullptr (node)
 * \param[in]     pValue
 * \return a human readable error message
 */
QString SECoP_S_Node::checkOrder(bool& bError, const SECoP_S_Module* pModule, const CSECoPbaseType* pOrder)
{
    nlohmann::json vOrder(pOrder->exportSECoPjson());
    QString szFocus(QString("node \"%1\"").arg(m_szNodeID));
    if (pModule != nullptr)
        szFocus.append(QString(" module \"%1\"").arg(pModule->getModuleID()));
    if (!vOrder.is_array())
        goto wrongOrderType;
    for (unsigned int j = 0; j < vOrder.size(); ++j)
    {
        const nlohmann::json &v(vOrder[j]);
        if (!v.is_string())
            goto wrongOrderType;
        QString szItem(QString::fromStdString(v.get<std::string>()));
        if (pModule != nullptr)
        {
            if (pModule->parameterPosition(szItem) < 0 &&
                pModule->commandPosition(szItem) < 0)
                goto wrongItemName;
        }
        else if (modulePosition(szItem) < 0)
            goto wrongItemName;
    }
    return QString();

wrongOrderType:
    bError = true;
    return QString("\nproperty \"order\" of %1 is not a JSON array of strings").arg(szFocus);

wrongItemName:
    bError = true;
    return QString("\nproperty \"order\" of %1 contains an invalid name").arg(szFocus);
}

/// \return the descriptive json for this node
nlohmann::json SECoP_S_Node::getJSON() const
{
    return m_szDescribingJSON;
}

/// \return the warning and error text of this node
QString SECoP_S_Node::getErrors() const
{
    return m_szErrorsWarnings;
}

/// \return the node ID
QString SECoP_S_Node::getNodeID() const
{
    return m_szNodeID;
}

/**
 * \brief This function returns the instance of a module given by index or nullptr.
 * \param[in] iModule zero based module index
 * \return the instance of a module or nullptr
 */
SECoP_S_Module* SECoP_S_Node::getModule(int iModule) const
{
    if (iModule >= 0 && iModule < m_apModules.size())
        return m_apModules[iModule];
    else
        return nullptr;
}

/**
 * \brief This function searchs for the index of a module name.
 * \param[in] szModuleName name of the module to search for
 * \return the zero based module index or a SECoP_S_error
 */
int SECoP_S_Node::modulePosition(QString szModuleName) const
{
    int iResult(SECoP_S_ERROR_INVALID_MODULE);
    for (int i = 0; i < m_apModules.size(); i++)
        if (szModuleName.compare(m_apModules[i]->getModuleID(), Qt::CaseInsensitive) == 0)
            return i;
    return iResult;
}

/**
 * \brief This function searchs for the index of a property name.
 * \param[in] szKey name of the property to search for
 * \return the zero based property index or a SECoP_S_error
 */
int SECoP_S_Node::propertyPosition(QString szKey) const
{
    for (int i = 0; i < m_apProperties.size(); ++i)
        if (szKey.compare(m_apProperties[i]->getKey(), Qt::CaseInsensitive) == 0)
            return i;
    return static_cast<int>(SECoP_S_ERROR_INVALID_PROPERTY);
}

/**
 * \brief This function stores a to-do actio in the global list for the polling
 *        interface.
 * \param[in] qwRequestId unique request id for this action
 * \param[in] pTarget     QT object, which gets invoked for the result
 * \param[in] pModule     the module, which is read, changed or called
 * \param[in] iAction     what is to do
 * \param[in] pParameter  for read/change requests: the parameter
 * \param[in] pCommand    for do requests: the command
 * \param[in] pValue      the new "change" value or "do" argument
 */
void SECoP_S_Node::storeCommand(quint64 qwRequestId, QObject *pTarget, SECoP_S_Module* pModule, enum SECoP_S_action iAction,
                              SECoP_S_Parameter* pParameter, SECoP_S_Command* pCommand, const SECoP_dataPtr pValue)
{
    SECoP_S_Main::getInstance()->storeCommand(qwRequestId, pTarget, this, pModule, iAction, pParameter, pCommand, pValue);
}

/**
 * \brief Qt event handler when a new client connection comes in. This function
 *        creates a worker and a thread for every connection and moves the
 *        worker into this thread.
 */
void SECoP_S_Node::newClient()
{
    while (m_pServer->hasPendingConnections())
    {
        QTcpSocket* pSocket(m_pServer->nextPendingConnection());
        if (pSocket != nullptr)
        {
            if (m_bChangeable)
            {
                // do not allow clients while configuring the node
                pSocket->close();
                pSocket->deleteLater();
            }
            else
            {
                SECoP_S_Worker* pWorker(new SECoP_S_Worker(pSocket, this));
                m_pMutex->lock();
                m_apClients.append(pWorker);
                m_pMutex->unlock();
                connect(pWorker, SIGNAL(destroyed(QObject*)), this, SLOT(disconnectedClient(QObject*)), Qt::DirectConnection);

                if (SECoP_S_Main::manyThreads())
                {
                    QThread* pWorkerThread(new QThread());
                    pWorker->moveToThread(pWorkerThread);
                    pWorker->quitThreadWhileDelete(true);
                    connect(pWorkerThread, SIGNAL(finished()), pWorkerThread, SLOT(deleteLater()));
                    pWorkerThread->start();
                }
            }
        }
    }
}

/**
 * \brief Qt event handler when a client connection is closed.
 */
void SECoP_S_Node::disconnectedClient(QObject* pObject)
{
    QMutexLocker locker(m_pMutex);
    for (int i = 0; i < m_apClients.size(); ++i)
    {
        QObject* pClient(m_apClients[i]);
        if (pClient == pObject)
            m_apClients.removeAt(i--);
    }
}
