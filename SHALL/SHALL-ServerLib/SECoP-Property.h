/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_PROPERTY_H__E2098C49_2314_4A0B_B6CE_4D297324789070__
#define __SECOP_PROPERTY_H__E2098C49_2314_4A0B_B6CE_4D297324789070__

#include <QVector>
#include <QString>
#include "SECoP-Variant.h"
#include "SECoP-types.h"

/**
 * \brief The SECoP_S_Property class stores data about a property of a node,
 *        module, parameter or command. This class has a list m_aSECoPPropertyList
 *        (QVector) of struct Property and functions to check and property key
 *        and value. The struct holds information of property type, the
 *        property name and an integer, if a property is mandatory or optional.
 *        The integer is needed for checking mandatory parameters and this
 *        should prevent the user from creating and using unknown properties.
 */
class SECoP_S_Property
{
    // disable copy
    explicit SECoP_S_Property(const SECoP_S_Property &) = delete;
    SECoP_S_Property& operator=(const SECoP_S_Property &) = delete;
public:
    explicit SECoP_S_Property(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic);
    ~SECoP_S_Property();

    bool setValue(const SECoP_dataPtr pValue);

    QString getKey() const;
    const SECoP_dataPtr getValue() const;
    bool isAuto() const;

    static int SECoPPropertyListHas(QString szName, QString szType);
    static QList<SECoP_V_type> SECoPPropertyTypes(QString szName, QString szType);
    static QStringList SECoPPropertyNeeds(QString szType);

private:
    /**
     * \brief The PropertyStruct struct is used for checking SECoP standard properties
     */
    struct PropertyStruct
    {
        PropertyStruct();
        PropertyStruct(QString szType, QString szName, int iMandatory, ...);
        PropertyStruct(const PropertyStruct &src);
        PropertyStruct& operator=(const PropertyStruct &src);
        /// type: Node; Module; Parameter; Command
        QString m_szType;
        /// name of standard property
        QString m_szName;
        /// mandatory flag: 0=no warning, 1=warning/optional, 2=error/mandatory
        int     m_iMandatory;
        /// usable data type for property
        QList<enum SECoP_V_type> m_aiDatatypes;
    };

    /// name of this property
    QString       m_szKey;
    /// value of property
    SECoP_dataPtr m_pValue;
    /// true: is is an overwritable automatic property, false: normal property
    bool          m_bAutomatic;
    /// static list of SECoP standard properties
    static QVector<PropertyStruct> m_aSECoPPropertyList;

    static void fillSECoPPropertyList();
};

#endif /*__SECOP_PROPERTY_H__E2098C49_2314_4A0B_B6CE_4D297324789070__*/
