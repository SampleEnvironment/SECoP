/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include "clientgui.h"
#include "ui_clientgui.h"

#include <QtWidgets>
#include <QColor>
#include <QTimer>
#include <QLayoutItem>

/**
 * \brief preprocessor helper macros to show line numbers as strings inside warnings
 * \code{.cpp}
 *    #pragma message(CPP_WARNINGPREFIX "TODO: this a warning")
 * \endcode
 * \ingroup exptype
 */
#define CPP_NUMBER2STRING2(x) #x
#define CPP_NUMBER2STRING(x) CPP_NUMBER2STRING2(x)
#ifdef _MSC_VER
#define CPP_WARNINGPREFIX __FILE__ "(" CPP_NUMBER2STRING(__LINE__) "): warning C0000: "
#else
#define CPP_WARNINGPREFIX __FILE__ "(" CPP_NUMBER2STRING(__LINE__) "): "
#endif

//header
static int numberOfHeaderRows=7; //number of header rows
static int messageRow=0;
static int valueRow=2;
static int statusRow=3;          //
static int targetRow=4;          //
static int stopRow=5;            //


//columns
static int numberOfColumns=7;   //columns for the tablelayout
static int firstCol=0;          //unused
static int nameOfAccCol=1;      //name of parameter etc
static int accValueCol=2;       //value
static int accUnitCol=3;        //unit
static int lineEditCol=4;       //paramter for commands or value  for a parameter
static int vertikalLineCol=5;   //seperator for button column
static int buttonCol=6;         //buttons

/*
entry point from is addNode or connectWithServer
*/

ClientGui::ClientGui(bool bShowGUI, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ClientGui)
    , m_bAllowClose(false)
    , m_bShowGUI(bShowGUI)
{
    ui->setupUi(this);
    ui->SendButton->setEnabled(true);
    ui->CommandLabel->setEnabled(true);
    ui->CommandLineEdit->setEnabled(true);
    ui->iPLineEdit->setText("127.0.0.1");
    ui->portLineEdit->setText("2055");
    ui->logEdit->setMaximumBlockCount(10000);
    ui->tabWidget->setCurrentIndex(0);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    connect(ui->ConnectButton, SIGNAL(clicked()),
            this, SLOT(connectWithServer()), Qt::QueuedConnection);

    connect(ui->disconnectButton, SIGNAL(clicked()),
            this, SLOT(disconnectFromServer()), Qt::QueuedConnection);

    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem* ,int)),
            this, SLOT(showItem(QTreeWidgetItem*)), Qt::QueuedConnection);

    connect(ui->checkBox, SIGNAL(clicked(bool)),
            this, SLOT(toggleAsyncMSGBool(bool)), Qt::QueuedConnection);

    connect(ui->SendButton, SIGNAL(clicked()),
            this, SLOT(writeTcpDatastream()), Qt::QueuedConnection);

    QTimer* pTimer = new QTimer(this);
    connect(pTimer, SIGNAL(timeout()), this, SLOT(logUpdate()));
    pTimer->start(500);

    Qt::WindowFlags iFlags(windowFlags());
    iFlags |= Qt::WindowMinMaxButtonsHint;
    iFlags &= ~Qt::WindowCloseButtonHint;
    setWindowFlags(iFlags);
}

ClientGui::~ClientGui()
{
    m_bAllowClose = true;
    delete ui;
}

/**
 * \brief This function sets the flag, which allows to close the status window.
 * \param[in] bAllow true: allow closing, false: forbid closing the window
 */
void ClientGui::allowClose(bool bAllow)
{
    m_bAllowClose = bAllow;
}

/**
 * \brief Qt event handler, when someone wants to close the status window. It
 *        looks to the allow flag and allows or forbids this.
 * \param[in] pEvent close event
 */
void ClientGui::closeEvent(QCloseEvent* pEvent)
{
    if (m_bAllowClose)
        pEvent->accept();
    else
        pEvent->ignore();
}

void ClientGui::log(QString szLogText)
{
    QMutexLocker locker(&m_Mutex);
    m_aLog.append(szLogText);
}

/**
 * \brief Regulary called Qt event handler to update the status window content.
 */
void ClientGui::logUpdate()
{
    QStringList tmpLog;
    m_Mutex.lock();
    m_aLog.swap(tmpLog); // this function never fails
    m_Mutex.unlock();
    if (m_bShowGUI != isVisible())
        setVisible(m_bShowGUI);
    while (!tmpLog.isEmpty())
        ui->logEdit->appendPlainText(tmpLog.takeFirst());
}

/**
 * \brief handles the gui interaction to connect with server.
 */

void ClientGui::connectWithServer()
{
    quint16 port = ui->portLineEdit->text().toUShort();
    QHostAddress IP = QHostAddress(ui->iPLineEdit->text());
    ui->textBox->setPlainText("connecting ...");
    ui->textBox->repaint();
    if(!ui->activeCheck->isChecked())
        SECoP_C_startInactive();
    ClientGui::addNode(IP,port);
    ui->tabWidget->setCurrentIndex(0);
}

/**
 * \brief handles the gui interaction to disconnect from server.
 */
void ClientGui::disconnectFromServer()
{
    //ui->checkBoxPrevent->setChecked(true);
    m_bCheckPrevent=true;
    if(ui->checkAllNodes->isChecked())
    {                
       //unsure if it is a good idea to allow all nodes disconnect that may be unwanted behaviour
       //this gui is made
    }
    else
    {
        quint16 port = ui->portLineEdit->text().toUShort();
        QHostAddress IP = QHostAddress(ui->iPLineEdit->text());
        ClientGui::removeNode(IP,port);
    }
}

/**
 * \brief Exported function that adds a node and connects.
 * \param[in] IP   ip address of SEC node
 * \param[in] Port TCP port of SEC node
 * \return true if success else false
 */
bool ClientGui::addNode(QHostAddress IP, quint16 Port)
{
    //if the address (ip+port) is already used exit
    for (auto i=m_apConnections.constBegin(); i!=m_apConnections.constEnd();i++)
    {
        QTcpSocket* socket=(*i)->pSocket;
        if (socket->peerAddress()==IP && socket->peerPort()==Port)
        {
            ui->textBox->append("already connected to: "+IP.toString()+" on Port: "+ QString::number(Port));
            ui->textBox->repaint();
            return false;
        }
    }
    //address is new so open connection and send describe msg
    NodeConnection* pConnection=new NodeConnection;
    m_apConnections.append(pConnection);
    pConnection->Host    = IP;
    pConnection->wPort   = Port;
    pConnection->pSocket = new QTcpSocket(this);
    m_hConnectionHash[pConnection->pSocket] = pConnection;
    pConnection->pSocket->connectToHost(IP, Port);
    if (pConnection->pSocket->waitForConnected(2000))
    {
        connect(pConnection->pSocket, SIGNAL(readyRead()),
                this, SLOT(readTcpDatastream()), Qt::QueuedConnection);
        connect(pConnection->pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(errorTcpDatastream(QAbstractSocket::SocketError)), Qt::QueuedConnection);
        connect(pConnection->pSocket, SIGNAL(connected()),
                this, SLOT(connectedTcpDatastream()), Qt::QueuedConnection);
        pConnection->iState = ClientGui::NodeConnection::STATE_DESCRIBING;
        ui->textBox->append("describing ...");
        ui->textBox->repaint();
        sendDescribeMsg(pConnection->pSocket);
    }
    else
        pConnection->iState = ClientGui::NodeConnection::STATE_REJECTED;
    //wait for answer; if no answer after 2 seconds remove connection
    qint64 qiStart(QDateTime::currentMSecsSinceEpoch());
    for (;;)
    {
        if (pConnection->pSocket->waitForReadyRead(10))
            readTcpDatastream(pConnection);
        if (pConnection->pSocket->bytesAvailable() > 0)
            readTcpDatastream(pConnection);
        if (pConnection->iState!=ClientGui::NodeConnection::STATE_CONNECTED)
            if ((QDateTime::currentMSecsSinceEpoch() - qiStart) > 2000)
                pConnection->iState=ClientGui::NodeConnection::STATE_REJECTED;
        switch (pConnection->iState)
        {
            case ClientGui::NodeConnection::STATE_CONNECTING:
            case ClientGui::NodeConnection::STATE_DESCRIBING:
            case ClientGui::NodeConnection::STATE_ACTIVATING:
                QThread::msleep(10);
                continue;
            case ClientGui::NodeConnection::STATE_CONNECTED:
                break;
            default:
                    removeNode(pConnection, true);
                    ui->iPLineEdit->setText(IP.toString());
                    ui->portLineEdit->setText(QString::number(Port));
                    ui->textBox->append("could not establish connection to :"+IP.toString()+" on Port: "+ QString::number(Port));
                    return false;
        }
        break;
    }
    return true;
}

/**
 * \brief  function that removes a node.
 * \param[in] pConnection pointer to connection
 * \param[in] bDelete
 */
void ClientGui::removeNode(NodeConnection* pConnection, bool bDelete)
{
    if (pConnection->pSocket != nullptr)
    {
        pConnection->pSocket->disconnect();
        pConnection->pSocket->deleteLater();
        pConnection->pSocket = nullptr;
    }
    if (pConnection->pTimer != nullptr)
    {
        pConnection->pTimer->stop();
        pConnection->pTimer->deleteLater();
        pConnection->pTimer = nullptr;
    }
    if (bDelete)
    {
        ConnectedNodesHash.remove(pConnection->szNodeName);
        removeNodeFromTree(pConnection->szNodeName);
        m_hConnectionHash.remove(pConnection->pSocket);
        m_hConnections.remove(pConnection->szNodeName);
        m_apConnections.removeAll(pConnection);
        delete pConnection;
    }
    else
        pConnection->iState=ClientGui::NodeConnection::STATE_REJECTED;
}

/**
 * \brief  Exported function that removes a node.
 * \param[in] IP
 * \param[in] Port
 */
void ClientGui::removeNode(QHostAddress IP, quint16 Port)
{
    for (auto it=m_apConnections.begin(); it!=m_apConnections.end(); ++it)
    {
        QTcpSocket* pSocket((*it)->pSocket);
        if (pSocket != nullptr && pSocket->peerPort()==Port && pSocket->peerAddress()==IP)
        {
            removeNode(*it, true);
            break;
        }
    }
    ui->tabWidget->setTabText(static_tabs-1, "module");
    removeUnusedTabs();
}

/**
 * \brief  function calls the SECoP describe message.
 * \param[in] socket pointer to socket
 */
void ClientGui::sendDescribeMsg(QTcpSocket* socket)
{
    QByteArray comm ="describe";
    socket->write(comm.append(10));
    socket->waitForBytesWritten();
    //
}
/**
 * \brief  function returns the SECoP describe message.
 * \param[in] IP
 * \param[in] Port
 * \return QString: nodename if success else null
 */
