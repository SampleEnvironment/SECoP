/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#ifndef __SECOP_VARIANT_H__F003E0AD_31E3_4575_971F_F25EAD29F788__
#define __SECOP_VARIANT_H__F003E0AD_31E3_4575_971F_F25EAD29F788__

/**
 * \defgroup expfunc exported functions
 * \defgroup exptype exported definitions and types
 * \defgroup intfunc internal functions
 * \mainpage
 *    This library handles and stores SECoP data types and values.
 *    It has a C compatible interface with most of the functionality and
 *    a C++ interface for more convenient operation with data. If you
 *    are able to use the C++ interface, please do it.
 *
 *    The C interface checks the pointers for known object types, but
 *    it cannot prevent bugs of the caller.
 *
 *    See the GitHub web site for description of the SECoP protocol,
 *    definitions, data types and for formats:
 *
 *      https://github.com/SampleEnvironment/SECoP
 *
 *    The data here is stored only. The library will not automatically
 *    do things in background. There is a concept of ownership of memory,
 *    normally the library owns the memory or copies data. In special
 *    documented cases, the library takes ownership of the data, you
 *    do not have to free memory then.
 *
 * \code{.cpp}
 *    // create a simple integer array of 5 elements
 *    SECoP_V_create("{\"type\":\"array\",\"members\":{\"type\":\"int\"},\"maxlen\":5}");
 *    // create an tuple of an integer array of 5 elements and a string with 255 characters
 *    SECoP_V_create("{\"type\":\"tuple\",\"members\":[{\"type\":\"array\",\"members\":{\"type\":\"int\"},\"maxlen\":5},{\"type\":\"string\"}]}");
 *
 *    // use of SECoP_V_getInfo:
 *    unsigned int uPos = 0;
 *    SECoP_V_type iType;
 *    unsigned int uCount;
 *    const char* szName;
 *    double d;
 *    while (SECoP_V_getInfo(pData, uPos, &iType, &uCount, &szName))
 *    {
 *        if (szName != NULL)
 *            print("name=%s: ", szName);
 *        print("type=%u  count=%u\n", iType, uCount);
 *        if (iType == SECoP_VT_DOUBLE || iType == SECoP_VT_ARRAY_DOUBLE)
 *        {
 *            SECoP_V_getDouble(pData, uPos, 0, &d);
 *            print("a value=%f\n", d);
 *        }
 *        uPos = uPos + 1
 *    }
 *
 *    // use of SECoP_V_getInfo2:
 *    unsigned int uPos = 0;
 *    SECoP_V_type iType;
 *    unsigned int uCount, uLen;
 *    char szName[128];
 *    double d;
 *    uLen = sizeof(szName);
 *    while (SECoP_V_getInfo2(pData, uPos, &iType, &uCount, szName, &uLen))
 *    {
 *        if (uLen > 0)
 *            print("name=%.*s: ", uLen, szName);
 *        print("type=%u  count=%u\n", iType, uCount);
 *        if (iType == SECoP_VT_DOUBLE || iType == SECoP_VT_ARRAY_DOUBLE)
 *        {
 *            SECoP_V_getDouble(pData, uPos, 0, &d);
 *            print("a value=%f\n", d);
 *        }
 *        uPos = uPos + 1;
 *        uLen = sizeof(szName);
 *    }
 * \endcode
 */

#include <stdio.h>

//strcasecmp() is a function specified by posix, which MSVC does not support
#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4661)
#endif

