/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QNetworkInterface>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>
#include <QDesktopWidget>
#include <QMutex>
#include <QThread>
#include <QTextDocument>
#include <QTextBlock>
#include <QDateTime>
#include <QScreen>
#include "SECoP-StatusGui.h"
#include "ui_SECoP-StatusGui.h"
#include "SECoP-Node.h"
#include "SECoP-Main.h"

/**
 * \brief maximum entries in log window
 * \ingroup intfunc
 */
#define MAXIMUM_LOG_ENTRIES 1000

/**
 * \brief constructor of SECoP_S_StatusGui creates the status window and start a
 *        timer, which does the rest of initialization.
 * \param[in] parent
 */
SECoP_S_StatusGui::SECoP_S_StatusGui(bool bShowGUI, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SECoP_S_StatusGui)
    , m_pMutex(nullptr)
    , m_bAllowClose(false)
    , m_bShowGUI(bShowGUI)
    , m_iClients(0)
{
    ui->setupUi(this);
    ui->eLog->setMaximumBlockCount(MAXIMUM_LOG_ENTRIES);
    m_pMutex = new QMutex(QMutex::Recursive);

    QList<QHostAddress> aAdresses(QNetworkInterface::allAddresses());
    ui->eAddresses->clear();
    foreach (const QHostAddress &addr, aAdresses)
        ui->eAddresses->appendPlainText(addr.toString());
    QTimer::singleShot(0, this, SLOT(startUp()));

    Qt::WindowFlags iFlags(windowFlags());
    iFlags |= Qt::WindowMinMaxButtonsHint;
    iFlags &= ~Qt::WindowCloseButtonHint;
    setWindowFlags(iFlags);
}

/**
 * \brief destructor of SECoP_S_StatusGui
 */
SECoP_S_StatusGui::~SECoP_S_StatusGui()
{
    m_bAllowClose = true;
    delete ui;
    delete m_pMutex;
}

/**
 * \brief This function sets the flag, which allows to close the status window.
 * \param[in] bAllow true: allow closing, false: forbid closing the window
 */
void SECoP_S_StatusGui::allowClose(bool bAllow)
{
    m_bAllowClose = bAllow;
}

/**
 * \brief Qt event handler, when someone wants to close the status window. It
 *        looks to the allow flag and allows or forbids this.
 * \param[in] pEvent close event
 */
void SECoP_S_StatusGui::closeEvent(QCloseEvent* pEvent)
{
    if (m_bAllowClose)
        pEvent->accept();
    else
        pEvent->ignore();
}

/**
 * \brief This function will be called after the status window was successfully
 *        created. It triggers the move to upper right corner and it starts a
 *        timer, which will look for new log entries in regular intervals.
 */
void SECoP_S_StatusGui::startUp()
{
    // move window to top right corner of screen
    setVisible(m_bShowGUI);
    QApplication* pApplication(dynamic_cast<QApplication*>(QApplication::instance()));
    if (pApplication != nullptr)
    {
        QScreen* pScreen(QGuiApplication::primaryScreen());
        if (pScreen != nullptr)
        {
            QRect screen(pScreen->availableGeometry());
            QSize size(frameSize());
            if (size.width() < 10)
                size = sizeHint();
            move(screen.right() - size.width(), 0);
        }
    }
    QByteArray szGitVersion(SECoP_S_Main::getGitVersion());
    if (!szGitVersion.isEmpty())
    {
        QString szTitle(windowTitle());
        if (!szTitle.isEmpty())
            szTitle.append(" / ");
        szTitle.append(szGitVersion);
        setWindowTitle(szTitle);
        ui->eLog->appendPlainText(QString("Git %1").arg(szGitVersion.constData()));
    }
    showConnectionList();
    QTimer* pTimer = new QTimer(this);
    connect(pTimer, SIGNAL(timeout()), this, SLOT(logUpdate()));
    pTimer->start(500);
}

/**
 * \brief This function is called, when a new SEC-node was created. It creates
 *        a new tab for logging purposes of this node.
 * \param[in] pNode new SEC-node instance
 */
void SECoP_S_StatusGui::addNode(SECoP_S_Node* pNode)
{
    for (int i = 0; i < m_aTabs.size(); ++i)
    {
        struct TabInfo &tab = m_aTabs[i];
        if (tab.m_pNode == pNode)
            return;
    }
    log(pNode, QString("added node"), false);
    if (QThread::currentThread() == thread())
        addRemoveNode(pNode, -1);
    else
        QMetaObject::invokeMethod(this, "addRemoveNode", Qt::BlockingQueuedConnection,
                                  Q_ARG(SECoP_S_Node*, pNode), Q_ARG(int, -1));
}

