/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef CLIENTGUI_H
#define CLIENTGUI_H

#include <QDialog>
#include <QTcpSocket>
#include <QDataStream>
#include <QtNetwork>
#include <QtWidgets>
#include <QTreeWidgetItem>
#include "SECoP-Variant.h"

namespace Ui {
class ClientGui;
}

#include "exports.h"
class Client_Main;



/**
*The task of this class is
* 1.    to handle the connections to nodes and provide add node and remove node functionality as well as a GUI
* 2.    build up an own structure from JSON
*   a   a connected node is asked for a JSON describe be message that contains all information of a node this information is decomposed via the FromJSON functions
*   b   the FromJSON functions are all only called from the buildStructure function which builds up an easier to access NodeStructure and call afterwards the node to activate itself
*   c   some of the exported functions are handled here
*/
class ClientGui : public QWidget
{
    Q_OBJECT


friend class Client_Main;

public:
    explicit ClientGui(bool bShowGUI, QWidget *parent = Q_NULLPTR);
    ~ClientGui();

    void allowClose(bool bAllow = true);
    void closeEvent(QCloseEvent* pEvent);

    void sendDescribeMsg(QTcpSocket* socket);
    bool addNode(QHostAddress IP, quint16 Port);
    void removeNode(QHostAddress IP, quint16 Port);
    void showGUI(bool bShowGUI);
    void setInactiveBool();
    void log(QString szLogText);
    QString getNodeID();//returns actual node
    int getNumberOfNodes();

    nlohmann::json getNodeProperties(QString Node);
    nlohmann::json getModuleProperties(QString Node, QString Module);
    nlohmann::json getAccProperties(QString Node, QString Module, QString Acc);

    QStringList getNodeNamesList();
    QStringList getNodePropertiesKeyList();

    int getNumberOfModules(QString NodeName);
    QStringList getModuleNamesList(QString NodeName);

    QStringList getAccNameList(QString node, QString mod);
    int getNumberOfAcc(QString Node, QString modName);

    QString getNodeNameByIP(QHostAddress IP, int Port);
    QString getUnitOfAcc(QString node,QString mod,QString acc);
    void writeToNode(QString node, QByteArray data);
    std::string readParameter(QString Node, QString Module, QString ParamName, SECoP_dataPtr &Value, double &dTimestamp,SECoP_dataPtr &ErrorVal, bool  forced);
    std::string testRead(QString Node, QString Module, QString ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal, std::string &SECoPtripel, qint64 &respTime, qint64 maxTime);
/*    std::string readParam(QString Node, QString Module, QString Param, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal, bool forced);
    std::string forcedReadParam(QString Node, QString Module, QString ParamName, SECoP_dataPtr &Value, double &dTimestamp,SECoP_dataPtr &ErrorVal);
*/    std::string writeParam(QString Node, QString Module , QString Acc, SECoP_dataPtr Value);

signals:
    void newNodeConnected(QString newValue);

private:
    struct NodeConnection
    {
    private:
        //deleting copy constructor and assignment operator
        NodeConnection(const NodeConnection &) = delete;
        NodeConnection& operator=(const NodeConnection &) = delete;
    public:
        NodeConnection() : pSocket(nullptr), iState(STATE_DESCRIBING), wPort(0), pTimer(nullptr)
          , pWatchdog(nullptr), iCount(0), bReconnect(false) {}
        QTcpSocket* pSocket;
        enum State { STATE_CONNECTING, STATE_DESCRIBING, STATE_ACTIVATING, STATE_CONNECTED, STATE_REJECTED, STATE_RECONNECT } iState;
        QString szNodeName;
        QHostAddress Host;
        quint16 wPort;
        QTimer* pTimer;
        QTimer* pWatchdog;//
        QHash<QString,qint64> request;
        int     iCount;
        bool    bReconnect;
    };

private slots:
    //void on_checkBoxPrevent_stateChanged();
    void checkBoxHandle(int iState);
    void connectedTcpDatastream();
    void errorTcpDatastream(QAbstractSocket::SocketError iError);
    void readTcpDatastream();
    void writeTcpDatastream();
    void connectWithServer();
    void disconnectFromServer();
    void showItem(QTreeWidgetItem* ptrToItem);
    void CallAcc(QString call,int row,QTableWidget* pTTW);
    void CalltestRead(QString module);
    void logUpdate();
    void removeNode(NodeConnection* pConnection, bool bDelete);
    void toggleAsyncMSGBool(bool bChecked);
    void reconnectTimer();

private:
    void SECoP_C_initLibraryHelper(void);
    void readTcpDatastream(NodeConnection* pConnection);
    void removeNodeFromTree(QString NodeName);
    void addNodeToTree(QString name);
    void addModulesToTree(QTreeWidgetItem* ptrTreeParent, QString Node, QString mod);
    void addAccToTree(QTreeWidgetItem* ptrTreeParent,  QString acc, char cType);
    void showModuleToTab(QString module);
    void removeUnusedTabs();
    void addRowToGrpTable(QTableWidget* grptab, char rwxType, QString key);