#ifdef __cplusplus
#include "json.hpp"
extern "C"
{
#endif

/**
 * \brief SECoP type codes and flags for variant type, \ref CSECoPbaseType
 * \ingroup exptype
 */
enum SECoP_V_type
{
    /** value is undefined */
    SECoP_VT_NONE          =  0,

    /** value is a floating point value */
    SECoP_VT_DOUBLE        =  1, // uCount == 1

    /** value is an integer */
    SECoP_VT_INTEGER       =  2, // uCount == 1

    /** value is a boolean */
    SECoP_VT_BOOL          =  3, // uCount == 1

    /** value is an enumeration (integer with named values) */
    SECoP_VT_ENUM          =  4, // uCount == 1

    /** value is a scaled integer */
    SECoP_VT_SCALED        =  5, // uCount == 1

    /** value is an array of a SECoP type (all values have same type) */
    SECoP_VT_ARRAY         = 10,

    /** value is an array of floating point values */
    SECoP_VT_ARRAY_DOUBLE  = 11,

    /** value is an array of integer values */
    SECoP_VT_ARRAY_INTEGER = 12,

    /** value is an array of boolean values */
    SECoP_VT_ARRAY_BOOL    = 13,

    /** value is an array of enumerations (integers with named values, all same type) */
    SECoP_VT_ARRAY_ENUM    = 14,

    /** value is an array of scaled integers (same multiplicator) */
    SECoP_VT_ARRAY_SCALED  = 15,

    /** value is a string */
    SECoP_VT_STRING        = 20,

    /** value is a binary object */
    SECoP_VT_BLOB          = 21,

    /** value is a JSON string */
    SECoP_VT_JSON          = 22,

    /** value is a structured type (key -> value of SECoP types) */
    SECoP_VT_STRUCT        = 30, // uCount > 0

    /** value is a fixed length array of different SECoP types */
    SECoP_VT_TUPLE         = 31, // uCount > 0

    /** value is a SECoP command, which may be special */
    SECoP_VT_COMMAND       = 40,
};

#if defined(__cplusplus)
class CSECoPbaseType;
#else
struct CSECoPbaseType;
typedef struct {} CSECoPbaseType;
#endif

/**
 * \brief result of SECoP_V_compare
 * \ingroup exptype
 */
enum SECoP_V_compareResult
{
    /** the two values are equal (equal types and equal values) */
    SECoP_VC_EQUAL = 0,

    /** the two values are similar (compatible types and equal values) */
    SECoP_VC_SIMILAR_VALUE = 1,

    /** the two values are different */
    SECoP_VC_DIFF_VALUE = 2,

    /** the two values have different types */
    SECoP_VC_DIFF_TYPE = 3
};


/**
 * \brief Function to create an pre-configured CSECoPbaseType structure from the
 *        description, which can be used in the SECoP client and server.
 *        Valid types are "double", "int", "bool", "string", "blob", "scaled", "enum",
 *        "array", "tuple" or "struct" like allowed data types in SECoP.
 * \ingroup expfunc
 * \param[in] szDescription structure of the data as described in SECoP standard
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType SHALL_EXPORT * SECoP_V_create(const char* szDescription);

/**
 * \brief Function to create a simple boolean value for convenience in SECoP.
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType SHALL_EXPORT * SECoP_V_createSimpleBool();

/**
 * \brief Function to create a simple double value (64bit without limits) for convenience in SECoP.
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType SHALL_EXPORT * SECoP_V_createSimpleDouble();

/**
 * \brief Function to create a simple integer value (signed 64bit) for convenience in SECoP.
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType SHALL_EXPORT * SECoP_V_createSimpleInteger();

/**
 * \brief Function to create a simple scaled integer value (signed 64bit) for convenience in SECoP.
 * \ingroup expfunc
 * \param[in] dScale scaling factor (>0.0)
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType SHALL_EXPORT * SECoP_V_createSimpleScaled(double dScale);

/**
 * \brief Function to create a simple string value for convenience in SECoP.
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType SHALL_EXPORT * SECoP_V_createSimpleString();

/**
 * \brief Function to create a SECoP status for convenience in SECoP.
 *        This contains an enumeration (with 100=IDLE) and a string.
 *        {"type":"tuple","members":[{"type":"enum","members":{"IDLE":100}},{"type":"string"}]}
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType SHALL_EXPORT * SECoP_V_createSECoPStatus();

/**
 * \brief Use this function to get the SECoP data type of a CSECoPbaseType variant.
 * \ingroup expfunc
 * \param[in]  pData       input data
 * \param[out] szBuffer    buffer to write the SECoP data type description, if not NULL
 * \param[in]  iBufferSize size of szBuffer in bytes
 * \return negative for errors else the needed buffer size
 */
int SHALL_EXPORT SECoP_V_getType(const CSECoPbaseType* pData, char* szBuffer, int iBufferSize);

/**
 * \brief Use this function to convert SECoP-compatible data into a CSECoPbaseType variant.
 * \ingroup expfunc
 * \param[in] szJSON data to convert
 * \param[in] pHint  existing data to return a compatible type
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType SHALL_EXPORT * SECoP_V_fromJSON(const char* szJSON, const CSECoPbaseType* pHint);

/**
 * \brief Use this function to convert a CSECoPbaseType variant into SECoP-compatible json data.
 * \ingroup expfunc
 * \param[in]  pData        input data to convert
 * \param[out] szJSONBuffer buffer to write the json data, if not NULL
 * \param[in]  iBufferSize  size of szJSONBuffer in bytes
 * \return negative for errors else the needed buffer size for the json data
 */
int SHALL_EXPORT SECoP_V_toJSON(const CSECoPbaseType* pData, char* szJSONBuffer, int iBufferSize);

/**
 * \brief Function to destroy an existing CSECoPbaseType structure with all its children.
 *        The storage pointer value is reset to NULL.
 * \ingroup expfunc
 * \param[in] ppData pointer to data to destroy
 */
void SHALL_EXPORT SECoP_V_destroy(CSECoPbaseType** ppData);

/**
 * \brief Function to copy existing CSECoPbaseType into a new
 * \ingroup expfunc
 * \param[out] ppDst destination (pointer will be allocated)
 * \param[in]  pSrc  source data
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_copy(CSECoPbaseType** ppDst, const CSECoPbaseType* pSrc);

/**
 * \brief print SECoP data to file using SECoP_V_printStream
 * \ingroup expfunc
 * \param[in] pOutfile   output stream to print to
 * \param[in] pData      input data to print
 * \param[in] iVerbosity 0=print data only, 1=print data with type, 2=print with additional info
 */
void SHALL_EXPORT SECoP_V_printFile(FILE* pOutfile, const CSECoPbaseType* pData, int iVerbosity);

#if defined(__cplusplus) && ((defined(QT_VERSION) && defined(QT_VERSION_CHECK)) || defined(DOXYGEN))
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) || defined(DOXYGEN)
}
/**
 * \brief print SECoP data to UTF-8 byte array using SECoP_V_printStream
 * \ingroup expfunc
 * \param[in] pData      input data to print
 * \param[in] iVerbosity 0=print data only, 1=print data with type, 2=print with additional info
 * \return on success: the converted string, on error: an QByteArray::null object
 */
QByteArray SHALL_EXPORT SECoP_V_printString(const CSECoPbaseType* pData, int iVerbosity);