/**
 * \brief This function is called, when a SEC-node was destroyed. It deletes
 *        the associated tab of this node.
 * \param[in] pNode deleted SEC-node instance
 */
void SECoP_S_StatusGui::removeNode(SECoP_S_Node* pNode)
{
    for (int i = 0; i < m_aTabs.size(); ++i)
    {
        struct TabInfo &tab = m_aTabs[i];
        if (tab.m_pNode == pNode)
        {
            log(pNode, QString("removed node"), false);
            if (QThread::currentThread() == thread())
                addRemoveNode(pNode, i--);
            else
                QMetaObject::invokeMethod(this, "addRemoveNode", Qt::BlockingQueuedConnection,
                                          Q_ARG(SECoP_S_Node*, pNode), Q_ARG(int, i--));
        }
    }
}

/**
 * \brief This function does the work creating/deleteing tabs for each node.
 * \param[in] pNode  new SEC-node instance
 * \param[in] iIndex index of SEC-node tab to delete or -1
 */
void SECoP_S_StatusGui::addRemoveNode(SECoP_S_Node* pNode, int iIndex)
{
    if (iIndex < 0)
    {
        // create tab
        QString szName(pNode->getNodeID());
        struct TabInfo tab;
        tab.m_pNode = pNode;
        tab.m_pWidget = new QWidget(ui->tRegister);
        tab.m_pWidget->setObjectName(QString("tNode%1").arg(szName));
        tab.m_pLayout = new QGridLayout(tab.m_pWidget);
        tab.m_pLayout->setObjectName(QString("layout%1").arg(szName));
        tab.m_pLog = new QPlainTextEdit(tab.m_pWidget);
        tab.m_pLog->setObjectName(QString("eNode%1").arg(szName));
        tab.m_pLog->setUndoRedoEnabled(false);
        tab.m_pLog->setReadOnly(true);
        tab.m_pLog->setMaximumBlockCount(MAXIMUM_LOG_ENTRIES);
        tab.m_pLayout->addWidget(tab.m_pLog, 0, 0, 1, 1);
        ui->tRegister->addTab(tab.m_pWidget, szName);
        m_aTabs.append(tab);
    }
    else
    {
        // remove tab
        struct TabInfo &tab = m_aTabs[iIndex];
        for (int i = 0; i < ui->tRegister->count(); ++i)
            if (ui->tRegister->widget(i) == tab.m_pWidget)
                ui->tRegister->removeTab(i);
        delete tab.m_pWidget;
        m_aTabs.removeAt(iIndex);
    }
}

/**
 * \brief This function will be called for every new client connection.
 * \param[in] pNode   associated node, which handles the connection
 * \param[in] pClient new client instance
 */
void SECoP_S_StatusGui::addConnection(SECoP_S_Node* pNode, QTcpSocket* pClient)
{
    QMutexLocker locker(m_pMutex);
    for (auto it = m_aClients.begin(); it != m_aClients.end(); ++it)
    {
        if (it->m_pClient == pClient)
        {
            if (it->m_pNode == pNode)
                return;
            // update node
            it->m_pNode  = pNode;
            it->m_szNode = pNode->getNodeID();
            goto updateinfo;
        }
    }
    do
    {
        struct ClientInfo client;
        client.m_pNode   = pNode;
        client.m_szNode  = pNode->getNodeID();
        client.m_pClient = pClient;
        client.m_szInfo  = QString("%1:%2").arg(pClient->peerAddress().toString()).arg(pClient->peerPort());
        m_aClients.append(client);
        log(pNode, QString("added client from %1").arg(client.m_szInfo), false);
    } while (0);
updateinfo:
    if (QThread::currentThread() == thread())
        showConnectionList();
    else
        QTimer::singleShot(0, this, SLOT(showConnectionList()));
}

/**
 * \brief This function will be called, when a client connection was closed.
 * \param[in] pClient deleted client instance
 */
