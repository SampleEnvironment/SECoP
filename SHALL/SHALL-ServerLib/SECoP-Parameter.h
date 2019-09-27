/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_PARAMETER_H__3E90A34B_2314_F3FE_B6CE_4D243AF65CE007__
#define __SECOP_PARAMETER_H__3E90A34B_2314_F3FE_B6CE_4D243AF65CE007__

#include <QObject>
#include <QString>
#include <QVector>
#include "SECoP-Variant.h"
#include "SECoP-types.h"

// forward declarations
class SECoP_S_Module;
class SECoP_S_Property;

/**
 * \brief The SECoP_S_Parameter class stores data about a parameter and its
 *        properties. It is a data class only, the handling is done in the
 *        module and the worker classes.
 */
class SECoP_S_Parameter : public QObject
{
    friend class SECoP_S_Module;
    Q_OBJECT
    Q_DISABLE_COPY(SECoP_S_Parameter)
public:
    explicit SECoP_S_Parameter(QString szParameterID, bool bWritable, SECoP_S_getsetFunction pGet, SECoP_S_getsetFunction pSet, SECoP_S_Module* pParent);
    ~SECoP_S_Parameter();

    enum SECoP_S_error addProperty(QString szKey, const SECoP_dataPtr pValue);
    QString getParameterID() const;
    bool isWritable() const;
    bool isConstant() const;
    bool hasValue() const;
    SECoP_S_getsetFunction getter() const;
    SECoP_S_getsetFunction setter() const;
    const SECoP_dataPtr value() const;
    const SECoP_dataPtr sigma() const;
    double timestamp() const;
    int pollinterval() const;
    bool poll(int iTimeIncrement);

    int getNumberOfProperties() const;
    int propertyPosition(QString szKey) const;
    const SECoP_S_Property* getProperty(int iProperty) const;
    SECoP_S_Module* getParentModule() const;

private:
    enum SECoP_S_error addPropertyInternal(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic);
    void setValue(const SECoP_dataPtr pValue, const SECoP_dataPtr pSigma, double dblTimestamp);

    /// the parameter name
    QString                    m_szParameterID;
    /// the list of properties of this parameter
    QVector<SECoP_S_Property*> m_apProperties;
    /// false: readonly-parameter, true: read- and writable parameter
    bool                       m_bWritable;
    /// false: no cached value, true: a cached value is available
    bool                       m_bHasValue;
    /// false: variable parameter, true: constant parameter (even if writable)
    bool                       m_bConstant;
    /// pointer to getter function (not for polling case)
    SECoP_S_getsetFunction     m_pGet;
    /// pointer to setter function (for writable parameters, not for polling case)
    SECoP_S_getsetFunction     m_pSet;
    /// cached value of this parameter
    SECoP_dataPtr              m_pValue;
    /// error value of this parameter
    SECoP_dataPtr              m_pSigma;
    /// timestamp of the cached value
    double                     m_dblTimestamp;
    /// recommended polling interval for this parameter
    int                        m_iPollInterval;
    /// used for correcting of polling interval due rounding problems
    int                        m_iPollTime;
};

#endif /*__SECOP_PARAMETER_H__3E90A34B_2314_F3FE_B6CE_4D243AF65CE007__*/