extern "C"
{
class QTextStream;
/**
 * \brief print SECoP data to QTextStream
 * \ingroup expfunc
 * \param[in] rOutput    output stream to print to
 * \param[in] pData      input data to print
 * \param[in] iVerbosity 0=print data only, 1=print data with type, 2=print with additional info
 * \return true, on success; false, if any error was found
 */
bool SHALL_EXPORT SECoP_V_printStream(QTextStream &rOutput, const CSECoPbaseType* pData, int iVerbosity);
#endif
#endif

/**
 * \brief With this function you can get read access to any data stored in CSECoPbaseType.
 *        The position starts with zero and will go recursivly through the data. Every
 *        position has a data type and a number of values. When accessing a complex
 *        value, the first position is the complex data itself and returns a NULL data
 *        pointer. The next positions address the items of the complex data, while
 *        using recursive function calls to return the content of each item first,
 *        then the next item. A non-complex value has only one position regardless of
 *        the number of values. Access of a position outside all bounds gives a false
 *        return value.
 *        You may store this information to know the nesting level or structure of data.
 * \ingroup expfunc
 * \param[in]  pData     CSECoPbaseType with read access
 * \param[in]  uPosition zero-based position to data part
 * \param[out] piType    data type
 * \param[out] puCount   count of elements
 * \param[out] pszName   name of data
 * \return true, if information was accessible; false, if data or uPosition was invalid (end of data reached)
 */
bool SHALL_EXPORT SECoP_V_getInfo(const CSECoPbaseType* pData, unsigned int uPosition, enum SECoP_V_type* piType,
                                      unsigned int* puCount, const char** pszName);

/**
 * \brief With this function you can get read access to any data stored in CSECoPbaseType.
 *        The position starts with zero and will go recursivly through the data. Every
 *        position has a data type and a number of values. When accessing a complex
 *        value, the first position is the complex data itself and returns a NULL data
 *        pointer. The next positions address the items of the complex data, while
 *        using recursive function calls to return the content of each item first,
 *        then the next item. A non-complex value has only one position regardless of
 *        the number of values. Access of a position outside all bounds gives a false
 *        return value.
 *        You may store this information to know the nesting level or structure of data.
 * \ingroup expfunc
 * \param[in]     pData     CSECoPbaseType with read access
 * \param[in]     uPosition zero-based position to data part
 * \param[out]    piType    data type
 * \param[out]    puCount   count of elements
 * \param[out]    szName    name buffer
 * \param[in,out] puLen     size of "szName" buffer (in), name length (out)
 * \return true, if information was accessible; false, if data or uPosition was invalid (end of data reached)
 */
bool SHALL_EXPORT SECoP_V_getInfo2(const CSECoPbaseType* pData, unsigned int uPosition, enum SECoP_V_type* piType,
                                       unsigned int* puCount, char* szName, unsigned int* puLen);

/**
 * \brief Set minimum and maximum double value.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData      CSECoPbaseType to read
 * \param[in] uPosition  zero-based position to data part
 * \param[in] dblMinimum minimum value (or NaN)
 * \param[in] dblMaximum maximum value (minimum <= maximum or NaN)
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_modifyMinMaxDouble(CSECoPbaseType* pData, unsigned int uPosition,
                                                 double dblMinimum, double dblMaximum);

/**
 * \brief Set minimum and maximum integer value.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to modify
 * \param[in] uPosition zero-based position to data part
 * \param[in] llMinimum minimum value
 * \param[in] llMaximum maximum value (minimum <= maximum)
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_modifyMinMaxInteger(CSECoPbaseType* pData, unsigned int uPosition,
                                                  long long llMinimum, long long llMaximum);

/**
 * \brief Set current size, minimum size and maximum size of array with the constraint: minimum <= current <= maximum
 *        a special call is available: if minimum size == 1 and maximum size == 0, only the current size is changed and
 *        the minimum and maximum sizes are ignored. The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData          CSECoPbaseType to read
 * \param[in] uPosition      zero-based position to data part
 * \param[in] uLength        current size (minimum <= current <= maximum)
 * \param[in] uMinimumLength minimum array size (>=0)
 * \param[in] uMaximumLength maximum array size (minimum <= maximum)
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_modifyArrayLength(CSECoPbaseType* pData, unsigned int uPosition, unsigned int uLength,
                                                unsigned int uMinimumLength, unsigned int uMaximumLength);

/**
 * \brief Use this function to modify a specific double (numeric) value.
 *        This item is selected via its uPosition (\ref SECoP_V_getInfo) and must
 *        have the type SECoP_VT_DOUBLE. You cannot extend or shrink the array size.
 *        The data pointer is used but not modified - only the data itself is modified.
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to modify
 * \param[in] uPosition zero-based position to data part
 * \param[in] uIndex    zero-based array index into data part
 * \param[in] dblValue  new value
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_modifyDouble(CSECoPbaseType* pData, unsigned int uPosition,
                                           unsigned int uIndex, double dblValue);

/**
 * \brief Use this function to modify a specific integer (numeric) value.
 *        This item is selected via its uPosition (\ref SECoP_V_getInfo) and must
 *        have the type SECoP_VT_INTEGER, SECoP_VT_BOOL, SECoP_VT_ENUM or SECoP_VT_SCALED.
 *        You cannot extend or shrink the array size.
 *        The data pointer is used but not modified - only the data itself is modified.
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to modify
 * \param[in] uPosition zero-based position to data part
 * \param[in] uIndex    zero-based array index into data part
 * \param[in] llValue   new value
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_modifyInteger(CSECoPbaseType* pData, unsigned int uPosition,
                                            unsigned int uIndex, long long llValue);

/**
 * \brief Use this function to modify the factor of a scaled integer.
 *        This item is selected via its uPosition (\ref SECoP_V_getInfo) and must
 *        have the type SECoP_VT_SCALED. You cannot extend or shrink the array size.
 *        The data pointer is used but not modified - only the data itself is modified.
 * \ingroup expfunc
 * \param[in,out] pData          CSECoPbaseType to modify
 * \param[in]     uPosition      zero-based position to data part
 * \param[in]     dblScaleFactor new scaling factor (example: integer=999, scale=0.1 -> means 99.9 for transport)
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_modifyScale(CSECoPbaseType* pData, unsigned int uPosition,
                                          double dblScaleFactor);

/**
 * \brief Use this function to replace existing string data inside the SECoP variant.
 *        This string is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in,out] pData     CSECoPbaseType to modify
 * \param[in]     uPosition zero-based position to SECoP_VT_STRING, SECoP_VT_BLOB or SECoP_VT_JSON
 * \param[in]     szValue   string value to replace
 * \param[in]     iSize     length of string in bytes or -1 for null-terminated string
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_modifyString(CSECoPbaseType* pData, unsigned int uPosition,
                                           const char* szValue, int iSize);

/**
 * \brief Read minimum and maximum array or data sizes.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData           CSECoPbaseType to read
 * \param[in]  uPosition       zero-based position to data part
 * \param[out] puLength        current array size (minimum <= current <= maximum)
 * \param[out] puMinimumLength minimum array size (>=0)
 * \param[out] puMaximumLength maximum array size (minimum <= maximum)
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_getArrayLength(const CSECoPbaseType* pData, unsigned int uPosition,
                                             unsigned int* puLength, unsigned int* puMinimumLength,
                                             unsigned int* puMaximumLength);

/**
 * \brief Read minimum and maximum double value.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[out] pdblMinimum minimum value (or NaN)
 * \param[out] pdblMaximum maximum value (minimum <= maximum or NaN)
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_getMinMaxDouble(const CSECoPbaseType* pData, unsigned int uPosition,
                                              double* pdblMinimum, double* pdblMaximum);

/**
 * \brief Read minimum and maximum integer value.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[out] pllMinimum minimum value
 * \param[out] pllMaximum maximum value (minimum <= maximum)
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_getMinMaxInteger(const CSECoPbaseType* pData, unsigned int uPosition,
                                               long long* pllMinimum, long long* pllMaximum);

/**
 * \brief Use this function to read a single double value out of the SECoP variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData     CSECoPbaseType to read
 * \param[in]  uPosition zero-based position to data part (complete array)
 * \param[in]  uIndex    zero-based index into double array
 * \param[out] pdblValue memory location which will get the double value
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_getDouble(const CSECoPbaseType* pData, unsigned int uPosition,
                                        unsigned int uIndex, double* pdblValue);

/**
 * \brief Use this function to read a single integer value out of the SECoP variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData     CSECoPbaseType to read
 * \param[in]  uPosition zero-based position to data part (complete array)
 * \param[in]  uIndex    zero-based index into integer array
 * \param[out] pllValue  memory location which will get the integer value
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_getInteger(const CSECoPbaseType* pData, unsigned int uPosition,
                                         unsigned int uIndex, long long* pllValue);

/**
 * \brief Use this function to read a string/blob value out of the SECoP variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[in]  uBufferSize size of memory location (function will not copy more than this number of bytes)
 * \param[out] pszValue    memory location which will get the string value
 * \return zero for errors, number of bytes needed (including a trailing NUL)
 */
unsigned int SHALL_EXPORT SECoP_V_getString(const CSECoPbaseType* pData, unsigned int uPosition,
                                                unsigned int uBufferSize, char* pszValue);

/**
 * \brief Function to read the factor of a scaled integer out of the SECoP variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData     CSECoPbaseType to read
 * \param[in]  uPosition zero-based position to data part
 * \param[out] pdblValue memory location which will get the scale factor
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_getScale(const CSECoPbaseType* pData, unsigned int uPosition,
                                       double* pdblValue);

/**
 * \brief Return count of enumeration items in list or structure names.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \return return count of enumeration items in list, structure names or 0 on error
 */
unsigned int SHALL_EXPORT SECoP_V_getEnumStructCount(const CSECoPbaseType* pData, unsigned int uPosition);

/**
 * \brief Return enumeration item from list with zero based index.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData      CSECoPbaseType to read
 * \param[in]  uPosition  zero-based position to data part
 * \param[in]  uIndex     zero based index to enum item list
 * \param[out] pllEnumVal enumeration value for valid index or NULL
 * \return for valid index: enumeration name, for invalid index: NULL
 */
const char SHALL_EXPORT * SECoP_V_getEnumeration(const CSECoPbaseType* pData, unsigned int uPosition,
                                                     unsigned int uIndex, long long* pllEnumVal);

/**
 * \brief Return enumeration item from list with zero based index.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData      CSECoPbaseType to read
 * \param[in]  uPosition  zero-based position to data part
 * \param[in]  llEnumVal  enumeration value
 * \param[in]  szEnumName enumeration name
 * \return true, if successful
 */
bool SHALL_EXPORT SECoP_V_putEnumeration(CSECoPbaseType* pData, unsigned int uPosition,
                                             long long llEnumVal, const char* szEnumName);

/**
 * \brief Return name or struct item from any variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \return for valid index: struct or dictionary name, for invalid index: NULL
 */
const char SHALL_EXPORT * SECoP_V_getStructName(const CSECoPbaseType* pData, unsigned int uPosition);

/**
 * \brief Compare two values and check for equal/compatible types and equal values.
 * \ingroup expfunc
 * \param[in] pData1 first value
 * \param[in] pData2 second value
 * \return result \ref SECoP_V_compareResult
 */
enum SECoP_V_compareResult SHALL_EXPORT SECoP_V_compare(const CSECoPbaseType* pData1, const CSECoPbaseType* pData2);

/**
 * \brief Retrieve number of additional items of variant (e.g. unit).
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \return number of additional items or zero
 */
unsigned int SHALL_EXPORT SECoP_V_getAdditionalCount(const CSECoPbaseType* pData, unsigned int uPosition);

/**
 * \brief Retrieve index of additional item of a variant by name.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \param[in] szName    name of item
 * \return index to additional items or UINT_MAX (-1 as invalid index)
 */
unsigned int SHALL_EXPORT SECoP_V_getAdditionalIndex(const CSECoPbaseType* pData, unsigned int uPosition, const char* szName);

/**
 * \brief Retrieve the key name of additional (string,json-array,json-object) items of a variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[in]  uIndex      zero-based index to additional item (\ref SECoP_V_getAdditionalCount)
 * \param[in]  uBufferSize size of memory location (function will not copy more than this number of bytes)
 * \param[out] pszValue    memory location which will get the string value
 * \return zero for errors, number of bytes needed (including a trailing NUL)
 */
unsigned int SHALL_EXPORT SECoP_V_getAdditionalKey(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex,
                                                       unsigned int uBufferSize, char* pszValue);

/**
 * \brief Retrieve a string of additional (string,json-array,json-object) items of a variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[in]  uIndex      zero-based index to additional item (\ref SECoP_V_getAdditionalCount)
 * \param[in]  uBufferSize size of memory location (function will not copy more than this number of bytes)
 * \param[out] pszValue    memory location which will get the string value
 * \return zero for errors, number of bytes needed (including a trailing NUL)
 */
unsigned int SHALL_EXPORT SECoP_V_getAdditionalString(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex,
                                                          unsigned int uBufferSize, char* pszValue);

/**
 * \brief Retrieve the numeric value as double of additional items of a variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \param[in] uIndex    zero-based index to additional item (\ref SECoP_V_getAdditionalCount)
 * \return on success the value, NaN on errors
 */
double SHALL_EXPORT SECoP_V_getAdditionalDouble(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex);

/**
 * \brief Retrieve the numeric value as integer of additional items of a variant.
 *        The item is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \param[in] uIndex    zero-based index to additional item (\ref SECoP_V_getAdditionalCount)
 * \return on success the value, zero on errors (with might be a valid value)
 */
long long SHALL_EXPORT SECoP_V_getAdditionalInteger(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex);

#ifdef __cplusplus
}
#endif