void SECoP_S_StatusGui::removeConnection(QTcpSocket* pClient)
{
    QMutexLocker locker(m_pMutex);
    bool bUpdated(false);
    for (int i = 0; i < m_aClients.size(); ++i)
    {
        struct ClientInfo &client(m_aClients[i]);
        if (client.m_pClient == pClient)
        {
            bUpdated = true;
            log(client.m_pNode, QString("removed client %1").arg(client.m_szInfo), false);
            m_aClients.removeAt(i--);
        }
    }
    if (bUpdated)
    {
        if (QThread::currentThread() == thread())
            showConnectionList();
        else
            QTimer::singleShot(0, this, SLOT(showConnectionList()));
    }
}

/**
 * \brief The function updates the connection list of all clients.
 */
void SECoP_S_StatusGui::showConnectionList()
{
    ui->eConnections->clear();
    ui->eConnections->appendPlainText(QString("%1 clients").arg(m_aClients.size()));
    for (auto it = m_aClients.constBegin(); it != m_aClients.constEnd(); ++it)
        ui->eConnections->appendPlainText(QString("%1  %2").arg(it->m_pNode->getNodeID()).arg(it->m_szInfo));
    ui->eConnections->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
}

/**
 * \brief The function generates a log entry to show inside the status window.
 * \param[in] pNode     SEC-node or nullptr
 * \param[in] szData    text to show
 * \param[in] bNodeOnly false: show in common window, true: show in node tab only
 */
void SECoP_S_StatusGui::log(SECoP_S_Node* pNode, QString szData, bool bNodeOnly)
{
    QMutexLocker locker(m_pMutex);
    m_aLog.append(LogEntry(pNode, szData, bNodeOnly));
}

/**
 * \brief Clear the status window, which is used by \ref SECoP_S_initLibrary
 *        or \ref SECoP_S_doneLibrary.
 */
void SECoP_S_StatusGui::clearLog()
{
    QMutexLocker locker(m_pMutex);
    m_aLog.append(LogEntry(static_cast<SECoP_S_Node*>(nullptr), QString(), true));
}

/**
 * \brief shows or hides the status window
 * \param[in] bShowGUI true: show status window, false: hide it
 */
void SECoP_S_StatusGui::showGUI(bool bShowGUI)
{
    m_bShowGUI = bShowGUI;
}

/**
 * \brief Regulary called Qt event handler to update the status window content.
 */
void SECoP_S_StatusGui::logUpdate()
{
    QList<LogEntry> tmpLog;
    if (m_pMutex != nullptr)
        m_pMutex->lock();
    m_aLog.swap(tmpLog); // this function never fails
    if (m_pMutex != nullptr)
        m_pMutex->unlock();
    if (m_bShowGUI != isVisible())
    {
        setVisible(m_bShowGUI);
        if (m_bShowGUI)
            activateWindow();
    }
    for (auto it = tmpLog.constBegin(); it != tmpLog.constEnd(); ++it)
        logFunction(it->m_qiTime, it->m_pNode, it->m_szNode, it->m_szData, it->m_bNodeOnly);
}

/**
 * \brief The function append a log entry to the status window. It looks, where
 *        is what for update.
 * \param[in] qiTime    timestamp in ms of the logging text
 * \param[in] pNode     SEC-node or nullptr
 * \param[in] szNode    name of the SEC-node
 * \param[in] szData    text to log
 * \param[in] bNodeOnly false: common logging tab, true: node tab only
 */
void SECoP_S_StatusGui::logFunction(qint64 qiTime, SECoP_S_Node* pNode, QString szNode, QString szData, bool bNodeOnly)
{
    QString szTime(QDateTime::fromMSecsSinceEpoch(qiTime).toString("yyyy/MM/dd HH:mm:ss.zzz "));
    while (szData.endsWith(QChar('\r')) || szData.endsWith(QChar('\n')))
        szData.chop(1);
    setUpdatesEnabled(false);
    for (auto it = m_aTabs.constBegin(); it != m_aTabs.constEnd(); ++it)
        if (it->m_pNode == pNode)
            it->m_pLog->appendPlainText(szTime + szData);
    if (bNodeOnly)
    {
        if (pNode == nullptr)
            ui->eLog->clear();
    }
    else
    {
        if (pNode != nullptr && !szNode.isEmpty())
        {
            szData.prepend(": ");
            szData.prepend(szNode);
        }
        ui->eLog->appendPlainText(szTime + szData);
    }
    setUpdatesEnabled(true);
}

/**
 * \brief default constructor of SECoP_S_StatusGui::TabInfo
 */
