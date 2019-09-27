/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <cmath>
#include "SECoP-Main.h"
#include "SECoP-Module.h"
#include "SECoP-Parameter.h"
#include "SECoP-Property.h"
#include "SECoP-Variant.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define qUtf8Printable(string) QString(string).toUtf8().constData()
#define local_qInfo qDebug
#else
#define local_qInfo qInfo
#endif

/**
 * \brief constructor of SECoP_S_Parameter creates a SECoP parameter instance
 *        with some default properties, which should be changed.
 * \param[in] szParameterID name if the parameter
 * \param[in] bWritable     true, if the parameter if writable
 * \param[in] pGet          pointer to the function to read the current value
 * \param[in] pSet          pointer to the function for changing the value
 * \param[in] pParent       parent module (Qt parent class)
 */
SECoP_S_Parameter::SECoP_S_Parameter(QString szParameterID, bool bWritable, SECoP_S_getsetFunction pGet, SECoP_S_getsetFunction pSet, SECoP_S_Module* pParent)
    : QObject(pParent)
    , m_szParameterID(szParameterID)
    , m_bWritable(bWritable)
    , m_bHasValue(false)
    , m_bConstant(false)
    , m_pGet(pGet)
    , m_pSet(pSet)
    , m_dblTimestamp(std::numeric_limits<double>::quiet_NaN())
    , m_iPollInterval(0)
    , m_iPollTime(0)
{
    Q_ASSERT(pParent != nullptr);
    addPropertyInternal("description", CSECoPbaseType::simpleString("parameter without description"), true);
}

/**
 * \brief destructor of SECoP_S_Parameter, it destroys all child properties and
 *        the cached value.
 */
SECoP_S_Parameter::~SECoP_S_Parameter()
{
    for (auto it = m_apProperties.begin(); it != m_apProperties.end(); ++it)
        delete (*it);
}

/// \returns the number of properties this parameter has
int SECoP_S_Parameter::getNumberOfProperties() const
{
    return m_apProperties.size();
}

/// \returns the position as positive number if the property exists otherwise returns SECoP_S_ERROR_INVALID_PROPERTY
int SECoP_S_Parameter::propertyPosition(QString szKey) const
{
    for (int i = 0; i < m_apProperties.size(); ++i)
        if (szKey.compare(m_apProperties[i]->getKey(), Qt::CaseInsensitive) == 0)
            return i;
    return static_cast<int>(SECoP_S_ERROR_INVALID_PROPERTY);
}

/// \returns the instance of the property selected by index or nullptr
const SECoP_S_Property* SECoP_S_Parameter::getProperty(int iProperty) const
{
    if (iProperty >= 0 && iProperty < m_apProperties.size())
        return m_apProperties[iProperty];
    else
        return nullptr;
}