#if defined(__cplusplus) && ((defined(QT_VERSION) && defined(QT_VERSION_CHECK)) || defined(DOXYGEN))
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) || defined(DOXYGEN)
#include <QByteArray>
#include <QList>
#include <QString>
#include <memory>

/*****************************************************************************
 * class definitions
 *****************************************************************************/
class CSECoPbaseType;
class CSECoPstruct;
class CSECoPtuple;
class CSECoParray;
class CSECoPcommand;

/**
 * \brief a reference counted SECoP variant object
 */
typedef std::shared_ptr<CSECoPbaseType> SECoP_dataPtr;

/**
 * \brief the base class of all SECoP types
 */
class SHALL_EXPORT CSECoPbaseType
{
    // disable copying, moving
    CSECoPbaseType(const CSECoPbaseType &) = delete;
    CSECoPbaseType(const CSECoPbaseType &&) = delete;
    CSECoPbaseType& operator=(const CSECoPbaseType &) = delete;
    CSECoPbaseType& operator=(const CSECoPbaseType &&) = delete;
protected:
    explicit CSECoPbaseType();
    explicit CSECoPbaseType(SECoP_V_type iType);
    explicit CSECoPbaseType(const CSECoPbaseType* pOther); // special copy constructor for "duplicate"
public:
    virtual ~CSECoPbaseType();
    SECoP_V_type getType() const;
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    static CSECoPbaseType* createSECoP(const char* szDescription, bool bAllowCommand);
    static CSECoPbaseType* createSECoP(nlohmann::json json, bool bAllowCommand);
    static CSECoPbaseType* importSECoP(const char* szValue);
    static CSECoPbaseType* importSECoP(const nlohmann::json &data);
    bool importSECoP(const char* szValue, bool bStrict);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    nlohmann::json exportType() const;
    virtual nlohmann::json exportSECoPjson() const;
    QByteArray exportSECoP(bool bNull = true) const;
    CSECoPbaseType* duplicate() const;
    static SECoP_dataPtr simpleBool(bool bValue);
    static SECoP_dataPtr simpleDouble(double dblValue);
    static SECoP_dataPtr simpleInteger(long long llValue);
    static SECoP_dataPtr simpleString(const char* szValue);
    static SECoP_dataPtr simpleString(std::string szValue);
    static SECoP_dataPtr simpleJSON(const char* szValue);
    static SECoP_dataPtr simpleJSON(std::string szValue);
    nlohmann::json additional() const;
    nlohmann::json& additional();
protected:
    void setType(SECoP_V_type iType);
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
private:
    SECoP_V_type m_iType;
    nlohmann::json m_Additional;
};