QString ClientGui::getNodeNameByIP(QHostAddress IP, int Port)
{
    QString retval = nullptr;
    for (auto it=m_apConnections.begin(); it!=m_apConnections.end(); ++it)
    {
        QTcpSocket* pSocket((*it)->pSocket);
        if (pSocket != nullptr && pSocket->peerAddress() == IP && pSocket->peerPort() == Port)
        {
            retval= (*it)->szNodeName;
            break;
        }
    }
    return retval;
}
/*
this will be rewritten
*/

void ClientGui::errorTcpDatastream(QAbstractSocket::SocketError iError)
{
    Q_UNUSED(iError);
    QTcpSocket* pServer(dynamic_cast<QTcpSocket*>(sender()));
    if (pServer == nullptr || !m_hConnectionHash.contains(pServer))
        return;
    NodeConnection* pConnection(m_hConnectionHash[pServer]);
    if (pConnection == nullptr || pConnection->pSocket != pServer)
        return;
    if (pConnection->iState != ClientGui::NodeConnection::STATE_RECONNECT)
    {
        pConnection->pSocket->disconnectFromHost();
        pConnection->iState = ClientGui::NodeConnection::STATE_RECONNECT;
        pConnection->iCount = 0;
        if (pConnection->pTimer == nullptr)
            pConnection->pTimer = new QTimer();
        pConnection->pTimer->stop();
        pConnection->pTimer->disconnect();
        connect(pConnection->pTimer, SIGNAL(timeout()),
                this, SLOT(reconnectTimer()));
        pConnection->pTimer->start(500);
    }
}

void ClientGui::reconnectTimer()
{
    QTimer* pTimer(dynamic_cast<QTimer*>(sender()));
    if (pTimer == nullptr)
        return;
    for (auto it = m_apConnections.begin(); it != m_apConnections.end(); ++it)
    {
        NodeConnection* pConnection(*it);
        if (pConnection->pTimer == pTimer && pConnection->iState == ClientGui::NodeConnection::STATE_RECONNECT)
        {
            if (pConnection->iCount >= 100)
            {
                // reconnect error
                pConnection->iState = ClientGui::NodeConnection::STATE_REJECTED;
                pConnection->iCount = 0;
#pragma message(CPP_WARNINGPREFIX "TODO: handle reconnect error")
            }
            else
            {
                switch (++pConnection->iCount)
                {
                    case 1:  // 500ms
                    case 2:  //  1sec
                    case 10: //  5sec
                    case 20: // 10sec
                    case 40: // 20sec
                    case 60: // 30sec
                        switch (pConnection->pSocket->state())
                        {
                            case QAbstractSocket::ConnectingState:
                            case QAbstractSocket::ConnectedState:
                                pConnection->pSocket->disconnectFromHost();
                                pConnection->pSocket->disconnect();
                                connect(pConnection->pSocket, SIGNAL(readyRead()),
                                        this, SLOT(readTcpDatastream()), Qt::QueuedConnection);
                                connect(pConnection->pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                                        this, SLOT(errorTcpDatastream(QAbstractSocket::SocketError)), Qt::QueuedConnection);
                                connect(pConnection->pSocket, SIGNAL(connected()),
                                        this, SLOT(connectedTcpDatastream()));
                                pConnection->pSocket->connectToHost(pConnection->Host, pConnection->wPort);
                                break;
                            default:
                                break;
                        }
                        break;
                }
            }
        }
    }
}

void ClientGui::connectedTcpDatastream()
{
    QTcpSocket* pServer(dynamic_cast<QTcpSocket*>(sender()));
    if (pServer != nullptr && m_hConnectionHash.contains(pServer))
    {
        NodeConnection* pConnection(m_hConnectionHash[pServer]);
        if (pConnection->pSocket == pServer)
        {
            switch (pConnection->iState)
            {
                case ClientGui::NodeConnection::STATE_CONNECTING:
                case ClientGui::NodeConnection::STATE_RECONNECT:
                    pConnection->bReconnect = (pConnection->iState == ClientGui::NodeConnection::STATE_RECONNECT);
                    pConnection->iState     = ClientGui::NodeConnection::STATE_DESCRIBING;
                    pConnection->iCount     = 0;
                    ui->textBox->append("describing ...");
                    ui->textBox->repaint();
                    sendDescribeMsg(pConnection->pSocket);
                    break;
                default:
                    break;
            }
        }
    }
}

void ClientGui::readTcpDatastream()
{
    QTcpSocket* pServer(dynamic_cast<QTcpSocket*>(sender()));
    if (pServer != nullptr && m_hConnectionHash.contains(pServer))
        readTcpDatastream(m_hConnectionHash[pServer]);
}

void ClientGui::readTcpDatastream(NodeConnection* pConnection)
{
    QString modname= ui->tabWidget->tabText(module_tab);
    if (pConnection == nullptr || pConnection->pSocket == nullptr)
        return;
    while (pConnection->pSocket->canReadLine())
    {
        QByteArray szText(pConnection->pSocket->readLine());
        bool bErrMsg(true), bTabVisible(false);
        qDebug().nospace() << "recv: " << szText;
        int iPos(szText.indexOf(' '));
        QString szAction, szSpecifier;
        nlohmann::json vJsonValue;
        if (iPos >= 0)
        {
            szAction = szText.left(iPos);
            int iPos2(szText.indexOf(' ', iPos + 1));
            if (iPos >= 0)
            {
                szSpecifier = szText.mid(iPos + 1, iPos2 - iPos - 1);
                try
                {
                    vJsonValue = nlohmann::json::parse(szText.mid(iPos2 + 1).toStdString());
                }
                catch (nlohmann::json::exception& e)
                {
                    qDebug().nospace() << "got json exception " << e.what();
                }
                catch (...)
                {
                    qDebug().nospace() << "got unknown generic exception while json parsing";
                }
            }
            else
            {
                szSpecifier = szText;
                while (szSpecifier.endsWith('\r') || szSpecifier.endsWith('\n'))
                    szSpecifier.chop(1);
            }
        }
        else
        {
            szAction = szText;
            while (szAction.endsWith('\r') || szAction.endsWith('\n'))
                szAction.chop(1);
        }

        switch (pConnection->iState)
        {
        case ClientGui::NodeConnection::STATE_DESCRIBING:
            if (szAction.compare("describing") == 0)
            {
                buildStructure(pConnection, szText);
                if (!pConnection->szNodeName.isEmpty())
                    bErrMsg = false;
            }
            break;
        case ClientGui::NodeConnection::STATE_ACTIVATING:
        case ClientGui::NodeConnection::STATE_CONNECTED:
            {
                if (ui->tabWidget->currentIndex()==module_tab)//test if tab is visible
                    bTabVisible=true;
                if (szAction.compare("reply", Qt::CaseInsensitive) == 0 ||
                    szAction.compare("changed", Qt::CaseInsensitive) == 0)
                {
                    pConnection->request.remove(szAction + " " + szSpecifier);
                    bool bChanged(szAction.compare("changed", Qt::CaseInsensitive) == 0);

                    if (ConnectedNodesHash.contains(pConnection->szNodeName))
                    {
                        QHash<QString, NodeStructureItem> &h = ConnectedNodesHash[pConnection->szNodeName];
                        if (h.contains(szSpecifier))
                        {
                            SECoP_dataPtr newval(h[szSpecifier].pValueOrArg->duplicate());
                            nlohmann::json vJsonQualifier;

                            if (vJsonValue.is_array() && vJsonValue.size() >= 2 && vJsonValue[1].is_object())
                            {
                                vJsonQualifier = vJsonValue[1];
                                vJsonValue = vJsonValue[0];
                                if (vJsonValue.size() > 2)
                                {
#pragma message(CPP_WARNINGPREFIX "TODO: what about additional SECoP data")
                                }
                            }

                            if (newval.get() == nullptr || !newval->importSECoP(vJsonValue, true))
                            {
                                bErrMsg = true;
                               // anything went wrong ...: wrong data type or no memory left;
                            }
                            // newval contains new value, you may use it
                            h[szSpecifier].pValueOrArg = newval;
                            h[szSpecifier].init=false;
                            if (vJsonQualifier.contains("t"))
                            {
                                nlohmann::json vTimestamp(vJsonQualifier["t"]);
                                if (vTimestamp.is_number_float())
                                    h[szSpecifier].timestamp = vTimestamp.get<double>();
                                else if (vTimestamp.is_number_integer())
                                    h[szSpecifier].timestamp = vTimestamp.get<int64_t>();
                                else if (vTimestamp.is_number_unsigned())
                                    h[szSpecifier].timestamp = vTimestamp.get<uint64_t>();
                                else
                                    h[szSpecifier].timestamp = std::numeric_limits<double>::quiet_NaN();
                            }
                            if (vJsonQualifier.contains("e"))
                            {
                                SECoP_dataPtr newsigma(h[szSpecifier].pValueOrArg->duplicate());
                                if (newsigma.get() != nullptr && newsigma->importSECoP(vJsonQualifier["e"], true))
                                    h[szSpecifier].pSigmaOrResult = newsigma;
                            }
                            else
                                h[szSpecifier].pSigmaOrResult = nullptr;
                            int iRow(hash[szSpecifier]);
                            if (bTabVisible)//test if tab is visible
                            {
                                QWidget* pPage = h[modname+":"].pModuleTabPage;
                                bool bSetValue(false);
                                if(pPage)
                                {
                                    if((pConnection->szNodeName+":"+modname+":").startsWith(getStatusLabelText(pPage)+":"))
                                        bSetValue=true;
                                    if(bSetValue)
                                    {
                                        QString text = QString(newval->exportSECoP());
                                        setTableItemText(pPage,"tableWidget",text,iRow,accValueCol);
                                        if(bChanged)
                                            setTableItemText(pPage,"tableWidget",szText,0,0);
                                        else
                                            setTableItemText(pPage,"tableWidget",text,0,0);
                                    }
                                }
                            }
                        }
                    }
                    ui->textBox->setPlainText(szText);
                }
                if (szAction.compare("update") == 0)
                {
                    bErrMsg = false;
                    if (ConnectedNodesHash.contains(pConnection->szNodeName))
                    {
                        QHash<QString, NodeStructureItem> &h = ConnectedNodesHash[pConnection->szNodeName];
                        if (h.contains(szSpecifier))
                        {
                            SECoP_dataPtr newval(h[szSpecifier].pValueOrArg->duplicate());
                            nlohmann::json vJsonQualifier;

                            if (vJsonValue.is_array() && vJsonValue.size() >= 2 && vJsonValue[1].is_object())
                            {
                                vJsonQualifier = vJsonValue[1];
                                vJsonValue = vJsonValue[0];
                                if (vJsonValue.size() > 2)
                                {
#pragma message(CPP_WARNINGPREFIX "TODO: what about additional SECoP data")
                                }
                            }

                            if (newval.get() == nullptr || !newval->importSECoP(vJsonValue, true))
                            {
                                bErrMsg = true;
                               // anything went wrong ...: wrong data type or no memory left;
                            }
                            // newval contains new value, you may use it
                            h[szSpecifier].pValueOrArg = newval;
                            h[szSpecifier].init=false;
                            if (vJsonQualifier.contains("t"))
                            {
                                nlohmann::json vTimestamp(vJsonQualifier["t"]);
                                if (vTimestamp.is_number_float())
                                    h[szSpecifier].timestamp = vTimestamp.get<double>();
                                else if (vTimestamp.is_number_integer())
                                    h[szSpecifier].timestamp = vTimestamp.get<int64_t>();
                                else if (vTimestamp.is_number_unsigned())
                                    h[szSpecifier].timestamp = vTimestamp.get<uint64_t>();
                                else
                                    h[szSpecifier].timestamp = std::numeric_limits<double>::quiet_NaN();
                            }
                            if (vJsonQualifier.contains("e"))
                            {
                                SECoP_dataPtr newsigma(h[szSpecifier].pValueOrArg->duplicate());
                                if (newsigma.get() != nullptr && newsigma->importSECoP(vJsonQualifier["e"], true))
                                    h[szSpecifier].pSigmaOrResult = newsigma;
                            }
                            else
                                h[szSpecifier].pSigmaOrResult = nullptr;
                            int iRow(hash[szSpecifier]);
                            if (bTabVisible)//test if tab is visible
                            {
                                QWidget* pPage = h[modname+":"].pModuleTabPage;
                                if(pPage)
                                {
                                    if((pConnection->szNodeName+":"+modname+":").startsWith(getStatusLabelText(pPage)+":"))
                                        setTableItemText(pPage,"tableWidget",QString(newval->exportSECoP()),iRow,accValueCol);
                                }
                            }
                        }
                    }
                    if(!m_bHideAsyncMSG)
                        ui->textBox->setPlainText(szText);
                }
                else if (szAction.compare("describing") == 0)//
                {
                    bErrMsg = false;
                    ui->textBox->setPlainText(szText);
                }
                else if (szAction.compare("done") == 0)//this is expected after a call of a command
                {
                    if(pConnection->request.contains(szText))
                    {
                        pConnection->request.remove(szText);
                    }
                    bErrMsg = false;
                    QList<QByteArray> aParts(szText.split(' '));
                    QByteArray modpar = aParts[1];
                    QByteArray val    = aParts[2];
                    ui->textBox->setPlainText(szText);
                    if (bTabVisible && ConnectedNodesHash.contains(pConnection->szNodeName))//test if tab is visible
                    {
                        QWidget* pPage = ConnectedNodesHash[pConnection->szNodeName][modname+":"].pModuleTabPage;
                        bool bSetValue(false);
                        if(pPage)
                        {
                            if((pConnection->szNodeName+":"+modname+":").startsWith(getStatusLabelText(pPage)+":"))
                                bSetValue=true;
                            if(bSetValue)
                            {
/*
                                if(hash.contains(modpar))
                                {
                                    int iRow=(hash[modpar]);
                                    setTableItemText(pPage,"tableWidget",szText,iRow,accValueCol);
                                }
*/
                                setTableItemText(pPage,"tableWidget",szText,0,0);
                            }
                        }
                    }
                }
                else if (szAction.compare("pong") == 0)//this is expected after a call of a ping
                {
                    bErrMsg = false;
                    ui->textBox->setPlainText(szText);
                }
                else if (szAction.compare("commands") == 0)//this is expected after a call of help
                {
                    bErrMsg = false;
                    ui->textBox->setPlainText(szText);
                }
                else if (szAction.startsWith("ISSE&SINE2020,SECoP", Qt::CaseInsensitive) ||
                         szAction.startsWith("SINE2020&ISSE,SECoP", Qt::CaseInsensitive))//this is expected after a call of a *IDN?
                {
                    bErrMsg = false;
                    ui->textBox->setPlainText(szText);
                }
                else if (szAction.compare("error") == 0)//
                {
                    bErrMsg = false;
                    if (vJsonValue.is_string())
                    {
                        QString szRequest(QString::fromStdString(vJsonValue.get<std::string>()));
                        int iPos(szRequest.indexOf(' '));
                        while (iPos >= 0)
                        {
                            iPos = szRequest.indexOf(' ', iPos + 1);
                            if (iPos < 0)
                                iPos = szRequest.size();
                            szRequest.chop(szRequest.size() - iPos);
                            if (szRequest.startsWith("read "))
                                szRequest.replace("read ", "reply ");
                            else if (szRequest.startsWith("change "))
                                szRequest.replace("change ", "changed ");
                            else if (szRequest.startsWith("do "))
                                szRequest.replace("do ", "done ");
                            else
                                break;
                            pConnection->request.remove(szRequest);
                            break;
                        }
                    }
                    ui->textBox->setPlainText(szText);
                    if (bTabVisible && ConnectedNodesHash.contains(pConnection->szNodeName))//test if tab is visible
                    {
                        QWidget* pPage = ConnectedNodesHash[pConnection->szNodeName][modname+":"].pModuleTabPage;
                        bool bSetValue(false);
                        if(pPage)
                        {
                            if((pConnection->szNodeName+":"+modname+":").startsWith(getStatusLabelText(pPage)+":"))
                                bSetValue=true;
                            if(bSetValue)
                                setTableItemText(pPage,"tableWidget",szText,0,0);
                        }
                    }
                }
                else if (szAction.compare("active") == 0 || szAction.compare("inactive") == 0)//
                {
                    bErrMsg = false;
                    ui->textBox->setPlainText(g_actualNode+" is "+szText);
                    if (pConnection->iState == ClientGui::NodeConnection::STATE_ACTIVATING)
                        pConnection->iState = ClientGui::NodeConnection::STATE_CONNECTED;
                }
                break;
            }
        case ClientGui::NodeConnection::STATE_REJECTED:
        default:
            bErrMsg = false;
            removeNode(pConnection, true);
            break;
        }
        if (bErrMsg)
        {
            if(!m_bHideAsyncMSG)
                ui->textBox->setPlainText("this message was unexpected, did you try to send a special command?\n" + szText);
        }
    }
}

