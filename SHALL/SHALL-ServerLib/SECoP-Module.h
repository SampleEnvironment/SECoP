/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_MODULE_H__3E9B6092_2314_4A0B_B6CE_4D243AF65CE070__
#define __SECOP_MODULE_H__3E9B6092_2314_4A0B_B6CE_4D243AF65CE070__

#include <QThread>
#include <QVector>
#include <QString>
#include "SECoP-Variant.h"
#include "SECoP-types.h"

// forward declarations
class QMutex;
class SECoP_S_Command;
class SECoP_S_Node;
class SECoP_S_Parameter;
class SECoP_S_Property;
class SECoP_S_Worker;

/**
 * \brief The SECoP_S_Module class stores data about a module,
 *        which has parameters, commands and properties.
 */
class SECoP_S_Module : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SECoP_S_Module)

public:
    struct s_Accessibles
    {
        QString name; //command or parametername
        bool command; //true for commands
        int position; //position inside m_apParameters for parameters and m_apCommands for commands
    };

    explicit SECoP_S_Module(QString szModuleName, SECoP_S_Node* pNode = nullptr, QObject *pParent = nullptr);
    virtual ~SECoP_S_Module();
    QString getModuleID() const;
    QString getPrintableActive(bool bWithAccessible) const;
    enum SECoP_S_error addCommand(QString szKey, SECoP_S_callFunction ptrToCmd);
    enum SECoP_S_error addReadableParameter(QString szParameterName, SECoP_S_getsetFunction ptrToGet);
    enum SECoP_S_error addWritableParameter(QString szParameterName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet);
    enum SECoP_S_error addProperty(QString szKey, const SECoP_dataPtr pValue);
    enum SECoP_S_error setAddFocus(QString szKey);
    void nodeIsComplete();
    int getNumberOfAccessibles() const;
    int getNumberOfProperties() const;
    int parameterPosition(QString szParameterName) const;
    int commandPosition(QString szCommandName) const;
    int propertyPosition(QString szKey) const;
    SECoP_S_Parameter*       getParameter(int iParameter) const;
    SECoP_S_Command*         getCommand(int iCommand) const;
    const SECoP_S_Property*  getProperty(int iProperty) const;
    struct s_Accessibles*  getAccessibles(int iAcc);
    enum SECoP_S_error readParameter(SECoP_S_Parameter* pParameter, quint64* pqwRequestId);
    enum SECoP_S_error changeParameter(SECoP_S_Parameter* pParameter, const SECoP_dataPtr pValue, quint64* pqwRequestId);
    void doParameterResult(enum SECoP_S_error iErrorCode, quint64 qwRequestId, SECoP_S_Parameter* pParameter,
                           const SECoP_dataPtr pValue, const SECoP_dataPtr pSigma, double dblTimestamp);
    void doCommandResult(QObject* pTarget, enum SECoP_S_error iErrorCode, SECoP_S_Command* pCommand, const SECoP_dataPtr pValue,
                         double dblTimestamp);
    void updateParameter(int iParameter, SECoP_dataPtr pValue, SECoP_dataPtr pSigma, double dblTimestamp);

    void quitThreadWhileDelete(bool bQuit);

signals:
    /**
     * \brief This signal is triggered for a new value of a parameter
     * \param[in] pParameter   changed parameter instance
     * \param[in] qwRequestId  module request id for this change
     * \param[in] iErrorCode   result code of the change
     * \param[in] pValue       new value of the parameter
     * \param[in] pSigma       error of value of the parameter
     * \param[in] dblTimestamp timestamp of the change
     */
    void newParameterValue(SECoP_S_Parameter* pParameter, quint64 qwRequestId, enum SECoP_S_error iErrorCode,
                           const SECoP_dataPtr pValue, const SECoP_dataPtr pSigma, double dblTimestamp);

private slots:
    void readParameter(SECoP_S_Parameter* pParameter, quint64 qwRequestId);
    void changeParameter(SECoP_S_Parameter* pParameter, const SECoP_dataPtr pValue, quint64 qwRequestId);
    void doCommand(SECoP_S_Command* pCommand, const SECoP_dataPtr pArgument, QObject* pSender);
    void addCommand(QString szKey , SECoP_S_callFunction ptrToFunc, SECoP_S_error* piResult);
    void addReadableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_error* piResult);
    void addWritableParameter(QString szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet, SECoP_S_error* piResult);
    void addProperty(QString szKey, const SECoP_dataPtr pValue, bool bAutomatic, SECoP_S_error* piResult);
    void setAddFocus(QString szKey, SECoP_S_error* piResult);
    void nodeIsCompleteSlot();

private:
    virtual void timerEvent(QTimerEvent* pEvent);
    quint64 nextRequestId();

    /// exclusive access to module internals
    QMutex*                     m_pMutex;
    /// associated parent node for this module
    SECoP_S_Node*               m_pNode;
    /// should the module thread end together with module
    bool                        m_bQuitThread;
    /// the name of the module
    QString                     m_szModuleID;
    /// the list of properties of the module
    QVector<SECoP_S_Property*>  m_apProperties;
    /// the list of parameters inside the module
    QVector<SECoP_S_Parameter*> m_apParameters;
    /// the list of commands inside the module
    QVector<SECoP_S_Command*>   m_apCommands;
    /// the list of the names of parameters and  commands in order of their appearence while creating; this is used for json creating where commands and parameters appear in only one list
    QVector<s_Accessibles> m_AccList;
    /// timer id for automatic polling of all parameters
    int                         m_iPollTimerId;
    /// standard interval for automatic module polling
    int                         m_iWantedInterval;
    /// used interval (lowest common multiple) for automatic polling
    int                         m_iPollInterval;
    /// used for correcting of polling interval due rounding problems
    int                         m_iPollTime;
    /// while creation: index of last created parameter or -1
    int                         m_iParameterFocus;
    /// while creation: index of last created command or -1
    int                         m_iCommandFocus;
    /// last request id for parameter changes
    quint64                     m_qwRequestId;
};

#endif /*__SECOP_MODULE_H__3E9B6092_2314_4A0B_B6CE_4D243AF65CE070__*/
