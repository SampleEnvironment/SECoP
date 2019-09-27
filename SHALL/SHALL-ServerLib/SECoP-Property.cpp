/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QStringList>
#include <stdarg.h>
#include "SECoP-defines.h"
#include "SECoP-Property.h"

/**
 * \brief static list of SECoP standard properties
 */
QVector<SECoP_S_Property::PropertyStruct> SECoP_S_Property::m_aSECoPPropertyList;

/**
 * \brief constructor of SECoP_S_Property creates a property of a node, module,
 *        parameter or command with value.
 * \param[in] szKey      name of the property
 * \param[in] pValue     value
 * \param[in] bAutomatic true: overwritable automatic property, false: normal property
 */
SECoP_S_Property::SECoP_S_Property(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic)
    : m_szKey(szKey)
    , m_pValue(pValue)
    , m_bAutomatic(bAutomatic)
{
}

/**
 * \brief destructor of SECoP_S_Property
 */
SECoP_S_Property::~SECoP_S_Property()
{
}

/**
 * \brief Set/overwrite the value of a property and clears the automatic/
 *        overwritable flag.
 * \param[in] pValue new value
 * \return true: successful, false: the property was not automatic/overwritable
 */
bool SECoP_S_Property::setValue(const SECoP_dataPtr pValue)
{
    if (m_bAutomatic)
    {
        m_pValue     = pValue;
        m_bAutomatic = false;
        return true;
    }
    else
        return false;
}

/// \returns the name of the property
QString SECoP_S_Property::getKey() const
{
    return m_szKey;
}

/// \returns the value of the property
const SECoP_dataPtr SECoP_S_Property::getValue() const
{
    return m_pValue;
}

/// \returns the automatic/overwritability status
bool SECoP_S_Property::isAuto() const
{
    return m_bAutomatic;
}

/// \brief This function fille the SECoP standard property list.
void SECoP_S_Property::fillSECoPPropertyList()
{
    // mandatory levels: 0=no warning,  1=optional (warning if missing), 2=mandatory (error if missing)
    if (m_aSECoPPropertyList.isEmpty())
    {
        //----------------------------Node Properties
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Node", "equipment_id", 2, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Node", "description",  2, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Node", "firmware",     0, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Node", "implementor",  0, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Node", "timeout",      0, SECoP_VT_DOUBLE, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Node", "order",        0, SECoP_VT_JSON, 0));
//      SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Node", "_test",        2, SECoP_VT_STRING, 0));
        //----------------------------Module Properties
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "interface_class", 2, SECoP_VT_JSON, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "description",     2, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "pollinterval",    0, SECoP_VT_DOUBLE, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "visibility",      0, SECoP_VT_INTEGER, SECoP_VT_DOUBLE, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "group",           0, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "meaning",         0, SECoP_VT_INTEGER, SECoP_VT_DOUBLE, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "importance",      0, SECoP_VT_INTEGER, SECoP_VT_DOUBLE, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "implementor",     0, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "order",           0, SECoP_VT_JSON, 0));
//      SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Module", "_test",           2, SECoP_VT_STRING, 0));
        //----------------------------Parameter Properties
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Parameter", "description",  2, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Parameter", "datainfo",     2, SECoP_VT_JSON, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Parameter", "constant",     0, SECoP_VT_NONE, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Parameter", "readonly",     2, SECoP_VT_BOOL, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Parameter", "pollinterval", 0, SECoP_VT_DOUBLE, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Parameter", "visibility",   0, SECoP_VT_INTEGER, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Parameter", "group",        0, SECoP_VT_STRING, 0));
//      SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Parameter", "_test",        2, SECoP_VT_STRING, 0));
        //----------------------------Command Properties
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Command", "description", 2, SECoP_VT_STRING, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Command", "datainfo",    2, SECoP_VT_JSON, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Command", "visibility",  0, SECoP_VT_INTEGER, 0));
        SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Command", "group",       0, SECoP_VT_STRING, 0));
//      SECoP_S_Property::m_aSECoPPropertyList.append(PropertyStruct("Command", "_test",       2, SECoP_VT_STRING, 0));
    }
}