/**
 * \brief a null type to show, there is nothing
 */
class SHALL_EXPORT CSECoPnull final : virtual public CSECoPbaseType
{
    friend class CSECoPbaseType;
public:
    CSECoPnull();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPnull(const CSECoPnull* pOther); // special copy constructor for "duplicate"
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
};

/**
 * \brief base of simple data types (double, long long and special bool)
 */
template <typename T> class SHALL_EXPORT CSECoPsimpleType : virtual public CSECoPbaseType
{
public:
    CSECoPsimpleType();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool clear();
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
    virtual bool getValue(T &value) const;
    virtual bool setValue(const T value);
protected:
    CSECoPsimpleType(const CSECoPsimpleType<T>* pOther); // special copy constructor for "duplicate"
    T m_value;
};
template class SHALL_EXPORT CSECoPsimpleType<double>;
template class SHALL_EXPORT CSECoPsimpleType<long long>;

/**
 * \brief base for minimum and maximum of a thing
 */
template <typename T> class SHALL_EXPORT CSECoPminmaxType : virtual public CSECoPbaseType
{
protected:
    CSECoPminmaxType();
public:
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool isValid(T value) const;
    virtual bool clear();
    virtual void getMinMaxValue(T &minimum, T &maximum) const;
    virtual bool setMinMaxValue(T minimum, T maximum);
protected:
    CSECoPminmaxType(const CSECoPminmaxType<T>* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    T m_minimum;
    T m_maximum;
};
template class SHALL_EXPORT CSECoPminmaxType<double>;
template class SHALL_EXPORT CSECoPminmaxType<long long>;

/**
 * \brief base for arrays
 */
class SHALL_EXPORT CSECoParrayBase : virtual public CSECoPbaseType
{
protected:
    explicit CSECoParrayBase(SECoP_V_type iType);
public:
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual void getMinMaxSize(unsigned int &uMinimum, unsigned int &uMaximum) const;
    bool setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum);
    virtual unsigned int getSize() const;
    virtual bool setSize(unsigned int uSize);
protected:
    CSECoParrayBase(const CSECoParrayBase* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    virtual bool setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum, bool bForceRealloc);
    unsigned int m_uSize;
    unsigned int m_uMinSize;
    unsigned int m_uMaxSize;
};