SECoP_S_StatusGui::TabInfo::TabInfo()
    : m_pNode(nullptr)
    , m_pWidget(nullptr)
    , m_pLayout(nullptr)
    , m_pLog(nullptr)
{
}

/**
 * \brief copy constructor of SECoP_S_StatusGui::TabInfo
 * \param[in] source to copy
 */
SECoP_S_StatusGui::TabInfo::TabInfo(const SECoP_S_StatusGui::TabInfo &src)
    : m_pNode(src.m_pNode)
    , m_pWidget(src.m_pWidget)
    , m_pLayout(src.m_pLayout)
    , m_pLog(src.m_pLog)
{
}

/**
 * \brief copy operator of SECoP_S_StatusGui::TabInfo
 * \param[in] source to copy
 * \return a reference to this instance
 */
SECoP_S_StatusGui::TabInfo& SECoP_S_StatusGui::TabInfo::operator=(const SECoP_S_StatusGui::TabInfo &src)
{
    m_pNode   = src.m_pNode;
    m_pWidget = src.m_pWidget;
    m_pLayout = src.m_pLayout;
    m_pLog    = src.m_pLog;
    return *this;
}

/**
 * \brief default constructor of SECoP_S_StatusGui::ClientInfo
 */
SECoP_S_StatusGui::ClientInfo::ClientInfo()
    : m_pNode(nullptr)
    , m_pClient(nullptr)
{
}

/**
 * \brief copy constructor of SECoP_S_StatusGui::ClientInfo
 * \param[in] source to copy
 */
SECoP_S_StatusGui::ClientInfo::ClientInfo(const SECoP_S_StatusGui::ClientInfo &src)
    : m_pNode(src.m_pNode)
    , m_szNode(src.m_szNode)
    , m_pClient(src.m_pClient)
    , m_szInfo(src.m_szInfo)
{
}

/**
 * \brief copy operator of SECoP_S_StatusGui::ClientInfo
 * \param[in] source to copy
 * \return a reference to this instance
 */
SECoP_S_StatusGui::ClientInfo& SECoP_S_StatusGui::ClientInfo::operator=(const SECoP_S_StatusGui::ClientInfo &src)
{
    m_pNode   = src.m_pNode;
    m_szNode  = src.m_szNode;
    m_pClient = src.m_pClient;
    m_szInfo  = src.m_szInfo;
    return *this;
}

/**
 * \brief default constructor of SECoP_S_StatusGui::LogEntry
 */
SECoP_S_StatusGui::LogEntry::LogEntry()
    : m_qiTime(QDateTime::currentMSecsSinceEpoch())
    , m_bNodeOnly(false)
{
}

/**
 * \brief constructor of SECoP_S_StatusGui::LogEntry
 * @param[in] pNode     SEC-node or nullptr
 * @param[in] szData    text to log
 * @param[in] bNodeOnly is this only node specific?
 */
SECoP_S_StatusGui::LogEntry::LogEntry(SECoP_S_Node* pNode, QString szData, bool bNodeOnly)
    : m_qiTime(QDateTime::currentMSecsSinceEpoch())
    , m_pNode(pNode)
    , m_szData(szData)
    , m_bNodeOnly(bNodeOnly)
{
    if (pNode != nullptr)
        m_szNode = pNode->getNodeID();
}

/**
 * \brief copy constructor of SECoP_S_StatusGui::LogEntry
 * \param[in] source to copy
 */
SECoP_S_StatusGui::LogEntry::LogEntry(const SECoP_S_StatusGui::LogEntry &src)
    : m_qiTime(src.m_qiTime)
    , m_pNode(src.m_pNode)
    , m_szNode(src.m_szNode)
    , m_szData(src.m_szData)
    , m_bNodeOnly(src.m_bNodeOnly)
{
}

/**
 * \brief copy operator of SECoP_S_StatusGui::LogEntry
 * \param[in] source to copy
 * \return a reference to this instance
 */
SECoP_S_StatusGui::LogEntry& SECoP_S_StatusGui::LogEntry::operator=(const SECoP_S_StatusGui::LogEntry &src)
{
    m_qiTime    = src.m_qiTime;
    m_pNode     = src.m_pNode;
    m_szNode    = src.m_szNode;
    m_szData    = src.m_szData;
    m_bNodeOnly = src.m_bNodeOnly;
    return *this;
}