/**
* \brief  this function sends data to node.
* \param[in] node nodename
* \param[in] data data
*/
void ClientGui::writeToNode(QString node, QByteArray data)
{
    NodeConnection* pConnection = m_hConnections[node];
    if (pConnection == nullptr)
        return;

    if(!data.endsWith(10))
        data.append(10);
    pConnection->pSocket->write(data);
    qDebug().nospace() << "send: " << data;
}

/**
 * \brief  function sends data from the commandline.
 */
void ClientGui::writeTcpDatastream()
{
    NodeConnection* pConnection = m_hConnections[g_actualNode];
    if (pConnection == nullptr)
        return;

    QString szRequest(ui->CommandLineEdit->text());
    QByteArray block(szRequest.toUtf8());
    block.append(10);
    pConnection->pSocket->write(block);
    qDebug().nospace() << "send: " << block;
    int iPos(szRequest.indexOf(' '));
    while (iPos >= 0)
    {
        iPos = szRequest.indexOf(' ', iPos + 1);
        if (iPos < 0)
            iPos = szRequest.size();
        szRequest.chop(szRequest.size() - iPos);
        if (szRequest.startsWith("read "))
            szRequest.replace("read ", "reply ");
        else if (szRequest.startsWith("change "))
            szRequest.replace("change ", "changed ");
        else if (szRequest.startsWith("do "))
            szRequest.replace("do ", "done ");
        else
            break;
        pConnection->request.insert(szRequest, 0);
        break;
    }
}

void ClientGui::CalltestRead(QString module)
{
    //std::string ClientGui::testRead(QString Node, QString Module, QString ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal, std::string &SECoPtripel, qint64 &respTime, qint64 maxTime)

    SECoP_dataPtr pSecVal;
    double dTimSta(std::numeric_limits<double>::quiet_NaN());
    SECoP_dataPtr pSecErr;
    std::string SECoPTripel;
    qint64 resp=0;
    std::string answer=testRead(g_actualNode,module,"value",pSecVal,dTimSta,pSecErr,SECoPTripel,resp,5000);
    QString respTime = QString::number(resp,10);
    if(answer.empty())
    {
//        if(ui->tableWidget->item(0,0))
//            ui->tableWidget->item(0,0)->setText(QString::fromStdString( pSecVal->toStdString())+" "+respTime);
        //ui->statusLabel->setText(QString::fromStdString( pSecVal->toStdString())+" "+respTime);
    }
    else
    {
//        if(ui->tableWidget->item(0,0))
//            ui->tableWidget->item(0,0)->setText(QString::fromStdString(answer)+" "+respTime);
       // ui->statusLabel->setText(QString::fromStdString(answer)+" "+respTime);
    }
}

/**
* \brief  this function calls an accessible from the tablewidget. depending on the rwx type a read, change or do message is send.
* \param[in] call
* \param[in] row
* \param[in] pTTW
*/
void ClientGui::CallAcc(QString call, int row, QTableWidget* pTTW)
{
    NodeConnection* pConnection = m_hConnections[g_actualNode];
    if (pConnection == nullptr)
        return;
    QString valuefromLineEdit = nullptr;
    if(!call.startsWith("read"))
    {
        valuefromLineEdit =qobject_cast<QLineEdit*>(pTTW->cellWidget(row,lineEditCol))->text().trimmed();
    }
    QByteArray SECoPCommand=nullptr;
    if (valuefromLineEdit.isEmpty())
    {
        SECoPCommand=call.toUtf8();
    }
    else
    {
        SECoPCommand=(call+" "+valuefromLineEdit).toUtf8();
    }
    SECoPCommand.append(10);
    pConnection->pSocket->write(SECoPCommand);
    qDebug().nospace() << "send: " << SECoPCommand;

    SECoPCommand.chop(1);
    int iPos(SECoPCommand.indexOf(' '));
    while (iPos >= 0)
    {
        iPos = SECoPCommand.indexOf(' ', iPos + 1);
        if (iPos < 0)
            iPos = SECoPCommand.size();
        SECoPCommand.chop(SECoPCommand.size() - iPos);
        if (SECoPCommand.startsWith("read "))
            SECoPCommand.replace("read ", "reply ");
        else if (SECoPCommand.startsWith("change "))
            SECoPCommand.replace("change ", "changed ");
        else if (SECoPCommand.startsWith("do "))
            SECoPCommand.replace("do ", "done ");
        else
            break;
        pConnection->request.insert(SECoPCommand, 0);
        break;
    }
}

/**
* \brief  this function returns the actual nodename.
* \return nodename
*/
QString ClientGui::getNodeID()
{
    return g_actualNode;
}

/**
* \brief  this function returns all connected nodenames as list.
* \return nodenames
*/
QStringList ClientGui::getNodeNamesList()
{
    return ConnectedNodesHash.keys();
}

/**
* \brief  this function sets the start inactive bool to false. a new connected node then will not be activated.
*/
void ClientGui::setInactiveBool()
{
    m_bStartActive=false;
}

/**
* \brief  this function toggles the asynchronous message bool. This bool is used to hide or show asynchronous messages.
*/
void ClientGui::toggleAsyncMSGBool(bool bChecked)
{
    m_bHideAsyncMSG = bChecked;
}

