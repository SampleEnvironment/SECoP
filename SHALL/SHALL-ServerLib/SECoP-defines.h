/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_GLOBAL_H__7DDAD5D3_12B7_411E_B88D_6435933EFCF7__
#define __SECOP_GLOBAL_H__7DDAD5D3_12B7_411E_B88D_6435933EFCF7__

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

// the pointer which points to nothing
#ifndef NULL
#  ifdef __cplusplus
#    if __cplusplus >= 201103L
#      define NULL nullptr
#    else
#      define NULL 0
#    endif
#  else
#    define NULL ((void*)0)
#  endif
#endif

/**
 * \brief default timeout in milliseconds, which is used by polling SECoP commands
 * \ref SECoP_S_getStoredCommand or \ref SECoP_S_getStoredCommand2
 * \ingroup exptype
 */
#define SECOP_POLLING_TIMEOUT     60000 /* ms (1 minute) */

/**
 * \brief default polling interval for SECoP modules and their parameters
 * \ingroup exptype
 */
#define SECOP_DEFAULT_POLLINTERVAL 1000 /* ms */

/** \brief maximum possible polling interval for SECoP modules and their parameters
 * \ingroup exptype
 */
#define SECOP_MAX_POLLINTERVAL  3600000 /* ms (1 hour) */

#endif /*__SECOP_GLOBAL_H__7DDAD5D3_12B7_411E_B88D_6435933EFCF7__*/