/**
 * \brief base of arrays of simple things
 */
template <typename T> class SHALL_EXPORT CSECoParraySimple : virtual public CSECoParrayBase
{
protected:
    explicit CSECoParraySimple();
public:
    virtual ~CSECoParraySimple();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual bool setSize(unsigned int uSize);
    virtual bool getValue(unsigned int uIndex, T &value) const;
    virtual const T* getArray() const;
    virtual bool setValue(unsigned int uIndex, const T value);
    virtual bool setArray(const T* pData, unsigned int uItems);
    virtual bool appendValue(const T value);
protected:
    CSECoParraySimple(const CSECoParraySimple<T>* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    virtual bool setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum, bool bForceRealloc);
    bool m_bAdditional;
    T* m_pData;
};

/**
 * \brief base class of enumerations (simple or array)
 */
class SHALL_EXPORT CSECoPenumBase : virtual public CSECoPbaseType
{
    friend class CSECoParrayEnum;
protected:
    explicit CSECoPenumBase();
public:
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool isValid(const long long llValue) const;
    virtual bool clear();
    virtual unsigned int getItemCount() const;
    virtual const char* getItemName(unsigned int uIndex) const;
    virtual long long getItemValue(unsigned int uIndex) const;
    virtual bool addItem(long long llValue, const char* szName);
protected:
    struct internal
    {
        internal();
        internal(const internal &src);
        internal& operator=(const internal &src);
        long long  llValue;
        QByteArray szName;
    };
    CSECoPenumBase(const CSECoPenumBase* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    QList<internal> m_aItems;
};

/**
 * \brief base class of scaled data (simple or array)
 */
class SHALL_EXPORT CSECoPscaledBase : virtual public CSECoPbaseType
{
    friend class CSECoParrayScaled;
protected:
    explicit CSECoPscaledBase();
public:
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual double getScale() const;
    virtual bool setScale(double dFactor);
protected:
    CSECoPscaledBase(const CSECoPscaledBase* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    double m_dScale;
};

/**
 * \brief a simple boolean
 */
class SHALL_EXPORT CSECoPsimpleBool : virtual public CSECoPsimpleType<long long>
{
    friend class CSECoPbaseType;
public:
    explicit CSECoPsimpleBool();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual bool getBoolValue(bool &bValue) const;
    virtual bool setValue(const bool bValue);
    virtual bool setValue(const long long llValue);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPsimpleBool(const CSECoPsimpleBool* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
};

/**
 * \brief an array of booleans
 */
class SHALL_EXPORT CSECoParrayBool : virtual public CSECoParraySimple<long long>
{
    friend class CSECoPbaseType;
public:
    explicit CSECoParrayBool();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool getBoolValue(unsigned int uIndex, bool &bValue) const;
    virtual bool setValue(unsigned int uIndex, const bool bValue);
    virtual bool setValue(unsigned int uIndex, const long long llValue);
    virtual bool setArray(const bool* pData, unsigned int uItems);
    virtual bool setArray(const long long* pData, unsigned int uItems);
    virtual bool appendValue(const bool bValue);
    virtual bool appendValue(const long long llValue);
    virtual bool appendValue(const CSECoPsimpleBool &value);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoParrayBool(const CSECoParrayBool* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
};

/**
 * \brief a simple floating point (double)
 */
class SHALL_EXPORT CSECoPsimpleDouble : virtual public CSECoPsimpleType<double>, virtual public CSECoPminmaxType<double>
{
    friend class CSECoPbaseType;
    friend class CSECoParrayDouble;
public:
    explicit CSECoPsimpleDouble();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual bool setValue(const double dValue);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPsimpleDouble(const CSECoPsimpleDouble* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
};

/**
 * \brief an array of floating points (double)
 */
class SHALL_EXPORT CSECoParrayDouble : virtual public CSECoParraySimple<double>, virtual public CSECoPminmaxType<double>
{
    friend class CSECoPbaseType;
public:
    explicit CSECoParrayDouble();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual bool setValue(unsigned int uIndex, const double dValue);
    virtual bool setArray(const double* pData, unsigned int uItems);
    virtual bool appendValue(const double dValue);
    virtual bool appendValue(const CSECoPsimpleDouble &value);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoParrayDouble(const CSECoParrayDouble* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
};

/**
 * \brief a simple integer (int64)
 */
class SHALL_EXPORT CSECoPsimpleInt : virtual public CSECoPsimpleType<long long>, virtual public CSECoPminmaxType<long long>
{
    friend class CSECoPbaseType;
    friend class CSECoParrayInt;
public:
    explicit CSECoPsimpleInt();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual bool setValue(const long long llValue);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPsimpleInt(const CSECoPsimpleInt* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
};

/**
 * \brief an array of integers (int64)
 */
class SHALL_EXPORT CSECoParrayInt : virtual public CSECoParraySimple<long long>, virtual public CSECoPminmaxType<long long>
{
    friend class CSECoPbaseType;
public:
    explicit CSECoParrayInt();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual bool setValue(unsigned int uIndex, const long long llValue);
    virtual bool setArray(const long long* pData, unsigned int uItems);
    virtual bool appendValue(const long long llValue);
    virtual bool appendValue(const CSECoPsimpleInt &value);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoParrayInt(const CSECoParrayInt* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
};

/**
 * \brief a simple scaled integer (integer/int64)
 */
class SHALL_EXPORT CSECoPsimpleScaled : virtual public CSECoPscaledBase, virtual public CSECoPsimpleInt
{
    friend class CSECoPbaseType;
public:
    explicit CSECoPsimpleScaled();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual void getMinMaxValue(double &dMinimum, double &dMaximum) const;
    virtual bool setMinMaxValue(double dMinimum, double dMaximum);
    virtual bool getValue(double &dValue) const;
    virtual bool setValue(const double dValue);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPsimpleScaled(const CSECoPsimpleScaled* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    virtual void getMinMaxValue(long long &dMinimum, long long &dMaximum) const;
    virtual bool setMinMaxValue(long long dMinimum, long long dMaximum);
    virtual bool getValue(long long &llValue) const;
    virtual bool setValue(const long long llValue);
};

/**
 * \brief an array of scaled integers (integer/int64, all of same type)
 */
class SHALL_EXPORT CSECoParrayScaled : virtual public CSECoPscaledBase, virtual public CSECoParrayInt
{
    friend class CSECoPbaseType;
public:
    explicit CSECoParrayScaled();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual void getMinMaxValue(double &dMinimum, double &dMaximum) const;
    virtual bool setMinMaxValue(double dMinimum, double dMaximum);
    virtual bool getValue(unsigned int uIndex, double &dValue) const;
    virtual bool setValue(unsigned int uIndex, const double dValue);
    virtual bool setArray(const double* pData, unsigned int uItems);
    virtual bool appendValue(const double dValue);
    virtual bool appendValue(const CSECoPsimpleScaled &value);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoParrayScaled(const CSECoParrayScaled* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    virtual void getMinMaxValue(long long &llMinimum, long long &llMaximum) const;
    virtual bool setMinMaxValue(long long llMinimum, long long llMaximum);
    virtual bool getValue(unsigned int uIndex, long long &llValue) const;
    virtual const long long* getArray() const;
    virtual bool setValue(unsigned int uIndex, const long long llValue);
    virtual bool setArray(const long long* pData, unsigned int uItems);
    virtual bool appendValue(const long long llValue);
    virtual bool appendValue(const CSECoPsimpleInt &value);
};

/**
 * \brief a simple enumeration (integer/int64)
 */
class SHALL_EXPORT CSECoPsimpleEnum : virtual public CSECoPenumBase, virtual public CSECoPsimpleInt
{
    friend class CSECoPbaseType;
public:
    explicit CSECoPsimpleEnum();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual bool setValue(const long long llValue);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
    virtual bool addItem(long long llValue, const char* szName);
protected:
    CSECoPsimpleEnum(const CSECoPsimpleEnum* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    virtual void getMinMaxValue(long long &llMinimum, long long &llMaximum) const;
    virtual bool setMinMaxValue(long long llMinimum, long long llMaximum);
};

/**
 * \brief an array of enumerations (integer/int64, all of same type)
 */
class SHALL_EXPORT CSECoParrayEnum : virtual public CSECoPenumBase, virtual public CSECoParrayInt
{
    friend class CSECoPbaseType;
public:
    explicit CSECoParrayEnum();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual bool setValue(unsigned int uIndex, const long long llValue);
    virtual bool setArray(const long long* pData, unsigned int uItems);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual bool appendValue(const long long llValue);
    virtual bool appendValue(const CSECoPsimpleEnum &value);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoParrayEnum(const CSECoParrayEnum* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    virtual void getMinMaxValue(long long &llMinimum, long long &llMaximum) const;
    virtual bool setMinMaxValue(long long llMinimum, long long llMaximum);
    virtual bool appendValue(const CSECoPsimpleInt &value);
};

/**
 * \brief a string or a blob
 */
class SHALL_EXPORT CSECoPstring : virtual public CSECoParraySimple<unsigned char>
{
    friend class CSECoPbaseType;
public:
    explicit CSECoPstring(SECoP_V_type iType = SECoP_VT_STRING);
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual QByteArray getValue() const;
    virtual bool setValue(QByteArray abyValue);
    virtual void getMinMaxSize(unsigned int &uMinimum, unsigned int &uMaximum) const;
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPstring(const CSECoPstring* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    virtual bool setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum, bool bForceRealloc);
private:
    virtual bool getValue(unsigned int uIndex, unsigned char &value) const;
    virtual bool setValue(unsigned int uIndex, const unsigned char cValue);
    virtual bool appendValue(const unsigned char value);
    bool         m_bIsUTF8;
    unsigned int m_uMaxChars;
};

/**
 * \brief a struct with named items
 */
class SHALL_EXPORT CSECoPstruct : virtual public CSECoPbaseType
{
    friend class CSECoPbaseType;
public:
    explicit CSECoPstruct();
    virtual ~CSECoPstruct();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual unsigned int getItemCount() const;
    virtual bool findItem(const char* szName, unsigned int &uIndex) const;
    virtual QByteArray getItemName(unsigned int uIndex) const;
    virtual CSECoPbaseType* getItem(const char* szName) const;
    virtual CSECoPbaseType* getItem(unsigned int uIndex) const;
    virtual bool removeItem(const char* szName);
    virtual bool removeItem(unsigned int uIndex);
    virtual bool appendValue(const char* szName, CSECoPbaseType* pValue);
    virtual bool appendValue(const char* szName, const CSECoPbaseType* pValue);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPstruct(const CSECoPstruct* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
private:
    QList<QByteArray>      m_asNames;
    QList<CSECoPbaseType*> m_apItems;
};

/**
 * \brief a tuple of fixed indexed items (not an array, some or all may have different types)
 */
class SHALL_EXPORT CSECoPtuple : virtual public CSECoPbaseType
{
    friend class CSECoPbaseType;
public:
    explicit CSECoPtuple();
    virtual ~CSECoPtuple();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual unsigned int getSize() const;
    virtual bool setSize(unsigned int uNewSize);
    virtual CSECoPbaseType* getValue(unsigned int uIndex) const;
    virtual bool setValue(unsigned int uIndex, CSECoPbaseType* pValue);
    virtual bool setValue(unsigned int uIndex, const CSECoPbaseType* pValue);
    virtual bool appendValue(CSECoPbaseType* pValue);
    virtual bool appendValue(const CSECoPbaseType* pValue);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPtuple(const CSECoPtuple* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
private:
    QList<CSECoPbaseType*> m_apItems;
};

/**
 * \brief an array of SECoP data (all of same type)
 */
class SHALL_EXPORT CSECoParray : virtual public CSECoParrayBase
{
    friend class CSECoPbaseType;
public:
    explicit CSECoParray();
    virtual ~CSECoParray();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual CSECoPbaseType* getArrayType() const;
    virtual bool setArrayType(CSECoPbaseType* pType);
    virtual bool setArrayType(const CSECoPbaseType* pType);
    virtual unsigned int getSize() const;
    virtual bool setSize(unsigned int uSize);
    virtual bool setValue(unsigned int uIndex, CSECoPbaseType* pValue);
    virtual bool setValue(unsigned int uIndex, const CSECoPbaseType* pValue);
    virtual CSECoPbaseType* getValue(unsigned int uIndex) const;
    virtual bool appendValue(CSECoPbaseType* pValue);
    virtual bool appendValue(const CSECoPbaseType* pValue);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoParray(const CSECoParray* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
    virtual bool setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum, bool bForceRealloc);
private:
    CSECoPbaseType*        m_pType;
    QList<CSECoPbaseType*> m_apItems;
};

/**
 * \brief a SECoP command with argument and result
 */
class SHALL_EXPORT CSECoPcommand : virtual public CSECoPbaseType
{
    friend class CSECoPbaseType;
public:
    explicit CSECoPcommand();
    virtual ~CSECoPcommand();
    virtual bool compareType(const CSECoPbaseType* pOther) const;
    virtual SECoP_V_compareResult compareValue(const CSECoPbaseType* pOther) const;
    virtual bool isValid() const;
    virtual bool clear();
    virtual CSECoPbaseType* getArgument() const;
    virtual CSECoPbaseType* getResult() const;
    virtual bool setArgument(const CSECoPbaseType* pArgument);
    virtual bool setResult(const CSECoPbaseType* pResult);
    virtual bool importSECoP(const nlohmann::json &data, bool bStrict);
    virtual nlohmann::json exportSECoPjson() const;
protected:
    CSECoPcommand(const CSECoPcommand* pOther); // special copy constructor for "duplicate"
    virtual bool createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys);
    virtual bool exportTypeHelper(nlohmann::json &json, bool bArray) const;
private:
    CSECoPbaseType* m_pArgument;
    CSECoPbaseType* m_pResult;
};

#endif
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /*__SECOP_VARIANT_H__F003E0AD_31E3_4575_971F_F25EAD29F788__*/