/**
 * \brief This function creates a SECoP property to this parameter.
 * \param[in] szKey  name of the property, which is unique in this parameter.
 *                   You may prepend an unterscore for own properties
 * \param[in] pValue value of the property; note: use the recommended data type
 *                   for the standard SECoP properties
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Parameter::addProperty(QString szKey, const SECoP_dataPtr pValue)
{
    return addPropertyInternal(szKey, pValue, false);
}

/**
 * \brief This function creates a SECoP property to this parameter. This
 *        function parses the properties "datainfo" and "pollinterval" and sets
 *        the cached value or the pollinterval.
 * \param[in]  szKey      name of the property, which is unique in this parameter.
 *                        You may prepend an unterscore for own properties
 * \param[in]  pValue     value of the property; note: use the recommended data type
 *                        for the standard SECoP properties
 * \param[in]  bAutomatic false: finally create property, true: create overwritable property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SECoP_S_Parameter::addPropertyInternal(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic)
{
    enum SECoP_S_error iResult(SECoP_S_SUCCESS);
    int iPollInterval(0);
    SECoP_dataPtr pNewValue;
    local_qInfo("SECoP_S_Parameter::addProperty(szKey=%s szValue=%s), parameter=%s",
                qUtf8Printable(szKey), qUtf8Printable(pValue->exportSECoP()), qUtf8Printable(m_szParameterID));
    if (!SECoP_S_Main::isValidName(szKey))
        return SECoP_S_ERROR_INVALID_NAME;
    int iPos(propertyPosition(szKey));
    if (iPos >= 0)
        if (!m_apProperties[iPos]->isAuto())
            return SECoP_S_ERROR_NAME_ALREADY_USED;
    const CSECoPbaseType* pTmpValue(nullptr);
    if (pValue != nullptr)
        pTmpValue = pValue.get();
    if (!szKey.startsWith('_'))
    {
        if (pTmpValue == nullptr || pTmpValue->getType() == SECoP_VT_NONE)
            iResult = SECoP_S_WARNING_NO_DESCRIPTION;
        if (SECoP_S_Property::SECoPPropertyListHas(szKey, "Parameter") < 0)
            iResult = SECoP_S_WARNING_CUSTOM_PROPERTY;
    }

    CSECoPbaseType* pData(nullptr);
    if (szKey.compare("datainfo", Qt::CaseInsensitive) == 0)
    {
        // convert data type description to value
        CSECoPstring* pString(dynamic_cast<CSECoPstring*>(const_cast<CSECoPbaseType*>(pTmpValue)));
        nlohmann::json j;
        if (pString != nullptr)
        {
            pData = CSECoPbaseType::createSECoP(pString->getValue().constData(), false);
            if (pData == nullptr && pString->getType() == SECoP_VT_JSON)
                return SECoP_S_ERROR_INVALID_VALUE;
        }
        if (pData == nullptr)
        {
            pData = pTmpValue->duplicate();
            j = pTmpValue->exportType();
        }
        else
            j = pData->exportType();
        pString = new CSECoPstring(SECoP_VT_JSON);
        if (pString == nullptr)
            return SECoP_S_ERROR_NO_MEMORY;
        pString->setValue(QByteArray::fromStdString(j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace)));
        pNewValue = SECoP_dataPtr(pString);
    }
    else if (szKey.compare("pollinterval", Qt::CaseInsensitive) == 0)
    {
        // convert poll interval from property
        iPollInterval = -1;
        const CSECoPsimpleDouble* pDouble(dynamic_cast<const CSECoPsimpleDouble*>(pTmpValue));
        if (pDouble != nullptr)
        {
            double dPollInterval;
            if (pDouble->getValue(dPollInterval) && dPollInterval >= 0.0 && dPollInterval <= (INT_MAX / 1000))
                iPollInterval = static_cast<int>(1000.0 * dPollInterval);
        }
        const CSECoPsimpleInt* pInt(dynamic_cast<const CSECoPsimpleInt*>(pTmpValue));
        if (pInt != nullptr)
        {
            long long llPollInterval;
            if (pInt->getValue(llPollInterval) && llPollInterval >= 0 && llPollInterval <= (INT_MAX / 1000))
                iPollInterval = 1000 * static_cast<int>(llPollInterval);
        }
        if (iPollInterval < 0)
            return SECoP_S_ERROR_INVALID_VALUE;
    }
    else if (szKey.compare("constant", Qt::CaseInsensitive) == 0)
    {
        // validate constant value
        if (pTmpValue->getType() == SECoP_VT_JSON)
        {
            if (propertyPosition(QString("datainfo")) < 0)
                return SECoP_S_ERROR_MISSING_MANDATORY;
            const CSECoPstring* pString(dynamic_cast<const CSECoPstring*>(pTmpValue));
            if (pString == nullptr)
                return SECoP_S_ERROR_INVALID_VALUE;
            pData = m_pValue->duplicate();
            if (pData == nullptr)
                return SECoP_S_ERROR_NO_MEMORY;
            if (!pData->importSECoP(pString->getValue().constData(), true) || !pData->isValid())
            {
                delete pData;
                return SECoP_S_ERROR_INVALID_VALUE;
            }
            pNewValue = SECoP_dataPtr(pData->duplicate());
        }
        else
        {
            if (propertyPosition(QString("datainfo")) < 0)
            {
                CSECoPstring* pString(new CSECoPstring(SECoP_VT_JSON));
                if (pString == nullptr)
                    return SECoP_S_ERROR_NO_MEMORY;
                pString->setValue(QByteArray::fromStdString(pTmpValue->exportType().
                                  dump(-1, ' ', false, nlohmann::json::error_handler_t::replace)));
                enum SECoP_S_error iTmpResult(addPropertyInternal("datainfo", SECoP_dataPtr(pString), true));
                if (iTmpResult < 0)
                    return SECoP_S_ERROR_MISSING_MANDATORY;
            }
            if (!m_pValue->compareType(pTmpValue))
                return SECoP_S_ERROR_INVALID_VALUE;
            pData = m_pValue->duplicate();
            pData->importSECoP(pTmpValue->exportSECoPjson());
        }
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

    if (pProperty != nullptr)
    {
        if (szKey.compare("datainfo", Qt::CaseInsensitive) == 0 && pData != nullptr)
        {
            // use converted description of data type as value
            m_pValue = SECoP_dataPtr(pData);
            m_pSigma = nullptr;
            m_bHasValue = false;
            pData = nullptr;
        }
        else if (szKey.compare("pollinterval", Qt::CaseInsensitive) == 0)
        {
            // use poll interval from property
            if (iPollInterval < 0)
                iPollInterval = 0;
            else if (iPollInterval > SECOP_MAX_POLLINTERVAL)
                iPollInterval = SECOP_MAX_POLLINTERVAL;
            m_iPollInterval = iPollInterval;
        }
        else if (szKey.compare("constant", Qt::CaseInsensitive) == 0)
        {
            // store constant value
            m_pValue = SECoP_dataPtr(pData);
            m_bConstant = m_bHasValue = true;
            pData = nullptr;
        }
    }
    SECoP_V_destroy(&pData);
    return iResult;
}

/// \returns the parameter name
QString SECoP_S_Parameter::getParameterID() const
{
    return m_szParameterID;
}

/// \returns the writability of this parameter
bool SECoP_S_Parameter::isWritable() const
{
    return m_bWritable;
}

/// \returns true, if the parameter has a cached value
bool SECoP_S_Parameter::hasValue() const
{
    return m_bHasValue;
}

/// \returns if parameter is constant
bool SECoP_S_Parameter::isConstant() const
{
    return m_bConstant;
}

/// \returns the pointer to the function to read the current value
SECoP_S_getsetFunction SECoP_S_Parameter::getter() const
{
    return m_bConstant ? nullptr : m_pGet;
}

/// \returns the pointer to the function for changing the value
SECoP_S_getsetFunction SECoP_S_Parameter::setter() const
{
    return m_bConstant ? nullptr : m_pSet;
}

/**
 * \brief This function stores a value in the parameter cache.
 * \param[in] pValue       value to store (discards old cache entry)
 * \param[in] pSigma       error value
 * \param[in] dblTimestamp timestamp of the value
 */
