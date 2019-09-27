/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_COMMAND_H__3E9B6092_7598_EF09_EC83_FA243AF6645070__
#define __SECOP_COMMAND_H__3E9B6092_7598_EF09_EC83_FA243AF6645070__

#include <QString>
#include <QVector>
#include "SECoP-Variant.h"
#include "SECoP-types.h"

// forward declarations
class SECoP_S_Module;
class SECoP_S_Property;

/**
 * \brief The SECoP_S_Command class stores data about a command,
 *        which is callable via a SECoP client.
 */
class SECoP_S_Command
{
    // disable copy
    explicit SECoP_S_Command(const SECoP_S_Command &) = delete;
    SECoP_S_Command& operator=(const SECoP_S_Command &) = delete;
public:
    explicit SECoP_S_Command(QString szCommandID, SECoP_S_callFunction pFunction, SECoP_S_Module* pParent);
    ~SECoP_S_Command();
    enum SECoP_S_error addProperty(QString szKey, const SECoP_dataPtr pValue);
    QString getCommandID() const;
    SECoP_dataPtr getArgument() const;
    SECoP_dataPtr getResultType() const;
    SECoP_S_callFunction getFunction() const;
    int getNumberOfProperties() const;
    int propertyPosition(QString szKey) const;
    const SECoP_S_Property* getProperty(int iProperty) const;
    SECoP_S_Module* getParentModule() const;

private:
    enum SECoP_S_error addPropertyInternal(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic);

    /// a list of all command property instances
    QVector<SECoP_S_Property*> m_apProperties;

    /// the name of this command
    QString                    m_szCommandID;

    /// converted argument for calling this function
    SECoP_dataPtr              m_pArgument;

    /// converted result type after calling this function
    SECoP_dataPtr              m_pResultType;

    /// the function which is called on command invokation
    SECoP_S_callFunction       m_pFunction;

    /// the parent SECoP module
    SECoP_S_Module*            m_pParent;
};

#endif /*__SECOP_COMMAND_H__3E9B6092_7598_EF09_EC83_FA243AF6645070__*/