/**
* \brief  this function returns all node property names as list.
* \return node property names
*/
QStringList ClientGui::getNodePropertiesKeyList()
{
    QStringList NodeKeys;
    NodeKeys.clear();

    qjoNodeProps = JSONdoc;                             //copy complete JSON to qjoNodeProps
    //qjoMods.insert("modules",qjoNodeProps["modules"]);  //copy the key "modules" to qjoMods
    qjoNodeProps.erase("modules");                      //remove the key "modules" from NodeProps
    for (auto &it : qjoNodeProps.items())
        NodeKeys.append(QString::fromStdString(it.key()));
    return NodeKeys;
}

/**
* \brief  this function returns all modules names of a node as list.
* \param[in] NodeName node name
* \return list of module names
*/
QStringList ClientGui::getModuleNamesList(QString NodeName)
{
    if (ConnectedNodesHash.contains(NodeName))
        return ConnectedNodesHash[NodeName][":"].ContentList;
    return QStringList();
}

/**
* \brief  this function returns how much accessibles are in a nodes module.
* \param[in] node   node name
* \param[in] module module name
* \return number of accessibles of a module
*/
int ClientGui::getNumberOfAcc(QString Node,QString modName)
{
    return ConnectedNodesHash[Node][modName+":"].ContentList.count();
}

/**
* \brief  this function reads the value of a parameter from the structure. if bool forced is true an additional read message is send.
*         the return message is success or a error/warning message
* \param[in]  Node       node name
* \param[in]  Module     module name
* \param[in]  ParamName  parameter name
* \param[out] Value      value
* \param[out] dTimestamp timestamp
* \param[out] ErrorVal   error
* \param[in]  forced     bool that if given allows to send a read request
* \return error message
*/
std::string ClientGui::readParameter(QString Node, QString Module, QString ParamName, SECoP_dataPtr &Value, double &dTimestamp,SECoP_dataPtr &ErrorVal, bool  forced)
{
    QByteArray SECoPCommand=nullptr;
    std::string retval;
    NodeStructureItem item;
    NodeConnection* pConnection;

    Value=nullptr;
    dTimestamp=0;
    ErrorVal=nullptr;

    if(ConnectedNodesHash.contains(Node))//testing for Node
    {
        pConnection = m_hConnections[Node];
        if(ConnectedNodesHash[Node].contains(Module+":"+ParamName))//testing for module and parameter
        {
            item = (ConnectedNodesHash[Node])[Module+":"+ParamName];
        }
        else
        {
            if(ConnectedNodesHash[Node].contains(Module+":value"))
            {
                retval = "error Node "+Node.toStdString()+" has no parameter "+ParamName.toStdString()+" in module: "+Module.toStdString();
            }
            else
            {
                retval = "error Node "+Node.toStdString()+" has no module: "+Module.toStdString();
            }

            return retval;
        }
    }
    else
    {
        retval ="Node: "+Node.toStdString()+" does not exist";
        return retval;
    }

    if( item.rwxType=='x')//
    {
            retval="can't read a command";
            return  retval;
    }

    if(item.rwxType=='r'||item.rwxType=='w')
    {
        Value=item.pValueOrArg;
        dTimestamp=item.timestamp;
        ErrorVal=item.pSigmaOrResult;
        if(forced)//try to send a message if forced is true
        {
            SECoPCommand.append("reply "+Module+":"+ParamName);            //first the expected answer is created to store this as hash in the request list
            if (!pConnection->request.contains(SECoPCommand))               //answer of a change request is changed
            {
                qint64 qiStart(QDateTime::currentMSecsSinceEpoch());
                pConnection->request.insert(SECoPCommand,qiStart);
                SECoPCommand.append(10);
                SECoPCommand.replace("reply ","read ");
                pConnection->pSocket->write(SECoPCommand);                  //send >>change mod:accessible value
                retval="read message send";
            }
            else
            {
                retval=""+Module.toStdString()+":"+ParamName.toStdString()+"on Node: "+Node.toStdString()+" is still busy with reading no additional read send";
            }
        }
    }

    return retval;
}
/**
* \brief  this function extends the standard read function readParameter. It reads the value of a parameter not from the structure it sends a read message and waits maxTime for an answer.
*         the return message is success or a error/warning message.
* \param[in]  Node        node name
* \param[in]  Module      module name
* \param[in]  ParamName   parameter name
* \param[out] Value       value
* \param[out] dTimestamp  timestamp
* \param[out] ErrorVal    error
* \param[out] SECoPtripel
* \param[out] respTime    time that an answer needed
* \param[in]  maxTime     maxtime to wait for an answer
* \return error message
*/
std::string ClientGui::testRead(QString Node, QString Module, QString ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal, std::string &SECoPtripel, qint64 &respTime, qint64 maxTime)
{
    QByteArray SECoPCommand=nullptr;
    std::string retval;
    NodeStructureItem item;
    NodeConnection* pConnection;
    qint64 start,now,time;

    Value=nullptr;
    dTimestamp=0;
    ErrorVal=nullptr;
    start = now = QDateTime::currentMSecsSinceEpoch();
    respTime = time = 0;

    if(ConnectedNodesHash.contains(Node))//testing for Node
    {
        pConnection = m_hConnections[Node];
        if(ConnectedNodesHash[Node].contains(Module+":"+ParamName))//testing for module and parameter
        {
            item = (ConnectedNodesHash[Node])[Module+":"+ParamName];
        }
        else
        {
            if(ConnectedNodesHash[Node].contains(Module+":value"))//each parameter must have a value
            {
                retval = "error Node "+Node.toStdString()+" has no parameter "+ParamName.toStdString()+" in module: "+Module.toStdString();
            }
            else
            {
                retval = "error Node "+Node.toStdString()+" has no module: "+Module.toStdString();
            }

            return retval;
        }
    }
    else
    {
        retval ="error Node: "+Node.toStdString()+" does not exist";
        return retval;
    }

    if (item.rwxType == 'R' || item.rwxType=='W')
        retval = "the parameter is constant";
    if( item.rwxType=='x')//
    {
            retval="error can't read a command";
            return  retval;
    }
    if(item.rwxType=='r'||item.rwxType=='w')
    {
        Value=item.pValueOrArg;
        dTimestamp=item.timestamp;
        ErrorVal=item.pSigmaOrResult;

        SECoPCommand.append("reply "+Module+":"+ParamName);            //first the expected answer is created to store this as hash in the request list
        if (!pConnection->request.contains(SECoPCommand))               //answer of a change request is changed
        {
            start=QDateTime::currentMSecsSinceEpoch();
            pConnection->request.insert(SECoPCommand,start);
            SECoPCommand.append(10);
            SECoPCommand.replace("reply ","read ");
            pConnection->pSocket->write(SECoPCommand);                  //send >>change mod:accessible value
            retval="success";
        }
        else
        {
            retval="warning "+Module.toStdString()+":"+ParamName.toStdString()+"on Node: "+Node.toStdString()+" is still busy with reading no additional read send";
        }

    }
    if(retval=="success")
    {
        while(time<maxTime)
        {
            if (pConnection->pSocket->waitForReadyRead(10))
                readTcpDatastream(pConnection);
            if (pConnection->pSocket->bytesAvailable() > 0)
                readTcpDatastream(pConnection);
            if(!pConnection->request.contains("reply "+Module+":"+ParamName))//answer came so entry was deleted from readTcpDatastream
            {
                now=QDateTime::currentMSecsSinceEpoch();
                time=now-start;

                Value=item.pValueOrArg;
                dTimestamp=item.timestamp;
                ErrorVal=item.pSigmaOrResult;
                respTime=time;
                retval.clear();

                retval.append("[");
                std::string val;
                if (!item.init)
                    val=item.pValueOrArg->exportSECoP().toStdString();
                retval.append(val);
                bool bFirst(true);
                if(!item.init)
                {
                using std::isnan;
                using std::isinf;
                    if (!isnan(item.timestamp) && !isinf(item.timestamp))
                    {
                        retval.append(",");
                        if (bFirst)
                        {
                            retval.append("{");
                            bFirst = false;
                        }
                        retval.append("\"t\":");
                        retval.append(QByteArray::number(item.timestamp).toStdString());
                    }
                    if (item.pSigmaOrResult.get() != nullptr && item.pSigmaOrResult->isValid())
                    {
                        retval.append(",");
                        if (bFirst)
                        {
                            retval.append("{");
                            bFirst = false;
                        }
                        retval.append("\"e\":");
                        retval.append(item.pSigmaOrResult->exportSECoP().toStdString());
                    }
                }
                if (!bFirst)
                    retval.append("}");
                retval.append("]\n");

                SECoPtripel=retval;
                retval.clear();
                return retval;
            }
            now=QDateTime::currentMSecsSinceEpoch();
            time=now-start;
        }
    retval="timeout";
    }
    return retval;
}


/**
 * \brief This function sends a change or a do message depending on (r)wx type. returns success or warning/error message.
 * \param[in]  Node   node name
 * \param[in]  Module module name
 * \param[in]  Acc    accessible name
 * \param[out] Value  value
 * \return success or warning/error message
 */
std::string ClientGui::writeParam(QString Node, QString Module, QString Acc, SECoP_dataPtr Value)
{
    QByteArray SECoPCommand=nullptr;
    std::string retval;
    NodeConnection* pConnection = m_hConnections[Node];
    if (pConnection == nullptr)
    {
        retval="node does not exist";
        return retval;
    }
    //test for readable writable or command (rwx)
    NodeStructureItem item = (ConnectedNodesHash[Node])[Module+":"+Acc];
    if (item.rwxType == 'R' || item.rwxType=='W')
        retval = "the parameter is constant";
    if (item.rwxType=='r')//Acc is readable
    {
        retval="the parameter is readonly nothing send";
    }
    if (item.rwxType=='w')//Acc is writable
    {
        SECoPCommand.append("changed "+Module+":"+Acc);                 //first the expected answer is created to store this as hash in the request list
        if (!pConnection->request.contains(SECoPCommand))               //answer of a change request is changed
        {
            qint64 qiStart(QDateTime::currentMSecsSinceEpoch());
            pConnection->request.insert(SECoPCommand,qiStart);
            SECoPCommand.append(" ");
            SECoPCommand.append(Value->exportSECoP());              //unsure about force JSON
            SECoPCommand.append(10);
            SECoPCommand.remove(6,1);
            pConnection->pSocket->write(SECoPCommand);                  //send >>change mod:accessible value
            retval="";//means is send
        }
        else
        {
            retval=""+Module.toStdString()+":"+Acc.toStdString()+" is busy";
        }
    }
    if(item.rwxType=='x')
    {
        SECoPCommand.append("done+ "+Module+":"+Acc);                   //first the expected answer is created to store this as hash in the request list
        if (!pConnection->request.contains(SECoPCommand))               //answer of a do request is done
        {
            qint64 qiStart(QDateTime::currentMSecsSinceEpoch());
            pConnection->request.insert(SECoPCommand,qiStart);
            if(Value)
            {
                SECoPCommand.append(" ");
                SECoPCommand.append(Value->exportSECoP());          //unsure about force JSON
            }
            SECoPCommand.append(10);
            SECoPCommand.remove(2,2);                                   //send >>do mod:accessible[ value] *the part in angular brackets is optional for commands
            pConnection->pSocket->write(SECoPCommand);
            retval="";//means is send
        }
        else
        {
            retval=""+Module.toStdString()+":"+Acc.toStdString()+" is busy";
        }
    }
    //TODO this is non blocking but after sending you cant rely that the changes where made because with this function the answer is not catched
    return retval;
}

