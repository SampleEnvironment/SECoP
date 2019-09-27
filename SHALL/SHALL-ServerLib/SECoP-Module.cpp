/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <cmath>
#include <QDebug>
#include <QMutexLocker>
#include <QTimerEvent>
#include "SECoP-Command.h"
#include "SECoP-Main.h"
#include "SECoP-Module.h"
#include "SECoP-Node.h"
#include "SECoP-Parameter.h"
#include "SECoP-Property.h"
#include "SECoP-Variant.h"
#include "SECoP-Worker.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define qUtf8Printable(string) QString(string).toUtf8().constData()
#define local_qInfo qDebug
#else
#define local_qInfo qInfo
#endif

/**
 * \brief constructor of SECoP_S_Module creates a SECoP module instance with some
 *        default properties, which should be changed.
 * \param[in] szModuleName name of the module
 * \param[in] pNode        parent node of the module
 * \param[in] pParent      Qt parent class (should be Q_NULLPTR)
 */
SECoP_S_Module::SECoP_S_Module(QString szModuleName, SECoP_S_Node *pNode, QObject *pParent)
    : QObject(pParent)
    , m_pNode(pNode)
    , m_bQuitThread(false)
    , m_szModuleID(szModuleName)
    , m_iPollTimerId(0)
    , m_iWantedInterval(SECOP_DEFAULT_POLLINTERVAL)
    , m_iPollInterval(0)
    , m_iPollTime(0)
    , m_iParameterFocus(-1)
    , m_iCommandFocus(-1)
    , m_qwRequestId(10)
{
    enum SECoP_S_error iError(SECoP_S_SUCCESS);
    m_pMutex = new QMutex(QMutex::Recursive);
    addProperty("description", CSECoPbaseType::simpleString("module without description"), true, &iError);
    addProperty("pollinterval", CSECoPbaseType::simpleDouble(SECOP_DEFAULT_POLLINTERVAL / 1000.0), true, &iError);
}

/**
 * \brief destructor of SECoP_S_Module, it destroys all child parameters,
 *        commands and properties. If enabled, it also triggers the exit of the
 *        Qt thread, which hosted this module.
 */
