/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef EXPORTS_H
#define EXPORTS_H
#include <QApplication>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "SECoP-Variant.h"

// SHALL_EXPORT is used to mark exported functions (needed by some compilers)
// SHALL_EXPORT is used to mark exported functions (needed by some compilers)
#if !defined(SHALL_EXPORT)
#  if defined(SHALL_LIBRARY)
#    include <QtCore/qglobal.h>
#    define SHALL_EXPORT Q_DECL_EXPORT
#  elif defined(QT_VERSION)
#    include <QtCore/qglobal.h>
#    define SHALL_EXPORT Q_DECL_IMPORT
#  elif defined(_MSC_VER)
#    define SHALL_EXPORT __declspec(dllimport)
#  else
#    define SHALL_EXPORT
#  endif
#endif

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

#ifndef QT_VERSION
#ifdef __cplusplus
class QApplication;
#else
typedef void QApplication;
#endif
#endif

enum SECoP_C_error
{
    SECOP_EC_SUCCESS = 0,
    SECoP_EC_NOT_INITIALIZED = -1,
    SECoP_EC_ItemNotFound=-2,
    SECoP_EC_NodeNotFound=-3,
    SECOP_EC_FAILED=-4,
};

#ifdef __cplusplus
/* plain C++ */
enum SECoP_C_error SHALL_EXPORT SECoP_C_addNode(std::string IP, int Port, std::string &Node);
std::string SHALL_EXPORT SECoP_C_getNodeNameByIP(std::string IP, int Port);
std::list<std::string> SHALL_EXPORT getNodeNamesList();
std::list<std::string> SHALL_EXPORT getModuleNamesList(std::string Node);
std::list<std::string> SHALL_EXPORT getAccNameList(std::string Node, std::string Module);

//Accessible Acc is a parameter or command

std::list<std::string> SHALL_EXPORT getNodePropertiesKeyList(std::string Node);
std::list<std::string> SHALL_EXPORT getModulePropertiesKeyList(std::string Node, std::string Module);
std::list<std::string> SHALL_EXPORT getAccPropertiesKeyList(std::string Node, std::string Module, std::string Acc);

std::string SHALL_EXPORT getNodePropertyAsString(std::string Node, std::string Key);
std::string SHALL_EXPORT getModulePropertyAsString(std::string Node, std::string Module, std::string Key);
std::string SHALL_EXPORT getAccPropertyAsString(std::string Node, std::string Module, std::string Acc, std::string Key);

//enum SECoP_C_error SHALL_EXPORT readParamVariant(std::string Node, std::string Module, std::string ParamName, SECoP_data** ppValue, double &dTimestamp, SECoP_data** ppErrorVal);
//enum SECoP_C_error SHALL_EXPORT readParamDouble(std::string Node, std::string Module, std::string ParamName, double *pdValue, double* pdTimestamp, double *pdErrorVal);
std::string SHALL_EXPORT readParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal);
std::string SHALL_EXPORT forcedReadParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp,SECoP_dataPtr  &ErrorVal);
std::string SHALL_EXPORT writeParam(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr Value);
std::string SHALL_EXPORT testRead(std::string Node, std::string Module, std::string ParamName, SECoP_dataPtr &Value, double &dTimestamp, SECoP_dataPtr &ErrorVal, std::string &SECoPtripel, qint64 &respTime, qint64 maxTime);
std::list<std::string> SHALL_EXPORT splitSECoPTriple(std::string Triple);

//enum SECoP_C_error SHALL_EXPORT callModule(std::string Node, std::string Module, std::string ParamName, std::shared_ptr<SECoP_data> Arguments, std::shared_ptr<SECoP_data> &Result);

/**
 * \brief Call SECoP_C_ShowGui, if you want to switch the visibility of the Gui. If you
 *        call this with bShowGui==true, then the Gui is shown.
 *        If you call this with bShowGui==false the Gui hides.
 *        If using the Clientlibrary together with an ECS that provides an own Gui,
 *        this function may be used for supervision.
 * \param[in] bShowGui 0=false: show Gui, 1=true: hide Gui
 */
void SHALL_EXPORT SECoP_C_ShowGui(bool bShowGui);


/**
 * \brief Call SECoP_C_startInactive, if you want to disable the the automatic activation of a Node.
 *        This has to be called before the calling of addNode because addNode otherwise activates the Node;
 *        There may be an few Situations where this function is needed but dont forget to activate the Node!
 *        It is against the intention of SECoP having an unactivated Node.
 *        Look at SECoP_C_activateNode.
 */
void SHALL_EXPORT SECoP_C_startInactive();

/**
 * \brief Call SECoP_C_activateNode, to activate an inactive started Node. Only needed if activation was disabled via SECoP_C_startInactive.
 * \param[in] Node Name of the Node
 */
void SHALL_EXPORT SECoP_C_activateNode(std::string Node);

extern "C" {
#endif

/* plain C */

enum SECoP_C_error SHALL_EXPORT SECoP_C_initLibrary(QApplication *pApplication, int bGUI);
void SHALL_EXPORT SECoP_C_doneLibrary(int bNodeOnly);
enum SECoP_C_error SHALL_EXPORT addNode(const char* szIP, unsigned short wPort, char* szNode, int szNodeBufferLen);
extern "C" void SHALL_EXPORT SECoP_C_showStatusWindow(int bShowGUI);
enum SECoP_C_error SHALL_EXPORT SECoP_C_removeNode(const char* szIP, unsigned short wPort);
int SHALL_EXPORT SECoP_C_getNumberOfNodes();
int SHALL_EXPORT SECoP_C_getNumberOfModules(const char* szNode);
int SHALL_EXPORT getNumberOfAcc(const char* szNode, const char* szModule);

//const char* SHALL_EXPORT getNodeName(int iIndex); this causes C2059 the versions below should fix
SHALL_EXPORT const char* getNodeName(int iIndex);
SHALL_EXPORT const char* getModuleName(const char* szNode, int iIndex);
SHALL_EXPORT const char* getModuleNameI(int iNodeIndex, int iModuleIndex);
SHALL_EXPORT const char* getAccName(const char* szNode, const char* szModule, int iIndex);
SHALL_EXPORT const char* getAccNameI(int iNodeIndex, int iModuleIndex, int iAccIndex);

//error "TODO: weitere Funktionen für Zugriff"

//some function to set and get value not implemented yet
void callAcc(const char* szNode, const char* szModule, const char* Acc, const char* Value);

#ifdef __cplusplus
}
#endif

#endif // EXPORTS_H