/**
 * \brief This returns the number connected nodes.
 * \return number of nodes
 */
int ClientGui::getNumberOfNodes()
{
    return m_apConnections.size();
}

/**
 * \brief This returns the number of modules belonging to an node
 * \param[in] Node node name
 * \return number of modules in a node
 */
int ClientGui::getNumberOfModules(QString NodeName)
{
    if (ConnectedNodesHash.contains(NodeName))
       return ConnectedNodesHash[NodeName][":"].ContentList.size();
    return 0;
}

/**
 * \brief This returns the properties of an node.
 * \param[in] Node node name
 * \return JSON with properties
 */
nlohmann::json ClientGui::getNodeProperties(QString Node)
{
    return ConnectedNodesHash[Node][":"].properties;
}

/**
 * \brief This returns the properties of an module from the structure
 * \param[in] Node   node name
 * \param[in] Module module name
 * \return JSON with properties
 */
nlohmann::json ClientGui::getModuleProperties(QString Node, QString Module)
{
    return ConnectedNodesHash[Node][Module+":"].properties;
}

/**
 * \brief This returns the properties of an accessible from the structure
 * \param[in] Node   node name
 * \param[in] Module module name
 * \param[in] Acc    accessible name
 * \return JSON with properties
 */
nlohmann::json ClientGui::getAccProperties(QString Node, QString Module, QString Acc)
{
    return ConnectedNodesHash[Node][Module+":"+Acc].properties;
}

/**
 * \brief This function returns the list of names of accessibles from anode                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        belonging to a module .
 * \param[in] node node name
 * \param[in] mod  module name
 * \return list of all accessible names
 */
QStringList ClientGui::getAccNameList(QString node, QString mod)
{
    return ConnectedNodesHash[node][mod+":"].ContentList;
}

/**
 * \brief This returns the unit of an accessible from the structure
 * \param[in] node node name
 * \param[in] mod  module name
 * \param[in] acc  accessible name
 * \return unit of accessible
 */
QString ClientGui::getUnitOfAcc(QString node,QString mod,QString acc)
{
    return ConnectedNodesHash[node][mod+":"+acc].unit;
}

/**
 * \brief This function removes a node from the treewidget
 * \param[in] NodeName node name
 */
void ClientGui::removeNodeFromTree(QString NodeName)
{
    QTreeWidgetItem* itm=storedTreeItems.value(NodeName);
    delete itm;
    storedTreeItems.remove(NodeName);
}

/**
 * \brief This function adds a node to the treewidget
 * \param[in] NodeName node name
 */
void ClientGui::addNodeToTree(QString NodeName)
{
    QTreeWidgetItem* itm= new QTreeWidgetItem(ui->treeWidget);
    itm->setText(0,NodeName);
    storedTreeItems.insert(NodeName,itm);
    QStringList modules = getModuleNamesList(NodeName);//getModuleNamesListFromJSON();
    for (int i=0; i<modules.size(); i++)
    {
        addModulesToTree(itm,NodeName,modules.at(i));
    }
}

/**
 * \brief This function adds a module below the node to the treewidget
 * \param[in] ptrTreeParent pointer to treewidgetitem
 * \param[in] Node          name of node
 * \param[in] Mod           name of module
 */
void ClientGui::addModulesToTree(QTreeWidgetItem* ptrTreeParent, QString Node, QString mod)
{

    QTreeWidgetItem *itm= new QTreeWidgetItem();
    itm->setText(0,mod);

    ptrTreeParent->addChild(itm);
    QStringList acc(getAccNameList(Node,mod));
    for (int i = 0; i < acc.size(); ++i)
    {
        nlohmann::json qjoProp(getAccProperties(Node, mod, acc.at(i)));
        if (qjoProp.contains("readonly"))
        {
            bool b(false);
            if (qjoProp["readonly"].is_boolean())
                b = qjoProp["readonly"].get<bool>();
            addAccToTree(itm, acc.at(i), b ? 'r' : 'w');
        }
        else
            addAccToTree(itm, acc.at(i), 'x');
    }
}

/**
 * \brief This function adds a accessible below the module to the treewidget
 * \param[in] ptrTreeParent : pointer to treewidgetitem
 * \param[in] acc : name of accessible
 * \param[in] cType : Type of accessible (rwx)
 */
void ClientGui::addAccToTree(QTreeWidgetItem* ptrTreeParent,  QString acc, char cType)
{
    QTreeWidgetItem *itm = new QTreeWidgetItem();
    itm->setText(0, acc);
    itm->setText(1, QString(cType));
    switch (cType)
    {
        case 'r': itm->setBackgroundColor(0, QColor(0xE0,0xE0,0xE0)); break;
        case 'w': itm->setBackgroundColor(0, QColor(0xC0,0xC0,0xC0)); break;
        case 'x': itm->setBackgroundColor(0, QColor(0xA0,0xA0,0xA0)); break;
        default: itm->setBackgroundColor(0, QColor(0xC0,0xC0,0xFF)); break;
    }
    ptrTreeParent->addChild(itm);
}

/**
 * \brief This function removes additional group tabs that appear in a module if the shown module is changed
 */
void ClientGui::removeUnusedTabs()
{
    while (ui->tabWidget->count() > static_tabs)
        ui->tabWidget->removeTab(static_tabs);
}

/**
 * \brief This function sets additional tabs for each group that appears in a module
 * \param[in] grptab  pointer of tabwidget
 * \param[in] rwxType hint if the accessible is readable writeable or a command
 * \param[in] key
 */
//if a module has groups another tab with the name of the group is created
void ClientGui::addRowToGrpTable(QTableWidget* grptab, char rwxType, QString key)
{
    //QString grp=grptab->;
    int row=grptab->rowCount()+1;
    grptab->setRowCount(row);
    row--;

    QString name=key.split(":")[1];
    QString mod=key.split(":")[0];
    QString unit = getUnitOfAcc(g_actualNode,mod,name);
    QPushButton* pBut= new QPushButton();
    QLineEdit* pLineEdit= new QLineEdit();
    grptab->setItem(row,nameOfAccCol,new QTableWidgetItem(name));
    grptab->setItem(row,accValueCol,new QTableWidgetItem(mod));
    grptab->setItem(row,accUnitCol,new QTableWidgetItem(unit));
    switch (rwxType)
    {
        case 'r':
            break;
        case 'w':
            grptab->setCellWidget(row,lineEditCol,pLineEdit);
            pBut->setText("change");
            grptab->setCellWidget(row,buttonCol,pBut);
            break;
        case 'x':
            grptab->setCellWidget(row,lineEditCol,pLineEdit);
            pBut->setText("do");
            grptab->setCellWidget(row,buttonCol,pBut);
            break;
    }
}

/**
 * \brief This function sets the basic layout for each table in the moduletab
 * \param[in] ptabWidget pointer of tabwidget
 */
void ClientGui::setTableLayout(QTableWidget* ptabWidget)
{
    //settings of tableWidget

    ptabWidget->setShowGrid(false);
    ptabWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ptabWidget->horizontalHeader()->hide();
    ptabWidget->verticalHeader()->hide();
    ptabWidget->setColumnCount(numberOfColumns);//label|actValueLabel|unit|editLine|buttons
    ptabWidget->setSpan(0,0,1,numberOfColumns);

    ptabWidget->setColumnWidth(firstCol,25);
    ptabWidget->setColumnWidth(accUnitCol,45);

    QFrame* pHLine = new QFrame;
    pHLine->setFrameShape(QFrame::HLine);
    pHLine->setFrameShadow(QFrame::Sunken);
    pHLine->setLineWidth(1);

    QFrame* pHLine2 = new QFrame;
    pHLine2->setFrameShape(QFrame::HLine);
    pHLine2->setFrameShadow(QFrame::Sunken);
    pHLine2->setLineWidth(1);

    ptabWidget->setSpan(1,0,1,numberOfColumns);
    ptabWidget->setCellWidget(1,0,pHLine);
    ptabWidget->setRowHeight(1,9);

    ptabWidget->setSpan(6,0,1,numberOfColumns);
    ptabWidget->setCellWidget(6,0,pHLine2);
    ptabWidget->setRowHeight(6,9);


//    tabWidget->setCellWidget(1,vertikalLineCol,pVLine);
    ptabWidget->setColumnWidth(vertikalLineCol,9);

}

/*
 //used to switch between module view and console view
void ClientGui::showItems()
{
    if (ui->stdTAB->parent()->objectName()=="modtab")
    {
        QWidget* ptrSTD= ui->stdTAB->parentWidget();
        QWidget* ptrMOD= ui->tableWidget->parentWidget();
        //QWidget* ptrLOG= ui->logEdit->parentWidget();
        ui->stdTAB->setParent(ptrMOD);
        ui->tableWidget->setParent(ptrSTD);
        ui->stdTAB->show();
        ui->tableWidget->show();
    }
}
*/

/**
 * \brief This function handles user interaction wit the treewidgets shows properties to the info tab or switches the tab if a module is clicked.
 *        The infotab is used to show all descriptive JSON belonging to a node module or accessible.
 *
 * \param[in] ptrToItem pointer to treewidgetitem
 */
void ClientGui::showItem(QTreeWidgetItem* ptrToItem)
{
   QString MSG=nullptr;
   if (ptrToItem->childCount()==0)//is an accessible
   {
       g_actualItem = "accessible";
       QString node(ptrToItem->parent()->parent()->text(0));
       g_actualNode = node;
       QString module(ptrToItem->parent()->text(0));
       QString acc(ptrToItem->text(0));
       MSG = QString::fromStdString(getAccProperties(node, module, acc).dump(2, ' ', false, nlohmann::json::error_handler_t::replace));
   }
   else if (!ptrToItem->parent())//is a node
   {
       g_actualItem = "node";
       QString node(ptrToItem->text(0));
       g_actualNode = node;
       MSG = QString::fromStdString(getNodeProperties(node).dump(2, ' ', false, nlohmann::json::error_handler_t::replace));
   }
   else //is a module; it has a parent and it has childs (a module is at least readable so value as parameter must exist)
   {
       g_actualItem = "module";
       QString node(ptrToItem->parent()->text(0));
       g_actualNode = node;
       QString mod(ptrToItem->text(0));
       MSG = QString::fromStdString(getModuleProperties(node, mod).dump(2, ' ', false, nlohmann::json::error_handler_t::replace));
       //showModuleToTab(mod);
       switchShownTab(node, mod);
   }
   ui->infoedit->setPlainText("focus is on node: " + g_actualNode);
   ui->infoedit->appendPlainText("selected item is: " + g_actualItem);
   ui->infoedit->appendPlainText("Property:");
   ui->infoedit->appendPlainText(MSG);
   ui->focuslabel->setText("focus is on node: " + g_actualNode);
   NodeConnection* pConnection = m_hConnections[g_actualNode];
   if (pConnection != nullptr)
   {
       QHostAddress ip = pConnection->pSocket->peerAddress();
       int port = pConnection->pSocket->peerPort();
       ui->iPLineEdit->setText(ip.toString());
       ui->portLineEdit->setText(QString::number(port));
   }
}

