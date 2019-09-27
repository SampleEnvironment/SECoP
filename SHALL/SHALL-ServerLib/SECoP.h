/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_H__D8182F65_C81B_4D87_BA7C_366EEA1709C1__
#define __SECOP_H__D8182F65_C81B_4D87_BA7C_366EEA1709C1__

/**
 * \defgroup expfunc exported functions
 * \defgroup exptype exported definitions and types
 * \defgroup intfunc internals
 * \mainpage
 *    This is the main include header for the SECoP node library (server side).
 *    For compatibility to future versions, please do not directly include
 *    the sub header files, this should be enough:
 *
 * \code{.cpp}
 *    #include "SECoP.h"
 * \endcode
 *
 *    This library has a C compatible interface, which should be usable by
 *    other languages. Depending, if your preferred language deals with
 *    function pointers, your code may be called through the library or you
 *    have to poll for client requests. After call of \ref SECoP_S_initLibrary
 *    all is ready for use. You should create a node (opens a TCP port), at
 *    least one module with one or more accessibles (parameters or commands).
 *    All of them should have some properties (e.g. a description). After all
 *    things were defined, you have to call \ref SECoP_S_nodeComplete, which
 *    will do basic checks and on success open the TCP port. When a client
 *    requests something, your handler function is called or you have to poll
 *    with \ref SECoP_S_getStoredCommand (or \ref SECoP_S_getStoredCommand2)
 *    for the requests. You should handle the request as SECoP defines (e.g.
 *    write parameter change request to hardware) and answer the client
 *    directly (function call) or with a call of \ref SECoP_S_putCommandAnswer
 *    (or \ref SECoP_S_putCommandAnswer2). Normally a SEC node works for a
 *    longer time, but for a clean shut down, you should call
 *    \ref SECoP_S_doneLibrary.
 *
 *    See the GitHub web site for description of the SECoP protocol,
 *    definitions, data types and for formats:
 *
 *      https://github.com/SampleEnvironment/SECoP
 *
 * \code{.cpp}
 *    // create a very limited SEC node without Qt nor function pointers
 *    SECoP_S_initLibrary(0, 1, 0);
 *    SECoP_S_createNode("my_node", "best SEC node ever", 2055);
 *    SECoP_S_addModule("mod1");
 *    SECoP_S_addPropertyString("description", "the first module");
 *    SECoP_S_addReadableParameter("value", 0);
 *    SECoP_S_addPropertyString("description", "actual value");
 *    SECoP_S_addPropertyString("datainfo", "{\"type\":\"double\", \"unit\":\"mm\"}");
 *    SECoP_S_nodeComplete();
 *    // setup complete, handle requests if a loop
 *    while (allowed_to_loop)
 *    {
 *       unsigned long long id; // maps to an uint64_t
 *       SECoP_S_action action; // maps to an int
 *       SECoP_S_error error;   // maps to an int
 *       char parameter[50], value[1000];
 *       int parameter_size = 50, value_size = 1000;
 *       if (SECoP_S_getStoredCommand2(&id, &action, parameter, &parameter_size, value, &value_size) >= 0)
 *       {
 *          // got a request, handle it
 *          ....
 *          error = SECoP_S_SUCCESS;
 *          SECoP_S_putCommandAnswer2(id, error, value, strlen(value), 0, 0, 0.0);
 *       }
 *    }
 *    // clean up
 *    SECoP_S_doneLibrary(0);
 * \endcode
 */

// some defines for library use
#include "SECoP-defines.h"

// SECoP variant data type, which is used for data transport
#include "SECoP-Variant.h"

// SECoP data types
#include "SECoP-types.h"

// exported functions for normal SECoP nodes
#include "SECoP-exports.h"

#endif /*__SECOP_H__D8182F65_C81B_4D87_BA7C_366EEA1709C1__*/