    void setTableLayout(QTableWidget* ptabWidget);
    void showItems();
    void changeModuleView();
    void buildStructure(NodeConnection* pConnection, QByteArray &szText);
    void switchShownTab(QString node, QString module);
    QWidget* createModuleTabs(QString node, QString module);
    void buildTabs(QString node);
    bool checkInit(QString node);
    void setTableWidgetItem(int iRow, int iCol, QString vfs,QTableWidget* pTable, QTableWidgetItem* pToItem);
    void setTableItemText(QWidget* pPage, QString obj, QString text, int iRow, int iCol);
    QString getStatusLabelText(QWidget* pPage);
    std::string getUnit(const CSECoPbaseType* pValue);

    struct
    {
        QHostAddress ip;
        int port=0;
    }retryCandidate;


    struct NodeStructureItem                        //used for nodes modules and accessibles to store the information from JSON
    {
        char rwxType;                               //only used for accessibles x=command r=readable w=writable
        QString group;                              //only used for modules and accessibles
        int gpos=0;                                 //0 if no group is given is intended to give an preferred order in groupings not decided yet if needed
        QString unit;                               //only used for parameters
        nlohmann::json properties;                     //used for nodes/modules and accessibles
        QStringList ContentList;                    //used only by node or module
        QString value;                              //value as string soon obsolete
        bool init=true;                             //maybe as indicator if an parameter has still its init value something the watchdog can scan for true means still init value
        SECoP_dataPtr pValueOrArg = nullptr;        //points to the databuffer
        double timestamp;                           //last timestamp
        SECoP_dataPtr pSigmaOrResult;               //last error
        QWidget* pModuleTabPage=nullptr;            //only used by modules
    };

    QHash<QString, int> hash;

    //key QString <nodename>  stores several nodestructures by nodename
    //   subkey <:>                        stores all information of a node
    //   subkey <modulename:>              stores all information of a module
    //   subkey <modulename:parametername> stores all information of a accessible (command or parameter)
    QHash<QString, QHash<QString, NodeStructureItem>> ConnectedNodesHash;
    QList<NodeConnection*> m_apConnections;
    QHash<QTcpSocket*, NodeConnection*> m_hConnectionHash;
    QHash<QString, NodeConnection*> m_hConnections;
    QHash<QString,QTreeWidgetItem*>storedTreeItems;             //key QString Nodename

    Ui::ClientGui *ui;
    QGridLayout *grid=nullptr;
    QString JSON= nullptr;

    QString g_actualNode = nullptr;//used for gui
    QString g_actualItem = nullptr;
    nlohmann::json JSONdoc;
    nlohmann::json qjoNodeProps;
    nlohmann::json qjoMods;
    bool connected = false;
    int static_tabs = 5;//tabs that are always shown console, info, log, errorlog and module the additional tabs are shown if grouping on parameter is used
    int module_tab = 4;
    int errorlog_tab= 3;
    int log_tab = 2;
    int info_tab = 1;
    int console_tab = 0;

    bool        m_bAllowClose;
    bool        m_bCheckPrevent=true;
    bool        m_bShowGUI;
    bool        m_bHideAsyncMSG=false;
    bool        m_bUsePointers=false;
    bool        m_bStartActive=true;
    QMutex      m_Mutex;
    QStringList m_aLog;
};

#endif // CLIENTGUI_H