/**
 * \brief This function is used by showModuleToTab and readtcpdatastream it initialises all the tablewidgetitems and it updates the value from each parameter after a read
 *        or asynchronous update. In case the where the space for text does not fit it sets a tooltip. The calculation if the text does fit sets the bEllipses bool.
 * \param[in] iRow     Rownumber
 * \param[in] iCol     Columnnumber
 * \param[in] Itemtext String for name value unit etc.
 * \param[in] pTable
 * \param[in] pToItem  for init this is the nullptr(showModuleToTab) for update the ptr to of the item (readtcpdatastream)
 */
void ClientGui::setTableWidgetItem(int iRow, int iCol, QString ItemText,QTableWidget* pTable ,QTableWidgetItem* pToItem)
{
    if(pToItem==nullptr)
    {
        pToItem = new QTableWidgetItem();
        pTable->setItem(iRow,iCol,pToItem);
    }
    int lines = 1;
    int cellwidth = pTable->columnWidth(iCol);
    bool bEllipsis(false);
    QFontMetrics fm(pToItem->font());
    if(ItemText.contains(" "))
        lines = (pTable->rowHeight(iRow))/fm.height();
    pToItem->setText(ItemText);
    if((iRow==statusRow) && (iCol=accValueCol))
        cellwidth  = pTable->columnWidth(accValueCol) + pTable->columnWidth(accUnitCol) + pTable->columnWidth(lineEditCol);  
    if(iRow==messageRow)
        cellwidth = pTable->columnWidth(firstCol) + pTable->columnWidth(nameOfAccCol) + pTable->columnWidth(accValueCol) + pTable->columnWidth(accUnitCol) + pTable->columnWidth(lineEditCol) + pTable->columnWidth(vertikalLineCol) + pTable->columnWidth(buttonCol);
    if((lines*cellwidth) <= (fm.width(ItemText)))
        bEllipsis = true;
    if(bEllipsis)
        pToItem->setToolTip("<html>"+ItemText+"<\\html>");//dirty hack 8) just for the show
    else
        pToItem->setToolTip(QString());
}

/**
 * \brief This function just replaces the old moduletab to the actual moduletab. The pointers to the tabs are taken from the structure.
 *
 * \param[in] node   node name
 * \param[in] module module name
 */
void ClientGui::switchShownTab(QString node, QString module)
{
    //removeUnusedTabs(); //will be used again if we decide to bring back the group tabs
    ui->tabWidget->setTabText(module_tab, module);
    bool bTabVisible = false;
    if (ui->tabWidget->currentIndex()==module_tab)//test if tab is visible
        bTabVisible=true;
    if (ConnectedNodesHash.contains(node))
    {
        QHash<QString, NodeStructureItem> &h = ConnectedNodesHash[node];
        if(h[module+":"].pModuleTabPage)
        {           
            ui->tabWidget->removeTab(module_tab);
            ui->tabWidget->insertTab(module_tab,h[module+":"].pModuleTabPage,module);
            if(bTabVisible)
                ui->tabWidget->setCurrentIndex(module_tab);
        }
    }
}

/**
 * \brief This function builds the tabpage for all modules of a node and returns the pointer to tabpage.
 *
 * \param[in] node   node name
 * \param[in] module module name
 * \return pointer to tabpage
 */
QWidget* ClientGui::createModuleTabs(QString node, QString module)
{

    QWidget* page = new QWidget();

    QVBoxLayout* pVBox= new QVBoxLayout(page);
    QHBoxLayout* pHBox= new QHBoxLayout();
    QLabel* statusLabel = new QLabel();
    statusLabel->setObjectName("statusLabel");
    QCheckBox* checkBoxPrevent = new QCheckBox();
    checkBoxPrevent->setObjectName("checkBoxPrevent");
    checkBoxPrevent->setChecked(m_bCheckPrevent);
    checkBoxPrevent->setText("prevent changes");

    connect(checkBoxPrevent, SIGNAL(stateChanged(int)),
            this, SLOT(checkBoxHandle(int)), Qt::QueuedConnection);

    QTableWidget* pTW = new QTableWidget();
    pTW->setObjectName("tableWidget");
    pHBox->addWidget(statusLabel);
    pHBox->addWidget(checkBoxPrevent,0,Qt::AlignRight);
    //pVBox->addItem(pHBox);
    pVBox->addLayout(pHBox);
    pVBox->addWidget(pTW);

/*
    //the tabs for the groups unclear if we bring this again or not
    NodeStructureHash = ConnectedNodesHash[node];
    QString grps = NodeStructureHash[module+":"].unit;
    if (grps.contains(":"))//we have groups in this module
    {
        grps.remove(grps.size()-1,1);//remove last sign :
        QStringList grpslst= grps.split(":");
        for (int i=0; i<grpslst.size(); i++)
        {
            QTableWidget* tabs = new QTableWidget();
            setTableLayout(tabs,true);

            ui->tabWidget->addTab(tabs,grpslst[i]);
            foreach (const QString &key,NodeStructureHash.keys())
            {
                if (NodeStructureHash[key].group==grpslst[i])
                {
                    addRowToGrpTable(tabs, NodeStructureHash[key].rwxType, key);//
                }
            }
        }
    }
*/
    std::string vfs; //this is used to get the value from nodestruct

    //header
    int skippedAcc = 1; //if driveable we skip 4 accessibles; writeable skip 3; readable skip 2 because they are shown in the header
    int numberOfAcc=getNumberOfAcc(node,module);//number of accessibles

    pTW->setRowCount(numberOfAcc+numberOfHeaderRows);
    setTableLayout(pTW);

    ui->tabWidget->setTabText(static_tabs-1, module);
    QTableWidgetItem* p_firstTextLine =new QTableWidgetItem("");
    pTW->setItem(0,0,p_firstTextLine);
    statusLabel->setText(g_actualNode+":"+module);

    QPushButton* pBut3= new QPushButton();
    pBut3->setText("read");

    pTW->setItem(valueRow,nameOfAccCol,new QTableWidgetItem("value"));
    vfs= (ConnectedNodesHash[node])[module+":value"].pValueOrArg->exportSECoP().toStdString(); //
    pTW->setItem(valueRow,accValueCol,new QTableWidgetItem(QString::fromStdString(vfs)));//set value
    pTW->setItem(valueRow,accUnitCol,new QTableWidgetItem(getUnitOfAcc(g_actualNode,module,"value")));
    pTW->setCellWidget(valueRow,buttonCol,pBut3);

    connect(pBut3, &QPushButton::clicked,
                  [=]() {CallAcc("read "+module+":value",valueRow,pTW); });

    QPushButton* pBut4= new QPushButton();
    pBut4->setText("read");

    pTW->setItem(statusRow,nameOfAccCol,new QTableWidgetItem("status"));
    vfs= (ConnectedNodesHash[node])[module+":status"].pValueOrArg->exportSECoP().toStdString(); //
    pTW->setSpan(statusRow,accValueCol,1,3);//status has no unit or lineedit but a lot of text so give more space
    setTableWidgetItem(statusRow,accValueCol,QString::fromStdString(vfs),pTW,nullptr);
    pTW->setCellWidget(statusRow,buttonCol,pBut4);

    connect(pBut4, &QPushButton::clicked,
                  [=]() {CallAcc("read "+module+":status",statusRow,pTW); });


    //check moduleclass for driveable writable or readable and show the mandatory accessibles in the header
    if (getAccNameList(g_actualNode,module).contains("stop"))// we are drivable
    {
        pTW->setRowHidden(targetRow,false);
        pTW->setRowHidden(stopRow,false);
        QLineEdit* pLineEdit= new QLineEdit();
        QLineEdit* pLineEdit2= new QLineEdit();
        QPushButton* pBut= new QPushButton();
        QPushButton* pBut2= new QPushButton();


        pTW->setItem(targetRow,nameOfAccCol,new QTableWidgetItem("target"));
        vfs= (ConnectedNodesHash[node])[module+":target"].pValueOrArg->exportSECoP().toStdString();
        pTW->setItem(targetRow,accValueCol,new QTableWidgetItem(QString::fromStdString(vfs)));//set value
        pTW->setItem(targetRow,accUnitCol,new QTableWidgetItem(getUnitOfAcc(g_actualNode,module,"target")));
        pTW->setCellWidget(targetRow,lineEditCol,pLineEdit);
        pTW->setCellWidget(targetRow,buttonCol,pBut);
        pTW->setItem(stopRow,nameOfAccCol,new QTableWidgetItem("stop"));
        pTW->setCellWidget(stopRow,buttonCol,pBut2);
        pTW->setCellWidget(stopRow,lineEditCol,pLineEdit2);

        pBut2->setText("do");
        if(m_bCheckPrevent)
        {
            pBut->setText("read");
            pBut2->setDisabled(true);
            pLineEdit->setDisabled(true);
            pLineEdit2->setDisabled(true);
            connect(pBut, &QPushButton::clicked,
                          [=]() {CallAcc("read "+module+":target",targetRow,pTW); });
        }
        else
        {
            pBut->setText("change");
            pBut2->setEnabled(true);
            pLineEdit->setEnabled(true);
            pLineEdit2->setEnabled(true);
            connect(pBut, &QPushButton::clicked,
                          [=]() {CallAcc("change "+module+":target",targetRow,pTW); });
            connect(pBut2, &QPushButton::clicked,
                          [=]() {CallAcc("do "+module+":stop",stopRow,pTW); });
        }
        hash[module+":value"]=valueRow;
        hash[module+":status"]=statusRow;
        hash[module+":target"]=targetRow;
        hash[module+":stop"]=stopRow;
        skippedAcc = 4;

    }
    else
    {
        bool bWritable(getAccNameList(g_actualNode,module).contains("target"));
        pTW->setRowHidden(targetRow,!bWritable);
        pTW->setRowHidden(stopRow,true);
        if (bWritable)//we are writable
        {
            QLineEdit* pLineEdit= new QLineEdit();
            QPushButton* pBut= new QPushButton();

            pTW->setItem(targetRow,nameOfAccCol,new QTableWidgetItem("target"));
            vfs= (ConnectedNodesHash[node])[module+":target"].pValueOrArg->exportSECoP().toStdString();
            pTW->setItem(targetRow,accValueCol,new QTableWidgetItem(QString::fromStdString(vfs)));//set value
            pTW->setItem(targetRow,accUnitCol,new QTableWidgetItem(getUnitOfAcc(g_actualNode,module,"target")));
            pTW->setCellWidget(targetRow,lineEditCol,pLineEdit);
            pTW->setCellWidget(targetRow,buttonCol,pBut);

            if(m_bCheckPrevent)
            {
                pBut->setText("read");
                pLineEdit->setDisabled(true);
                connect(pBut, &QPushButton::clicked,
                          [=]() {CallAcc("read "+module+":target",targetRow,pTW); });
            }
            else
            {
                pBut->setText("change");
                pLineEdit->setEnabled(true);
                connect(pBut, &QPushButton::clicked,
                          [=]() {CallAcc("change "+module+":target",targetRow,pTW); });
            }
            hash[module+":target"]=targetRow;
            skippedAcc = 3;
        }
        else//we are readable
        {
            pTW->setRowHidden(targetRow,true);
            skippedAcc = 2;
        }
        hash[module+":status"]=statusRow;
        hash[module+":value"]=valueRow;
    }

    //this tablelayout settings need information from the moduleclass
    pTW->setRowCount(numberOfAcc+numberOfHeaderRows-skippedAcc);
    int rowspan = numberOfAcc-skippedAcc;
    if (rowspan>1)
        pTW->setSpan(numberOfHeaderRows,firstCol,rowspan,1);

    int accessRow=numberOfHeaderRows;//the not mandatory accessibles start after the header

    //iterating over the accessiblelist and create the gui entries
    for (int i=0; i < numberOfAcc; i++)
    {
        QString acc = getAccNameList(g_actualNode,module)[i];
        QString call = module+":"+acc;
        QString unit = getUnitOfAcc(g_actualNode,module,acc);
        if (acc== "value"||acc=="target"||acc=="stop"||acc=="status")//skip the accessibles from the header
        {
            continue;
        }
        else
        {
            QLineEdit* pLineEdit= new QLineEdit();
            QPushButton* pBut= new QPushButton();



            nlohmann::json qjoacc(getAccProperties(g_actualNode, module, acc));
            if (qjoacc.contains("readonly"))
            {
                bool b(false);
                if (qjoacc["readonly"].is_boolean())
                    b = qjoacc["readonly"].get<bool>();
                if (b)//readable
                {
                    pBut->setText("read");
                    setTableWidgetItem(accessRow,nameOfAccCol,acc,pTW,nullptr);
                    vfs= (ConnectedNodesHash[node])[module+":"+acc].pValueOrArg->exportSECoP().toStdString();
                    setTableWidgetItem(accessRow,accValueCol,QString::fromStdString(vfs),pTW,nullptr);//setitem
                    setTableWidgetItem(accessRow,accUnitCol,unit,pTW,nullptr);
                    pTW->setCellWidget(accessRow,buttonCol,pBut);
                    connect(pBut, &QPushButton::clicked,
                              [=]() {CallAcc("read "+call, accessRow,pTW); });
                    hash[module+":"+acc]=accessRow;
                }
                else
                {//writable
                    setTableWidgetItem(accessRow,nameOfAccCol,acc,pTW,nullptr);
                    vfs= (ConnectedNodesHash[node])[module+":"+acc].pValueOrArg->exportSECoP().toStdString();
                    setTableWidgetItem(accessRow,accValueCol,QString::fromStdString(vfs),pTW,nullptr);//set value
                    setTableWidgetItem(accessRow,accUnitCol,unit,pTW,nullptr);
                    pTW->setCellWidget(accessRow,lineEditCol,pLineEdit);
                    pTW->setCellWidget(accessRow,buttonCol,pBut);

                    if(m_bCheckPrevent)
                    {
                        pBut->setText("read");
                        pLineEdit->setDisabled(true);
                        connect(pBut, &QPushButton::clicked,
                                  [=]() {CallAcc("read "+call, accessRow,pTW); });//maybe use testread to do some analytics read only reads from buffer testread reads from machine
                    }
                    else
                    {
                        pBut->setText("change");
                        pLineEdit->setEnabled(true);
                        connect(pBut, &QPushButton::clicked,
                                  [=]() {CallAcc("change "+call+" ",accessRow,pTW); });
                    }
                    hash[module+":"+acc]=accessRow;
                }
            }
            else
            {//acc is command
                setTableWidgetItem(accessRow,nameOfAccCol,acc,pTW,nullptr);
                setTableWidgetItem(accessRow,accUnitCol,unit,pTW,nullptr);
                pTW->setCellWidget(accessRow,lineEditCol,pLineEdit);
                pTW->setCellWidget(accessRow,buttonCol,pBut);
                if(m_bCheckPrevent)
                {
                    pBut->setText("do");
                    pBut->setDisabled(true);
                    pLineEdit->setDisabled(true);
                }
                else
                {
                    pBut->setText("do");
                    pBut->setEnabled(true);
                    pLineEdit->setEnabled(true);
                    connect(pBut, &QPushButton::clicked,
                              [this,call,accessRow,pTW]() {CallAcc("do "+call+" ",accessRow,pTW); });
                }
            hash[module+":"+acc]=accessRow;
            }
            accessRow++;
        }
    }
return page;
}