SECoP_S_Module::~SECoP_S_Module()
{
    if (m_iPollTimerId > 0)
        killTimer(m_iPollTimerId);
    for (auto it = m_apCommands.begin(); it != m_apCommands.end(); ++it)
        delete (*it);
    for (auto it = m_apParameters.begin(); it != m_apParameters.end(); ++it)
        delete (*it);
    for (auto it = m_apProperties.begin(); it != m_apProperties.end(); ++it)
        delete (*it);
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

/// \returns the module name
QString SECoP_S_Module::getModuleID() const
{
    return m_szModuleID;
}

/// \returns the printable active element
QString SECoP_S_Module::getPrintableActive(bool bWithAccessible) const
{
    QString szResult(QString("module \"%1\"").arg(m_szModuleID));
    if (bWithAccessible)
    {
        if (m_iCommandFocus >= 0)
            szResult.append(QString(" command \"%1\"").arg(m_apCommands[m_iCommandFocus]->getCommandID()));
        if (m_iParameterFocus >= 0)
            szResult.append(QString(" parameter \"%1\"").arg(m_apParameters[m_iParameterFocus]->getParameterID()));
    }
    return szResult;
}

/**
 * \brief This function creates a new command inside this module.
 * \param[in] szKey    name of the command, which is unique inside the SECoP module
 * \param[in] ptrToCmd function, which is called when a client invokes the command
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Module::addCommand(QString szKey, SECoP_S_callFunction ptrToCmd)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (QThread::currentThread() == thread())
        addCommand(szKey, ptrToCmd, &iResult);
    else
        QMetaObject::invokeMethod(this, "addCommand", Qt::BlockingQueuedConnection, Q_ARG(QString, szKey),
                                  Q_ARG(SECoP_S_callFunction, ptrToCmd), Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a new command inside this module.
 * \param[in]  szKey    name of the command, which is unique inside the SECoP module
 * \param[in]  ptrToCmd function, which is called when a client invokes the command
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Module::addCommand(QString szKey, SECoP_S_callFunction ptrToCmd, SECoP_S_error* piResult)
{
    if (!SECoP_S_Main::isValidName(szKey))
        *piResult = SECoP_S_ERROR_INVALID_NAME;
    else if (commandPosition(szKey) < 0)
    {
        m_iParameterFocus = m_iCommandFocus = -1;
        SECoP_S_Command* pCommand(new SECoP_S_Command(szKey, ptrToCmd, this));
        if (pCommand == nullptr)
            *piResult = SECoP_S_ERROR_NO_MEMORY;
        else
        {
            m_iCommandFocus = m_apCommands.size();
            m_apCommands.append(pCommand);
            struct SECoP_S_Module::s_Accessibles entry;
            entry.name     = szKey;
            entry.command  = true;
            entry.position = commandPosition(szKey);
            m_AccList.append(entry);
            *piResult = SECoP_S_SUCCESS;
        }
    }
    else
        *piResult = SECoP_S_ERROR_NAME_ALREADY_USED;
}

/**
 * \brief This function creates a read only SECoP parameter inside this module.
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Module::addReadableParameter(QString szName, SECoP_S_getsetFunction ptrToGet)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (QThread::currentThread() == thread())
        addReadableParameter(szName, ptrToGet, &iResult);
    else
        QMetaObject::invokeMethod(this, "addReadableParameter", Qt::BlockingQueuedConnection, Q_ARG(QString, szName),
                                  Q_ARG(SECoP_S_getsetFunction, ptrToGet), Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a read only SECoP parameter inside this module.
 * \param[in]  szName   name of the parameter, which is unique inside the SECoP module
 * \param[in]  ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Module::addReadableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_error* piResult)
{
    if (!SECoP_S_Main::isValidName(szName))
        *piResult = SECoP_S_ERROR_INVALID_NAME;
    else if (parameterPosition(szName) < 0)
    {
        m_iParameterFocus = m_iCommandFocus = -1;
        SECoP_S_Parameter* pParameter(new SECoP_S_Parameter(szName, false, ptrToGet, nullptr, this));
        if (pParameter == nullptr)
            *piResult = SECoP_S_ERROR_NO_MEMORY;
        else
        {
            m_iParameterFocus = m_apParameters.size();
            m_apParameters.append(pParameter);
            pParameter->addProperty("readonly", CSECoPbaseType::simpleBool(true));
            struct SECoP_S_Module::s_Accessibles entry;
            entry.name     = szName;
            entry.command  = false;
            entry.position = parameterPosition(szName);
            m_AccList.append(entry);
            *piResult = SECoP_S_SUCCESS;
        }
    }
    else
        *piResult = SECoP_S_ERROR_NAME_ALREADY_USED;
}

/**
 * \brief This function creates a read- and writable SECoP parameter inside this module.
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[in] ptrToSet function, which is called when a client wants to set a new value
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Module::addWritableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (QThread::currentThread() == thread())
        addWritableParameter(szName, ptrToGet, ptrToSet, &iResult);
    else
        QMetaObject::invokeMethod(this, "addWritableParameter", Qt::BlockingQueuedConnection, Q_ARG(QString, szName),
                                  Q_ARG(SECoP_S_getsetFunction, ptrToGet), Q_ARG(SECoP_S_getsetFunction, ptrToSet),
                                  Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a read- and writable SECoP parameter inside this module.
 * \param[in]  szName   name of the parameter, which is unique inside the SECoP module
 * \param[in]  ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[in]  ptrToSet function, which is called when a client wants to set a new value
 * \param[out] piResult on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Module::addWritableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet,
                                          SECoP_S_error* piResult)
{
    if (!SECoP_S_Main::isValidName(szName))
        *piResult = SECoP_S_ERROR_INVALID_NAME;
    else if (parameterPosition(szName) < 0)
    {
        m_iParameterFocus = m_iCommandFocus = -1;
        SECoP_S_Parameter* pParameter(new SECoP_S_Parameter(szName, true, ptrToGet, ptrToSet, this));
        if (pParameter == nullptr)
            *piResult = SECoP_S_ERROR_NO_MEMORY;
        else
        {
            m_iParameterFocus = m_apParameters.size();
            m_apParameters.append(pParameter);
            pParameter->addProperty("readonly", CSECoPbaseType::simpleBool(false));
            struct SECoP_S_Module::s_Accessibles entry;
            entry.name     = szName;
            entry.command  = false;
            entry.position = parameterPosition(szName);
            m_AccList.append(entry);
            *piResult = SECoP_S_SUCCESS;
        }
    }
    else
        *piResult = SECoP_S_ERROR_NAME_ALREADY_USED;
}

/**
 * \brief This function creates a SECoP property to this module or last created
 *        command or parameter.
 * \param[in] szKey  name of the property, which is unique inside its context.
 *                   You may prepend an unterscore for own properties
 * \param[in] pValue value of the property; note: use the recommended data type
 *                   for the standard SECoP properties
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Module::addProperty(QString szKey, const SECoP_dataPtr pValue)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    if (QThread::currentThread() == thread())
        addProperty(szKey, pValue, false, &iResult);
    else
        QMetaObject::invokeMethod(this, "addProperty", Qt::BlockingQueuedConnection, Q_ARG(QString, szKey),
                                  Q_ARG(const SECoP_dataPtr, pValue), Q_ARG(bool, false), Q_ARG(SECoP_S_error*, &iResult));
    return iResult;
}

/**
 * \brief This function creates a SECoP property to this module or last created
 *        command or parameter.
 * \param[in]  szKey      name of the property, which is unique inside its context.
 *                        You may prepend an unterscore for own properties
 * \param[in]  pValue     value of the property; note: use the recommended data type
 *                        for the standard SECoP properties
 * \param[in]  bAutomatic false: finally create property, true: create overwritable property
 * \param[out] piResult   on success SECoP_S_SUCCESS or a SECoP_S_error
 */
void SECoP_S_Module::addProperty(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic, SECoP_S_error* piResult)
{
    int iWantedInterval(-1);
    local_qInfo("SECoP_S_Module::addProperty(szKey=%s szValue=%s), module=%s parameter=%d/%s command=%d/%s",
                qUtf8Printable(szKey), qUtf8Printable(pValue->exportSECoP()), qUtf8Printable(m_szModuleID),
                m_iParameterFocus, m_iParameterFocus >=0 ? qUtf8Printable(m_apParameters[m_iParameterFocus]->getParameterID()) : "?",
                m_iCommandFocus, m_iCommandFocus >=0 ? qUtf8Printable(m_apCommands[m_iCommandFocus]->getCommandID()) : "?");
    if (!SECoP_S_Main::isValidName(szKey))
    {
        *piResult = SECoP_S_ERROR_INVALID_NAME;
        return;
    }
    if (m_iParameterFocus >= 0)
    {
        *piResult = m_apParameters[m_iParameterFocus]->addProperty(szKey, pValue);
        return;
    }
    if (m_iCommandFocus >= 0)
    {
        *piResult = m_apCommands[m_iCommandFocus]->addProperty(szKey, pValue);
        return;
    }
    if (szKey.compare("accessibles", Qt::CaseInsensitive) == 0)
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
        if (SECoP_S_Property::SECoPPropertyListHas(szKey, "Module") < 0)
            *piResult = SECoP_S_WARNING_CUSTOM_PROPERTY;
    }

    if (szKey.compare("pollinterval", Qt::CaseInsensitive) == 0)
    {
        // convert poll interval
        CSECoPsimpleInt* pInt(dynamic_cast<CSECoPsimpleInt*>(pValue.get()));
        if (pInt != nullptr)
        {
            long long llValue;
            if (pInt->getValue(llValue))
                iWantedInterval = static_cast<int>(1000 * llValue);
        }
        CSECoPsimpleDouble* pDouble(dynamic_cast<CSECoPsimpleDouble*>(pValue.get()));
        if (pDouble != nullptr)
        {
            double dblValue;
            if (pDouble->getValue(dblValue))
                iWantedInterval = static_cast<int>(1000.0 * dblValue);
        }
        if (iWantedInterval < 0)
        {
            *piResult = SECoP_S_ERROR_INVALID_VALUE;
            return;
        }
        else if (iWantedInterval > SECOP_MAX_POLLINTERVAL)
            iWantedInterval = SECOP_MAX_POLLINTERVAL;
    }

    SECoP_S_Property* pProperty(nullptr);
    if (iPos >= 0)
    {
        if (!m_apProperties[iPos]->setValue(pValue))
            *piResult = SECoP_S_ERROR_NAME_ALREADY_USED;
        else
            pProperty = m_apProperties[iPos];
    }
    else
    {
        pProperty = new SECoP_S_Property(szKey, pValue, bAutomatic);
        if (pProperty == nullptr)
            *piResult = SECoP_S_ERROR_NO_MEMORY;
        else
            m_apProperties.append(pProperty);
    }

    // store poll interval
    if (pProperty != nullptr && iWantedInterval >= 0 && *piResult >= 0)
        m_iWantedInterval = iWantedInterval;
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
enum SECoP_S_error SECoP_S_Module::setAddFocus(QString szKey)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
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
void SECoP_S_Module::setAddFocus(QString szKey, SECoP_S_error* piResult)
{
    int iPos, iParamIndex, iCmdIndex;
    if (szKey.isEmpty())
    {
        m_iParameterFocus = -1;
        m_iCommandFocus   = -1;
        *piResult         = SECoP_S_SUCCESS;
        return;
    }

    *piResult = SECoP_S_ERROR_INVALID_PARAMETER;
    iPos = szKey.indexOf(':');
    if (iPos < 0)
    {
        iParamIndex = parameterPosition(szKey);
        iCmdIndex   = commandPosition(szKey);
        szKey.clear();
    }
    else
    {
        iParamIndex = parameterPosition(szKey.left(iPos));
        iCmdIndex   = commandPosition(szKey.left(iPos));
        szKey.remove(0, iPos + 1);
    }
    if (iParamIndex >= 0)
    {
        if (iParamIndex >= m_apParameters.size())
            return;
        if (m_apParameters[iParamIndex] == nullptr)
            return;
        m_iParameterFocus = iParamIndex;
        m_iCommandFocus   = -1;
    }
    else
    {
        if (iCmdIndex < 0)
            return;
        *piResult = SECoP_S_ERROR_INVALID_COMMAND;
        if (iCmdIndex >= m_apCommands.size())
            return;
        if (m_apCommands[iCmdIndex] == nullptr)
            return;
        m_iParameterFocus = -1;
        m_iCommandFocus   = iCmdIndex;
    }
    *piResult = SECoP_S_SUCCESS;
}

/**
 * \brief This function is called by SECoP_S_nodeComplete to inform this module,
 *        that the SECoP node is complete. This function starts the polling
 *        timer for this module.
 */
void SECoP_S_Module::nodeIsComplete()
{
    if (QThread::currentThread() == thread())
        nodeIsCompleteSlot();
    else
        QMetaObject::invokeMethod(this, "nodeIsCompleteSlot", Qt::BlockingQueuedConnection);
}

/**
 * \brief This function is called by SECoP_S_nodeComplete to inform this module,
 *        that the SECoP node is complete. This function starts the polling
 *        timer for this module.
 */
void SECoP_S_Module::nodeIsCompleteSlot()
{
    if (m_iPollTimerId > 0)
        killTimer(m_iPollTimerId);

    // find minimal poll interval for parameter polling
    m_iPollInterval = m_iWantedInterval;
    for (auto it = m_apParameters.constBegin(); it != m_apParameters.constEnd(); ++it)
    {
        int iInterval((*it)->pollinterval());
        if (iInterval > 0 && (m_iPollInterval <= 0 || iInterval < m_iPollInterval))
            m_iPollInterval = iInterval;
    }
    if (m_iPollInterval > 0)
    {
        if (m_iPollInterval < 10)
            m_iPollInterval = 10; // not more than 100Hz
        else if (m_iPollInterval > SECOP_MAX_POLLINTERVAL)
            m_iPollInterval = SECOP_MAX_POLLINTERVAL; // not longer than 1 hour
        m_iPollTimerId = startTimer(m_iPollInterval);
    }
    else
        m_iPollTimerId = 0;
}

/**
 * \brief This function informs the module, if it should while deletion quit
 *        the thread, which hosts this module.
 * \param[in] bQuit true: quit thread too, false: do nothing with threads
 */
void SECoP_S_Module::quitThreadWhileDelete(bool bQuit)
{
    m_bQuitThread = bQuit;
}

/// \returns the number of accessibles (number of parameters and commands together)
int SECoP_S_Module::getNumberOfAccessibles() const
{
    return m_AccList.size();
}

/// \returns the number of properties this module has
int SECoP_S_Module::getNumberOfProperties() const
{
    return m_apProperties.size();
}

/// \returns the position as positive number if the parameter exists otherwise returns SECoP_S_ERROR_INVALID_PARAMETER
int SECoP_S_Module::parameterPosition(QString szParameterName) const
{
    for (int i = 0; i < m_apParameters.size(); i++)
        if (szParameterName.compare(m_apParameters[i]->getParameterID(), Qt::CaseInsensitive) == 0)
            return i;
    return static_cast<int>(SECoP_S_ERROR_INVALID_PARAMETER);
}

/// \returns the position as positive number if the command exists otherwise returns SECoP_S_ERROR_INVALID_COMMAND
int SECoP_S_Module::commandPosition(QString szCommandName) const
{
    for (int i = 0; i < m_apCommands.size(); i++)
        if (szCommandName.compare(m_apCommands[i]->getCommandID(), Qt::CaseInsensitive) == 0)
            return i;
    return static_cast<int>(SECoP_S_ERROR_INVALID_COMMAND);
}

/// \returns the position as positive number if the property exists otherwise returns SECoP_S_ERROR_INVALID_PROPERTY
int SECoP_S_Module::propertyPosition(QString szKey) const
{
    for (int i = 0; i < m_apProperties.size(); ++i)
        if (szKey.compare(m_apProperties[i]->getKey(), Qt::CaseInsensitive) == 0)
            return i;
    return static_cast<int>(SECoP_S_ERROR_INVALID_PROPERTY);
}

struct SECoP_S_Module::s_Accessibles* SECoP_S_Module::getAccessibles(int iAcc)
{
    if (iAcc >= 0 && iAcc < m_AccList.size())
        return &m_AccList[iAcc];
    else
        return nullptr;
}

/// \returns the instance of the parameter selected by index or nullptr
SECoP_S_Parameter* SECoP_S_Module::getParameter(int iParameter) const
{
    if (iParameter >= 0 && iParameter < m_apParameters.size())
        return m_apParameters[iParameter];
    else
        return nullptr;
}

/// \returns the instance of the command selected by index or nullptr
SECoP_S_Command* SECoP_S_Module::getCommand(int iCommand) const
{
    if (iCommand >= 0 && iCommand < m_apCommands.size())
        return m_apCommands[iCommand];
    else
        return nullptr;
}

/// \returns the instance of the property selected by index or nullptr
const SECoP_S_Property* SECoP_S_Module::getProperty(int iProperty) const
{
    if (iProperty >= 0 && iProperty < m_apProperties.size())
        return m_apProperties[iProperty];
    else
        return nullptr;
}

/**
 * \brief This function triggers a read request for a parameter.
 * \param[in]  pParameter   parameter instance to read
 * \param[out] pqwRequestId request id
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Module::readParameter(SECoP_S_Parameter* pParameter, quint64* pqwRequestId)
{
    // check node
    if (m_pNode == nullptr)
        return SECoP_S_ERROR_NOT_IMPLEMENTED;
    // check parameter
    if (pParameter->isConstant())
        return SECoP_S_ERROR_INVALID_PARAMETER;
    // parameter exists, check node configuration
    if (!SECoP_S_Main::hasFunctionPointers())
    {
        m_pNode->storeCommand(*pqwRequestId = nextRequestId(), nullptr, this, SECoP_S_ACTION_READ, pParameter, nullptr, SECoP_dataPtr());
        return SECoP_S_SUCCESS;
    }
    // check getter function
    if (pParameter->getter() == nullptr)
        return SECoP_S_ERROR_NO_GETTER;
    QMetaObject::invokeMethod(this, "readParameter", Qt::QueuedConnection, Q_ARG(SECoP_S_Parameter*, pParameter),
                              Q_ARG(quint64, *pqwRequestId = nextRequestId()));
    return SECoP_S_SUCCESS;
}

/**
 * \brief This function implements the triggered read request for a parameter.
 *        After completion this function triggers the sending of the result.
 * \param[in] pParameter  parameter instance to read
 * \param[in] qwRequestId request id
 */
void SECoP_S_Module::readParameter(SECoP_S_Parameter* pParameter, quint64 qwRequestId)
{
    double dblTimestamp(std::numeric_limits<double>::quiet_NaN());
    enum SECoP_S_error iErrorCode(SECoP_S_SUCCESS);
    SECoP_S_getsetFunction pGetter(pParameter->getter());
    CSECoPbaseType* pData(nullptr);
    CSECoPbaseType* pSigma(nullptr);
    SECoP_dataPtr pValueOut(pParameter->value()->duplicate());
    SECoP_dataPtr pSigmaOut;
    if (pGetter == nullptr)
        iErrorCode = SECoP_S_ERROR_NO_GETTER;
    else
    {
        // allocate memory for parameter data
        if (pValueOut == nullptr || pValueOut.get() == nullptr)
            pValueOut = SECoP_dataPtr(new CSECoPnull);
        pData = pValueOut.get();
        if (pData == nullptr || !pData->isValid())
        {
            iErrorCode = SECoP_S_ERROR_NO_MEMORY;
            pValueOut = nullptr;
        }
        else
        {
            // call getter function
            (*pGetter)(qUtf8Printable(QString("%1:%2:%3").arg(m_pNode->getNodeID()).arg(m_szModuleID).arg(pParameter->getParameterID())),
                       &iErrorCode, &pData, &pSigma, &dblTimestamp);
            if (iErrorCode == SECoP_S_SUCCESS)
            {
                if (pData != pValueOut.get())
                    pValueOut = SECoP_dataPtr(pData);
                if (pSigma != nullptr)
                    pSigmaOut = SECoP_dataPtr(pSigma);
            }
            else
            {
                pValueOut = nullptr;
                pSigmaOut = nullptr;
            }
        }
    }
    doParameterResult(iErrorCode, qwRequestId, pParameter, pValueOut, pSigmaOut, dblTimestamp);
}

/**
 * \brief This function triggers a change request for a parameter.
 * \param[in]  pParameter   parameter instance to change
 * \param[in]  pValue       new value
 * \param[out] pqwRequestId request id
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Module::changeParameter(SECoP_S_Parameter* pParameter, const SECoP_dataPtr pValue, quint64* pqwRequestId)
{
    // check node
    if (m_pNode == nullptr)
        return SECoP_S_ERROR_NOT_IMPLEMENTED;
    // check parameter
    if (pParameter->isConstant())
        return SECoP_S_ERROR_INVALID_PARAMETER;
    if (!pParameter->isWritable())
        return SECoP_S_ERROR_READONLY;
    // check node configuration
    if (!SECoP_S_Main::hasFunctionPointers())
    {
        m_pNode->storeCommand(*pqwRequestId = nextRequestId(), nullptr, this, SECoP_S_ACTION_CHANGE, pParameter, nullptr, pValue);
        return SECoP_S_SUCCESS;
    }
    // check setter function
    if (pParameter->setter() == nullptr)
        return SECoP_S_ERROR_NO_SETTER;
    QMetaObject::invokeMethod(this, "changeParameter", Qt::QueuedConnection, Q_ARG(SECoP_S_Parameter*, pParameter),
                              Q_ARG(const SECoP_dataPtr, pValue), Q_ARG(quint64, *pqwRequestId = nextRequestId()));
    return SECoP_S_SUCCESS;
}

/**
 * \brief This function implements the triggered change request for a parameter.
 *        After completion this function triggers the sending of the result.
 * \param[in] pParameter  parameter instance to read
 * \param[in] pValue      new value
 * \param[in] qwRequestId request id
 */
void SECoP_S_Module::changeParameter(SECoP_S_Parameter* pParameter, const SECoP_dataPtr pValue, quint64 qwRequestId)
{
    double dblTimestamp(std::numeric_limits<double>::quiet_NaN());
    local_qInfo().nospace() << "changeModuleParameter";
    enum SECoP_S_error iErrorCode(SECoP_S_SUCCESS);
    SECoP_S_getsetFunction pSetter(pParameter->setter());
    CSECoPbaseType* pData(nullptr);
    SECoP_dataPtr pInOutValue(pValue);
    if (pSetter == nullptr)
    {
        iErrorCode = SECoP_S_ERROR_NO_SETTER;
        goto finish;
    }
    // allocate memory for in/out value
    if (pInOutValue != nullptr && pInOutValue.get() != nullptr)
    {
        pData = pInOutValue.get();
        if (pData == nullptr || !pData->isValid())
        {
            iErrorCode = SECoP_S_ERROR_NO_MEMORY;
            goto finish;
        }
    }
    // call setter function
    (*pSetter)(qUtf8Printable(QString("%1:%2:%3").arg(m_pNode->getNodeID()).arg(m_szModuleID).arg(pParameter->getParameterID())),
               &iErrorCode, &pData, nullptr, &dblTimestamp);
    if (iErrorCode == SECoP_S_SUCCESS)
        if (pData != pInOutValue.get())
            pInOutValue = SECoP_dataPtr(pData); // setter created new memory
finish:
    doParameterResult(iErrorCode, qwRequestId, pParameter, pInOutValue, SECoP_dataPtr(), dblTimestamp);
}

/**
 * \brief This function informs, that a parameter value has been changed.
 * \param[in] iParameter   index of the parameter which has been changed
 * \param[in] pValue       new value of the parameter
 * \param[in] pSigma       error of value of the parameter
 * \param[in] dblTimestamp timestamp of the change
 */
void SECoP_S_Module::updateParameter(int iParameter, SECoP_dataPtr pValue, SECoP_dataPtr pSigma, double dblTimestamp)
{
    if (iParameter >= 0 && iParameter < m_apParameters.size())
    {
        SECoP_S_Parameter* pParameter(m_apParameters[iParameter]);
        if (pParameter != nullptr)
            doParameterResult(SECoP_S_SUCCESS, 0, pParameter, pValue, pSigma, dblTimestamp);
    }
}

/**
 * \brief This function will be called, after a parameter was read or changed.
 *        It stores the current value in cache and sends the new value to
 *        interested parties.
 * \param[in] iErrorCode   result of the read or change
 * \param[in] qwRequestId  the module request id
 * \param[in] pParameter   the parameter instance, which has a new value
 * \param[in] pValue       the new value
 * \param[in] pSigma       error of value of the parameter
 * \param[in] dblTimestamp timestamp of the value
 */
void SECoP_S_Module::doParameterResult(enum SECoP_S_error iErrorCode, quint64 qwRequestId, SECoP_S_Parameter* pParameter,
                                       const SECoP_dataPtr pValue, const SECoP_dataPtr pSigma, double dblTimestamp)
{
    using std::isnan;
    using std::isinf;
    if (isnan(dblTimestamp) || isinf(dblTimestamp))
        dblTimestamp = SECoP_S_Main::getCurrentTime();
    if (iErrorCode >= 0)
        pParameter->setValue(pValue, pSigma, dblTimestamp);
    emit newParameterValue(pParameter, qwRequestId, iErrorCode, pValue, pSigma, dblTimestamp);
}

/**
 * \brief This function is triggered for a command call to this module.
 * \param[in] pCommand  command instance to call
 * \param[in] pArgument input argument
 * \param[in] pSender   send results to this Qt object (normally a SECoP_S_Worker)
 */
void SECoP_S_Module::doCommand(SECoP_S_Command* pCommand, const SECoP_dataPtr pArgument, QObject* pSender)
{
    // if you are here, the commando exists
    enum SECoP_S_error iErrorCode(SECoP_S_SUCCESS);
    CSECoPbaseType* pDataOut(nullptr);
    SECoP_dataPtr pValueOut;
    double dblTimestamp(std::numeric_limits<double>::quiet_NaN());
    if (!SECoP_S_Main::hasFunctionPointers())
    {
        // store command for polling
        m_pNode->storeCommand(0, pSender, this, SECoP_S_ACTION_DO, nullptr, pCommand, pArgument);
        return;
    }
    SECoP_S_callFunction pFunction(pCommand->getFunction());
    if (pFunction != nullptr)
    {
        // get result hint
        pValueOut = SECoP_dataPtr(pCommand->getResultType()->duplicate());
        if (pValueOut == nullptr || pValueOut.get() == nullptr)
            pValueOut = SECoP_dataPtr(new CSECoPnull);
        pDataOut = pValueOut.get();
        SECoP_dataPtr pArgumentTmp(pArgument);
        if (pArgumentTmp == nullptr || pArgumentTmp.get() == nullptr)
            pArgumentTmp = SECoP_dataPtr(new CSECoPnull);
        // do function call
        (*pFunction)(qUtf8Printable(QString("%1:%2:%3").arg(m_pNode->getNodeID()).arg(m_szModuleID).arg(pCommand->getCommandID())),
                     pArgumentTmp.get(), &iErrorCode, &pDataOut, &dblTimestamp);

        if (iErrorCode == SECoP_S_SUCCESS)
        {
            if (pDataOut != pValueOut.get())
                pValueOut = SECoP_dataPtr(pDataOut); // setter created new memory
        }
        else
            pValueOut = nullptr;
    }
    else
        iErrorCode = SECoP_S_ERROR_INVALID_COMMAND;
    doCommandResult(pSender, iErrorCode, pCommand, pValueOut, dblTimestamp);
}

/**
 * \brief This function is called after the called command has been finished
 *        and it triggers the sending of the result.
 * \param[in] pTarget      Qt object to inform
 * \param[in] iErrorCode   error code of the command invocation
 * \param[in] pCommand     command which was invoked
 * \param[in] pValue       return value from command call
 * \param[in] dblTimestamp timestamp of command call
 */
void SECoP_S_Module::doCommandResult(QObject* pTarget, enum SECoP_S_error iErrorCode, SECoP_S_Command* pCommand,
                                     const SECoP_dataPtr pValue, double dblTimestamp)
{
    if (pTarget != nullptr)
    {
        using std::isnan;
        using std::isinf;
        if (isnan(dblTimestamp) || isinf(dblTimestamp))
            dblTimestamp = SECoP_S_Main::getCurrentTime();
        QMetaObject::invokeMethod(pTarget, "doneCommand", Qt::QueuedConnection, Q_ARG(SECoP_S_Command*, pCommand),
                                  Q_ARG(SECoP_S_error, iErrorCode), Q_ARG(const SECoP_dataPtr, pValue), Q_ARG(double, dblTimestamp));
    }
}

/**
 * \brief This function is called regulary to poll all/selected parameters
 * \param[in] pEvent timer event information
 */
void SECoP_S_Module::timerEvent(QTimerEvent* pEvent)
{
    quint64 qwRequestId(0);
    if (pEvent == nullptr)
        return;
    if (pEvent->timerId() == m_iPollTimerId && m_iPollInterval > 0)
    {
        // check, if any parameter (or the complete module) should be polled
        bool bModulePoll(false);
        if (m_iWantedInterval > 0)
        {
            m_iPollTime += m_iPollInterval;
            if (m_iPollTime >= m_iWantedInterval)
            {
                // module should be polled
                bModulePoll = true;
                m_iPollTime %= m_iWantedInterval;
            }
        }
        for (auto it = m_apParameters.begin(); it != m_apParameters.end(); ++it)
        {
            SECoP_S_Parameter* pParameter(*it);
            bool bPoll(bModulePoll);
            if (pParameter->pollinterval() > 0)
                bPoll = pParameter->poll(m_iPollInterval);
            if (bPoll) // poll parameter
                readParameter(pParameter, &qwRequestId);
        }
    }
}

/// \returns a new request id for parameter read or change
quint64 SECoP_S_Module::nextRequestId()
{
    // get a request number with minimum of 10
    QMutexLocker locker(m_pMutex);
    if (m_qwRequestId < 10)
        m_qwRequestId = 10;
    return m_qwRequestId++;
}