void SECoP_S_Parameter::setValue(const SECoP_dataPtr pValue, const SECoP_dataPtr pSigma, double dblTimestamp)
{
    if (!m_bConstant)
    {
        using std::isnan;
        using std::isinf;
        if (isnan(dblTimestamp) || isinf(dblTimestamp))
            dblTimestamp = SECoP_S_Main::getCurrentTime();
        m_pValue       = pValue;
        m_pSigma       = pSigma;
        m_dblTimestamp = dblTimestamp;
        m_bHasValue    = m_pValue.get() != nullptr && m_pValue->isValid();
    }
}

/// \returns the cached value of this parameter
const SECoP_dataPtr SECoP_S_Parameter::value() const
{
    return m_pValue;
}

/// \returns the error value of this parameter
const SECoP_dataPtr SECoP_S_Parameter::sigma() const
{
    return m_pSigma;
}

/// \returns the timestamp of the cached value
double SECoP_S_Parameter::timestamp() const
{
    return m_dblTimestamp;
}

/// \returns the recommended pollinterval for this parameter in ms
int SECoP_S_Parameter::pollinterval() const
{
    return m_iPollInterval;
}

/**
 * \brief This function checks, if this parameter should be polled.
 * \param[in] iTimeIncrement time increment in ms
 * \return true, if this parameter needs to be readed
 */
bool SECoP_S_Parameter::poll(int iTimeIncrement)
{
    if (m_iPollInterval > 0 && !m_bConstant)
    {
        m_iPollTime += iTimeIncrement;
        if (m_iPollTime >= m_iPollInterval)
        {
            // parameter should be polled
            m_iPollTime %= m_iPollInterval;
            return true;
        }
    }
    return false;
}

/// \returns the parent module of this parameter
SECoP_S_Module* SECoP_S_Parameter::getParentModule() const
{
    SECoP_S_Module* pModule(dynamic_cast<SECoP_S_Module*>(parent()));
    Q_ASSERT(pModule != nullptr);
    return pModule;
}