/**
 * \brief This function is used by buildStructure it calls createModuleTabs for all module of a node and stores the pointer to tab in the structure.
 *
 * \param[in] node Node name
 */
void ClientGui::buildTabs(QString node)
{
     if (ConnectedNodesHash.contains(node))
     {
         QHash<QString, NodeStructureItem> &h = ConnectedNodesHash[node];
         foreach (const QString &str, h[":"].ContentList)
         {
             h[str+":"].pModuleTabPage = createModuleTabs(node, str);
         }
     }
}

/**
 * \brief This function uses the SECoP describing message and builds an internal structure.
 *        A lot of helpfunctions (*fromJSON) are called to get the information needed.
 *
 * \param[in]     pConnection Connection
 * \param[in,out] szText      SECoP describe message
 */

void ClientGui::buildStructure(NodeConnection* pConnection, QByteArray &szText)
{
#pragma message(CPP_WARNINGPREFIX "TODO: check, if reconnected (pConnection->bReconnect == true)")
    QByteArray SECoPCommand;
    QHash<QString, NodeStructureItem> actNode;
    QString testmsg = nullptr;
    int i=szText.indexOf(' ');
    if (i<0)
        return;
    szText.remove(0, i+1); // remove "describing " to get the descriptive JSON object
    i=szText.indexOf(' ');
    if (i<0)
        return;
    g_actualNode=pConnection->szNodeName=szText.left(i).trimmed();
    szText.remove(0, i+1);
    try
    {
        JSONdoc = nlohmann::json::parse(szText.toStdString());
    }
    catch (nlohmann::json::exception&)
    {
        JSONdoc.clear();
    }
    catch (...)
    {
        JSONdoc.clear();
    }
    if (JSONdoc.is_object() && JSONdoc.contains("equipment_id"))
    {
        const nlohmann::json &v(JSONdoc["equipment_id"]);
        if (v.is_string())
        {
            QString s(QString::fromStdString(v.get<std::string>()));
            if (!s.isEmpty())
                g_actualNode=pConnection->szNodeName=s;
        }
    }
    m_hConnections[g_actualNode]=pConnection;
    if (pConnection->pTimer!=nullptr)
    {
        pConnection->pTimer->stop();
        pConnection->pTimer->deleteLater();
        pConnection->pTimer=nullptr;
    }
    emit newNodeConnected(g_actualNode);//fire new node signal at this point we have an valid JSON (JSONdoc) extracted and put the name of node to the connection list

 //start changes   
    qjoNodeProps = JSONdoc;                       //copy complete JSON to qjoNodeProps
    qjoMods = qjoNodeProps["modules"]; //copy the key "modules" to qjoMods
    //modules:{}
    qjoNodeProps.erase("modules");                //remove the key "modules" from NodeProps

    NodeStructureItem nodeItem;
    QStringList ListOfModules;
    for (auto mods = qjoMods.items().begin(); mods != qjoMods.items().end(); ++mods)
    {
        NodeStructureItem moduleItem;
        QString modname = QString::fromStdString(mods.key());
        ListOfModules.append(modname);
        mods.value();
        nlohmann::json j_objectAccessibles = mods.value()["accessibles"];
        QStringList ListOfAccessibles;
        for (auto accs = j_objectAccessibles.begin(); accs != j_objectAccessibles.end(); ++accs)
        {
            NodeStructureItem accessibleItem;
            QString accname=QString::fromStdString(accs.key());
            ListOfAccessibles.append(accname);
            nlohmann::json acc = accs.value();

            accessibleItem.gpos = 0;
            accessibleItem.init = true; //still initial value
            accessibleItem.unit = nullptr;
            accessibleItem.group = nullptr;
            accessibleItem.value = nullptr;
            accessibleItem.rwxType  ='0';
            accessibleItem.timestamp = std::numeric_limits<double>::quiet_NaN();
            accessibleItem.properties= accs.value();
            accessibleItem.ContentList.removeAll(QString());
            accessibleItem.pValueOrArg = nullptr;
            accessibleItem.pModuleTabPage = nullptr;
            accessibleItem.pSigmaOrResult = nullptr;

            CSECoPbaseType* pType(CSECoPbaseType::createSECoP(accessibleItem.properties["datainfo"], true));
            if (pType != nullptr)
            {
                CSECoPcommand* pCommand(dynamic_cast<CSECoPcommand*>(pType));
                if (pCommand != nullptr)
                {
                    // handle command
                    accessibleItem.pValueOrArg    = SECoP_dataPtr(pCommand->getArgument()->duplicate());
                    accessibleItem.pSigmaOrResult = SECoP_dataPtr(pCommand->getResult()->duplicate());
                    accessibleItem.rwxType        = 'x';
                    SECoP_V_destroy(&pType);
                }
                else
                {
                    // handle parameter
                    accessibleItem.pValueOrArg    = SECoP_dataPtr(pType);
                    accessibleItem.pSigmaOrResult = accessibleItem.pValueOrArg;
                    if (accessibleItem.properties.contains("constant"))
                        accessibleItem.init = !accessibleItem.pValueOrArg->importSECoP(accessibleItem.properties["constant"], false);
                    else
                        accessibleItem.init = true;

                    bool b(true);
                    if (accessibleItem.properties.contains("readonly") && accessibleItem.properties["readonly"].is_boolean())
                        b = accessibleItem.properties["readonly"].get<bool>();
                    if (accessibleItem.init)
                        accessibleItem.rwxType = b ? 'r' : 'w';
                    else
                        accessibleItem.rwxType = b ? 'R' : 'W';
                }
                QString unit = QString::fromStdString(getUnit(pType));
            }

            actNode[modname+":"+accname]=accessibleItem;
        }

        moduleItem.gpos = 0;
        moduleItem.init = true; //still initial value
        moduleItem.unit = nullptr;
        moduleItem.group = nullptr;
        moduleItem.value = nullptr;
        moduleItem.rwxType = '\0';
        moduleItem.timestamp = std::numeric_limits<double>::quiet_NaN();
        moduleItem.properties= mods.value();
        moduleItem.properties.erase("accessibles");
        moduleItem.ContentList = ListOfAccessibles;
        moduleItem.pValueOrArg = nullptr;
        moduleItem.pModuleTabPage = nullptr;
        moduleItem.pSigmaOrResult = nullptr;

        actNode[modname+":"]=moduleItem;
    }

    nodeItem.gpos = 0;
    nodeItem.init = true; //still initial value
    nodeItem.unit = nullptr;
    nodeItem.group = nullptr;
    nodeItem.value = nullptr;
    nodeItem.rwxType = '\0';
    nodeItem.timestamp = std::numeric_limits<double>::quiet_NaN();
    nodeItem.properties= qjoNodeProps;
    nodeItem.ContentList = ListOfModules;
    nodeItem.pValueOrArg = nullptr;
    nodeItem.pModuleTabPage = nullptr;
    nodeItem.pSigmaOrResult = nullptr;

    actNode[":"]=nodeItem;
    JSON.clear();
    ui->textBox->setPlainText(testmsg);
    ConnectedNodesHash[g_actualNode]=actNode;
    addNodeToTree(g_actualNode);
    if(m_bStartActive)
    {
        SECoPCommand.clear();
        SECoPCommand.append("activate");
        SECoPCommand.append(10);
        writeToNode(g_actualNode,SECoPCommand);
        ui->textBox->append("activating ...");
        ui->textBox->repaint();
        pConnection->iState = ClientGui::NodeConnection::STATE_ACTIVATING;
    }
    else
        pConnection->iState = ClientGui::NodeConnection::STATE_CONNECTED;
    m_bStartActive=true;
    buildTabs(g_actualNode);

}

