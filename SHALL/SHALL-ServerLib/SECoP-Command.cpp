/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include "SECoP-defines.h"
#include "SECoP-Main.h"
#include "SECoP-Command.h"
#include "SECoP-Property.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define qUtf8Printable(string) QString(string).toUtf8().constData()
#define local_qInfo qDebug
#else
#define local_qInfo qInfo
#endif

/**
 * \brief constructor to create a SECoP_S_Command instance
 * \param[in] szCommandID the name of the command
 * \param[in] pFunction   call this function, when a client invokes the command
 * \param[in] pParent     SECoP module, to which this command belongs to
 */
SECoP_S_Command::SECoP_S_Command(QString szCommandID, SECoP_S_callFunction pFunction, SECoP_S_Module* pParent)
    : m_szCommandID(szCommandID)
    , m_pFunction(pFunction)
    , m_pParent(pParent)
{
    Q_ASSERT(m_pParent != nullptr);
    addPropertyInternal("description", CSECoPbaseType::simpleString("command without description"), true);
    addPropertyInternal("datainfo", CSECoPbaseType::simpleJSON("{\"type\":\"command\"}"), true);
}

/**
 * \brief destructor to delete internal data
 */
SECoP_S_Command::~SECoP_S_Command()
{
    for (auto it = m_apProperties.begin(); it != m_apProperties.end(); ++it)
        delete (*it);
}

/// \return the name of the command
QString SECoP_S_Command::getCommandID() const
{
    return m_szCommandID;
}

/// \return the argument data type hint
SECoP_dataPtr SECoP_S_Command::getArgument() const
{
    return m_pArgument;
}

/// \return the result data type hint
SECoP_dataPtr SECoP_S_Command::getResultType() const
{
    return m_pResultType;
}

/// \return the pointer of the function, which is called when a client invokes this command
SECoP_S_callFunction SECoP_S_Command::getFunction() const
{
    return m_pFunction;
}

/// \return the number of properties
int SECoP_S_Command::getNumberOfProperties() const
{
    return m_apProperties.size();
}

/**
 * \brief SECoP_S_Command::propertyPosition search for a command property name
 *        and returns its index or a negative SECoP_S_error
 * \param[in] szKey name of the property
 * \return the property index for a specific command property or a negative SECoP_S_error
 */
int SECoP_S_Command::propertyPosition(QString szKey) const
{
    for (int i = 0; i < m_apProperties.size(); ++i)
        if (szKey.compare(m_apProperties[i]->getKey(), Qt::CaseInsensitive) == 0)
            return i;
    return static_cast<int>(SECoP_S_ERROR_INVALID_PROPERTY);
}

/**
 * \brief SECoP_S_Command::getProperty returns a pointer to a property instance
 * \param[in] iProperty valid property index
 * \return the pointer to a property instance or nullptr
 */
const SECoP_S_Property* SECoP_S_Command::getProperty(int iProperty) const
{
    if (iProperty >= 0 && iProperty < m_apProperties.size())
        return m_apProperties[iProperty];
    else
        return nullptr;
}

/**
 * \brief add a SECoP property to this command
 * \param[in] szKey  name of the property
 * \param[in] pValue value of the property
 * \return returns SECoP_S_SUCCESS or a SECoP error code
 */
enum SECoP_S_error SECoP_S_Command::addProperty(QString szKey, const SECoP_dataPtr pValue)
{
    return addPropertyInternal(szKey, pValue, false);
}

/**
 * \brief internal function to add a SECoP property to this command
 * \param[in] szKey      name of the property
 * \param[in] pValue     value of the property
 * \param[in] bAutomatic automatic properties may be overwritten by user
 * \return returns SECoP_S_SUCCESS or a SECoP error code
 */
enum SECoP_S_error SECoP_S_Command::addPropertyInternal(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    SECoP_dataPtr pNewValue;
    local_qInfo("SECoP_S_Command::addProperty(szKey=%s szValue=%s), command=%s",
                qUtf8Printable(szKey), qUtf8Printable(pValue->exportSECoP()), qUtf8Printable(m_szCommandID));
    if (!SECoP_S_Main::isValidName(szKey))
        return SECoP_S_ERROR_INVALID_NAME;
    int iPos(propertyPosition(szKey));
    if (iPos >= 0)
        if (!m_apProperties[iPos]->isAuto())
            return SECoP_S_ERROR_NAME_ALREADY_USED;

    if (!szKey.startsWith('_'))
    {
        const CSECoPbaseType* pTmp(nullptr);
        if (pValue != nullptr)
            pTmp = pValue.get();
        if (pTmp == nullptr || pTmp->getType() == SECoP_VT_NONE)
            iResult = SECoP_S_WARNING_NO_DESCRIPTION;
        if (SECoP_S_Property::SECoPPropertyListHas(szKey, "Command") < 0)
            iResult = SECoP_S_WARNING_CUSTOM_PROPERTY;
    }

    SECoP_dataPtr pArgument, pResult;
    if (szKey.compare("datainfo", Qt::CaseInsensitive) == 0)
    {
        // convert argument/result data type
        CSECoPstring* pString(dynamic_cast<CSECoPstring*>(pValue.get()));
        CSECoPcommand* pCommand(nullptr);
        SECoP_dataPtr pTmpValue;
        if (pString != nullptr && pString->getType() == SECoP_VT_JSON)
        {
            pTmpValue = SECoP_dataPtr(CSECoPbaseType::createSECoP(pString->getValue().constData(), true));
            pCommand = dynamic_cast<CSECoPcommand*>(pTmpValue.get());
        }
        else
            pCommand = dynamic_cast<CSECoPcommand*>(pValue.get());
        if (pCommand == nullptr)
            return SECoP_S_ERROR_INVALID_VALUE;
        pArgument = SECoP_dataPtr(pCommand->getArgument()->duplicate());
        pResult   = SECoP_dataPtr(pCommand->getResult()->duplicate());

        pString = new CSECoPstring(SECoP_VT_JSON);
        if (pString == nullptr)
            return SECoP_S_ERROR_NO_MEMORY;
        pString->setValue(QByteArray::fromStdString(pCommand->exportType().
                          dump(-1, ' ', false, nlohmann::json::error_handler_t::replace)));
        pNewValue = SECoP_dataPtr(pString);
    }

    SECoP_S_Property* pProperty(nullptr);
    if (iPos >= 0)
    {
        if (!m_apProperties[iPos]->setValue(pNewValue.get() != nullptr ? pNewValue : pValue))
            iResult = SECoP_S_ERROR_NAME_ALREADY_USED;
        else
            pProperty = m_apProperties[iPos];
    }
    else
    {
        pProperty = new SECoP_S_Property(szKey, pNewValue.get() != nullptr ? pNewValue : pValue, bAutomatic);
        if (pProperty != nullptr)
            m_apProperties.append(pProperty);
        else
            iResult = SECoP_S_ERROR_NO_MEMORY;
    }
    if (pProperty != nullptr && iResult >= 0 && pArgument && pResult)
    {
        // use argument/result data type
        m_pArgument   = pArgument;
        m_pResultType = pResult;
    }
    return iResult;
}

/// \return the pointer to the parent module of a command
SECoP_S_Module* SECoP_S_Command::getParentModule() const
{
    Q_ASSERT(m_pParent != nullptr);
    return m_pParent;
}
