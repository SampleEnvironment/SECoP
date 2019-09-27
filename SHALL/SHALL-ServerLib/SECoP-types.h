/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_ERROR_H__3BB08092_2314_4A0B_B6CE_4D243DB4223080__
#define __SECOP_ERROR_H__3BB08092_2314_4A0B_B6CE_4D243DB4223080__

/**
 * \brief SECoP errors and warning codes
 * \ingroup exptype
 */
enum SECoP_S_error
{
    /** successful call */
    SECoP_S_SUCCESS                  =   0,

    /** no description was given*/
    SECoP_S_WARNING_NO_DESCRIPTION   =   1,

    /** custom name of property should be prefixed with an underscore */
    SECoP_S_WARNING_CUSTOM_PROPERTY  =   2,

    /** input buffer is too small, some information is missing (call again with greater buffer) */
    SECoP_S_WARNING_BUFFER_TOO_SMALL =   3,

    /** warning of missing optional properties */
    SECoP_S_WARNING_MISS_PROPERTIES  =   4,

    /** invalid SECoP command (SECoP error "SyntaxError") */
    SECoP_S_ERROR_UNKNOWN_COMMAND    =  -1, // not used inside library

    /** invalid SECoP name (SECoP error "SyntaxError") */
    SECoP_S_ERROR_INVALID_NAME       =  -2,

    /** invalid node reference */
    SECoP_S_ERROR_INVALID_NODE       =  -3,

    /** invalid module reference (SECoP error "NoSuchModule") */
    SECoP_S_ERROR_INVALID_MODULE     =  -4,

    /** invalid parameter reference (SECoP error "NoSuchParameter") */
    SECoP_S_ERROR_INVALID_PARAMETER  =  -5,

    /** invalid property reference */
    SECoP_S_ERROR_INVALID_PROPERTY   =  -6,

    /** invalid command or command reference (SECoP error "NoSuchCommand") */
    SECoP_S_ERROR_INVALID_COMMAND    =  -7,

    /** function call is not implemented (SECoP error "CommandFailed") */
    SECoP_S_ERROR_NOT_IMPLEMENTED    =  -8,

    /** read only node or parameter (SECoP error "ReadOnly") */
    SECoP_S_ERROR_READONLY           =  -9,

    /** function call cannot return any data */
    SECoP_S_ERROR_NO_DATA            = -10,

    /** memory allocation problem */
    SECoP_S_ERROR_NO_MEMORY          = -11,

    /** call \ref SECoP_S_initLibrary() first */
    SECoP_S_ERROR_NOT_INITIALIZED    = -12,

    /** datainfo or value was null or invalid (SECoP error "BadValue") */
    SECoP_S_ERROR_INVALID_VALUE      = -13,

    /** an expected property was null or empty */
    SECoP_S_ERROR_MISSING_MANDATORY  = -14,

    /** missing a pointer to getter and setter function */
    SECoP_S_ERROR_NO_SETTER_GETTER   = -15,

    /** missing a pointer to setter function */
    SECoP_S_ERROR_NO_SETTER          = -16,

    /** missing a pointer to getter function */
    SECoP_S_ERROR_NO_GETTER          = -17,

    /** the node, module, parameter, command or property name is used before */
    SECoP_S_ERROR_NAME_ALREADY_USED  = -18,

    /** parameter or command timeout (SECoP error "CommunicationFailed") */
    SECoP_S_ERROR_TIMEOUT            = -19,

    /** 32 bit or 64 bit systems are supported lower or higher systems not */
    SECoP_S_ERROR_WRONG_BITNESS      = -20,

    /** additional forced SECoP error "CommandFailed" */
    SECoP_S_ERROR_COMMAND_FAILED     = -21,

    /** additional forced SECoP error "CommandRunning" */
    SECoP_S_ERROR_COMMAND_RUNNING    = -22,

    /** additional forced SECoP error "CommunicationFailed" */
    SECoP_S_ERROR_COMM_FAILED        = -23,

    /** additional forced SECoP error "IsBusy" */
    SECoP_S_ERROR_IS_BUSY            = -24,

    /** additional forced SECoP error "IsError" */
    SECoP_S_ERROR_IS_ERROR           = -25,

    /** additional forced SECoP error "Disabled" */
    SECoP_S_ERROR_DISABLED           = -26,

    /** additional forced SECoP error "SyntaxError" */
    SECoP_S_ERROR_SYNTAX             = -27,

    /** additional forced SECoP error "InternalError" */
    SECoP_S_ERROR_INTERNAL           = -28,
};

/**
 * \brief SECoP action codes, \ref SECoP_S_getStoredCommand or \ref SECoP_S_getStoredCommand2
 * \ingroup exptype
 */
enum SECoP_S_action
{
    /** no action (should never be used) */
    SECoP_S_ACTION_NONE   = 0,

    /** SECoP read command */
    SECoP_S_ACTION_READ   = 1,

    /** SECoP change command */
    SECoP_S_ACTION_CHANGE = 2,

    /** SECoP do command */
    SECoP_S_ACTION_DO     = 3,
};

/**
 * \brief prototype of getter and setter functions for parameter access
 * \ingroup exptype
 * \param[in]     name        name of the accessible (parameter) with full name (node:module:accessible)
 * \param[out]    piError     result of the getter/setter (see \ref SECoP_S_error)
 * \param[in,out] ppDataInOut input/output value
 * \param[out]    ppDataSigma sigma/error value
 * \param[out]    timestamp   SECoP time stamp (fractional unix seconds)
 */
typedef void (*SECoP_S_getsetFunction)(const char* name, enum SECoP_S_error* piError, CSECoPbaseType** ppDataInOut, CSECoPbaseType** ppDataSigma, double* timestamp);

/**
 * \brief prototype of functions for command calls
 * \ingroup exptype
 * \param[in]     name        name of the accessible (command) with full name (node:module:accessible)
 * \param[in]     pArgument   call input value (if any)
 * \param[out]    piError     result of the getter/setter (see \ref SECoP_S_error)
 * \param[out]    pResult     call output value (if any)
 * \param[out]    timestamp   SECoP time stamp (fractional unix seconds)
 */
typedef void (*SECoP_S_callFunction)(const char* name, const CSECoPbaseType* pArgument, enum SECoP_S_error* piError, CSECoPbaseType** ppReturn, double* timestamp);

#endif /*__SECOP_ERROR_H__3BB08092_2314_4A0B_B6CE_4D243DB4223080__*/
