/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOPSTATUSGUI_H_F4B91002_E277_4191_895C_DFEC74451EA5__
#define __SECOPSTATUSGUI_H_F4B91002_E277_4191_895C_DFEC74451EA5__

#include <QWidget>
#include <QList>

// forward declarations
namespace Ui {
class SECoP_S_StatusGui;
}
class SECoP_S_Node;
class QGridLayout;
class QMutex;
class QPlainTextEdit;
class QTcpSocket;

/**
 * \brief The SECoP_S_StatusGui class creates a status window
 */
class SECoP_S_StatusGui : public QWidget
{
    Q_OBJECT

public:
    explicit SECoP_S_StatusGui(bool bShowGUI, QWidget *parent = nullptr);
    ~SECoP_S_StatusGui();

    void allowClose(bool bAllow = true);
    void closeEvent(QCloseEvent* pEvent);

    void addNode(SECoP_S_Node* pNode);
    void removeNode(SECoP_S_Node* pNode);
    void addConnection(SECoP_S_Node* pNode, QTcpSocket* pClient);
    void removeConnection(QTcpSocket* pClient);
    void log(SECoP_S_Node* pNode, QString szData, bool bNodeOnly);
    void clearLog();
    void showGUI(bool bShowGUI);

private slots:
    void addRemoveNode(SECoP_S_Node* pNode, int iIndex);
    void startUp();
    void showConnectionList();
    void logUpdate();

private:
    /// \brief The TabInfo struct stores data about a node tab
    struct TabInfo
    {
        explicit TabInfo();
        explicit TabInfo(const TabInfo &src);
        TabInfo& operator=(const TabInfo &src);
        /// the SEC-node, which is associated with the tab
        SECoP_S_Node*     m_pNode;
        /// the widget tab on the status window
        QWidget*        m_pWidget;
        /// the layouter for the tab
        QGridLayout*    m_pLayout;
        /// the logging output for the node
        QPlainTextEdit* m_pLog;
    };

    /// \brief The ClientInfo struct stores data about a connected client
    struct ClientInfo
    {
        explicit ClientInfo();
        explicit ClientInfo(const ClientInfo &src);
        ClientInfo& operator=(const ClientInfo &src);
        /// the SEC-node instance, which is associated with the client
        SECoP_S_Node* m_pNode;
        /// the SEC-node name, which is associated with the client
        QString     m_szNode;
        /// the client instance
        QTcpSocket* m_pClient;
        /// information text about the client
        QString     m_szInfo;
    };

    /// \brief The LogEntry struct stores data about an entry, which should be logged
    struct LogEntry
    {
        explicit LogEntry();
        explicit LogEntry(SECoP_S_Node* pNode, QString szData, bool bNodeOnly);
        explicit LogEntry(const LogEntry &src);
        LogEntry& operator=(const LogEntry &src);
        /// timestamp in ms of the log entry
        qint64      m_qiTime;
        /// associated logging tab
        SECoP_S_Node* m_pNode;
        /// name of the node
        QString     m_szNode;
        /// data, which should be logged
        QString     m_szData;
        /// true: show inside node only, false: show in command tab too
        bool        m_bNodeOnly;
    };

    /// the status window created by the Qt assist
    Ui::SECoP_S_StatusGui* ui;
    /// exclusive access to internal data
    QMutex*              m_pMutex;
    /// false: do not allow to close window, true: for exit allow closing the window
    bool                 m_bAllowClose;
    /// flag, if the status window should be visible
    bool                 m_bShowGUI;
    /// number of connected clients
    int                  m_iClients;
    /// the list of all node tabs
    QList<TabInfo>       m_aTabs;
    /// the list of all connected clients
    QList<ClientInfo>    m_aClients;
    /// a list of log entries to show next
    QList<LogEntry>      m_aLog;

    void logFunction(qint64 qiTime, SECoP_S_Node* pNode, QString szNode, QString szData, bool bNodeOnly);
};

#endif // __SECOPSTATUSGUI_H_F4B91002_E277_4191_895C_DFEC74451EA5__