/**
 * \brief This function returns the mandatory level a standard property has.
 * \param[in] szName name of the standard property
 * \param[in] szType type of thing: Node, Module, Parameter, Command
 * \return -1: error, 0: optional/invisible, 1: optional, 2: mandatory
 */
int SECoP_S_Property::SECoPPropertyListHas(QString szName, QString szType)
{
    fillSECoPPropertyList();
    for (int i = 0; i < m_aSECoPPropertyList.size(); i++)
    {
        struct SECoP_S_Property::PropertyStruct* p(&m_aSECoPPropertyList[i]);
        if (szName.compare(p->m_szName, Qt::CaseInsensitive) == 0 &&
            szType.compare(p->m_szType, Qt::CaseInsensitive) == 0)
            return p->m_iMandatory;
    }
    return -1;
}

/**
 * \brief This function returns a list of allowed simple data types of a standard property.
 * \param[in] szName name of the standard property
 * \param[in] szType type of thing: Node, Module, Parameter, Command
 * \return list of allowed simple data types
 */
QList<enum SECoP_V_type> SECoP_S_Property::SECoPPropertyTypes(QString szName, QString szType)
{
    fillSECoPPropertyList();
    for (int i = 0; i < m_aSECoPPropertyList.size(); i++)
    {
        struct SECoP_S_Property::PropertyStruct* p(&m_aSECoPPropertyList[i]);
        if (szName.compare(p->m_szName, Qt::CaseInsensitive) == 0 &&
            szType.compare(p->m_szType, Qt::CaseInsensitive) == 0)
            return p->m_aiDatatypes;
    }
    return QList<enum SECoP_V_type>();
}

/**
 * \brief This function returns a list of names of standard properties for
 *        a specific type of thing.
 * \param[in] szType type of thing: Node, Module, Parameter, Command
 * \return a string list of standard properties
 */
QStringList SECoP_S_Property::SECoPPropertyNeeds(QString szType)
{
    QStringList aszResult;
    fillSECoPPropertyList();
    for (int i = 0; i < m_aSECoPPropertyList.size(); i++)
    {
        struct SECoP_S_Property::PropertyStruct* p(&m_aSECoPPropertyList[i]);
        if (szType.compare(p->m_szType, Qt::CaseInsensitive) == 0)
            aszResult << p->m_szName;
    }
    return aszResult;
}

/**
 * \brief default constructor of SECoP_S_Property
 */
SECoP_S_Property::PropertyStruct::PropertyStruct()
    : m_iMandatory(2)
{
}

/**
 * \brief constructor of SECoP_S_Property
 * \param[in] szType     type of thing: Node, Module, Parameter, Command
 * \param[in] szName     name of property
 * \param[in] iMandatory mandatory level
 * \param[in] iDatatype  standard simple data type for property
 * \param[in] ...        optional usable data type for property, last element has to be SECoP_VT_NULL or 0
 */
SECoP_S_Property::PropertyStruct::PropertyStruct(QString szType, QString szName, int iMandatory, ...)
    : m_szType(szType)
    , m_szName(szName)
    , m_iMandatory(iMandatory)
{
    va_list args;
    va_start(args, iMandatory);
    for (;;)
    {
        enum SECoP_V_type iType(static_cast<enum SECoP_V_type>(va_arg(args, int)));
        if (iType == SECoP_VT_NONE)
            break;
        m_aiDatatypes.append(iType);
    }
    va_end(args);
}

/**
 * \brief copy constructor of SECoP_S_Property
 * \param[in] src source to copy
 */
SECoP_S_Property::PropertyStruct::PropertyStruct(const SECoP_S_Property::PropertyStruct &src)
    : m_szType(src.m_szType)
    , m_szName(src.m_szName)
    , m_iMandatory(src.m_iMandatory)
    , m_aiDatatypes(src.m_aiDatatypes)
{
}

/**
 * \brief copy operator
 * \param[in] src source to copy
 * \return the reference to this instance
 */
SECoP_S_Property::PropertyStruct&
    SECoP_S_Property::PropertyStruct::operator=(const SECoP_S_Property::PropertyStruct &src)
{
    m_szType      = src.m_szType;
    m_szName      = src.m_szName;
    m_iMandatory  = src.m_iMandatory;
    m_aiDatatypes = src.m_aiDatatypes;
    return *this;
}