/*
    testmsg = g_actualNode +"\n";
    QStringList ModNames = getModuleNamesListFromJSON();
    v.ContentList=ModNames;
    actNode[":"]=v;
    for (int i=0; i<ModNames.size(); i++)
    {
        testmsg.append("  "+ModNames[i]+"\n");
        QString modkey=ModNames[i]+":"+"";
        NodeStructureItem val;
        val.properties=getModulePropertiesFromJSON(ModNames[i]);
        QStringList AccNames = getAccNameListFromJSON(ModNames[i]);
        val.ContentList= AccNames;
        actNode[modkey]=val;
        for (int j=0; j<AccNames.size(); j++)
        {
            QString key = ModNames[i]+":"+AccNames[j];

            NodeStructureItem value;

//set properties
            nlohmann::json o(getAccPropertiesFromJSON(ModNames[i],AccNames[j]));
            if (o.is_object())
                value.properties = o; //cast json document to json object if it is not object the something was wrong with the descriptive JSON or the descriptive JSON deconstruction
            else
                break;//TODO throw SECoP Error secop exports libs etc
//set datainfo
            value.rwxType   = '\0';
            value.timestamp = std::numeric_limits<double>::quiet_NaN();
            value.unit.clear();
            if (value.properties.contains("datainfo"))
            {

                if (pType != nullptr)
                {
                    CSECoPcommand* pCommand(dynamic_cast<CSECoPcommand*>(pType));
                    if (pCommand != nullptr)
                    {
                        // handle command
                        value.pValueOrArg    = SECoP_dataPtr(pCommand->getArgument()->duplicate());
                        value.pSigmaOrResult = SECoP_dataPtr(pCommand->getResult()->duplicate());
                        value.rwxType        = 'x';
                        SECoP_V_destroy(&pType);
                    }
                    else
                    {
                        // handle parameter
                        value.pValueOrArg    = SECoP_dataPtr(pType);
                        value.pSigmaOrResult = value.pValueOrArg;
                        if (value.properties.contains("constant"))
                            value.init = !value.pValueOrArg->importSECoP(value.properties["constant"], false);
                        else
                            value.init = true;

                        bool b(true);
                        if (value.properties.contains("readonly") && value.properties["readonly"].is_boolean())
                            b = value.properties["readonly"].get<bool>();
                        if (value.init)
                            value.rwxType = b ? 'r' : 'w';
                        else
                            value.rwxType = b ? 'R' : 'W';
                    }
                    value.unit = QString::fromStdString(getUnit(pType));
                }
            }
//set group  grouping is not defined with parent or path ...
            if (value.properties.contains("group"))//accessible has group
            {
                std::string g;
                if (value.properties.contains("group") && value.properties["group"].is_string())
                    g = value.properties["group"].get<std::string>();
                if (g.find(':') != std::string::npos)//accessible has group with given order
                {
                    QStringList gl(QString::fromStdString(g).split(':'));
                    value.group=gl[0];//group name
                    QString number=gl[1] ;//position
                    value.gpos=number.toInt();
                    if (!actNode[modkey].unit.contains(value.group))
                    {
                        actNode[modkey].unit.append(value.group+":");
                    }
                }
                else//has unordered group
                {
                    value.group=QString::fromStdString(g);
                    value.gpos=std::numeric_limits<int>::max();//unordered; position is fifo
                    if (!actNode[modkey].unit.contains(value.group))
                    {
                        actNode[modkey].unit.append(value.group+":");
                    }
                }
            }
            actNode[key]=value;
            testmsg.append("    "+AccNames[j]+"\n");
        }
    }

//end changes

    JSON.clear();
    ui->textBox->setPlainText(testmsg);
    ConnectedNodesHash[g_actualNode]=actNode;
    addNodeToTree(g_actualNode);
    if(m_bStartActive)
    {
        SECoPCommand.clear();
        SECoPCommand.append("activate");
        SECoPCommand.append(10);
        writeToNode(g_actualNode,SECoPCommand);
        ui->textBox->append("activating ...");
        ui->textBox->repaint();
        pConnection->iState = ClientGui::NodeConnection::STATE_ACTIVATING;
    }
    else
        pConnection->iState = ClientGui::NodeConnection::STATE_CONNECTED;
    m_bStartActive=true;
    buildTabs(g_actualNode);
}
*/

/**
 * \brief This function looks at each parameter in each module of a node;
 *        if one parameter still unintialized it returns false
 *        else true.
 *
 * \param[in] node nodename
 * \return true: success, false: there is something uninitialized
 */

bool ClientGui::checkInit(QString node)
{
    if (ConnectedNodesHash.contains(node))
    {
        QHash<QString, NodeStructureItem> &h = ConnectedNodesHash[node];
        int numberOfMod=getNumberOfModules(node);
        for (int i=0;i<numberOfMod;i++)
        {
            QString mod = getModuleNamesList(node)[i];
            int numberOfAcc=getNumberOfAcc(node,mod);
            for (int j=0;j<numberOfAcc;j++)
            {
                QString acc = getAccNameList(node,mod)[j];
                if(h[mod+":"+acc].rwxType=='r' ||h[mod+":"+acc].rwxType=='w')
                {
                    if(h[mod+":"+acc].init == true)//found one init that is still true
                        return false;
                    else
                        continue;
                }
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}


/**
 * \brief this function returns the status label of a page (nodename:modulename)the status label is needed because the modulname may appear in another node as well
 *        but the combination of nodename and modulename is unique
 *
 * \param[in] pPage pointer to Tabpage
 * \return (nodename:modulename)
 */
QString ClientGui::getStatusLabelText(QWidget* pPage)
{
    QString retval=QString();
    foreach (const QObject* child, pPage->children())
    {
        QString childName = child->objectName();
        if(childName=="statusLabel")
        {
            QLabel* plabel = const_cast<QLabel*>( reinterpret_cast<const QLabel*>(child));
            retval = plabel->text();        }
    }
    return retval;
}

/**
 * \brief This function searches a child of the tabPage and changes the text of that item in case for the tableWidget it sets the text on the item specified by row and column.
 *
 * \param[in] pPage   pointer to Tabpage
 * \param[in] objName name of object
 * \param[in] text    text to change
 * \param[in] iRow    row of tablewidgetitem
 * \param[in] iCol    column of tablewidgetitem
 */
void ClientGui::setTableItemText(QWidget* pPage, QString objName, QString text, int iRow, int iCol)
{
    foreach (const QObject* child, pPage->children())
    {
        QString childName = child->objectName();
        if(childName==objName)
        {
            if(child->objectName()=="tableWidget")
            {
                QTableWidget* pTable=const_cast<QTableWidget*>( reinterpret_cast<const QTableWidget*>(child));
                QTableWidgetItem* pItem= pTable->item(iRow,iCol);
                setTableWidgetItem(iRow,iCol,text,pTable,pItem);
                //pTable->item(row,col)->setText(text);
            }
        }
    }
}

/**
 * \brief This function handles the needed actions after checkBox toggle. All pages are build new with the right connections.
 *        If data is uninitialised a warning is shown.
 */
void ClientGui::checkBoxHandle(int iState)
{
    m_bCheckPrevent = (iState != Qt::Unchecked);
    QString modname = ui->tabWidget->tabText(module_tab);
    buildTabs(g_actualNode);
    switchShownTab(g_actualNode,modname);
    QWidget* pChild = ConnectedNodesHash[g_actualNode][modname+":"].pModuleTabPage;
    if(!checkInit(g_actualNode)&&!m_bCheckPrevent)
    {
        setTableItemText(pChild,"tableWidget","warning blind flight!!! YOU are working on uninitialised data!",0,0);
    }
    else
    {
        setTableItemText(pChild,"tableWidget","",0,0);
    }
}

/**
 * \brief this function sets the m_bShowGUI bool which is used to hide/show the GUI
 * \param[in] bShowGUI true shows GUI false hides GUI
 */
void ClientGui::showGUI(bool bShowGUI)
{
    m_bShowGUI=bShowGUI;
}

/**
 * \brief this function returns a visible unit string of a value
 * \param[in] pValue the value with a unit
 * \return extracted unit string for display or empty, if there is none
 */
std::string ClientGui::getUnit(const CSECoPbaseType* pValue)
{
    if (pValue == nullptr)
        return std::string();
    const CSECoPstruct*  pStruct (dynamic_cast<const CSECoPstruct*> (pValue));
    const CSECoPtuple*   pTuple  (dynamic_cast<const CSECoPtuple*>  (pValue));
    const CSECoParray*   pArray  (dynamic_cast<const CSECoParray*>  (pValue));
    const CSECoPcommand* pCommand(dynamic_cast<const CSECoPcommand*>(pValue));
    nlohmann::json o;
    if (pStruct != nullptr)
    {
        if (pStruct->getItemCount() < 1)
            return std::string();
        for (unsigned int i = 0; i < pStruct->getItemCount(); ++i)
            o[pStruct->getItemName(i).toStdString()] = getUnit(pStruct->getItem(i));
        return o.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    }
    if (pTuple != nullptr)
    {
        if (pTuple->getSize() < 1)
            return std::string();
        for (unsigned int i = 0; i < pTuple->getSize(); ++i)
            o.push_back(getUnit(pTuple->getValue(i)));
        return o.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    }
    if (pArray != nullptr)
    {
        if (pArray->getSize() < 1)
            return std::string();
        for (unsigned int i = 0; i < pArray->getSize(); ++i)
            o.push_back(getUnit(pArray->getValue(i)));
        return o.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    }
    if (pCommand != nullptr)
    {
        std::string szArg(getUnit(pCommand->getArgument()));
        std::string szRes(getUnit(pCommand->getResult()));
        if (!szArg.empty())
            o["argument"] = szArg;
        if (!szRes.empty())
            o["result"] = szRes;
        if (o.empty())
            return std::string();
        return o.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    }
    o = pValue->additional();
    if (o.contains("unit") && o["unit"].is_string())
        return o["unit"].get<std::string>();
    return std::string();
}
