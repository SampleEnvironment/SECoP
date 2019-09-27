/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_EXPORTS_H__81F95F4C_9E08_494D_8073_A68C018A83D1__
#define __SECOP_EXPORTS_H__81F95F4C_9E08_494D_8073_A68C018A83D1__

#include "SECoP-defines.h"
#include "SECoP-Variant.h"
#include "SECoP-types.h"

#ifdef __cplusplus
class QApplication; // forward declaration for C++ programs
extern "C"
{
#else
typedef void QApplication; // declaration for C programs
#endif

/**
 * \brief SECoP_S_initLibrary is the first function to call. It initializes
 *        the library and prepares internal data. The pApplication pointer
 *        is used, if you already have a QApplication instance in your program.
 *        Provide a NULL/Q_NULLPTR/nullptr/0 pointer here for all other programs
 *        and the library will instanciate its own QApplication class. If you
 *        disable function pointers, you should provide NULL pointers to functions
 *        (\ref SECoP_S_addCommand, \ref SECoP_S_addReadableParameter,
 *        \ref SECoP_S_addWritableParameter).
 * \ingroup expfunc
 * \param[in] pApplication pointer to existing Qt application or NULL
 * \param[in] bGUI   0=false: do not show status window, 1=true: show status window
 * \param[in] bEnableFunctionPointers
 *                   0=false: disable function pointers and force polling,
 *                   1=true: use function pointers
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_initLibrary(QApplication *pApplication, int bGUI, int bEnableFunctionPointers);

/**
 * \brief Call SECoP_S_doneLibrary, if you finished using this library. If you
 *        call this with bNodeOnly==false, then the library will do its best
 *        to clean up all allocated data and stop created threads. If this
 *        does not work for you, try to call it with bNodeOnly==true, which
 *        only stops some threads and clean up all nodes with content. Cleaning
 *        up the rest should be done by the runtime library.
 * \ingroup expfunc
 * \param[in] bNodeOnly 0=false: try to clean up everything, 1=true: clean up only nodes
 */
void SHALL_EXPORT SECoP_S_doneLibrary(int bNodeOnly);

/**
 * \brief SECoP_S_setManyThreads configures thread creation inside the library
 *        0:      minimize thread creation (nevertheless this library is not single threaded but eases debugging)
 *        other:  create a thread for every node, module, client connection (default)
 * \ingroup expfunc
 * \param[in] bManyThreads 0=false: minimize thread creation, other: thread for node/module/connection
 */
void SHALL_EXPORT SECoP_S_setManyThreads(int bManyThreads);

/**
 * \brief Call SECoP_S_createNode to create an empty SECoP node. You need at
 *        least one for a working SEC-node. Every node exists independent of
 *        each other, but it is up to the user, how they act.
 * \ingroup expfunc
 * \param[in] szID   unique name of the SECoP node
 * \param[in] szDesc a mandatory description text
 * \param[in] wPort  TCP port to listen to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_createNode(const char* szID, const char* szDesc, unsigned short wPort);

/**
 * \brief Call SECoP_S_createNode2 to create an empty SECoP node and bind it to
 *        a specific interface and port. You need at least one for a working
 *        SEC-node. Every node exists independent of each other, but it is up
 *        to the user, how they act.
 * \ingroup expfunc
 * \param[in] szID        unique name of the SECoP node
 * \param[in] szDesc      a mandatory description text
 * \param[in] szInterface the TCP interface to listen to
 * \param[in] wPort       TCP port to listen to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_createNode2(const char* szID, const char* szDesc, const char* szInterface, unsigned short wPort);

/**
 * \brief Delete an existing SECoP node with SECoP_S_deleteNode.
 * \ingroup expfunc
 * \param[in] szID name of the SECoP node to delete
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_deleteNode(const char* szID);

/**
 * \brief Call SECoP_S_addModule to create a SECoP module inside the last created SECoP node.
 * \ingroup expfunc
 * \param[in] szName name of the module, which is unique inside the SECoP node
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addModule(const char* szName);

/**
 * \brief Call SECoP_S_addCommand to create a SECoP command inside the last created SECoP module.
 * \ingroup expfunc
 * \param[in] szKey     name of the command, which is unique inside the SECoP module
 * \param[in] ptrToFunc function, which is called when a client invokes the command
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addCommand(const char* szKey, SECoP_S_callFunction ptrToFunc);

/**
 * \brief Call SECoP_S_addReadableParameter to create a read only SECoP parameter
 *        inside the last created SECoP module.
 * \ingroup expfunc
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addReadableParameter(const char* szName, SECoP_S_getsetFunction ptrToGet);

/**
 * \brief Call SECoP_S_addWritableParameter to create a read- and writable SECoP
 *        parameter inside the last created SECoP module.
 * \ingroup expfunc
 * \param[in] szName   name of the parameter, which is unique inside the SECoP module
 * \param[in] ptrToGet function, which is called when a client asks for the value of this parameter
 * \param[in] ptrToSet function, which is called when a client wants to set a new value
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addWritableParameter(const char* szName, SECoP_S_getsetFunction ptrToGet, SECoP_S_getsetFunction ptrToSet);

/**
 * \brief Call SECoP_S_addProperty to append a SECoP property to the last SECoP
 *        node, module, command or parameter. You need this function or some of
 *        the convenience functions SECoP_S_addProperty... , because there are
 *        mandatory and some optional properties for everything.
 * \ingroup expfunc
 * \param[in] szKey  name of the property, which is unique inside its context.
 *                   You may prepend an unterscore for own properties
 * \param[in] pValue value of the property; note: use the recommended data type
 *                   for the standard SECoP properties
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addProperty(const char* szKey, const CSECoPbaseType* pValue);

/**
 * \brief Convenience function for \ref SECoP_S_addProperty with a boolean value.
 * \ingroup expfunc
 * \param[in] szKey  name of the property, which is unique inside its context.
 * \param[in] bValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyBoolean(const char* szKey, long long bValue);

/**
 * \brief Convenience function for \ref SECoP_S_addProperty with an integer value.
 * \ingroup expfunc
 * \param[in] szKey   name of the property, which is unique inside its context.
 * \param[in] llValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyInteger(const char* szKey, long long llValue);

/**
 * \brief Convenience function for \ref SECoP_S_addProperty with a floating point value.
 * \ingroup expfunc
 * \param[in] szKey    name of the property, which is unique inside its context.
 * \param[in] dblValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyDouble(const char* szKey, double dblValue);

/**
 * \brief Convenience function for \ref SECoP_S_addProperty with a string value.
 * \ingroup expfunc
 * \param[in] szKey   name of the property, which is unique inside its context.
 * \param[in] szValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyString(const char* szKey, const char* szValue);

/**
 * \brief Convenience function for \ref SECoP_S_addProperty with a JSON value.
 * \ingroup expfunc
 * \param[in] szKey   name of the property, which is unique inside its context.
 * \param[in] szValue value of the property
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_addPropertyJSON(const char* szKey, const char* szValue);

/**
 * \brief Call SECoP_S_setAddFocus to change the focus while SECoP node creation.
 *        If your creation is not able to guarantee the order of SECoP_add...
 *        functions, you could use this function. Normally you should not need
 *        this. Use a colon (':') separated name for selecting the current item.
 *        Selectable items are nodes, modules, command and parameters.
 * \ingroup expfunc
 * \param[in] szKey   name of the SECoP item to point to
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_setAddFocus(const char* szKey);

/**
 * \brief Inform the SECoP node that it is complete. This function makes some
 *        checks and on success, it opens the TCP port and marks the node that
 *        you cannot change its structure.
 * \ingroup expfunc
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_nodeComplete();

/**
 * \brief The function SECoP_S_showErrors prints all errors and warnings regarding
 *        node creation to the status window and/or terminal output.
 * \ingroup expfunc
 */
void SHALL_EXPORT SECoP_S_showErrors();

/**
 * \brief Call SECoP_S_updateParameter, if you have a new value for an existing
 *        parameter (read- or writable). All clients which subscribed to this
 *        module should be informed about the new value.
 * \ingroup expfunc
 * \param[in] szParameterName name of the parameter (<node>:<module>:<parameter>)
 * \param[in] pData           new value of the parameter
 * \param[in] pSigma          error of value of the parameter
 * \param[in] dblTimestamp    SECoP timestamp of the value or NaN
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_updateParameter(const char* szParameterName, const CSECoPbaseType* pData,
                                                            const CSECoPbaseType* pSigma, double dblTimestamp);

/**
 * \brief Call SECoP_S_updateParameter, if you have a new value for an existing
 *        parameter (read- or writable). All clients which subscribed to this
 *        module should be informed about this.
 * \ingroup expfunc
 * \param[in] szParameterName name of the parameter (<node>:<module>:<parameter>)
 * \param[in] szValue         new JSON value of the parameter
 * \param[in] iValueSize      size of the JSON value
 * \param[in] szSigma         error of value of the parameter
 * \param[in] iSigmaSize      size of the JSON sigma value or 0 (no sigma value)
 * \param[in] dblTimestamp    SECoP timestamp of the value or NaN
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_updateParameter2(const char* szParameterName, const char* szValue, int iValueSize,
                                                           const char* szSigma, int iSigmaSize, double dblTimestamp);

/**
 * \brief When polling is enabled (\ref SECoP_S_createNode) call SECoP_S_getStoredCommand
 *        regulary to check, if there is something to do. If yes, it will return
 *        successfully with an id, what to do, a parameter/command name and new set
 *        value or argument. The function will never write more characters to the
 *        parameter/command name, which are provided inside piParameterSize.
 * \ingroup expfunc
 * \param[out]    pllId           id to use with \ref SECoP_S_putCommandAnswer
 * \param[out]    piAction        what is to do: read or change parameter, do command invokation
 * \param[out]    szParameter     buffer which gets the parameter/command name
 * \param[in,out] piParameterSize input: maximum buffer size, out: needed buffer size
 * \param[out]    ppValue         on "change": set value, on "do": function call argument
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_getStoredCommand(unsigned long long* pllId, enum SECoP_S_action *piAction,
                                                             char* szParameter, int* piParameterSize, CSECoPbaseType** ppValue);

/**
 * \brief When polling is enabled (\ref SECoP_S_createNode) call SECoP_S_getStoredCommand2
 *        regulary to check, if there is something to do. If yes, it will return
 *        successfully with an id, what to do, a parameter/command name and new set
 *        value or argument. The function will never write more characters to the
 *        parameter/command name, which are provided inside piParameterSize.
 * \ingroup expfunc
 * \param[out]    pllId           id to use with \ref SECoP_S_putCommandAnswer2
 * \param[out]    piAction        what is to do: read or change parameter, do command invokation
 * \param[out]    szParameter     buffer which gets the parameter/command name
 * \param[in,out] piParameterSize input: maximum name buffer size, out: needed name buffer size
 * \param[out]    szValue         buffer which gets the set value or command argument as json
 * \param[in,out] piValueSize     input: maximum value buffer size, out: needed value buffer size
 * \return on success SECoP_S_SUCCESS or a SECoP_S_error
 */
enum SECoP_S_error SHALL_EXPORT SECoP_S_getStoredCommand2(unsigned long long* pllId, enum SECoP_S_action *piAction,
                                                              char* szParameter, int* piParameterSize, char* szValue,
                                                              int* piValueSize);

/**
 * \brief Call SECoP_S_putCommandAnswer, when \ref SECoP_S_getStoredCommand provided
 *        a to do action. You have to provide the read value, value after change
 *        or command call result. All clients which subscribed to this module
 *        should be informed about parameter values.
 * \ingroup expfunc
 * \param[in] llId         id to use with \ref SECoP_S_putCommandAnswer2
 * \param[in] iErrorCode   SECoP_S_SUCCESS or SECoP_S_error for this action.
 * \param[in] pValue       new value of the parameter or command result
 * \param[in] pSigma       error of value of the parameter
 * \param[in] dblTimestamp SECoP timestamp of the value or NaN
 */
void SHALL_EXPORT SECoP_S_putCommandAnswer(unsigned long long llId, enum SECoP_S_error iErrorCode,
                                               const CSECoPbaseType* pValue, const CSECoPbaseType* pSigma,
                                               double dblTimestamp);

/**
 * \brief Call SECoP_S_putCommandAnswer2, when \ref SECoP_S_getStoredCommand2 provided
 *        a to do action. You have to provide the read value, value after change
 *        or command call result. All clients which subscribed to this module
 *        should be informed about parameter values.
 * \ingroup expfunc
 * \param[in] llId         id to use with \ref SECoP_S_putCommandAnswer2
 * \param[in] iErrorCode   SECoP_S_SUCCESS or SECoP_S_error for this action.
 * \param[in] szValue      new JSON value of the parameter or command result
 * \param[in] iValueSize   size of the JSON value
 * \param[in] szSigma      error of value of the parameter
 * \param[in] iSigmaSize   size of the JSON sigma value or 0 (no sigma value)
 * \param[in] dblTimestamp SECoP timestamp of the value or NaN
 */
void SHALL_EXPORT SECoP_S_putCommandAnswer2(unsigned long long llId, enum SECoP_S_error iErrorCode, const char* szValue,
                                                int iValueSize, const char* szSigma, int iSigmaSize, double dblTimestamp);

/**
 * \brief Call SECoP_S_getErrorString to convert a SECoP_S_error value into a human
 *        readable text, which describes the error.
 * \ingroup expfunc
 * \param[in] iError SECoP_S_error to convert
 * \return a constant static string pointer to the description
 */
const char SHALL_EXPORT * SECoP_S_getErrorString(enum SECoP_S_error iError);

/**
 * \brief shows or hides the status window
 * \ingroup expfunc
 * \param[in] bShowGUI true: show status window, false: hide it
 */
void SHALL_EXPORT SECoP_S_showStatusWindow(int bShowGUI);

#ifdef __cplusplus
}
#endif

#endif /*__SECOP_EXPORTS_H__81F95F4C_9E08_494D_8073_A68C018A83D1__*/
