/*
SPDX-License-Identifier: LGPL-3.0-or-later
Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>
*/
#include <QHash>
#include <QMutexLocker>
#include <QRegExp>
#include <QTextStream>
#include <QVariant>
#include <cstdlib>
#include <limits.h>
#include <math.h>
#include <memory>
#include <string.h>
#include "SECoP-Variant.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define qUtf8Printable(string) QString(string).toUtf8().constData()
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

/**
 * \brief hash of all local allocated memory
 * \ingroup intfunc
 */
static QHash<const CSECoPbaseType*, unsigned int> SECoP_V_g_huItems;

/**
 * \brief mutex for multi-threaded access to \ref SECoP_V_g_huItems
 * \ingroup intfunc
 */
static QMutex* SECoP_V_g_pMutex(nullptr);

/**
 * \brief initialize global mutex
 * \ingroup intfunc
 */
static void SECoP_V_initMutex()
{
    if (SECoP_V_g_pMutex == nullptr)
        SECoP_V_g_pMutex = new QMutex(QMutex::Recursive);
}

/*
 * \brief Function to create an pre-configured CSECoPbaseType structure from the description,
 *        which can be used in the SECoP client and server.
 *        Valid types are "double", "int", "bool", "string", "blob", "scaled", "enum",
 *        "array", "tuple" or "struct" like allowed data types in SECoP.
 * \ingroup expfunc
 * \param[in] szDescription structure of the data as described in SECoP standard
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType* SECoP_V_create(const char* szDescription)
{
    return CSECoPbaseType::createSECoP(szDescription, false);
}

/*
 * \brief Function to create a simple boolean value for convenience in SECoP.
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType* SECoP_V_createSimpleBool()
{
    return new CSECoPsimpleDouble();
}

/*
 * \brief Function to create a simple double value (64bit without limits) for convenience in SECoP.
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType* SECoP_V_createSimpleDouble()
{
    return new CSECoPsimpleBool();
}

/*
 * \brief Function to create a simple integer value (signed 64bit) for convenience in SECoP.
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType* SECoP_V_createSimpleInteger()
{
    return new CSECoPsimpleInt();
}

/*
 * \brief Function to create a simple scaled integer value (signed 64bit) for convenience in SECoP.
 * \ingroup expfunc
 * \param[in] dScale scaling factor (>0.0)
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType* SECoP_V_createSimpleScaled(double dScale)
{
    CSECoPsimpleScaled* pResult(new CSECoPsimpleScaled());
    if (pResult)
    {
        if (pResult->setScale(dScale))
            return pResult;
        delete pResult;
    }
    return nullptr;
}

/*
 * \brief Function to create a simple string value for convenience in SECoP.
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType* SECoP_V_createSimpleString()
{
    return new CSECoPstring(SECoP_VT_STRING);
}

/*
 * \brief Function to create a SECoP status for convenience in SECoP.
 *        This contains an enumeration (with 100=IDLE) and a string.
 *        {"type":"tuple","members":[{"type":"enum","members":{"IDLE":100}},{"type":"string"}]}
 * \ingroup expfunc
 * \return a new CSECoPbaseType structure or NULL in case of errors
 */
CSECoPbaseType* SECoP_V_createSECoPStatus()
{
    return CSECoPbaseType::createSECoP(R"(
        {"type":"tuple","members":[{"type":"enum","members":{"IDLE":100}},{"type":"string"}]}
        )"_json, false);
}

/*
 * \brief Use this function to get the SECoP data type of a CSECoPbaseType variant.
 * \ingroup expfunc
 * \param[in]  pData       input data
 * \param[out] szBuffer    buffer to write the SECoP data type description, if not NULL
 * \param[in]  iBufferSize size of szBuffer in bytes
 * \return negative for errors else the needed buffer size
 */
int SECoP_V_getType(const CSECoPbaseType* pData, char* szBuffer, int iBufferSize)
{
    nlohmann::json o(pData->exportType());
    int iResult(-1);
    if (SECoP_V_g_huItems.contains(pData) && o.is_object() && o.size() == 1)
    {
        std::string aby(o.dump());
        iResult = static_cast<int>(aby.size());
        if (iResult > 0)
        {
            if (iResult >= iBufferSize)
                iResult = iBufferSize - 1;
            if (iResult > 0)
                memmove(szBuffer, aby.c_str(), static_cast<size_t>(iResult));
            szBuffer[iResult] = '\0';
            iResult = static_cast<int>(aby.size() + 1);
        }
        else
            iResult = -1;
    }
    return iResult;
}

/**
 * \brief helper function to find a special data type of SECoP-compatible data.
 * \ingroup intfunc
 * \param[in] v SECoP data
 * \return compatible data type or SECoP_VT_NONE
 */
static SECoP_V_type SECoP_V_fromJSONtype(const nlohmann::json& v)
{
    switch (v.type())
    {
        case nlohmann::detail::value_t::boolean:         return SECoP_VT_BOOL;
        case nlohmann::detail::value_t::number_float:    return SECoP_VT_DOUBLE;
        case nlohmann::detail::value_t::number_integer:
        case nlohmann::detail::value_t::number_unsigned: return SECoP_VT_INTEGER;
        case nlohmann::detail::value_t::string:          return SECoP_VT_STRING;
        case nlohmann::detail::value_t::object:          return SECoP_VT_STRUCT;
        case nlohmann::detail::value_t::array:
        {
            if (v.empty())
                return SECoP_VT_NONE;
            SECoP_V_type iType(SECoP_VT_NONE), iType2(SECoP_VT_NONE);
            for (unsigned int i = 0; i < v.size(); ++i)
            {
                switch (v[i].type())
                {
                    case nlohmann::detail::value_t::boolean:
                        iType2 = SECoP_VT_ARRAY_BOOL;
                        break;
                    case nlohmann::detail::value_t::number_float:
                        iType2 = SECoP_VT_ARRAY_DOUBLE;
                        break;
                    case nlohmann::detail::value_t::number_integer:
                    case nlohmann::detail::value_t::number_unsigned:
                        iType2 = SECoP_VT_ARRAY_INTEGER;
                        break;
                    case nlohmann::detail::value_t::string:
                        iType2 = SECoP_VT_ARRAY;
                        break;
                    default:
                        iType2 = SECoP_VT_TUPLE;
                }
                if (!i)
                {
                    iType = iType2;
                    if (iType == SECoP_VT_TUPLE)
                        return iType;
                }
                if (iType != iType2)
                    return SECoP_VT_TUPLE;
            }
            return iType;
        }
        default:
            break;
    }
    return SECoP_VT_NONE;
}

/*
 * \brief Use this function to convert SECoP-compatible data into a CSECoPbaseType.
 * \ingroup expfunc
 * \param[in] szJSON data to convert
 * \param[in] pHint  existing data to return a compatible type
 * \return a new CSECoPbaseType pointer or NULL in case of errors
 */
CSECoPbaseType* SECoP_V_fromJSON(const char* szJSON, const CSECoPbaseType* pHint)
{
    nlohmann::json v;
    try
    {
        v = nlohmann::json::parse(szJSON);
    }
    catch (nlohmann::detail::exception&)
    {
        return nullptr;
    }
    catch (...)
    {
        return nullptr;
    }

    CSECoPbaseType* pValue(nullptr);
    if (pHint != nullptr)
    {
        if (!SECoP_V_g_huItems.contains(pHint))
            return nullptr;
        pValue = pHint->duplicate();
        if (pValue == nullptr)
            return nullptr;
        if (pValue->importSECoP(v, true))
            return pValue;
        delete pValue;
        return nullptr;
    }
    return CSECoPbaseType::importSECoP(v);
}

/*
 * \brief Use this function to convert a CSECoPbaseType variant into SECoP-compatible json data.
 * \ingroup expfunc
 * \param[in]  pData        input data to convert
 * \param[out] szJSONBuffer buffer to write the json data, if not NULL
 * \param[in]  iBufferSize  size of szJSONBuffer in bytes
 * \return negative for errors else the needed buffer size for the json data
 */
int SECoP_V_toJSON(const CSECoPbaseType* pData, char* szJSONBuffer, int iBufferSize)
{
    if (pData == nullptr || !SECoP_V_g_huItems.contains(pData)) // invalid input data
        return -1;
    QByteArray ba(pData->exportSECoP());
    if (iBufferSize > 0 && szJSONBuffer != nullptr)
    {
        int iMax = ba.size();
        if (iMax >= iBufferSize)
            iMax = iBufferSize - 1;
        if (iMax > 0)
            memmove(szJSONBuffer, ba.constData(), static_cast<size_t>(iMax));
        szJSONBuffer[iMax] = '\0';
    }
    return ba.size() + 1;
}

/*
 * \brief Function to destroy an existing CSECoPbaseType structure with all its children.
 *        The storage pointer value is reset to NULL.
 * \ingroup expfunc
 * \param[in] ppData pointer to data to destroy
 */
void SECoP_V_destroy(CSECoPbaseType** ppData)
{
    if (ppData != nullptr)
    {
        CSECoPbaseType* pData(*ppData);
        if (pData != nullptr)
        {
            SECoP_V_initMutex();
            QMutexLocker locker(SECoP_V_g_pMutex);
            if (SECoP_V_g_huItems.remove(pData) > 0)
                delete pData;
            *ppData = nullptr;
        }
    }
}

/*
 * \brief Function to copy existing CSECoPbaseType into a new
 * \ingroup expfunc
 * \param[out] ppDst destination (pointer will be allocated)
 * \param[in]  pSrc  source data
 * \return true, if successful
 */
bool SECoP_V_copy(CSECoPbaseType** ppDst, const CSECoPbaseType* pSrc)
{
    if (pSrc == nullptr || ppDst == nullptr || !SECoP_V_g_huItems.contains(pSrc))
        return false;
    *ppDst = pSrc->duplicate();
    return ((*ppDst) != nullptr);
}

/*
 * \brief print SECoP data to file using SECoP_V_printStream
 *        This function has may have a minimum and a maximum pointer value. Only data inside this
 *        range is printed and this can be used to look into self-created complex data types.
 * \ingroup expfunc
 * \param[in] pOutfile   output stream to print to
 * \param[in] pData      input data to print
 * \param[in] iVerbosity 0=print data only, 1=print data with type, 2=print with additional info
 */
void SECoP_V_printFile(FILE* pOutfile, const CSECoPbaseType* pData, int iVerbosity)
{
    QTextStream output(pOutfile, QIODevice::WriteOnly);
    output.setAutoDetectUnicode(true);
    output.setGenerateByteOrderMark(false);
    output.setFieldAlignment(QTextStream::AlignLeft);
    output.setPadChar(QChar(' '));
    output.setFieldWidth(0);
    output.setNumberFlags(QTextStream::UppercaseDigits);
    output.setIntegerBase(10);
    output.setRealNumberNotation(QTextStream::SmartNotation);
    output.setRealNumberPrecision(16);
    SECoP_V_printStream(output, pData, iVerbosity);
}

/*
 * \brief print SECoP data to UTF-8 byte array using SECoP_V_printStream
 *        This function has may have a minimum and a maximum pointer value. Only data inside this
 *        range is printed and this can be used to look into self-created complex data types.
 * \ingroup expfunc
 * \param[in] pData      input data to print
 * \param[in] iVerbosity 0=print data only, 1=print data with type, 2=print with additional info
 * \return on success: the converted string, on error: an QByteArray::null object
 */
QByteArray SECoP_V_printString(const CSECoPbaseType* pData, int iVerbosity)
{
    QByteArray abyResult;
    QTextStream output(&abyResult, QIODevice::ReadWrite);
    output.setAutoDetectUnicode(true);
    output.setGenerateByteOrderMark(false);
    output.setFieldAlignment(QTextStream::AlignLeft);
    output.setPadChar(QChar(' '));
    output.setFieldWidth(0);
    output.setNumberFlags(QTextStream::UppercaseDigits);
    output.setIntegerBase(10);
    output.setRealNumberNotation(QTextStream::SmartNotation);
    output.setRealNumberPrecision(16);
    if (!SECoP_V_printStream(output, pData, iVerbosity))
        abyResult.clear();
    return abyResult;
}

/**
 * \brief print SECoP data to stream, this function is used by SECoP_V_printStream to recursive print data
 * \ingroup intfunc
 * \param[in] rOutput    output stream to print to
 * \param[in] pData      input data to print
 * \param[in] iVerbosity 0=print data only, 1=print data with type, 2=print with additional info
 * \param[in] iLevel     recursive level
 * \return true, on success; false, if any error was found
 */
static bool SECoP_V_printHelper(QTextStream &rOutput, const CSECoPbaseType* pData, int iVerbosity, int iLevel)
{
    const CSECoPnull*         pNull(nullptr);
    const CSECoPsimpleBool*   pSBool(nullptr);
    const CSECoParrayBool*    pABool(nullptr);
    const CSECoPsimpleDouble* pSDbl(nullptr);
    const CSECoParrayDouble*  pADbl(nullptr);
    const CSECoPsimpleInt*    pSInt(nullptr);
    const CSECoParrayInt*     pAInt(nullptr);
    const CSECoPsimpleScaled* pSScl(nullptr);
    const CSECoParrayScaled*  pAScl(nullptr);
    const CSECoPsimpleEnum*   pSEnm(nullptr);
    const CSECoParrayEnum*    pAEnm(nullptr);;
    const CSECoPstring*       pString(nullptr);
    const CSECoPstruct*       pStruct(nullptr);
    const CSECoPtuple*        pTuple(nullptr);
    const CSECoParray*        pArray(nullptr);
    const CSECoPcommand*      pCommand(nullptr);

    int iNextLevel(iLevel + 1);
    if (iLevel > 0)
        rOutput << QString().sprintf("%*c", 2 * iLevel, ' ').toUtf8();
    if (pData == nullptr)
    {
        rOutput << "?NULL?\n";
        return true;
    }
    if (!SECoP_V_g_huItems.contains(pData))
    {
        rOutput << "?INVALID?\n";
        return true;
    }

    do
    {
        pNull = dynamic_cast<const CSECoPnull*>(pData);
        if (pNull != nullptr)
            break;
        pSScl = dynamic_cast<const CSECoPsimpleScaled*>(pData);
        if (pSScl != nullptr)
            break;
        pAScl = dynamic_cast<const CSECoParrayScaled*>(pData);
        if (pAScl != nullptr)
            break;
        pSEnm = dynamic_cast<const CSECoPsimpleEnum*>(pData);
        if (pSEnm != nullptr)
            break;
        pAEnm = dynamic_cast<const CSECoParrayEnum*>(pData);
        if (pAEnm != nullptr)
            break;
        pSBool = dynamic_cast<const CSECoPsimpleBool*>(pData);
        if (pSBool != nullptr)
            break;
        pABool = dynamic_cast<const CSECoParrayBool*>(pData);
        if (pABool != nullptr)
            break;
        pSDbl = dynamic_cast<const CSECoPsimpleDouble*>(pData);
        if (pSDbl != nullptr)
            break;
        pADbl = dynamic_cast<const CSECoParrayDouble*>(pData);
        if (pADbl != nullptr)
            break;
        pSInt = dynamic_cast<const CSECoPsimpleInt*>(pData);
        if (pSInt != nullptr)
            break;
        pAInt = dynamic_cast<const CSECoParrayInt*>(pData);
        if (pAInt != nullptr)
            break;
        pString = dynamic_cast<const CSECoPstring*>(pData);
        if (pString != nullptr)
            break;
        pStruct = dynamic_cast<const CSECoPstruct*>(pData);
        if (pStruct != nullptr)
            break;
        pTuple = dynamic_cast<const CSECoPtuple*>(pData);
        if (pTuple != nullptr)
            break;
        pArray = dynamic_cast<const CSECoParray*>(pData);
        if (pArray != nullptr)
            break;
        pCommand = dynamic_cast<const CSECoPcommand*>(pData);
        if (pCommand != nullptr)
            break;
        rOutput << QString("?%1?\n").arg(static_cast<int>(pData->getType())).toUtf8();
        return true;
    } while (0);
    if (iVerbosity > 0)
    {
        if (pNull != nullptr)
            rOutput << "NULL";
        else if (pSBool != nullptr || pABool != nullptr)
            rOutput << "bool";
        else if (pSDbl != nullptr || pADbl != nullptr)
            rOutput << "double";
        else if (pSInt != nullptr || pAInt != nullptr)
            rOutput << "int";
        else if (pSScl != nullptr || pAScl != nullptr)
            rOutput << "scaled";
        else if (pSEnm != nullptr || pAEnm != nullptr)
        {
            const CSECoPenumBase* pEnum(pSEnm);
            if (pEnum == nullptr)
                pEnum = pAEnm;
            rOutput << "enum[";
            for (unsigned int i = 0; i < pEnum->getItemCount(); ++i)
            {
                if (i)
                    rOutput << ",";
                rOutput << pEnum->getItemValue(i) << ":" << pEnum->getItemName(i);
            }
            rOutput << "]";
        }
        else if (pString != nullptr)
            rOutput << "string";
        else if (pStruct != nullptr)
            rOutput << QString("struct[%1]").arg(pStruct->getItemCount()).toUtf8();
        else if (pTuple != nullptr)
            rOutput << QString("tuple[%1]").arg(pTuple->getSize()).toUtf8();
        else if (pArray != nullptr)
        {
            CSECoPbaseType* pType(pArray->getArrayType());
            rOutput << QString("array[%1]").arg(pArray->getSize()).toUtf8();
            if (iVerbosity > 0 && pArray->getSize() < 1 && pType != nullptr)
            {
                rOutput << " of";
                SECoP_V_printHelper(rOutput, pType, iVerbosity, iNextLevel);
                if (iLevel > 0)
                    rOutput << QString().sprintf("%*c", 2 * iLevel, ' ').toUtf8();
            }
        }
        else if (pCommand != nullptr)
            rOutput << "command";
        if (pABool != nullptr || pADbl != nullptr || pAInt != nullptr || pAScl != nullptr || pAEnm != nullptr)
            rOutput << QString("[%1]").arg(dynamic_cast<const CSECoParrayBase*>(pData)->getSize()).toUtf8();
    }
    if (iVerbosity > 1)
    {
        nlohmann::json a(pData->additional());
        if (!a.empty())
        {
            rOutput << "\n" << QString().sprintf("%*c  ", 2 * iLevel, ' ').toUtf8() << QString::fromStdString(a.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace)) << "\n";
            ++iNextLevel;
            if (iLevel > 0)
                rOutput << QString().sprintf("%*c", 2 * iLevel, ' ').toUtf8();
        }
    }
    if (iVerbosity > 0)
        rOutput << "=";
    if (pStruct != nullptr)
    {
        rOutput << "\n";
        for (unsigned int i = 0; i < pStruct->getItemCount(); ++i)
        {
            CSECoPbaseType* pItem(pStruct->getItem(i));
            if (iLevel > 0)
                rOutput << QString().sprintf("%*c", 2 * iLevel, ' ').toUtf8();
            rOutput << QString("%1: ").arg(pStruct->getItemName(i).constData()).toUtf8();
            if (!SECoP_V_printHelper(rOutput, pItem, iVerbosity, iNextLevel))
                return false;
        }
    }
    else if (pTuple != nullptr)
    {
        rOutput << "\n";
        for (unsigned int i = 0; i < pTuple->getSize(); ++i)
            if (!SECoP_V_printHelper(rOutput, pTuple->getValue(i), iVerbosity, iNextLevel))
                return false;
    }
    else if (pArray != nullptr)
    {
        rOutput << "\n";
        for (unsigned int i = 0; i < pArray->getSize(); ++i)
            if (!SECoP_V_printHelper(rOutput, pArray->getValue(i), iVerbosity, iNextLevel))
                return false;
    }
    else if (pCommand != nullptr)
    {
        rOutput << "\n";
        if (iLevel > 0)
            rOutput << QString().sprintf("%*c", 2 * iLevel, ' ').toUtf8();
        rOutput << "arg: ";
        const CSECoPbaseType* pArgument(pCommand->getArgument());
        if (pArgument != nullptr)
        {
            if (!SECoP_V_printHelper(rOutput, pArgument, iVerbosity, iNextLevel))
                return false;
        }
        else
            rOutput << "null\n";
        if (iLevel > 0)
            rOutput << QString().sprintf("%*c", 2 * iLevel, ' ').toUtf8();
        rOutput << "res:" ;
        const CSECoPbaseType* pResult(pCommand->getResult());
        if (pResult != nullptr)
        {
            if (!SECoP_V_printHelper(rOutput, pResult, iVerbosity, iNextLevel))
                return false;
        }
        else
            rOutput << "null\n";
    }
    else
        rOutput << pData->exportSECoP() << "\n";
    return true;
}

/*
 * \brief print SECoP data to QTextStream
 *        This function has may have a minimum and a maximum pointer value. Only data inside this
 *        range is printed and this can be used to look into self-created complex data types.
 * \ingroup expfunc
 * \param[in] rOutput    output stream to print to
 * \param[in] pData      input data to print
 * \param[in] iVerbosity 0=print data only, 1=print data with type, 2=print with additional info
 * \return true, on success; false, if any error was found
 */
bool SECoP_V_printStream(QTextStream &rOutput, const CSECoPbaseType* pData, int iVerbosity)
{
    bool bResult(SECoP_V_printHelper(rOutput, pData, iVerbosity, 0));
    rOutput.flush();
    return bResult;
}

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
 * \ingroup intfunc
 * \param[in]     pData       CSECoPbaseType with read access
 * \param[in,out] puPosition  zero-based position to data part or nullptr
 * \param[out]    puCount     count of sub data parts or nullptr
 * \param[out]    ppItem      pointer to selected data
 * \param[out]    pszItemName pointer to name of selected data or nullptr
 * \return true, if information was accessible; false, if data or uPosition was invalid (end of data reached)
 */
static bool SECoP_V_getInfoHelper(const CSECoPbaseType* pData, unsigned int* puPosition, unsigned int* puCount,
                                  const CSECoPbaseType** ppItem, const char** pszItemName)
{
    const CSECoPstruct*  pStruct (dynamic_cast<const CSECoPstruct*> (pData));
    const CSECoPtuple*   pTuple  (dynamic_cast<const CSECoPtuple*>  (pData));
    const CSECoParray*   pArray  (dynamic_cast<const CSECoParray*>  (pData));
    const CSECoPcommand* pCommand(dynamic_cast<const CSECoPcommand*>(pData));
    if (pData == nullptr || (puPosition == nullptr && puCount == nullptr) || !SECoP_V_g_huItems.contains(pData))
        return false;
    unsigned int uPos(0);
    if (puPosition != nullptr)
        uPos = (*puPosition);
    if (pStruct != nullptr)
    {
        if (uPos == 0)
        {
            if (ppItem != nullptr)
                *ppItem = pData;
            if (puCount != nullptr)
                *puCount = pStruct->getItemCount();
            if (pszItemName != nullptr)
                *pszItemName = nullptr;
            return true;
        }
        for (unsigned int i = 0; i < pStruct->getItemCount(); ++i)
        {
            --uPos;
            if (puPosition != nullptr)
                --(*puPosition);
            if (pszItemName != nullptr && uPos == 0)
                *pszItemName = pStruct->getItemName(i).constData();
            if (puCount != nullptr)
                ++(*puCount);
            if (SECoP_V_getInfoHelper(pStruct->getItem(i), puPosition, puCount, ppItem, pszItemName))
                return true;
            if (uPos == 0)
                break;
        }
        return false;
    }
    else if (pTuple != nullptr)
    {
        if (uPos == 0)
        {
            if (ppItem != nullptr)
                *ppItem = pData;
            if (puCount != nullptr)
                *puCount = pTuple->getSize();
            if (pszItemName != nullptr)
                *pszItemName = nullptr;
            return true;
        }
        for (unsigned int i = 0; i < pTuple->getSize(); ++i)
        {
            --uPos;
            if (puPosition != nullptr)
                --(*puPosition);
            if (puCount != nullptr)
                ++(*puCount);
            if (SECoP_V_getInfoHelper(pTuple->getValue(i), puPosition, puCount, ppItem, pszItemName))
                return true;
            if (uPos == 0)
                break;
        }
        return false;
    }
    else if (pArray != nullptr)
    {
        if (uPos == 0)
        {
            if (ppItem != nullptr)
                *ppItem = pData;
            if (puCount != nullptr)
                *puCount = pArray->getSize();
            if (pszItemName != nullptr)
                *pszItemName = nullptr;
            return true;
        }
        for (unsigned int i = 0; i < pArray->getSize(); ++i)
        {
            --uPos;
            if (puPosition != nullptr)
                --(*puPosition);
            if (puCount != nullptr)
                ++(*puCount);
            if (SECoP_V_getInfoHelper(pArray->getValue(i), puPosition, puCount, ppItem, pszItemName))
                return true;
            if (uPos == 0)
                break;
        }
        return false;
    }
    else if (pCommand != nullptr)
    {
        if (uPos == 0)
        {
            if (ppItem != nullptr)
                *ppItem = pData;
            if (puCount != nullptr)
                *puCount = 2;
            if (pszItemName != nullptr)
                *pszItemName = nullptr;
            return true;
        }
        --uPos;
        if (puPosition != nullptr)
            --(*puPosition);
        if (SECoP_V_getInfoHelper(pCommand->getArgument(), puPosition, puCount, ppItem, pszItemName))
            return true;
        if (uPos == 0)
            return false;
        --uPos;
        if (puPosition != nullptr)
            --(*puPosition);
        if (SECoP_V_getInfoHelper(pCommand->getResult(), puPosition, puCount, ppItem, pszItemName))
            return true;
        if (uPos == 0)
            return false;
    }
    else if (uPos > 0)
        return false;
    else if (puCount != nullptr)
    {
        const CSECoParrayBase* pArrayBase(dynamic_cast<const CSECoParrayBase*>(pData));
        *puCount = (pArrayBase != nullptr) ? pArrayBase->getSize() : 1;
    }
    if (ppItem != nullptr)
        *ppItem = pData;
    return true;
}

/*
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
bool SECoP_V_getInfo(const CSECoPbaseType* pData, unsigned int uPosition, enum SECoP_V_type* piType,
                     unsigned int* puCount, const char** pszName)
{
    const CSECoPbaseType* pItem(nullptr);
    SECoP_V_type iType(SECoP_VT_NONE);
    if (piType != nullptr)
        *piType = SECoP_VT_NONE;
    if (puCount != nullptr)
        *puCount = 0;
    if (pszName != nullptr)
        *pszName = nullptr;
    if (!SECoP_V_getInfoHelper(pData, &uPosition, puCount, &pItem, pszName) || uPosition > 0)
        return false;
    if (pItem == nullptr)
        return false;
    iType = pItem->getType();
    if (piType != nullptr)
        *piType = iType;
    return true;
}

/*
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
 * \param[in,out] puLen     size of "name" buffer (in), name length (out)
 * \return true, if information was accessible; false, if data or uPosition was invalid (end of data reached)
 */
bool SECoP_V_getInfo2(const CSECoPbaseType* pData, unsigned int uPosition, enum SECoP_V_type* piType,
                      unsigned int* puCount, char* szName, unsigned int* puLen)
{
    const char* szTmp(nullptr);
    if (!SECoP_V_getInfo(pData, uPosition, piType, puCount, &szTmp))
        return false;
    size_t iLen(0), iLenResult;
    if (szTmp != nullptr)
        iLen = strlen(szTmp);
    if (puLen != nullptr)
    {
        iLenResult = iLen;
        if (szName != nullptr && *puLen > 0)
        {
            if (*puLen <= iLen)
                iLen = *puLen - 1;
            memmove(szName, szTmp, iLen);
            szName[iLen] = '\0';
        }
        *puLen = static_cast<unsigned int>(iLenResult);
    }
    return true;
}

/*
 * \brief Use this function to modify a specific double (numeric) value of CSECoPbaseType.
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
bool SECoP_V_modifyDouble(CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex, double dblValue)
{
    const CSECoPbaseType* pItemTmp(nullptr);
    if (!SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItemTmp, nullptr) || pItemTmp == nullptr)
        return false;
    CSECoPbaseType* pItem(const_cast<CSECoPbaseType*>(pItemTmp));
    if (pItem == nullptr || uPosition > 0)
        return false;
    CSECoPsimpleScaled* pSScaled(dynamic_cast<CSECoPsimpleScaled*>(pItem));
    if (pSScaled != nullptr)
    {
        if (uIndex > 0)
            return false;
        return pSScaled->setValue(dblValue);
    }
    CSECoPsimpleType<double>* pSDouble(dynamic_cast<CSECoPsimpleType<double>*>(pItem));
    if (pSDouble != nullptr)
    {
        if (uIndex > 0)
            return false;
        return pSDouble->setValue(dblValue);
    }
    CSECoParrayScaled* pAScaled(dynamic_cast<CSECoParrayScaled*>(pItem));
    if (pAScaled != nullptr)
        return pAScaled->setValue(uIndex, dblValue);
    CSECoParraySimple<double>* pADouble(dynamic_cast<CSECoParraySimple<double>*>(pItem));
    if (pADouble != nullptr)
        return pADouble->setValue(uIndex, dblValue);
    return false;
}

/*
 * \brief Use this function to modify a specific integer (numeric) value of CSECoPbaseType.
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
bool SECoP_V_modifyInteger(CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex, long long llValue)
{
    const CSECoPbaseType* pItemTmp(nullptr);
    if (!SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItemTmp, nullptr) || pItemTmp == nullptr)
        return false;
    CSECoPbaseType* pItem(const_cast<CSECoPbaseType*>(pItemTmp));
    if (pItem == nullptr || uPosition > 0)
        return false;
    CSECoPsimpleType<long long>* pSInt(dynamic_cast<CSECoPsimpleType<long long>*>(pItem));
    if (pSInt != nullptr)
    {
        if (uIndex > 0)
            return false;
        return pSInt->setValue(llValue);
    }
    CSECoParraySimple<long long>* pAInt(dynamic_cast<CSECoParraySimple<long long>*>(pItem));
    if (pAInt != nullptr)
        return pAInt->setValue(uIndex, llValue);
    return false;
}

/*
 * \brief Convenience function to read a single double value out of the SECoP variant.
 * \ingroup expfunc
 * \param[in]  pData     CSECoPbaseType to read
 * \param[in]  uPosition zero-based position to data part (complete array)
 * \param[in]  uIndex    zero-based index into double array
 * \param[out] pdblValue memory location which will get the double value
 * \return true, if successful
 */
bool SECoP_V_getDouble(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex, double* pdblValue)
{
    const CSECoPbaseType* pItemTmp(nullptr);
    if (!SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItemTmp, nullptr) || pItemTmp == nullptr)
        return false;
    CSECoPbaseType* pItem(const_cast<CSECoPbaseType*>(pItemTmp));
    if (pItem == nullptr || uPosition > 0)
        return false;
    CSECoPsimpleScaled* pSScaled(dynamic_cast<CSECoPsimpleScaled*>(pItem));
    if (pSScaled != nullptr)
    {
        if (uIndex > 0)
            return false;
        return pSScaled->getValue(*pdblValue);
    }
    CSECoParrayScaled* pAScaled(dynamic_cast<CSECoParrayScaled*>(pItem));
    if (pAScaled != nullptr)
        return pAScaled->getValue(uIndex, *pdblValue);
    CSECoPsimpleType<double>* pSDouble(dynamic_cast<CSECoPsimpleType<double>*>(pItem));
    if (pSDouble != nullptr)
    {
        if (uIndex > 0)
            return false;
        return pSDouble->getValue(*pdblValue);
    }
    CSECoParraySimple<double>* pADouble(dynamic_cast<CSECoParraySimple<double>*>(pItem));
    if (pADouble != nullptr)
        return pADouble->getValue(uIndex, *pdblValue);
    return false;
}

/*
 * \brief Convenience function to read a single integer/boolean/enum/scaled value out of the SECoP variant.
 * \ingroup expfunc
 * \param[in]  pData     CSECoPbaseType to read
 * \param[in]  uPosition zero-based position to data part (complete array)
 * \param[in]  uIndex    zero-based index into integer array
 * \param[out] pllValue  memory location which will get the integer value
 * \return true, if successful
 */
bool SECoP_V_getInteger(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex, long long* pllValue)
{
    const CSECoPbaseType* pItemTmp(nullptr);
    if (!SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItemTmp, nullptr) || pItemTmp == nullptr)
        return false;
    CSECoPbaseType* pItem(const_cast<CSECoPbaseType*>(pItemTmp));
    if (pItem == nullptr || uPosition > 0)
        return false;
    CSECoPsimpleType<long long>* pSInt(dynamic_cast<CSECoPsimpleType<long long>*>(pItem));
    if (pSInt != nullptr)
    {
        if (uIndex > 0)
            return false;
        return pSInt->getValue(*pllValue);
    }
    CSECoParraySimple<long long>* pAInt(dynamic_cast<CSECoParraySimple<long long>*>(pItem));
    if (pAInt != nullptr)
        return pAInt->getValue(uIndex, *pllValue);
    return false;
}

/*
 * \brief Convenience function to read a string/blob value out of the SECoP variant.
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[in]  uBufferSize size of memory location (function will not copy more than this number of bytes)
 * \param[out] pszValue    memory location which will get the string value
 * \return zero for errors, number of bytes needed (including a trailing NUL)
 */
unsigned int SECoP_V_getString(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uBufferSize, char* pszValue)
{
    const CSECoPbaseType* pItem(nullptr);
    if (uBufferSize > 0 && pszValue == nullptr)
        return 0;
    if (!SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
        return 0;
    const CSECoPstring* pString(dynamic_cast<const CSECoPstring*>(pItem));
    if (pString == nullptr || uPosition > 0)
        return 0;
    unsigned int iLen(pString->getSize());
    if (uBufferSize >= (iLen + 1))
    {
        if (iLen > 0)
            memmove(pszValue, pString->getArray(), iLen);
        pszValue[iLen] = '\0';
    }
    else if (uBufferSize > 0)
    {
        --uBufferSize;
        if (uBufferSize > 0)
            memmove(pszValue, pString->getArray(), uBufferSize);
        pszValue[uBufferSize] = '\0';
    }
    return iLen + 1;
}

/*
 * \brief return count of enumeration items in list or structure names
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \return return count of enumeration items in list, structure names or 0 on error
 */
unsigned int SECoP_V_getEnumStructCount(const CSECoPbaseType* pData, unsigned int uPosition)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        if (pItem != nullptr && uPosition < 1)
        {
            const CSECoPstruct* pStruct(dynamic_cast<const CSECoPstruct*>(pItem));
            if (pStruct != nullptr)
                return pStruct->getItemCount();
            const CSECoPenumBase* pEnum(dynamic_cast<const CSECoPenumBase*>(pItem));
            if (pEnum != nullptr)
                return pEnum->getItemCount();
        }
    }
    return 0U;
}

/*
 * \brief return enumeration item from list with zero based index
 * \ingroup expfunc
 * \param[in]  pData      CSECoPbaseType to read
 * \param[in]  uPosition  zero-based position to data part
 * \param[in]  uIndex     zero based index to enum item list
 * \param[out] pllEnumVal enumeration value for valid index or NULL
 * \return for valid index: enumeration name, for invalid index: NULL
 */
const char* SECoP_V_getEnumeration(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex, long long* pllEnumVal)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        if (pItem != nullptr && uPosition < 1)
        {
            const CSECoPenumBase* pEnum(dynamic_cast<const CSECoPenumBase*>(pItem));
            if (pEnum != nullptr && uIndex < pEnum->getItemCount())
            {
                if (pllEnumVal != nullptr)
                    *pllEnumVal = pEnum->getItemValue(uIndex);
                return pEnum->getItemName(uIndex);
            }
        }
    }
    return nullptr;
}

/*
 * \brief return enumeration item from list with zero based index
 * \ingroup expfunc
 * \param[in] pData      CSECoPbaseType to read
 * \param[in] uPosition  zero-based position to data part
 * \param[in] llEnumVal  enumeration value
 * \param[in] szEnumName enumeration name
 * \return true, if successful
 */
bool SECoP_V_putEnumeration(CSECoPbaseType* pData, unsigned int uPosition,
                            long long llEnumVal, const char* szEnumName)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        if (pItem != nullptr && uPosition < 1 && szEnumName != nullptr && *szEnumName != '\0')
        {
            CSECoPenumBase* pEnum(dynamic_cast<CSECoPenumBase*>(const_cast<CSECoPbaseType*>(pItem)));
            if (pEnum != nullptr)
                return pEnum->addItem(llEnumVal, szEnumName);
        }
    }
    return false;
}

/*
 * \brief return name or struct item from any variant
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \return for valid index: struct or dictionary name, for invalid index: NULL
 */
const char* SECoP_V_getStructName(const CSECoPbaseType* pData, unsigned int uPosition)
{
    const char* szName(nullptr);
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, &szName))
        if (pItem != nullptr && uPosition < 1)
            return szName;
    return nullptr;
}

/*
 * \brief compare two values and check for equal/compatible types and equal values
 * \ingroup expfunc
 * \param[in] pData1 first value
 * \param[in] pData2 second value
 * \return result \ref SECoP_V_compareResult
 */
enum SECoP_V_compareResult SECoP_V_compare(const CSECoPbaseType* pData1, const CSECoPbaseType* pData2)
{
    if (pData1 == nullptr || pData2 == nullptr || !SECoP_V_g_huItems.contains(pData1) || !SECoP_V_g_huItems.contains(pData2))
        return SECoP_VC_DIFF_TYPE;
    return pData1->compareValue(pData2);
}

/*
 * \brief read minimum and maximum double value
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[out] pdblMinimum minimum value (or NaN)
 * \param[out] pdblMaximum maximum value (minimum <= maximum or NaN)
 * \return true, if successful
 */
bool SECoP_V_getMinMaxDouble(const CSECoPbaseType* pData, unsigned int uPosition, double* pdblMinimum, double* pdblMaximum)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        const CSECoPminmaxType<double>* pMinMax(dynamic_cast<const CSECoPminmaxType<double>*>(pItem));
        if (pMinMax != nullptr && uPosition < 1)
        {
            double dblMinimum, dblMaximum;
            pMinMax->getMinMaxValue(dblMinimum, dblMaximum);
            if (pdblMinimum != nullptr)
                *pdblMinimum = dblMinimum;
            if (pdblMaximum != nullptr)
                *pdblMaximum = dblMaximum;
            return true;
        }
    }
    return false;
}

/*
 * \brief set minimum and maximum double value
 * \ingroup expfunc
 * \param[in] pData      CSECoPbaseType to change
 * \param[in] uPosition  zero-based position to data part
 * \param[in] dblMinimum minimum value (or NaN)
 * \param[in] dblMaximum maximum value (minimum <= maximum or NaN)
 * \return true, if successful
 */
bool SECoP_V_modifyMinMaxDouble(CSECoPbaseType* pData, unsigned int uPosition, double dblMinimum, double dblMaximum)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        CSECoPminmaxType<double>* pMinMax(dynamic_cast<CSECoPminmaxType<double>*>(const_cast<CSECoPbaseType*>(pItem)));
        if (pMinMax != nullptr && uPosition < 1)
            return pMinMax->setMinMaxValue(dblMinimum, dblMaximum);
    }
    return false;
}

/*
 * \brief read minimum and maximum integer value
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[out] pllMinimum minimum value
 * \param[out] pllMaximum maximum value (minimum <= maximum)
 * \return true, if successful
 */
bool SECoP_V_getMinMaxInteger(const CSECoPbaseType* pData, unsigned int uPosition, long long* pllMinimum, long long* pllMaximum)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        const CSECoPminmaxType<long long>* pMinMax(dynamic_cast<const CSECoPminmaxType<long long>*>(pItem));
        if (pMinMax != nullptr && uPosition < 1)
        {
            long long llMinimum, llMaximum;
            pMinMax->getMinMaxValue(llMinimum, llMaximum);
            if (pllMinimum != nullptr)
                *pllMinimum = llMinimum;
            if (pllMaximum != nullptr)
                *pllMaximum = llMaximum;
            return true;
        }
    }
    return false;
}

/*
 * \brief set minimum and maximum integer value
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to change
 * \param[in] uPosition zero-based position to data part
 * \param[in] llMinimum minimum value
 * \param[in] llMaximum maximum value (minimum <= maximum)
 * \return true, if successful
 */
bool SECoP_V_modifyMinMaxInteger(CSECoPbaseType* pData, unsigned int uPosition, long long llMinimum, long long llMaximum)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        CSECoPminmaxType<long long>* pMinMax(dynamic_cast<CSECoPminmaxType<long long>*>(const_cast<CSECoPbaseType*>(pItem)));
        if (pMinMax != nullptr && uPosition < 1)
            return pMinMax->setMinMaxValue(llMinimum, llMaximum);
    }
    return false;
}

/*
 * \brief read current, minimum and maximum array or data sizes
 * \ingroup expfunc
 * \param[in]  pData           CSECoPbaseType to read
 * \param[in]  uPosition       zero-based position to data part
 * \param[out] puLength        current array size (minimum <= current <= maximum)
 * \param[out] puMinimumLength minimum array size (>=0)
 * \param[out] puMaximumLength maximum array size (minimum <= maximum)
 * \return true, if successful
 */
bool SECoP_V_getArrayLength(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int* puLength,
                            unsigned int* puMinimumLength, unsigned int* puMaximumLength)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        const CSECoParrayBase* pArray(dynamic_cast<const CSECoParrayBase*>(pItem));
        if (pArray != nullptr && uPosition < 1)
        {
            unsigned int uMinimum, uMaximum;
            if (puLength != nullptr)
                *puLength = pArray->getSize();
            pArray->getMinMaxSize(uMinimum, uMaximum);
            if (puMinimumLength != nullptr)
                *puMinimumLength = uMinimum;
            if (puMaximumLength != nullptr)
                *puMaximumLength = uMaximum;
            return true;
        }
    }
    return false;
}

/*
 * \brief set current size, minimum size and maximum size of array with the constraint: minimum <= current <= maximum
 *        a special call is available: if minimum size == 1 and maximum size == 0, only the current size is changed and
 *        the minimum and maximum sizes are ignored
 * \ingroup expfunc
 * \param[in] pData          CSECoPbaseType to change
 * \param[in] uPosition      zero-based position to data part
 * \param[in] uLength        current size (minimum <= current <= maximum)
 * \param[in] uMinimumLength minimum array size (>=0)
 * \param[in] uMaximumLength maximum array size (minimum <= maximum)
 * \return true, if successful
 */
bool SECoP_V_modifyArrayLength(CSECoPbaseType* pData, unsigned int uPosition, unsigned int uLength,
                               unsigned int uMinimumLength, unsigned int uMaximumLength)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        CSECoParrayBase* pArray(dynamic_cast<CSECoParrayBase*>(const_cast<CSECoPbaseType*>(pItem)));
        if (pArray != nullptr && uPosition < 1)
        {
            if (uMinimumLength <= uMaximumLength)
                if (!pArray->setMinMaxSize(uMinimumLength, uMaximumLength))
                    return false;
            return pArray->setSize(uLength);
        }
    }
    return false;
}

/*
 * \brief Use this function to replace existing string data inside the SECoP variant.
 *        This string is selected via its uPosition (\ref SECoP_V_getInfo).
 * \ingroup expfunc
 * \param[in,out] ppData    pointer to data structure to modify, pointer will be changed
 * \param[in]     uPosition zero-based position to SECoP_VT_STRING, SECoP_VT_BLOB or SECoP_VT_JSON
 * \param[in]     szValue   string value to replace
 * \param[in]     iSize     length of string in bytes or -1 for null-terminated string
 * \return true, if successful
 */
bool SECoP_V_modifyString(CSECoPbaseType* pData, unsigned int uPosition, const char* szValue, int iSize)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        CSECoPstring* pString(dynamic_cast<CSECoPstring*>(const_cast<CSECoPbaseType*>(pItem)));
        if (pString != nullptr && uPosition < 1)
            return pString->setValue(QByteArray(szValue, iSize));
    }
    return false;
}

/*
 * \brief function to read the factor of a scaled integer out of the SECoP variant.
 * \ingroup expfunc
 * \param[in]  pData     CSECoPbaseType to read
 * \param[in]  uPosition zero-based position to data part
 * \param[out] pdblValue memory location which will get the scale factor
 * \return true, if successful
 */
bool SECoP_V_getScale(const CSECoPbaseType* pData, unsigned int uPosition, double* pdblValue)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        const CSECoPscaledBase* pScale(dynamic_cast<const CSECoPscaledBase*>(pItem));
        if (pScale != nullptr && uPosition < 1)
        {
            double dScale(pScale->getScale());
            if (pdblValue != nullptr)
                *pdblValue = dScale;
            return true;
        }
    }
    return false;
}

/*
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
bool SECoP_V_modifyScale(CSECoPbaseType* pData, unsigned int uPosition, double dblScale)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        CSECoPscaledBase* pScale(dynamic_cast<CSECoPscaledBase*>(const_cast<CSECoPbaseType*>(pItem)));
        if (pScale != nullptr && uPosition < 1)
            return pScale->setScale(dblScale);
    }
    return false;
}

/*
 * \brief retrieve number of additional items of variant (e.g. unit)
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \return number of additional items or zero
 */
unsigned int SECoP_V_getAdditionalCount(const CSECoPbaseType* pData, unsigned int uPosition)
{
    const CSECoPbaseType* pItem(nullptr);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
    {
        nlohmann::json j(pItem->additional());
        if (j.is_object())
            return static_cast<unsigned int>(j.size());
    }
    return 0;
}

/*
 * \brief retrieve index of additional item of a variant by name
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \param[in] szName    name of item
 * \return index to additional items or UINT_MAX (-1 as invalid index)
 */
unsigned int SECoP_V_getAdditionalIndex(const CSECoPbaseType* pData, unsigned int uPosition, const char* szName)
{
    const CSECoPbaseType* pItem(nullptr);
    unsigned int uResult(UINT_MAX);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr) && uPosition < 1)
    {
        nlohmann::json o(pItem->additional());
        if (o.is_object())
        {
            unsigned int i(0);
            for (auto it = o.cbegin(); it != o.cend(); ++i, ++it)
            {
                if (strcasecmp(it.key().c_str(), szName) == 0)
                {
                    uResult = i;
                    break;
                }
            }
        }
    }
    return uResult;
}

/*
 * \brief retrieve the key name of additional (string,json-array,json-object) items of a variant
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[in]  uIndex      zero-based index to additional item (\ref SECoP_V_getAdditionalCount)
 * \param[in]  uBufferSize size of memory location (function will not copy more than this number of bytes)
 * \param[out] pszValue    memory location which will get the string value
 * \return zero for errors, number of bytes needed (including a trailing NUL)
 */
unsigned int SHALL_EXPORT SECoP_V_getAdditionalKey(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex,
                                                       unsigned int uBufferSize, char* pszValue)
{
    const CSECoPbaseType* pItem(nullptr);
    if (!SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
        return 0;
    if (uPosition > 0)
        return 0;
    unsigned int i(0);
    nlohmann::json j(pItem->additional());
    if (!j.is_object() || uIndex >= j.size())
        return 0;
    nlohmann::json::const_iterator it(j.cbegin());
    while (i < uIndex)
    {
        if (i >= j.size() || it == j.cend())
            return 0;
        ++it;
        ++i;
    }
    std::string szBuffer(it.key());
    unsigned int iLen(static_cast<unsigned int>(szBuffer.size()));
    if (uBufferSize >= (iLen + 1))
    {
        if (iLen > 0)
            memmove(pszValue, szBuffer.c_str(), iLen);
        pszValue[iLen] = '\0';
    }
    else if (uBufferSize > 0)
    {
        --uBufferSize;
        if (uBufferSize > 0)
            memmove(pszValue, szBuffer.c_str(), uBufferSize);
        pszValue[uBufferSize] = '\0';
    }
    return iLen + 1;
}

/*
 * \brief retrieve a string of additional (string,json-array,json-object) items of a variant
 * \ingroup expfunc
 * \param[in]  pData       CSECoPbaseType to read
 * \param[in]  uPosition   zero-based position to data part
 * \param[in]  uIndex      zero-based index to additional item (\ref SECoP_V_getAdditionalCount)
 * \param[in]  uBufferSize size of memory location (function will not copy more than this number of bytes)
 * \param[out] pszValue    memory location which will get the string value
 * \return zero for errors, number of bytes needed (including a trailing NUL)
 */
unsigned int SHALL_EXPORT SECoP_V_getAdditionalString(const CSECoPbaseType* pData, unsigned int uPosition,
                                                          unsigned int uIndex, unsigned int uBufferSize, char* pszValue)
{
    const CSECoPbaseType* pItem(nullptr);
    if (!SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr))
        return 0;
    if (uPosition > 0)
        return 0;
    unsigned int i(0);
    nlohmann::json j(pItem->additional());
    if (!j.is_object() || uIndex >= j.size())
        return 0;
    nlohmann::json::const_iterator it(j.cbegin());
    while (i < uIndex)
    {
        if (i >= j.size() || it == j.cend())
            return 0;
        ++it;
        ++i;
    }
    nlohmann::json v(it.value());
    std::string szBuffer;
    if (v.is_string())
        szBuffer = v.get<std::string>();
    else
        szBuffer = v.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
    unsigned int iLen(static_cast<unsigned int>(szBuffer.size()));
    if (uBufferSize >= (iLen + 1))
    {
        if (iLen > 0)
            memmove(pszValue, szBuffer.c_str(), iLen);
        pszValue[iLen] = '\0';
    }
    else if (uBufferSize > 0)
    {
        --uBufferSize;
        if (uBufferSize > 0)
            memmove(pszValue, szBuffer.c_str(), uBufferSize);
        pszValue[uBufferSize] = '\0';
    }
    return iLen + 1;
}

/*
 * \brief retrieve the numeric value as double of additional items of a variant
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \param[in] uIndex    zero-based index to additional item (\ref SECoP_V_getAdditionalCount)
 * \return on success the value, NaN on errors
 */
double SECoP_V_getAdditionalDouble(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex)
{
    const CSECoPbaseType* pItem(nullptr);
    double d(std::numeric_limits<double>::quiet_NaN());
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr) && uPosition < 1)
    {
        unsigned int i(0);
        nlohmann::json j(pItem->additional());
        if (!j.is_object() || uIndex >= j.size())
            return 0;
        nlohmann::json::const_iterator it(j.cbegin());
        while (i < uIndex)
        {
            if (i >= j.size() || it == j.cend())
                return d;
            ++it;
            ++i;
        }
        nlohmann::json v(it.value());
        switch (v.type())
        {
            case nlohmann::json::value_t::number_float:    d = v.get<double>(); break;
            case nlohmann::json::value_t::number_integer:  d = v.get<std::int64_t>(); break;
            case nlohmann::json::value_t::number_unsigned: d = v.get<std::uint64_t>(); break;
            default: break;
        }
    }
    return d;
}

/*
 * \brief retrieve the numeric value as integer of additional items of a variant
 * \ingroup expfunc
 * \param[in] pData     CSECoPbaseType to read
 * \param[in] uPosition zero-based position to data part
 * \param[in] uIndex    zero-based index to additional item (\ref SECoP_V_getAdditionalCount)
 * \return on success the value, zero on errors (with might be a valid value)
 */
long long SECoP_V_getAdditionalInteger(const CSECoPbaseType* pData, unsigned int uPosition, unsigned int uIndex)
{
    const CSECoPbaseType* pItem(nullptr);
    long long ll(0LL);
    if (SECoP_V_getInfoHelper(pData, &uPosition, nullptr, &pItem, nullptr) && uPosition < 1)
    {
        unsigned int i(0);
        nlohmann::json j(pItem->additional());
        if (!j.is_object() || uIndex >= j.size())
            return 0;
        nlohmann::json::const_iterator it(j.cbegin());
        while (i < uIndex)
        {
            if (i >= j.size() || it == j.cend())
                return ll;
            ++it;
            ++i;
        }
        nlohmann::json v(it.value());
        switch (v.type())
        {
            case nlohmann::json::value_t::number_float:    ll = static_cast<long long>(v.get<double>()); break;
            case nlohmann::json::value_t::number_integer:  ll = v.get<std::int64_t>(); break;
            case nlohmann::json::value_t::number_unsigned: ll = static_cast<long long>(v.get<std::uint64_t>()); break;
            default: break;
        }
    }
    return ll;
}

/*****************************************************************************
 * CSECoPbaseType
 *****************************************************************************/
/**
 * \brief This protected constructor disallows creation of unspecified objects.
 *        It puts the newly created object into a global list of known objects.
 */
CSECoPbaseType::CSECoPbaseType()
    : m_iType(SECoP_VT_NONE)
    , m_Additional()
{
    SECoP_V_initMutex();
    QMutexLocker locker(SECoP_V_g_pMutex);
    SECoP_V_g_huItems.insert(this, 0);
}

/**
 * \brief Internally used constructor: it stores the type and puts the newly
 *        created object into a global list of known objects.
 * \param[in] iType type of newly created object
 */
CSECoPbaseType::CSECoPbaseType(SECoP_V_type iType)
    : m_iType(iType)
{
    SECoP_V_initMutex();
    QMutexLocker locker(SECoP_V_g_pMutex);
    SECoP_V_g_huItems.insert(this, 0);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPbaseType::CSECoPbaseType(const CSECoPbaseType* pOther)
    : m_iType(pOther->m_iType)
    , m_Additional(pOther->m_Additional)
{
    SECoP_V_initMutex();
    QMutexLocker locker(SECoP_V_g_pMutex);
    SECoP_V_g_huItems.insert(this, 0);
}

/**
 * \brief The destructor removes the object from the global list of known objects.
 */
CSECoPbaseType::~CSECoPbaseType()
{
    SECoP_V_initMutex();
    QMutexLocker locker(SECoP_V_g_pMutex);
    SECoP_V_g_huItems.remove(this);
}

/**
 * \brief The function returns the type of the object. This function is called
 *        quite often, thats why the internal validity check is disabled.
 * \return type of the object
 */
SECoP_V_type CSECoPbaseType::getType() const
{
#if 0
    const CSECoPbaseType*                   pBaseType         (nullptr);
    const CSECoPnull*                       pNull             (nullptr);
    const CSECoPsimpleType<double>*         pSimpleTypeDouble (nullptr);
    const CSECoPsimpleType<long long>*      pSimpleTypeInt    (nullptr);
    const CSECoPminmaxType<double>*         pMinMaxTypeDouble (nullptr);
    const CSECoPminmaxType<long long>*      pMinMaxTypeInt    (nullptr);
    const CSECoParrayBase*                  pArrayBase        (nullptr);
    const CSECoParraySimple<double>*        pArraySimpleDouble(nullptr);
    const CSECoParraySimple<long long>*     pArraySimpleInt   (nullptr);
    const CSECoParraySimple<unsigned char>* pArraySimpleChar  (nullptr);
    const CSECoPenumBase*                   pEnumBase         (nullptr);
    const CSECoPscaledBase*                 pScaledBase       (nullptr);
    const CSECoPsimpleBool*                 pSimpleBool       (nullptr);
    const CSECoPsimpleDouble*               pSimpleDouble     (nullptr);
    const CSECoPsimpleInt*                  pSimpleInt        (nullptr);
    const CSECoPsimpleEnum*                 pSimpleEnum       (nullptr);
    const CSECoPsimpleScaled*               pSimpleScaled     (nullptr);
    const CSECoParrayBool*                  pArrayBool        (nullptr);
    const CSECoParrayDouble*                pArrayDouble      (nullptr);
    const CSECoParrayInt*                   pArrayInt         (nullptr);
    const CSECoParrayEnum*                  pArrayEnum        (nullptr);
    const CSECoParrayScaled*                pArrayScaled      (nullptr);
    const CSECoPstring*                     pString           (nullptr);
    const CSECoPstruct*                     pStruct           (nullptr);
    const CSECoPtuple*                      pTuple            (nullptr);
    const CSECoParray*                      pArray            (nullptr);
    const CSECoPcommand*                    pCommand          (nullptr);
    pBaseType = dynamic_cast<const CSECoPbaseType*>(this);
    Q_ASSERT(pBaseType != nullptr);
    switch (m_iType)
    {
        case SECoP_VT_NONE:
            pNull = dynamic_cast<const CSECoPnull*>(this);
            Q_ASSERT(pNull != nullptr);
            break;
        case SECoP_VT_DOUBLE:
            pSimpleTypeDouble = dynamic_cast<const CSECoPsimpleType<double>*>(this);
            Q_ASSERT(pSimpleTypeDouble != nullptr);
            pMinMaxTypeDouble = dynamic_cast<const CSECoPminmaxType<double>*>(this);
            Q_ASSERT(pMinMaxTypeDouble != nullptr);
            pSimpleDouble = dynamic_cast<const CSECoPsimpleDouble*>(this);
            Q_ASSERT(pSimpleDouble != nullptr);
            break;
        case SECoP_VT_INTEGER:
            pSimpleTypeInt = dynamic_cast<const CSECoPsimpleType<long long>*>(this);
            Q_ASSERT(pSimpleTypeInt != nullptr);
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            Q_ASSERT(pMinMaxTypeInt != nullptr);
            pSimpleInt = dynamic_cast<const CSECoPsimpleInt*>(this);
            Q_ASSERT(pSimpleInt != nullptr);
            break;
        case SECoP_VT_BOOL:
            pSimpleTypeInt = dynamic_cast<const CSECoPsimpleType<long long>*>(this);
            Q_ASSERT(pSimpleTypeInt != nullptr);
            pSimpleBool = dynamic_cast<const CSECoPsimpleBool*>(this);
            Q_ASSERT(pSimpleBool != nullptr);
            break;
        case SECoP_VT_ENUM:
            pEnumBase = dynamic_cast<const CSECoPenumBase*>(this);
            Q_ASSERT(pEnumBase != nullptr);
            pSimpleTypeInt = dynamic_cast<const CSECoPsimpleType<long long>*>(this);
            Q_ASSERT(pSimpleTypeInt != nullptr);
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            Q_ASSERT(pMinMaxTypeInt != nullptr);
            pSimpleInt = dynamic_cast<const CSECoPsimpleInt*>(this);
            Q_ASSERT(pSimpleInt != nullptr);
            pSimpleEnum = dynamic_cast<const CSECoPsimpleEnum*>(this);
            Q_ASSERT(pSimpleEnum != nullptr);
            break;
        case SECoP_VT_SCALED:
            pScaledBase = dynamic_cast<const CSECoPscaledBase*>(this);
            Q_ASSERT(pScaledBase != nullptr);
            pSimpleTypeInt = dynamic_cast<const CSECoPsimpleType<long long>*>(this);
            Q_ASSERT(pSimpleTypeInt != nullptr);
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            Q_ASSERT(pMinMaxTypeInt != nullptr);
            pSimpleInt = dynamic_cast<const CSECoPsimpleInt*>(this);
            Q_ASSERT(pSimpleInt != nullptr);
            pSimpleScaled = dynamic_cast<const CSECoPsimpleScaled*>(this);
            Q_ASSERT(pSimpleScaled != nullptr);
            break;
        case SECoP_VT_ARRAY_DOUBLE:
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            Q_ASSERT(pArrayBase != nullptr);
            pArraySimpleDouble = dynamic_cast<const CSECoParraySimple<double>*>(this);
            Q_ASSERT(pArraySimpleDouble != nullptr);
            pMinMaxTypeDouble = dynamic_cast<const CSECoPminmaxType<double>*>(this);
            Q_ASSERT(pMinMaxTypeDouble != nullptr);
            pArrayDouble = dynamic_cast<const CSECoParrayDouble*>(this);
            Q_ASSERT(pArrayDouble != nullptr);
            break;
        case SECoP_VT_ARRAY_INTEGER:
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            Q_ASSERT(pArrayBase != nullptr);
            pArraySimpleInt = dynamic_cast<const CSECoParraySimple<long long>*>(this);
            Q_ASSERT(pArraySimpleInt != nullptr);
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            Q_ASSERT(pMinMaxTypeInt != nullptr);
            pArrayInt = dynamic_cast<const CSECoParrayInt*>(this);
            Q_ASSERT(pArrayInt != nullptr);
            break;
        case SECoP_VT_ARRAY_BOOL:
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            Q_ASSERT(pArrayBase != nullptr);
            pArraySimpleInt = dynamic_cast<const CSECoParraySimple<long long>*>(this);
            Q_ASSERT(pArraySimpleInt != nullptr);
            pArrayBool = dynamic_cast<const CSECoParrayBool*>(this);
            Q_ASSERT(pArrayBool != nullptr);
            break;
        case SECoP_VT_ARRAY_ENUM:
            pEnumBase = dynamic_cast<const CSECoPenumBase*>(this);
            Q_ASSERT(pEnumBase != nullptr);
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            Q_ASSERT(pArrayBase != nullptr);
            pArraySimpleInt = dynamic_cast<const CSECoParraySimple<long long>*>(this);
            Q_ASSERT(pArraySimpleInt != nullptr);
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            Q_ASSERT(pMinMaxTypeInt != nullptr);
            pArrayInt = dynamic_cast<const CSECoParrayInt*>(this);
            Q_ASSERT(pArrayInt != nullptr);
            pArrayEnum = dynamic_cast<const CSECoParrayEnum*>(this);
            Q_ASSERT(pArrayEnum != nullptr);
            break;
        case SECoP_VT_ARRAY_SCALED:
            pScaledBase = dynamic_cast<const CSECoPscaledBase*>(this);
            Q_ASSERT(pScaledBase != nullptr);
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            Q_ASSERT(pArrayBase != nullptr);
            pArraySimpleInt = dynamic_cast<const CSECoParraySimple<long long>*>(this);
            Q_ASSERT(pArraySimpleInt != nullptr);
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            Q_ASSERT(pMinMaxTypeInt != nullptr);
            pArrayInt = dynamic_cast<const CSECoParrayInt*>(this);
            Q_ASSERT(pArrayInt != nullptr);
            pArrayScaled = dynamic_cast<const CSECoParrayScaled*>(this);
            Q_ASSERT(pArrayScaled != nullptr);
            break;
        case SECoP_VT_STRING:
        case SECoP_VT_BLOB:
        case SECoP_VT_JSON:
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            Q_ASSERT(pArrayBase != nullptr);
            pArraySimpleChar = dynamic_cast<const CSECoParraySimple<unsigned char>*>(this);
            Q_ASSERT(pArraySimpleChar != nullptr);
            pString = dynamic_cast<const CSECoPstring*>(this);
            Q_ASSERT(pString != nullptr);
            break;
        case SECoP_VT_STRUCT:
            pStruct = dynamic_cast<const CSECoPstruct*>(this);
            Q_ASSERT(pStruct != nullptr);
            break;
        case SECoP_VT_TUPLE:
            pTuple = dynamic_cast<const CSECoPtuple*>(this);
            Q_ASSERT(pTuple != nullptr);
            break;
        case SECoP_VT_ARRAY:
            pArray = dynamic_cast<const CSECoParray*>(this);
            Q_ASSERT(pArray != nullptr);
            break;
        case SECoP_VT_COMMAND:
            pCommand = dynamic_cast<const CSECoPcommand*>(this);
            Q_ASSERT(pCommand != nullptr);
            break;
        default:
            Q_ASSERT(false);
            break;
    }
#endif
    return m_iType;
}

/**
 * \brief Change the type of the object. Use this function with care,
 *        because old and new type are not checked, but have to be
 *        compatible to prevent errors.
 * \param[in] iType new type of the object
 */
void CSECoPbaseType::setType(SECoP_V_type iType)
{
    m_iType = iType;
}

/**
 * \brief This function compares, if the type of both objects are the same.
 *        This function has to be overloaded.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPbaseType::compareType(const CSECoPbaseType* pOther) const
{
    if (pOther == nullptr)
        return false;
    return (m_iType == pOther->m_iType);
}

/**
 * \brief This function compares two objects.
 *        This function has to be overloaded.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPbaseType::compareValue(const CSECoPbaseType* pOther) const
{
    if (pOther != nullptr && m_iType == pOther->m_iType)
        return (m_Additional == pOther->m_Additional) ? SECoP_VC_EQUAL : SECoP_VC_DIFF_VALUE;
    return SECoP_VC_DIFF_TYPE;
}

/**
 * \brief This function checks, if the object is valid and usable.
 *        This function has to be overloaded.
 * \return true: valid object, false: otherwise
 */
bool CSECoPbaseType::isValid() const
{
    const CSECoPbaseType*                   pBaseType         (nullptr);
    const CSECoPnull*                       pNull             (nullptr);
    const CSECoPsimpleType<double>*         pSimpleTypeDouble (nullptr);
    const CSECoPsimpleType<long long>*      pSimpleTypeInt    (nullptr);
    const CSECoPminmaxType<double>*         pMinMaxTypeDouble (nullptr);
    const CSECoPminmaxType<long long>*      pMinMaxTypeInt    (nullptr);
    const CSECoParrayBase*                  pArrayBase        (nullptr);
    const CSECoParraySimple<double>*        pArraySimpleDouble(nullptr);
    const CSECoParraySimple<long long>*     pArraySimpleInt   (nullptr);
    const CSECoParraySimple<unsigned char>* pArraySimpleChar  (nullptr);
    const CSECoPenumBase*                   pEnumBase         (nullptr);
    const CSECoPscaledBase*                 pScaledBase       (nullptr);
    const CSECoPsimpleBool*                 pSimpleBool       (nullptr);
    const CSECoPsimpleDouble*               pSimpleDouble     (nullptr);
    const CSECoPsimpleInt*                  pSimpleInt        (nullptr);
    const CSECoPsimpleEnum*                 pSimpleEnum       (nullptr);
    const CSECoPsimpleScaled*               pSimpleScaled     (nullptr);
    const CSECoParrayBool*                  pArrayBool        (nullptr);
    const CSECoParrayDouble*                pArrayDouble      (nullptr);
    const CSECoParrayInt*                   pArrayInt         (nullptr);
    const CSECoParrayEnum*                  pArrayEnum        (nullptr);
    const CSECoParrayScaled*                pArrayScaled      (nullptr);
    const CSECoPstring*                     pString           (nullptr);
    const CSECoPstruct*                     pStruct           (nullptr);
    const CSECoPtuple*                      pTuple            (nullptr);
    const CSECoParray*                      pArray            (nullptr);
    const CSECoPcommand*                    pCommand          (nullptr);
    pBaseType = dynamic_cast<const CSECoPbaseType*>(this);
    if (pBaseType == nullptr)
        return false;
    switch (m_iType)
    {
        case SECoP_VT_NONE:
            pNull = dynamic_cast<const CSECoPnull*>(this);
            if (pNull == nullptr)
                return false;
            break;
        case SECoP_VT_DOUBLE:
            pSimpleTypeDouble = dynamic_cast<const CSECoPsimpleType<double>*>(this);
            if (pSimpleTypeDouble == nullptr)
                return false;
            pMinMaxTypeDouble = dynamic_cast<const CSECoPminmaxType<double>*>(this);
            if (pMinMaxTypeDouble == nullptr)
                return false;
            pSimpleDouble = dynamic_cast<const CSECoPsimpleDouble*>(this);
            if (pSimpleDouble == nullptr)
                return false;
            break;
        case SECoP_VT_INTEGER:
            pSimpleTypeInt = dynamic_cast<const CSECoPsimpleType<long long>*>(this);
            if (pSimpleTypeInt == nullptr)
                return false;
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            if (pMinMaxTypeInt == nullptr)
                return false;
            pSimpleInt = dynamic_cast<const CSECoPsimpleInt*>(this);
            if (pSimpleInt == nullptr)
                return false;
            break;
        case SECoP_VT_BOOL:
            pSimpleTypeInt = dynamic_cast<const CSECoPsimpleType<long long>*>(this);
            if (pSimpleTypeInt == nullptr)
                return false;
            pSimpleBool = dynamic_cast<const CSECoPsimpleBool*>(this);
            if (pSimpleBool == nullptr)
                return false;
            break;
        case SECoP_VT_ENUM:
            pEnumBase = dynamic_cast<const CSECoPenumBase*>(this);
            if (pEnumBase == nullptr)
                return false;
            pSimpleTypeInt = dynamic_cast<const CSECoPsimpleType<long long>*>(this);
            if (pSimpleTypeInt == nullptr)
                return false;
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            if (pMinMaxTypeInt == nullptr)
                return false;
            pSimpleInt = dynamic_cast<const CSECoPsimpleInt*>(this);
            if (pSimpleInt == nullptr)
                return false;
            pSimpleEnum = dynamic_cast<const CSECoPsimpleEnum*>(this);
            if (pSimpleEnum == nullptr)
                return false;
            break;
        case SECoP_VT_SCALED:
            pScaledBase = dynamic_cast<const CSECoPscaledBase*>(this);
            if (pScaledBase == nullptr)
                return false;
            pSimpleTypeInt = dynamic_cast<const CSECoPsimpleType<long long>*>(this);
            if (pSimpleTypeInt == nullptr)
                return false;
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            if (pMinMaxTypeInt == nullptr)
                return false;
            pSimpleInt = dynamic_cast<const CSECoPsimpleInt*>(this);
            if (pSimpleInt == nullptr)
                return false;
            pSimpleScaled = dynamic_cast<const CSECoPsimpleScaled*>(this);
            if (pSimpleScaled == nullptr)
                return false;
            break;
        case SECoP_VT_ARRAY_DOUBLE:
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            if (pArrayBase == nullptr)
                return false;
            pArraySimpleDouble = dynamic_cast<const CSECoParraySimple<double>*>(this);
            if (pArraySimpleDouble == nullptr)
                return false;
            pMinMaxTypeDouble = dynamic_cast<const CSECoPminmaxType<double>*>(this);
            if (pMinMaxTypeDouble == nullptr)
                return false;
            pArrayDouble = dynamic_cast<const CSECoParrayDouble*>(this);
            if (pArrayDouble == nullptr)
                return false;
            break;
        case SECoP_VT_ARRAY_INTEGER:
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            if (pArrayBase == nullptr)
                return false;
            pArraySimpleInt = dynamic_cast<const CSECoParraySimple<long long>*>(this);
            if (pArraySimpleInt == nullptr)
                return false;
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            if (pMinMaxTypeInt == nullptr)
                return false;
            pArrayInt = dynamic_cast<const CSECoParrayInt*>(this);
            if (pArrayInt == nullptr)
                return false;
            break;
        case SECoP_VT_ARRAY_BOOL:
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            if (pArrayBase == nullptr)
                return false;
            pArraySimpleInt = dynamic_cast<const CSECoParraySimple<long long>*>(this);
            if (pArraySimpleInt == nullptr)
                return false;
            pArrayBool = dynamic_cast<const CSECoParrayBool*>(this);
            if (pArrayBool == nullptr)
                return false;
            break;
        case SECoP_VT_ARRAY_ENUM:
            pEnumBase = dynamic_cast<const CSECoPenumBase*>(this);
            if (pEnumBase == nullptr)
                return false;
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            if (pArrayBase == nullptr)
                return false;
            pArraySimpleInt = dynamic_cast<const CSECoParraySimple<long long>*>(this);
            if (pArraySimpleInt == nullptr)
                return false;
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            if (pMinMaxTypeInt == nullptr)
                return false;
            pArrayInt = dynamic_cast<const CSECoParrayInt*>(this);
            if (pArrayInt == nullptr)
                return false;
            pArrayEnum = dynamic_cast<const CSECoParrayEnum*>(this);
            if (pArrayEnum == nullptr)
                return false;
            break;
        case SECoP_VT_ARRAY_SCALED:
            pScaledBase = dynamic_cast<const CSECoPscaledBase*>(this);
            if (pScaledBase == nullptr)
                return false;
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            if (pArrayBase == nullptr)
                return false;
            pArraySimpleInt = dynamic_cast<const CSECoParraySimple<long long>*>(this);
            if (pArraySimpleInt == nullptr)
                return false;
            pMinMaxTypeInt = dynamic_cast<const CSECoPminmaxType<long long>*>(this);
            if (pMinMaxTypeInt == nullptr)
                return false;
            pArrayInt = dynamic_cast<const CSECoParrayInt*>(this);
            if (pArrayInt == nullptr)
                return false;
            pArrayScaled = dynamic_cast<const CSECoParrayScaled*>(this);
            if (pArrayScaled == nullptr)
                return false;
            break;
        case SECoP_VT_STRING:
        case SECoP_VT_BLOB:
        case SECoP_VT_JSON:
            pArrayBase = dynamic_cast<const CSECoParrayBase*>(this);
            if (pArrayBase == nullptr)
                return false;
            pArraySimpleChar = dynamic_cast<const CSECoParraySimple<unsigned char>*>(this);
            if (pArraySimpleChar == nullptr)
                return false;
            pString = dynamic_cast<const CSECoPstring*>(this);
            if (pString == nullptr)
                return false;
            break;
        case SECoP_VT_STRUCT:
            pStruct = dynamic_cast<const CSECoPstruct*>(this);
            if (pStruct == nullptr)
                return false;
            break;
        case SECoP_VT_TUPLE:
            pTuple = dynamic_cast<const CSECoPtuple*>(this);
            if (pTuple == nullptr)
                return false;
            break;
        case SECoP_VT_ARRAY:
            pArray = dynamic_cast<const CSECoParray*>(this);
            if (pArray == nullptr)
                return false;
            break;
        case SECoP_VT_COMMAND:
            pCommand = dynamic_cast<const CSECoPcommand*>(this);
            if (pCommand == nullptr)
                return false;
            break;
        default:
            return false;
    }
    return true;
}

/**
 * \brief This function clears the value of the object.
 *        This function has to be overloaded.
 * \return true: successful, false: not successful
 */
bool CSECoPbaseType::clear()
{
    return false;
}

/**
 * \brief This function creates a deep copy of the current object.
 * \return newly created object or nullptr on error
 */
CSECoPbaseType* CSECoPbaseType::duplicate() const
{
    const void* pMeMyselfAndI(reinterpret_cast<const void*>(this));
    if (pMeMyselfAndI != nullptr && SECoP_V_g_huItems.contains(this))
    {
#define HANDLE_TYPE(x) \
        do { const x* p = dynamic_cast<const x*>(this); if (p != nullptr) return new x(p); } while(0)
        HANDLE_TYPE(CSECoPnull);
        HANDLE_TYPE(CSECoPsimpleBool);
        HANDLE_TYPE(CSECoPsimpleEnum);
        HANDLE_TYPE(CSECoPsimpleScaled);
        HANDLE_TYPE(CSECoPsimpleDouble);
        HANDLE_TYPE(CSECoPsimpleInt);
        HANDLE_TYPE(CSECoParrayBool);
        HANDLE_TYPE(CSECoParrayEnum);
        HANDLE_TYPE(CSECoParrayScaled);
        HANDLE_TYPE(CSECoParrayDouble);
        HANDLE_TYPE(CSECoParrayInt);
        HANDLE_TYPE(CSECoPstring);
        HANDLE_TYPE(CSECoPstruct);
        HANDLE_TYPE(CSECoPtuple);
        HANDLE_TYPE(CSECoParray);
        HANDLE_TYPE(CSECoPcommand);
#undef HANDLE_TYPE
    }
    return nullptr;
}

/**
 * \brief This function returns all additional SECoP information of the object.
 * \return additional SECoP information of the object
 */
nlohmann::json CSECoPbaseType::additional() const
{
    return m_Additional;
}

/**
 * \brief This function provides read/write access to additional SECoP information of the object.
 * \return reference to additional SECoP information of the object
 */
nlohmann::json& CSECoPbaseType::additional()
{
    return m_Additional;
}

/**
 * \brief This function creates a new object from a SECoP data information
 *        property of an accessible.
 * \param[in] szDescription SECoP-JSON data information as string
 * \param[in] bAllowCommand true: allow type "command", false: values only
 * \return the newly created object or a nullptr on error
 */
CSECoPbaseType* CSECoPbaseType::createSECoP(const char* szDescription, bool bAllowCommand)
{
    nlohmann::json j;
    try
    {
        j = nlohmann::json::parse(szDescription);
    }
    catch (nlohmann::detail::exception&)
    {
        return nullptr;
    }
    catch (...)
    {
        return nullptr;
    }
    return CSECoPbaseType::createSECoP(j, bAllowCommand);
}

/**
 * \brief This function creates a new object from a SECoP data information
 *        property of an accessible.
 * \param[in] json          SECoP-JSON data information
 * \param[in] bAllowCommand true: allow type "command", false: values only
 * \return the newly created object or a nullptr on error
 */
CSECoPbaseType* CSECoPbaseType::createSECoP(nlohmann::json json, bool bAllowCommand)
{
    if (!json.is_object() || !json.contains("type"))
        return nullptr;
    const nlohmann::json &t(json["type"]);
    if (!t.is_string())
        return nullptr;
    CSECoPbaseType* pValue(nullptr);
    QString szType(QString::fromStdString(t.get<std::string>()));
    QStringList aszDelKeys;
    aszDelKeys.append("type");
    if (szType.compare("double", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPsimpleDouble();
    else if (szType.compare("int", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPsimpleInt();
    else if (szType.compare("bool", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPsimpleBool();
    else if (szType.compare("enum", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPsimpleEnum();
    else if (szType.compare("scaled", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPsimpleScaled();
    else if (szType.compare("string", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPstring(SECoP_VT_STRING);
    else if (szType.compare("blob", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPstring(SECoP_VT_BLOB);
    else if (szType.compare("tuple", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPtuple();
    else if (szType.compare("struct", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPstruct();
    else if (szType.compare("array", Qt::CaseInsensitive) == 0)
    {
        if (!json.contains("members"))
            return nullptr;
        const nlohmann::json &m(json["members"]);
        if (!m.is_object() || !m.contains("type"))
            return nullptr;
        const nlohmann::json &t(m["type"]);
        if (!t.is_string())
            return nullptr;
        szType = QString::fromStdString(t.get<std::string>());
        if (szType.compare("double", Qt::CaseInsensitive) == 0)
            pValue = new CSECoParrayDouble();
        else if (szType.compare("int", Qt::CaseInsensitive) == 0)
            pValue = new CSECoParrayInt();
        else if (szType.compare("bool", Qt::CaseInsensitive) == 0)
            pValue = new CSECoParrayBool();
        else if (szType.compare("enum", Qt::CaseInsensitive) == 0)
            pValue = new CSECoParrayEnum();
        else if (szType.compare("scaled", Qt::CaseInsensitive) == 0)
            pValue = new CSECoParrayScaled();
        else
            pValue = new CSECoParray();
        aszDelKeys.append("members");
    }
    else if (bAllowCommand && szType.compare("command", Qt::CaseInsensitive) == 0)
        pValue = new CSECoPcommand();
    if (pValue == nullptr)
        return nullptr;
    if (!pValue->createSECoPHelper(json, aszDelKeys))
    {
        delete pValue;
        return nullptr;
    }
    foreach (const QString &key, aszDelKeys)
        json.erase(key.toStdString());
    // TODO: check allowed additional datainfo properties
    pValue->m_Additional = json;
    return pValue;
}

/**
 * \brief This function takes SECoP-JSON and initializes the object.
 *        This function has to be overloaded.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPbaseType::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    Q_UNUSED(json);
    Q_UNUSED(aszDelKeys);
    return false;
}

/**
 * \brief This function validates and imports a SECoP-JSON value into the object.
 * \param[in] szValue SECoP-JSON value as string
 * \param[in] bStrict true: be strict, false: relax parsing
 * \return true: successful, false: not successful
 */
bool CSECoPbaseType::importSECoP(const char* szValue, bool bStrict)
{
    const void* pMeMyselfAndI(reinterpret_cast<const void*>(this));
    if (pMeMyselfAndI != nullptr && SECoP_V_g_huItems.contains(this))
    {
        nlohmann::json j;
        try
        {
            j = nlohmann::json::parse(szValue);
        }
        catch (nlohmann::detail::exception&)
        {
            return false;
        }
        catch (...)
        {
            return false;
        }
        return importSECoP(j, bStrict);
    }
    return false;
}

/**
 * \brief This function validates and imports a SECoP-JSON value into the object.
 *        This function has to be overloaded.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing
 * \return true: successful, false: not successful
 */
bool CSECoPbaseType::importSECoP(const nlohmann::json &data, bool bStrict)
{
    Q_UNUSED(data);
    Q_UNUSED(bStrict);
    return false;
}

/**
 * \brief This function creates an object and imports from a SECoP-JSON value.
 * \param[in] data SECoP-JSON value
 * \return the newly created object or a nullptr on error
 */
CSECoPbaseType* CSECoPbaseType::importSECoP(const nlohmann::json &data)
{
    if (data.is_null())
        return new CSECoPnull();
    CSECoPbaseType* pValue(nullptr);
    SECoP_V_type iType(SECoP_V_fromJSONtype(data));
try_again:
    switch (iType)
    {
        case SECoP_VT_BOOL:
            pValue = new CSECoPsimpleBool();
            break;
        case SECoP_VT_DOUBLE:
            pValue = new CSECoPsimpleDouble();
            break;
        case SECoP_VT_INTEGER:
            pValue = new CSECoPsimpleInt();
            break;
        case SECoP_VT_STRING:
            pValue = new CSECoPstring(SECoP_VT_STRING);
            break;
        case SECoP_VT_ARRAY_BOOL:
            pValue = new CSECoParrayBool();
            break;
        case SECoP_VT_ARRAY_DOUBLE:
            pValue = new CSECoParrayDouble();
            break;
        case SECoP_VT_ARRAY_INTEGER:
            pValue = new CSECoParrayInt();
            break;
        case SECoP_VT_ARRAY:
            pValue = new CSECoParray();
            break;
        case SECoP_VT_TUPLE:
            pValue = new CSECoPtuple();
            break;
        case SECoP_VT_STRUCT:
            pValue = new CSECoPstruct();
            break;
        default:
            return nullptr;
    }
    if (pValue == nullptr)
        return nullptr;
    if (!pValue->importSECoP(data, false))
    {
        delete pValue;
        pValue = nullptr;
        if (iType == SECoP_VT_ARRAY)
        {
            iType = SECoP_VT_TUPLE;
            goto try_again;
        }
    }
    return pValue;
}

/**
 * \brief This function creates an object and imports from a SECoP-JSON value.
 * \param[in] szValue SECoP-JSON value as string
 * \return the newly created object or a nullptr on error
 */
CSECoPbaseType* CSECoPbaseType::importSECoP(const char* szValue)
{
    nlohmann::json v;
    try
    {
        v = nlohmann::json::parse(szValue);
    }
    catch (nlohmann::detail::exception&)
    {
        return nullptr;
    }
    catch (...)
    {
        return nullptr;
    }
    return CSECoPbaseType::importSECoP(v);
}

/**
 * \brief This function returns the SECoP data information of this object.
 * \return SECoP data information
 */
nlohmann::json CSECoPbaseType::exportType() const
{
    nlohmann::json json;
    if (!exportTypeHelper(json, false))
        return nlohmann::json();
    return json;
}

/**
 * \brief This helper function returns the SECoP data information of this object.
 *        This function has to be overloaded.
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPbaseType::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    if (!bArray)
        json = m_Additional;
    switch (m_iType)
    {
        case SECoP_VT_DOUBLE:
            json["type"] = std::string("double");
            break;
        case SECoP_VT_INTEGER:
            json["type"] = std::string("int");
            break;
        case SECoP_VT_BOOL:
            json["type"] = std::string("bool");
            break;
        case SECoP_VT_ENUM:
            json["type"] = std::string("enum");
            break;
        case SECoP_VT_SCALED:
            json["type"] = std::string("scaled");
            break;
        case SECoP_VT_ARRAY:
        case SECoP_VT_ARRAY_DOUBLE:
        case SECoP_VT_ARRAY_INTEGER:
        case SECoP_VT_ARRAY_BOOL:
        case SECoP_VT_ARRAY_ENUM:
        case SECoP_VT_ARRAY_SCALED:
            json["type"] = std::string("array");
            break;
        case SECoP_VT_STRING:
            json["type"] = std::string("string");
            break;
        case SECoP_VT_BLOB:
            json["type"] = std::string("blob");
            break;
        case SECoP_VT_STRUCT:
            json["type"] = std::string("struct");
            break;
        case SECoP_VT_TUPLE:
            json["type"] = std::string("tuple");
            break;
        case SECoP_VT_COMMAND:
            json["type"] = std::string("command");
            break;
        default: // SECoP_VT_NONE, SECoP_VT_JSON
            return false;
    }
    return true;
}

/**
 * \brief This function exports the value as SECoP value.
 *        This function has to be overloaded.
 * \return SECoP value
 */
nlohmann::json CSECoPbaseType::exportSECoPjson() const
{
    return nlohmann::json();
}

/**
 * \brief This function exports the value as SECoP value.
 * \param[in] bNull true: use "null" for errors, false: use empty string for errors
 * \return SECoP value as JSON string
 */
QByteArray CSECoPbaseType::exportSECoP(bool bNull) const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    QByteArray abyResult;
    if (pMeMyselfAndI != nullptr && SECoP_V_g_huItems.contains(this))
        abyResult = QByteArray::fromStdString(exportSECoPjson().dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
    if (bNull && abyResult.isEmpty())
        abyResult = "null";
    return abyResult;
}

/**
 * \brief Convenience function to create a simple boolean with value.
 * \param[in] bValue value for boolean
 * \return newly created object
 */
SECoP_dataPtr CSECoPbaseType::simpleBool(bool bValue)
{
    CSECoPsimpleBool* pBool(new CSECoPsimpleBool());
    if (pBool != nullptr)
        pBool->setValue(bValue);
    return SECoP_dataPtr(pBool);
}

/**
 * \brief Convenience function to create a simple double with value.
 * \param[in] dblValue value for double
 * \return newly created object
 */
SECoP_dataPtr CSECoPbaseType::simpleDouble(double dblValue)
{
    CSECoPsimpleDouble* pDouble(new CSECoPsimpleDouble());
    if (pDouble != nullptr)
        pDouble->setValue(dblValue);
    return SECoP_dataPtr(pDouble);
}

/**
 * \brief Convenience function to create a simple integer with value.
 * \param[in] llValue value for integer
 * \return newly created object
 */
SECoP_dataPtr CSECoPbaseType::simpleInteger(long long llValue)
{
    CSECoPsimpleDouble* pInt(new CSECoPsimpleDouble());
    if (pInt != nullptr)
        pInt->setValue(llValue);
    return SECoP_dataPtr(pInt);
}

/**
 * \brief Convenience function to create a simple string with value.
 * \param[in] szValue value for string
 * \return newly created object
 */
SECoP_dataPtr CSECoPbaseType::simpleString(const char* szValue)
{
    CSECoPstring* pString(new CSECoPstring(SECoP_VT_STRING));
    if (pString != nullptr)
        pString->setValue(QByteArray(szValue));
    return SECoP_dataPtr(pString);
}

/**
 * \brief Convenience function to create a simple string with value.
 * \param[in] szValue value for string
 * \return newly created object
 */
SECoP_dataPtr CSECoPbaseType::simpleString(std::string szValue)
{
    CSECoPstring* pString(new CSECoPstring(SECoP_VT_STRING));
    if (pString != nullptr)
        pString->setValue(QByteArray::fromStdString(szValue));
    return SECoP_dataPtr(pString);
}

/**
 * \brief Convenience function to create a simple JSON string with value.
 * \param[in] szValue value for string
 * \return newly created object
 */
SECoP_dataPtr CSECoPbaseType::simpleJSON(const char* szValue)
{
    CSECoPstring* pString(new CSECoPstring(SECoP_VT_JSON));
    if (pString != nullptr)
        pString->setValue(QByteArray(szValue));
    return SECoP_dataPtr(pString);
}

/**
 * \brief Convenience function to create a simple JSON string with value.
 * \param[in] szValue value for string
 * \return newly created object
 */
SECoP_dataPtr CSECoPbaseType::simpleJSON(std::string szValue)
{
    CSECoPstring* pString(new CSECoPstring(SECoP_VT_JSON));
    if (pString != nullptr)
        pString->setValue(QByteArray::fromStdString(szValue));
    return SECoP_dataPtr(pString);
}

/*****************************************************************************
 * CSECoPnull
 *****************************************************************************/
/**
 * \brief Standard constructor for a null value.
 */
CSECoPnull::CSECoPnull()
    : CSECoPbaseType(SECoP_VT_NONE)
{
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPnull::CSECoPnull(const CSECoPnull* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPnull::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoPnull*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPnull::compareValue(const CSECoPbaseType* pOther) const
{
    Q_UNUSED(pOther);
    return SECoP_VC_DIFF_VALUE;
}

/**
 * \brief This overloaded function "checks", if the object is valid and usable.
 * \return true: valid object
 */
bool CSECoPnull::isValid() const
{
    return true;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful
 */
bool CSECoPnull::clear()
{
    return true;
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing
 * \return true: successful, false: not successful
 */
bool CSECoPnull::importSECoP(const nlohmann::json &data, bool bStrict)
{
    Q_UNUSED(bStrict);
    return data.is_null();
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        Because, SECoP has no "null" data type, this returns an error in any case.
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return false: not successful
 */
bool CSECoPnull::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    Q_UNUSED(json);
    Q_UNUSED(bArray);
    return false;
}

/**
 * \brief This function exports the value as SECoP value.
 *        This function has to be overloaded.
 * \return SECoP value
 */
nlohmann::json CSECoPnull::exportSECoPjson() const
{
    return nlohmann::json();
}

/*****************************************************************************
 * CSECoPsimpleType
 *****************************************************************************/
/**
 * \brief Standard constructor for a simple type. This is a template class
 *        for the C/C++ types double and long long aka int64.
 */
template <typename T> CSECoPsimpleType<T>::CSECoPsimpleType()
    : CSECoPbaseType(SECoP_VT_NONE)
{
    if (std::numeric_limits<T>::has_quiet_NaN)
        m_value = std::numeric_limits<T>::quiet_NaN();
    else
        m_value = static_cast<T>(0);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
template <typename T> CSECoPsimpleType<T>::CSECoPsimpleType(const CSECoPsimpleType<T>* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , m_value(pOther->m_value)
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
template <typename T> bool CSECoPsimpleType<T>::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoPsimpleType<T>*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
template <typename T> SECoP_V_compareResult CSECoPsimpleType<T>::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE)
    {
        if (!compareType(pOther))
            iResult = SECoP_VC_DIFF_TYPE;
        else if (std::numeric_limits<T>::is_exact && dynamic_cast<const CSECoPsimpleType<T>*>(pOther)->m_value != m_value)
            iResult = SECoP_VC_DIFF_VALUE;
        else if (std::abs(m_value - dynamic_cast<const CSECoPsimpleType<T>*>(pOther)->m_value) > 0.0)
            iResult = SECoP_VC_DIFF_VALUE;
    }
    return iResult;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful
 */
template <typename T> bool CSECoPsimpleType<T>::clear()
{
    if (std::numeric_limits<T>::has_quiet_NaN)
        m_value = std::numeric_limits<T>::quiet_NaN();
    else
        m_value = static_cast<T>(0);
    return true;
}

/**
 * \brief This function reads the current stored value.
 * \param[out] value current value output
 * \return true: successful
 */
template <typename T> bool CSECoPsimpleType<T>::getValue(T &value) const
{
    value = m_value;
    return true;
}

/**
 * \brief This function stores the value.
 * \param[in] value new value to store
 * \return true: successful
 */
template <typename T> bool CSECoPsimpleType<T>::setValue(const T value)
{
    m_value = value;
    return true;
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoPsimpleType<T>::importSECoP(const nlohmann::json &data, bool bStrict)
{
    Q_UNUSED(bStrict);
    if (data.is_null())
        return true;
    if (data.is_number())
    {
        if (data.is_number_unsigned())
        {
            m_value = static_cast<T>(data.get<std::uint64_t>());
            return true;
        }
        if (data.is_number_integer())
        {
            m_value = static_cast<T>(data.get<std::int64_t>());
            return true;
        }
        m_value = static_cast<T>(data.get<double>());
        return true;
    }
    if (data.is_string())
    {
#pragma message(CPP_WARNINGPREFIX "TODO: allow -inf +inf nan?")
        QString sValue(QString::fromStdString(data.get<std::string>()));
        if (std::numeric_limits<T>::has_infinity)
        {
            if (sValue.compare("inf", Qt::CaseInsensitive) == 0 ||
                sValue.compare("+inf", Qt::CaseInsensitive) == 0)
            {
                m_value = std::numeric_limits<T>::infinity();
                return true;
            }
            if (sValue.compare("-inf", Qt::CaseInsensitive) == 0)
            {
                m_value = -std::numeric_limits<T>::infinity();
                return true;
            }
        }
        if (std::numeric_limits<T>::has_quiet_NaN && sValue.compare("nan", Qt::CaseInsensitive) == 0)
        {
            m_value = std::numeric_limits<T>::quiet_NaN();
            return true;
        }
    }
    return false;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
template <typename T> nlohmann::json CSECoPsimpleType<T>::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    if (pMeMyselfAndI == nullptr)
        return nlohmann::json();
    return nlohmann::json(m_value);
}

/*****************************************************************************
 * CSECoPminmaxType
 *****************************************************************************/
/**
 * \brief Standard constructor for minimum and maximum value of a simple type.
 *        This is a template class for the C/C++ types double and long long aka int64.
 */
template <typename T> CSECoPminmaxType<T>::CSECoPminmaxType()
    : CSECoPbaseType(SECoP_VT_NONE)
{
    if (std::numeric_limits<T>::has_quiet_NaN)
    {
        m_minimum = std::numeric_limits<T>::quiet_NaN();
        m_maximum = std::numeric_limits<T>::quiet_NaN();
    }
    else
    {
        m_minimum = std::numeric_limits<T>::lowest();
        m_maximum = std::numeric_limits<T>::max();
    }
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
template <typename T> CSECoPminmaxType<T>::CSECoPminmaxType(const CSECoPminmaxType<T>* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , m_minimum(pOther->m_minimum)
    , m_maximum(pOther->m_maximum)
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 *        The minimum and maximum have to be the same (including NaN).
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
template <typename T> bool CSECoPminmaxType<T>::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    const CSECoPminmaxType<T>* pTmp(dynamic_cast<const CSECoPminmaxType<T>*>(pOther));
    if (pTmp == nullptr)
        return false;
    if (std::numeric_limits<T>::has_quiet_NaN)
    {
        if (std::isnan(static_cast<double>(m_minimum)))
        {
            if (!std::isnan(static_cast<double>(pTmp->m_minimum)))
                return false;
        }
        else
        {
            if (std::isnan(static_cast<double>(pTmp->m_minimum)))
                return false;
            if (fabs(pTmp->m_minimum - m_minimum) > 0.0)
                return false;
        }
        if (std::isnan(static_cast<double>(m_maximum)))
        {
            if (!std::isnan(static_cast<double>(pTmp->m_maximum)))
                return false;
        }
        else
        {
            if (std::isnan(static_cast<double>(pTmp->m_maximum)))
                return false;
            if (fabs(pTmp->m_maximum - m_maximum) > 0.0)
                return false;
        }
        return true;
    }
    else if (std::numeric_limits<T>::is_exact)
        return (std::abs(pTmp->m_minimum - m_minimum) <= 0 && std::abs(pTmp->m_maximum - m_maximum) <= 0);
    else
        return !((fabs(m_minimum - pTmp->m_minimum) > 0.0 || fabs(m_maximum - pTmp->m_maximum) > 0.0));
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
template <typename T> SECoP_V_compareResult CSECoPminmaxType<T>::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE && !compareType(pOther))
        iResult = SECoP_VC_DIFF_TYPE;
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
template <typename T> bool CSECoPminmaxType<T>::isValid() const
{
    if (!CSECoPbaseType::isValid())
        return false;
    if (std::numeric_limits<T>::has_quiet_NaN)
        if (std::isnan(static_cast<double>(m_minimum)) || std::isnan(static_cast<double>(m_maximum)))
            return true;
    return m_minimum <= m_maximum;
}

/**
 * \brief This function checks, if the given value is valid and usable.
 * \param[in] value value to check
 * \return true: valid value, false: otherwise
 */
template <typename T> bool CSECoPminmaxType<T>::isValid(T value) const
{
    if (std::numeric_limits<T>::has_quiet_NaN)
    {
        if (!std::isnan(static_cast<double>(m_minimum)) && value < m_minimum)
            return false;
        if (!std::isnan(static_cast<double>(m_maximum)) && value > m_maximum)
            return false;
    }
    else
    {
        if (value < m_minimum)
            return false;
        if (value > m_maximum)
            return false;
    }
    return true;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful
 */
template <typename T> bool CSECoPminmaxType<T>::clear()
{
    if (std::numeric_limits<T>::has_quiet_NaN)
        m_minimum = m_maximum = std::numeric_limits<T>::quiet_NaN();
    else
    {
        m_minimum = std::numeric_limits<T>::lowest();
        m_maximum = std::numeric_limits<T>::max();
    }
    return true;
}

/**
 * \brief This function reads the allowed minimum and maximum values.
 * \param[out] minimum minimum value
 * \param[out] maximum maximum value
 */
template <typename T> void CSECoPminmaxType<T>::getMinMaxValue(T &minimum, T &maximum) const
{
    minimum = m_minimum;
    maximum = m_maximum;
}

/**
 * \brief This function stores new allowed minimum and maximum values.
 *        The minimum has to be lower or equal the maximum value or
 *        if the data type allows it, use NaN.
 * \param[in] minimum minimum value
 * \param[in] maximum maximum value
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoPminmaxType<T>::setMinMaxValue(T minimum, T maximum)
{
    if (!std::numeric_limits<T>::has_quiet_NaN ||
        (!std::isnan(static_cast<double>(minimum)) && !std::isnan(static_cast<double>(maximum))))
        if (minimum > maximum)
            return false;
    m_minimum = minimum;
    m_maximum = maximum;
    return true;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        It takes the keys "min" and "max".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoPminmaxType<T>::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    Q_UNUSED(aszDelKeys);
    T aItems[2];
    const char* aNames[2] = {"min", "max"};
    if (std::numeric_limits<T>::has_quiet_NaN)
        aItems[0] = aItems[1] = std::numeric_limits<T>::quiet_NaN();
    else
    {
        aItems[0] = std::numeric_limits<T>::lowest();
        aItems[1] = std::numeric_limits<T>::max();
    }
    for (int i = 0; i < 2; ++i)
    {
        double d;
        if (!json.contains(aNames[i]))
            continue;
        nlohmann::json v(json[aNames[i]]);
        if (v.is_null())
            continue;
        if (!v.is_number())
            return false;
        CSECoPscaledBase* pScaled(dynamic_cast<CSECoPscaledBase*>(this));
        if (pScaled == nullptr || fabs(pScaled->getScale() - 1.0) <= 0.0)
        {
            if (v.is_number_unsigned())
            {
                aItems[i] = static_cast<T>(v.get<std::uint64_t>());
                goto erase_it;
            }
            else if (v.is_number_integer())
            {
                aItems[i] = static_cast<T>(v.get<std::int64_t>());
                goto erase_it;
            }
        }
        d = v.get<double>();
        if (pScaled != nullptr)
            aItems[i] = static_cast<T>(d / pScaled->getScale());
        else
            aItems[i] = static_cast<T>(d);
erase_it:
        json.erase(aNames[i]);
    }
    return setMinMaxValue(aItems[0], aItems[1]);
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the keys "min" and "max".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoPminmaxType<T>::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    if (!CSECoPbaseType::exportTypeHelper(json, bArray))
        return false;
    if (std::numeric_limits<T>::has_quiet_NaN)
    {
        if (std::isfinite(static_cast<double>(m_minimum)))
            json["min"] = nlohmann::json(m_minimum);
        if (std::isfinite(static_cast<double>(m_maximum)))
            json["max"] = nlohmann::json(m_maximum);
    }
    else
    {
        json["min"] = nlohmann::json(m_minimum);
        json["max"] = nlohmann::json(m_maximum);
    }
    return true;
}

/*****************************************************************************
 * CSECoParrayBase
 *****************************************************************************/
/**
 * \brief Constructor for an array like class.
 */
CSECoParrayBase::CSECoParrayBase(SECoP_V_type iType)
    : CSECoPbaseType(iType)
    , m_uSize(0U)
    , m_uMinSize(0U)
    , m_uMaxSize(UINT_MAX)
{
    setType(iType);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoParrayBase::CSECoParrayBase(const CSECoParrayBase* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , m_uSize(pOther->m_uSize)
    , m_uMinSize(pOther->m_uMinSize)
    , m_uMaxSize(pOther->m_uMaxSize)
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 *        The minimum and maximum size have to be the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoParrayBase::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    const CSECoParrayBase* pTmp(dynamic_cast<const CSECoParrayBase*>(pOther));
    if (pTmp == nullptr)
        return false;
    return (m_uMinSize == pTmp->m_uMinSize && m_uMaxSize == pTmp->m_uMaxSize);
}

/**
 * \brief This overloaded function compares two objects.
 *        The current array size have to be equal.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoParrayBase::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE)
    {
        if (!compareType(pOther))
            iResult = SECoP_VC_DIFF_TYPE;
        else if (iResult == SECoP_VC_EQUAL || iResult == SECoP_VC_SIMILAR_VALUE)
        {
            const CSECoParrayBase* pTmp(dynamic_cast<const CSECoParrayBase*>(pOther));
            if (m_uSize != pTmp->m_uSize)
                iResult = SECoP_VC_DIFF_VALUE;
        }
    }
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoParrayBase::isValid() const
{
    if (!CSECoPbaseType::isValid())
        return false;
    return (m_uMinSize <= m_uSize && m_uSize <= m_uMaxSize);
}

/**
 * \brief This function reads the allowed minimum and maximum array size.
 * \param[out] uMinimum minimum array size
 * \param[out] uMaximum maximum array size
 */
void CSECoParrayBase::getMinMaxSize(unsigned int &uMinimum, unsigned int &uMaximum) const
{
    uMinimum = m_uMinSize;
    uMaximum = m_uMaxSize;
}

/**
 * \brief This function stores new allowed minimum and maximum array size.
 *        The minimum array size has to be lower or equal the maximum array size.
 * \param[in] uMinimum minimum array size
 * \param[in] uMaximum maximum array size
 * \return true: successful, false: not successful
 */
bool CSECoParrayBase::setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum)
{
    return setMinMaxSize(uMinimum, uMaximum, false);
}

/**
 * \brief This function stores new allowed minimum and maximum array size and
 *        optionally resizes the content (ignored in this base class).
 *        The minimum array size has to be lower or equal the maximum array size.
 * \param[in] uMinimum      minimum array size
 * \param[in] uMaximum      maximum array size
 * \param[in] bForceRealloc true: force a reallocation of memory, false: no reallocation
 * \return true: successful, false: not successful
 */
bool CSECoParrayBase::setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum, bool bForceRealloc)
{
    Q_UNUSED(bForceRealloc);
    if (uMinimum > uMaximum)
        return false;
    m_uMinSize = uMinimum;
    m_uMaxSize = uMaximum;
    return true;
}

/**
 * \brief This function returns the current array size.
 * \return current array size
 */
unsigned int CSECoParrayBase::getSize() const
{
    return m_uSize;
}

/**
 * \brief This function changes the array size in the allowed ranges.
 * \param[in] uSize new array size
 * \return true: successful, false: not successful
 */
bool CSECoParrayBase::setSize(unsigned int uSize)
{
    if (m_uMinSize <= uSize && uSize <= m_uMaxSize)
    {
        m_uSize = uSize;
        return true;
    }
    else
        return false;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        Depending on internal type, it takes the keys
 *        - SECoP_VT_STRING:  "minchars" and "maxchars",
 *        - SECoP_VT_BLOB:    "minbytes" and "maxbytes",
 *        - other arrays:     "minlen" and "maxlen".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoParrayBase::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    Q_UNUSED(aszDelKeys);
    std::vector<std::string> aNames;
    unsigned int auItems[2];
    switch (getType())
    {
        case SECoP_VT_STRING:
            aNames.push_back("minchars");
            aNames.push_back("maxchars");
            break;
        case SECoP_VT_BLOB:
            aNames.push_back("minbytes");
            aNames.push_back("maxbytes");
            break;
        default:
            aNames.push_back("minlen");
            aNames.push_back("maxlen");
            break;
    }
    auItems[0] = 0U;
    auItems[1] = UINT_MAX;
    for (unsigned int i = 0; i < 2; ++i)
    {
        if (!json.contains(aNames[i]))
            continue;
        nlohmann::json v(json[aNames[i]]);
        if (!v.is_number_unsigned())
            return false;
        std::uint64_t j(v.get<std::uint64_t>());
        if (j > UINT_MAX)
            return false;
        auItems[i] = static_cast<unsigned int>(j);
        json.erase(aNames[i]);
    }
    return setMinMaxSize(auItems[0], auItems[1], true);
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        Depending on internal type, it puts the keys
 *        - SECoP_VT_STRING:  "minchars",
 *        - SECoP_VT_BLOB:    "minbytes" and "maxbytes",
 *        - other arrays:     "minlen" and "maxlen".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoParrayBase::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    if (!CSECoPbaseType::exportTypeHelper(json, bArray))
        return false;
    std::vector<std::string> aNames;
    bool bForce(false);
    unsigned int uMaxSize(UINT_MAX);
    switch (getType())
    {
        case SECoP_VT_STRING:
            aNames.push_back("minchars");
            aNames.push_back("");
            --uMaxSize;
            break;
        case SECoP_VT_BLOB:
            aNames.push_back("minbytes");
            aNames.push_back("maxbytes");
            --uMaxSize;
            bForce = true;
            break;
        case SECoP_VT_JSON:
            --uMaxSize;
            break;
        default:
            aNames.push_back("minlen");
            aNames.push_back("maxlen");
            break;
    }
    if (m_uMinSize > 0 && !aNames[0].empty())
        json[aNames[0]] = nlohmann::json(static_cast<qint64>(m_uMinSize));
    if ((bForce || m_uMaxSize < uMaxSize) && !aNames[1].empty())
        json[aNames[1]] = nlohmann::json(static_cast<qint64>(m_uMaxSize));
    return true;
}

/*****************************************************************************
 * CSECoParraySimple
 *****************************************************************************/
/**
 * \brief Standard constructor for a simple array type. This is a template class
 *        for the C/C++ types double and long long aka int64. It is also used for
 *        storing a string or blob with unsigned bytes.
 */
template <typename T> CSECoParraySimple<T>::CSECoParraySimple()
    : CSECoParrayBase(SECoP_VT_NONE)
    , m_bAdditional(false)
    , m_pData(nullptr)
{
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
template <typename T> CSECoParraySimple<T>::CSECoParraySimple(const CSECoParraySimple<T>* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoParrayBase(static_cast<const CSECoParrayBase*>(pOther))
    , m_bAdditional(pOther->m_bAdditional)
    , m_pData(nullptr)
{
    if (setMinMaxSize(m_uMinSize, m_uMaxSize, true))
        if (m_pData != nullptr && pOther->m_pData != nullptr && m_uSize > 0)
            memmove(m_pData, pOther->m_pData, m_uSize * sizeof(T));
}

/**
 * \brief Destructor for simple array type. It frees the allocated memory.
 */
template <typename T> CSECoParraySimple<T>::~CSECoParraySimple()
{
    if (m_pData != nullptr)
        delete[] m_pData;
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
template <typename T> bool CSECoParraySimple<T>::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoParrayBase::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoParraySimple<T>*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
template <typename T> SECoP_V_compareResult CSECoParraySimple<T>::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoParrayBase::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE)
    {
        if (!compareType(pOther))
            iResult = SECoP_VC_DIFF_TYPE;
        else if (iResult == SECoP_VC_EQUAL || iResult == SECoP_VC_SIMILAR_VALUE)
        {
            const CSECoParraySimple<T>* pTmp(dynamic_cast<const CSECoParraySimple<T>*>(pOther));
            if (memcmp(m_pData, pTmp->m_pData, m_uSize * sizeof(T)) != 0)
                iResult = SECoP_VC_DIFF_VALUE;
        }
    }
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object
 */
template <typename T> bool CSECoParraySimple<T>::isValid() const
{
    if (!CSECoParrayBase::isValid())
        return false;
    if (m_uSize > 0 && m_pData == nullptr)
        return false;
    return true;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::clear()
{
    if (m_pData != nullptr)
    {
        delete[] m_pData;
        m_pData = nullptr;
    }
    m_uSize = 0;
    return setMinMaxSize(m_uMinSize, m_uMaxSize, true);
}

/**
 * \brief This function changes the array size in the allowed ranges.
 * \param[in] uSize new array size
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::setSize(unsigned int uSize)
{
    if (!CSECoParrayBase::setSize(uSize))
        return false;
    return setMinMaxSize(m_uMinSize, m_uMaxSize, true); // reallocate memory
}

/**
 * \brief This function stores new allowed minimum and maximum array size and
 *        optionally resizes the content (ignored in this base class).
 *        The minimum array size has to be lower or equal the maximum array size.
 * \param[in] uMinimum      minimum array size
 * \param[in] uMaximum      maximum array size
 * \param[in] bForceRealloc true: force a reallocation of memory, false: no reallocation
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum, bool bForceRealloc)
{
    unsigned int uSize(m_uSize), uAdditional(m_bAdditional ? 1 : 0), uNewMax(uMaximum);
    if (uMinimum == m_uMinSize && uMaximum == m_uMaxSize && m_pData != nullptr && !bForceRealloc)
        return true;
    if ((static_cast<unsigned long long>(uMaximum) + uAdditional) > UINT_MAX)
    {
        if (uAdditional >= (UINT_MAX / 2))
            return false;
        uNewMax = uMaximum = UINT_MAX - uAdditional;
    }
    if (!CSECoParrayBase::setMinMaxSize(uMinimum, uMaximum, bForceRealloc))
        return false;
    if (uNewMax >= (UINT_MAX >> 4))
        uNewMax = uSize + uAdditional;
    else
        uNewMax = uMaximum + uAdditional;
    T* pData(nullptr);
    if (uNewMax > 0)
    {
        pData = new T[uNewMax];
        if (pData == nullptr)
            return false;
    }
    if (m_uSize > uNewMax)
        m_uSize = uNewMax;
    if (m_pData != nullptr)
    {
        if (m_uSize > 0)
            memmove(pData, m_pData, m_uSize * sizeof(T));
    }
    else
        uSize = 0U;
    delete[] m_pData;
    m_pData = pData;
    if (m_pData != nullptr)
    {
        if (std::numeric_limits<T>::has_quiet_NaN)
            for (unsigned i = uSize; i < uNewMax; ++i)
                m_pData[i] = std::numeric_limits<T>::quiet_NaN();
        else
            for (unsigned i = uSize; i < uNewMax; ++i)
                m_pData[i] = static_cast<T>(0);
    }
    return true;
}

/**
 * \brief This function reads the current stored value at a given index.
 * \param[in]  uIndex array index
 * \param[out] value  current value output
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::getValue(unsigned int uIndex, T &value) const
{
    if (uIndex >= m_uSize && m_pData != nullptr)
        return false;
    value = m_pData[uIndex];
    return true;
}

/**
 * \brief This function returns a pointer to array data.
 * \return array data pointer (read only)
 */
template <typename T> const T* CSECoParraySimple<T>::getArray() const
{
    return m_pData;
}

/**
 * \brief This function stores the value at a given index.
 * \param[in] uIndex array index
 * \param[in] value  new value to store
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::setValue(unsigned int uIndex, const T value)
{
    if (!setMinMaxSize(m_uMinSize, m_uMaxSize, false))
        return false;
    if (uIndex >= m_uSize || uIndex >= m_uMaxSize || m_pData == nullptr)
        return false;
    m_pData[uIndex] = value;
    return true;
}

/**
 * \brief This function (re)allocates memory to store the number of items and
 *        makes a deep copy of the data.
 * \param[in] pData  source array to copy
 * \param[in] uItems size of array
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::setArray(const T* pData, unsigned int uItems)
{
    unsigned int uNewMax(m_uMaxSize);
    if (uNewMax < uItems)
        uNewMax = uItems;
    if (!setSize(uItems))
        return false;
    if (!setMinMaxSize(m_uMinSize, uNewMax, true) || uItems < m_uMinSize)
        return false;
    memmove(m_pData, reinterpret_cast<const char*>(pData), uItems * sizeof(T));
    if (m_bAdditional)
    {
        if (std::numeric_limits<T>::has_quiet_NaN)
            m_pData[m_uSize] = std::numeric_limits<T>::quiet_NaN();
        else
            m_pData[m_uSize] = static_cast<T>(0);
    }
    return true;
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] value new value to append at end
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::appendValue(const T value)
{
    if (m_pData == nullptr)
        if (!setMinMaxSize(m_uMinSize, m_uMaxSize, true))
            return false;
    unsigned int uIndex(m_uSize);
    if (!setSize(m_uSize + 1U))
        return false;
    m_pData[uIndex] = value;
    return true;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        It takes the key "members".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoParrayBase::createSECoPHelper(json, aszDelKeys))
        return false;
    if (!json.contains("members"))
        return false;
    const nlohmann::json &v(json["members"]);
    if (!v.is_object() || !v.contains("type"))
        return false;
    if (!aszDelKeys.contains("members"))
        aszDelKeys.append("members");
    for (auto &it : v.items())
        if (QString::fromStdString(it.key()).compare("type", Qt::CaseInsensitive) != 0)
            json[it.key()] = it.value();
    if (m_uMinSize > 0)
        if (!setSize(m_uMinSize))
            return false;
    return true;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
template <typename T> bool CSECoParraySimple<T>::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    return CSECoParrayBase::exportTypeHelper(json, bArray);
}

/*****************************************************************************
 * CSECoPenumBase
 *****************************************************************************/
/**
 * \brief Standard constructor for enumeration class.
 */
CSECoPenumBase::CSECoPenumBase()
    : CSECoPbaseType(SECoP_VT_ENUM)
{
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPenumBase::CSECoPenumBase(const CSECoPenumBase* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
{
    m_aItems = pOther->m_aItems;
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 *        All enumeration items have to match in name and value.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPenumBase::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    const CSECoPenumBase* pTmp(dynamic_cast<const CSECoPenumBase*>(pOther));
    if (pTmp == nullptr)
        return false;
    if (m_aItems.size() != pTmp->m_aItems.size() || !isValid())
        return false;
    for (int i = 0; i < m_aItems.size(); ++i)
    {
        const internal* p1(&m_aItems[i]);
        int j;
        for (j = 0; j < pTmp->m_aItems.size(); ++j)
        {
            const internal* p2(&pTmp->m_aItems[j]);
            if (p1->llValue == p2->llValue)
            {
                if (p1->szName != p2->szName)
                    return false;
                break;
            }
        }
        if (j >= pTmp->m_aItems.size())
            return false;
    }
    return true;
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPenumBase::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE && !compareType(pOther))
        iResult = SECoP_VC_DIFF_TYPE;
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPenumBase::isValid() const
{
    if (!CSECoPbaseType::isValid())
        return false;
    for (int i = 0; i < m_aItems.size(); ++i)
    {
        const internal* p1(&m_aItems[i]);
        if (p1->szName.isEmpty())
            return false;
        for (int j = i + 1; j < m_aItems.size(); ++j)
        {
            const internal* p2(&m_aItems[j]);
            if (p1->llValue == p2->llValue)
                return false;
        }
    }
    return true;
}

/**
 * \brief This function checks, if the given value is valid and usable.
 * \param[in] llValue value to check
 * \return true: valid value, false: otherwise
 */
bool CSECoPenumBase::isValid(const long long llValue) const
{
    if (!CSECoPbaseType::isValid())
        return false;
    for (auto it = m_aItems.constBegin(); it != m_aItems.constEnd(); ++it)
        if (it->llValue == llValue)
            return true;
    return false;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful
 */
bool CSECoPenumBase::clear()
{
    m_aItems.clear();
    return true;
}

/**
 * \brief This function returns the number of enumerations.
 * \return number of enumerations
 */
unsigned int CSECoPenumBase::getItemCount() const
{
    return static_cast<unsigned int>(m_aItems.size());
}

/**
 * \brief This function returns the name to a given enumeration index.
 * \param[in] uIndex index of enumeration (0...no-1)
 * \return zero terminated string pointer to name or nullptr
 */
const char* CSECoPenumBase::getItemName(unsigned int uIndex) const
{
    int iIndex(static_cast<int>(uIndex));
    if (iIndex >= m_aItems.size())
        return nullptr;
    return m_aItems[iIndex].szName.constData();
}

/**
 * \brief This function returns the value to a given enumeration index.
 * \param[in] uIndex index of enumeration (0...no-1)
 * \return value or zero
 */
long long CSECoPenumBase::getItemValue(unsigned int uIndex) const
{
    int iIndex(static_cast<int>(uIndex));
    if (iIndex >= m_aItems.size())
        return 0ULL;
    return m_aItems[iIndex].llValue;
}

/**
 * \brief This function adds an enumeration.
 * \param[in] llValue value of enumeration
 * \param[in] szName  name of enumeration (zero terminated string pointer)
 * \return true: successful, false: not successful
 */
bool CSECoPenumBase::addItem(long long llValue, const char* szName)
{
    if (szName == nullptr || szName[0] == '\0')
        return false;
    if (isValid(llValue))
        return false;
    internal i;
    i.llValue = llValue;
    i.szName  = szName;
    m_aItems.push_back(i);
    return true;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        It takes the key "members".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPenumBase::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!json.contains("members"))
        return false;
    const nlohmann::json &vMembers(json["members"]);
    if (!vMembers.is_object())
        return false;
    m_aItems.clear();
    for (auto &it : vMembers.items())
    {
        const nlohmann::json &v(it.value());
        if (!v.is_number_integer())
            return false;
        if (!addItem(static_cast<long long>(v.get<std::int64_t>()), it.key().c_str()))
            return false;
    }
    if (!aszDelKeys.contains("members"))
        aszDelKeys.append("members");
    return true;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPenumBase::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    if (!CSECoPbaseType::exportTypeHelper(json, bArray))
        return false;
    nlohmann::json m;
    for (int i = 0; i < m_aItems.size(); ++i)
    {
        const internal &p(m_aItems[i]);
        m[p.szName.constData()] = nlohmann::json(p.llValue);
    }
    json["members"] = m;
    return true;
}

/**
 * \brief Standard constructor of the internal enumeration list.
 */
CSECoPenumBase::internal::internal()
    : llValue(0ULL)
{
}

/**
 * \brief Copy constructor of the internal enumeration list.
 */
CSECoPenumBase::internal::internal(const CSECoPenumBase::internal &src)
    : llValue(src.llValue)
    , szName(src.szName)
{
}

/**
 * \brief Assignment operator of the internal enumeration list.
 */
CSECoPenumBase::internal& CSECoPenumBase::internal::operator=(const CSECoPenumBase::internal &src)
{
    llValue = src.llValue;
    szName = src.szName;
    return *this;
}

/*****************************************************************************
 * CSECoPscaledBase
 *****************************************************************************/
/**
 * \brief Standard constructor for scale value.
 */
CSECoPscaledBase::CSECoPscaledBase()
    : CSECoPbaseType(SECoP_VT_SCALED)
    , m_dScale(1.0)
{
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPscaledBase::CSECoPscaledBase(const CSECoPscaledBase* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , m_dScale(pOther->m_dScale)
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 *        The scale has to be the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPscaledBase::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    const CSECoPscaledBase* pTmp(dynamic_cast<const CSECoPscaledBase*>(pOther));
    if (pTmp == nullptr || !std::isfinite(m_dScale) || !std::isfinite(pTmp->m_dScale))
        return false;
    return !(fabs(m_dScale - pTmp->m_dScale) > 0.0);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPscaledBase::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE && !compareType(pOther))
        iResult = SECoP_VC_DIFF_TYPE;
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPscaledBase::isValid() const
{
    if (!CSECoPbaseType::isValid())
        return false;
    return std::isfinite(m_dScale) && (m_dScale > 0.0);
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful
 */
bool CSECoPscaledBase::clear()
{
    m_dScale = 1.0;
    return true;
}

/**
 * \brief This function reads the scale for values.
 * \return scale value
 */
double CSECoPscaledBase::getScale() const
{
    return m_dScale;
}

/**
 * \brief This function changes the scale for values.
 * \param[in] dFactor scale value
 * \return true: successful, false: not successful
 */
bool CSECoPscaledBase::setScale(double dFactor)
{
    if (!std::isfinite(dFactor) || dFactor <= 0.0)
        return false;
    m_dScale = dFactor;
    return true;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        It takes the key "scale".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPscaledBase::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!json.contains("scale"))
        return false;
    nlohmann::json scale(json["scale"]);
    if (!scale.is_number())
        return false;
    m_dScale = scale.get<double>();
    if (!aszDelKeys.contains("scale"))
        aszDelKeys.append("scale");
    return true;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "scale".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPscaledBase::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    if (!CSECoPbaseType::exportTypeHelper(json, bArray))
        return false;
    json["scale"] = nlohmann::json(m_dScale);
    return true;
}

/*****************************************************************************
 * CSECoPsimpleBool
 *****************************************************************************/
/**
 * \brief Standard constructor for a boolean value.
 */
CSECoPsimpleBool::CSECoPsimpleBool()
    : CSECoPsimpleType<long long>()
{
    setType(SECoP_VT_BOOL);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPsimpleBool::CSECoPsimpleBool(const CSECoPsimpleBool* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoPsimpleType<long long>(static_cast<const CSECoPsimpleType<long long>*>(pOther))
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPsimpleBool::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPsimpleType<long long>::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoPsimpleBool*>(pOther) != nullptr);
}

/**
 * \brief This function reads the current value.
 * \param[out] bValue current value
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleBool::getBoolValue(bool &bValue) const
{
    long long llValue(0LL);
    if (!getValue(llValue))
        return false;
    bValue = (llValue != 0LL);
    return true;
}

/**
 * \brief This function changes the stored value.
 * \param[in] bValue new value
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleBool::setValue(const bool bValue)
{
    return setValue(static_cast<long long>(bValue ? 1 : 0));
}

/**
 * \brief This overloaded function changes the stored value.
 * \param[in] llValue new value (zero=false, others=true)
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleBool::setValue(const long long llValue)
{
    return CSECoPsimpleType<long long>::setValue(static_cast<long long>((llValue != 0LL) ? 1 : 0));
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful
 */
bool CSECoPsimpleBool::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    Q_UNUSED(json);
    Q_UNUSED(aszDelKeys);
    return true;
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (allow numbers and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleBool::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    if (data.is_boolean())
    {
        m_value = static_cast<long long>(data.get<bool>() ? 1 : 0);
        return true;
    }
    if (!bStrict)
    {
        if (!data.is_number())
        {
            m_value = static_cast<long long>(fabs(data.get<double>()) > 0.0);
            return true;
        }
        if (data.is_string())
        {
            QString sz(QString::fromStdString(data.get<std::string>()));
            if (sz.compare("f", Qt::CaseInsensitive) == 0 ||
                sz.compare("n", Qt::CaseInsensitive) == 0 ||
                sz.compare("false", Qt::CaseInsensitive) == 0 ||
                sz.compare("no", Qt::CaseInsensitive) == 0 ||
                sz.compare("off", Qt::CaseInsensitive) == 0)
            {
                m_value = 0LL;
                return true;
            }
            if (sz.compare("t", Qt::CaseInsensitive) == 0 ||
                sz.compare("y", Qt::CaseInsensitive) == 0 ||
                sz.compare("true", Qt::CaseInsensitive) == 0 ||
                sz.compare("yes", Qt::CaseInsensitive) == 0 ||
                sz.compare("on", Qt::CaseInsensitive) == 0)
            {
                m_value = 1LL;
                return true;
            }
        }
    }
    return false;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPsimpleBool::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    if (pMeMyselfAndI == nullptr)
        return nlohmann::json();
    return nlohmann::json(static_cast<bool>(m_value != 0LL));
}

/*****************************************************************************
 * CSECoParrayBool
 *****************************************************************************/
/**
 * \brief Standard constructor for a simple array (with min/max length) of booleans.
 */
CSECoParrayBool::CSECoParrayBool()
    : CSECoParrayBase(SECoP_VT_ARRAY_BOOL)
    , CSECoParraySimple<long long>()
{
    setType(SECoP_VT_ARRAY_BOOL);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoParrayBool::CSECoParrayBool(const CSECoParrayBool* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoParrayBase(static_cast<const CSECoParrayBase*>(pOther))
    , CSECoParraySimple<long long>(static_cast<const CSECoParraySimple<long long>*>(pOther))
{
    setType(SECoP_VT_ARRAY_BOOL);
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoParrayBool::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoParraySimple<long long>::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoParrayBool*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoParrayBool::compareValue(const CSECoPbaseType* pOther) const
{
    return CSECoParraySimple<long long>::compareValue(pOther);
}

/**
 * \brief This function reads the current value of a given index.
 * \param[in]  uIndex array index
 * \param[out] bValue current value
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::getBoolValue(unsigned int uIndex, bool &bValue) const
{
    long long llValue(0LL);
    if (!getValue(uIndex, llValue))
        return false;
    bValue = (llValue != 0LL);
    return true;
}

/**
 * \brief This function stores the value at a given index.
 * \param[in] uIndex array index
 * \param[in] bValue  new value to store
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::setValue(unsigned int uIndex, const bool bValue)
{
    return CSECoParraySimple<long long>::setValue(uIndex, static_cast<long long>(bValue ? 1 : 0));
}

/**
 * \brief This overloaded function stores the value at a given index.
 * \param[in] uIndex array index
 * \param[in] llValue new value (zero=false, others=true)
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::setValue(unsigned int uIndex, const long long llValue)
{
    return CSECoParraySimple<long long>::setValue(uIndex, static_cast<long long>((llValue != 0LL) ? 1 : 0));
}

/**
 * \brief This function (re)allocates memory to store the number of items and
 *        makes a deep copy of the data.
 * \param[in] pData  source array to copy
 * \param[in] uItems size of array
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::setArray(const bool* pData, unsigned int uItems)
{
    if (uItems < m_uMinSize || uItems > m_uMaxSize)
        return false;
    if (!setSize(uItems))
        return false;
    for (unsigned int i = 0; i < m_uSize; ++i)
        m_pData[i] = static_cast<long long>(pData[i] ? 1 : 0);
    m_uSize = uItems;
    return true;
}

/**
 * \brief This overloaded function (re)allocates memory to store the number of items and
 *        makes a deep copy of the data.
 * \param[in] pData  source array to copy
 * \param[in] uItems size of array
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::setArray(const long long* pData, unsigned int uItems)
{
    if (uItems < m_uMinSize || uItems > m_uMaxSize)
        return false;
    if (!setSize(uItems))
        return false;
    for (unsigned int i = 0; i < m_uSize; ++i)
        m_pData[i] = static_cast<long long>((pData[i] != 0LL) ? 1 : 0);
    m_uSize = uItems;
    return true;
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] bValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::appendValue(const bool bValue)
{
    return CSECoParraySimple<long long>::appendValue(static_cast<long long>(bValue ? 1 : 0));
}

/**
 * \brief This overloaded function reallocates memory and appends one value.
 * \param[in] llValue new value to append at end (zero=false, others=true)
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::appendValue(const long long llValue)
{
    return CSECoParraySimple<long long>::appendValue(static_cast<long long>((llValue != 0LL) ? 1 : 0));
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] value new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::appendValue(const CSECoPsimpleBool &value)
{
    bool bValue;
    if (!value.getBoolValue(bValue))
        return false;
    return appendValue(bValue);
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoParraySimple<long long>::createSECoPHelper(json, aszDelKeys))
        return false;
    return (QString::fromStdString(json["members"]["type"]).compare("bool", Qt::CaseInsensitive) == 0);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 *        It takes the key "members".
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (allow numbers and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    CSECoPsimpleBool value;
    if (data.is_array())
    {
        unsigned int uSize(static_cast<unsigned int>(data.size()));
        if (!setSize(uSize))
            return false;
        for (unsigned int i = 0; i < uSize; ++i)
        {
            value.setValue(m_pData[i]);
            if (!value.importSECoP(data[i], bStrict))
                return false;
            if (!value.getValue(m_pData[i]))
                return false;
        }
        return true;
    }
    if (!bStrict)
    {
        if (!setSize(1))
            return false;
        value.setValue(m_pData[0]);
        if (!value.importSECoP(data, bStrict))
            return false;
        return value.getValue(m_pData[0]);
    }
    return false;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoParrayBool::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    Q_UNUSED(bArray);
    if (!CSECoParraySimple<long long>::exportTypeHelper(json, true))
        return false;
    CSECoPsimpleBool b;
    b.additional() = additional();
    json["members"] = b.exportType();
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoParrayBool::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    nlohmann::json a;
    if (pMeMyselfAndI != nullptr)
        for (unsigned int i = 0; i < m_uSize; ++i)
            a.push_back(nlohmann::json(static_cast<bool>(m_pData[i] != 0LL)));
    return a;
}

/*****************************************************************************
 * CSECoPsimpleDouble
 *****************************************************************************/
/**
 * \brief Standard constructor for a double value (numeric with min/max).
 */
CSECoPsimpleDouble::CSECoPsimpleDouble()
    : CSECoPsimpleType<double>()
    , CSECoPminmaxType<double>()
{
    setType(SECoP_VT_DOUBLE);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPsimpleDouble::CSECoPsimpleDouble(const CSECoPsimpleDouble* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoPsimpleType<double>(static_cast<const CSECoPsimpleType<double>*>(pOther))
    , CSECoPminmaxType<double>(static_cast<const CSECoPminmaxType<double>*>(pOther))
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPsimpleDouble::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPsimpleType<double>::compareType(pOther) ||
        !CSECoPminmaxType<double>::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoPsimpleDouble*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPsimpleDouble::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPminmaxType<double>::compareValue(pOther));
    if (iResult == SECoP_VC_EQUAL)
        iResult = CSECoPsimpleType<double>::compareValue(pOther);
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPsimpleDouble::isValid() const
{
    return CSECoPsimpleType<double>::isValid() && CSECoPminmaxType<double>::isValid();
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleDouble::clear()
{
    return CSECoPsimpleType<double>::clear() && CSECoPminmaxType<double>::clear();
}

/**
 * \brief This function changes the stored value.
 * \param[in] dValue new value
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleDouble::setValue(const double dValue)
{
    if (!CSECoPminmaxType<double>::isValid(dValue))
        return false;
    return CSECoPsimpleType<double>::setValue(dValue);
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleDouble::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    return CSECoPminmaxType<double>::createSECoPHelper(json, aszDelKeys);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (boolean and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleDouble::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    if (data.is_number())
    {
        m_value = data.get<double>();
        return true;
    }
    if (data.is_string())
    {
        QString sz(QString::fromStdString(data.get<std::string>()));
        if (QRegExp("\\s*[+-]?nan\\s*", Qt::CaseInsensitive).exactMatch(sz))
        {
            m_value = std::numeric_limits<double>::quiet_NaN();
            return true;
        }
        if (QRegExp("\\s*\\+?inf(inity)\\s*?", Qt::CaseInsensitive).exactMatch(sz))
        {
            m_value = std::numeric_limits<double>::infinity();
            return true;
        }
        if (QRegExp("\\s*-inf(inity)?\\s*", Qt::CaseInsensitive).exactMatch(sz))
        {
            m_value = -std::numeric_limits<double>::infinity();
            return true;
        }
    }
    if (!bStrict)
    {
        if (data.is_boolean())
        {
            m_value = data.get<bool>() ? 1.0 : 0.0;
            return true;
        }
        if (data.is_string())
        {
            QByteArray aby(QByteArray::fromStdString(data.get<std::string>()));
            char* pEnd(nullptr);
            double d(strtod(aby.constData(), &pEnd));
            while (pEnd != nullptr)
                if (*pEnd == ' ' || *pEnd == '\t' || *pEnd == '\r' || *pEnd == '\n')
                    ++pEnd;
            if (pEnd != nullptr && *pEnd == '\0')
            {
                m_value = d;
                return true;
            }
        }
    }
    return false;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleDouble::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    return CSECoPminmaxType<double>::exportTypeHelper(json, bArray);
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPsimpleDouble::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    if (pMeMyselfAndI == nullptr)
        return nlohmann::json();
    return nlohmann::json(m_value);
}

/*****************************************************************************
 * CSECoParrayDouble
 *****************************************************************************/
/**
 * \brief Standard constructor for a simple array (with min/max length) of double (numeric with min/max).
 */
CSECoParrayDouble::CSECoParrayDouble()
    : CSECoParrayBase(SECoP_VT_ARRAY_DOUBLE)
    , CSECoParraySimple<double>()
    , CSECoPminmaxType<double>()
{
    setType(SECoP_VT_ARRAY_DOUBLE);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoParrayDouble::CSECoParrayDouble(const CSECoParrayDouble* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoParrayBase(static_cast<const CSECoParrayBase*>(pOther))
    , CSECoParraySimple<double>(static_cast<const CSECoParraySimple<double>*>(pOther))
    , CSECoPminmaxType<double>(static_cast<const CSECoPminmaxType<double>*>(pOther))
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoParrayDouble::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoParraySimple<double>::compareType(pOther) ||
        !CSECoPminmaxType<double>::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoParrayDouble*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoParrayDouble::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPminmaxType<double>::compareValue(pOther));
    if (iResult == SECoP_VC_EQUAL)
        iResult = CSECoParraySimple<double>::compareValue(pOther);
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoParrayDouble::isValid() const
{
    return CSECoParraySimple<double>::isValid() && CSECoPminmaxType<double>::isValid();
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoParrayDouble::clear()
{
    return CSECoParraySimple<double>::clear() && CSECoPminmaxType<double>::clear();
}

/**
 * \brief This function stores the value at a given index.
 * \param[in] uIndex array index
 * \param[in] dValue new value
 * \return true: successful, false: not successful
 */
bool CSECoParrayDouble::setValue(unsigned int uIndex, const double dValue)
{
    if (!CSECoPminmaxType<double>::isValid(dValue))
        return false;
    return CSECoParraySimple<double>::setValue(uIndex, dValue);
}

/**
 * \brief This function (re)allocates memory to store the number of items and
 *        makes a deep copy of the data.
 * \param[in] pData  source array to copy
 * \param[in] uItems size of array
 * \return true: successful, false: not successful
 */
bool CSECoParrayDouble::setArray(const double* pData, unsigned int uItems)
{
    if (pData != nullptr)
        for (unsigned int i = 0; i < uItems; ++i)
            if (CSECoPminmaxType<double>::isValid(pData[i]))
                return false;
    return CSECoParraySimple<double>::setArray(pData, uItems);
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] dValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayDouble::appendValue(const double dValue)
{
    if (!CSECoPminmaxType<double>::isValid(dValue))
        return false;
    return CSECoParraySimple<double>::appendValue(dValue);
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] value new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayDouble::appendValue(const CSECoPsimpleDouble &value)
{
    double dValue;
    if (!value.getValue(dValue))
        return false;
    return appendValue(dValue);
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoParrayDouble::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoParraySimple<double>::createSECoPHelper(json, aszDelKeys))
        return false;
    nlohmann::json &m(json["members"]);
    if (QString::fromStdString(m["type"]).compare("double", Qt::CaseInsensitive) != 0)
        return false;
    return CSECoPminmaxType<double>::createSECoPHelper(m, aszDelKeys);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (boolean and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoParrayDouble::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    CSECoPsimpleDouble value;
    if (data.is_array())
    {
        unsigned int uSize(static_cast<unsigned int>(data.size()));
        if (!setSize(uSize))
            return false;
        for (unsigned int i = 0; i < uSize; ++i)
        {
            value.setValue(m_pData[i]);
            if (!value.importSECoP(data[i], bStrict))
                return false;
            if (!value.getValue(m_pData[i]))
                return false;
        }
        return true;
    }
    if (!bStrict)
    {
        if (!setSize(1))
            return false;
        value.setValue(m_pData[0]);
        if (!value.importSECoP(data, bStrict))
            return false;
        return value.getValue(m_pData[0]);
    }
    return false;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoParrayDouble::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    nlohmann::json tmp;
    if (!CSECoPminmaxType<double>::exportTypeHelper(tmp, bArray) ||
        !CSECoParraySimple<double>::exportTypeHelper(json, true))
        return false;
    tmp["type"] = "double";
    json["members"] = tmp;
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoParrayDouble::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    nlohmann::json a;
    if (pMeMyselfAndI != nullptr)
        for (unsigned int i = 0; i < m_uSize; ++i)
            a.push_back(nlohmann::json(m_pData[i]));
    return a;
}

/*****************************************************************************
 * CSECoPsimpleInt
 *****************************************************************************/
/**
 * \brief Standard constructor for a integer value (numeric with min/max).
 */
CSECoPsimpleInt::CSECoPsimpleInt()
    : CSECoPsimpleType<long long>()
    , CSECoPminmaxType<long long>()
{
    setType(SECoP_VT_INTEGER);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPsimpleInt::CSECoPsimpleInt(const CSECoPsimpleInt* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoPsimpleType<long long>(static_cast<const CSECoPsimpleType<long long>*>(pOther))
    , CSECoPminmaxType<long long>(static_cast<const CSECoPminmaxType<long long>*>(pOther))
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPsimpleInt::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPsimpleType<long long>::compareType(pOther) ||
        !CSECoPminmaxType<long long>::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoPsimpleInt*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPsimpleInt::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPminmaxType<long long>::compareValue(pOther));
    if (iResult == SECoP_VC_EQUAL)
        iResult = CSECoPsimpleType<long long>::compareValue(pOther);
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPsimpleInt::isValid() const
{
    return CSECoPsimpleType<long long>::isValid() && CSECoPminmaxType<long long>::isValid();
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleInt::clear()
{
    return CSECoPsimpleType<long long>::clear() && CSECoPminmaxType<long long>::clear();
}

/**
 * \brief This function changes the stored value.
 * \param[in] llValue new value
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleInt::setValue(const long long llValue)
{
    if (!CSECoPminmaxType<long long>::isValid(llValue))
        return false;
    return CSECoPsimpleType<long long>::setValue(llValue);
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleInt::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    return CSECoPminmaxType<long long>::createSECoPHelper(json, aszDelKeys);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (boolean and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleInt::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    if (data.is_number_integer())
    {
        m_value = static_cast<long long>(data.get<std::int64_t>());
        return true;
    }
    if (!bStrict)
    {
        if (data.is_number())
        {
            m_value = static_cast<long long>(fabs(data.get<double>() + 0.5));
            return true;
        }
        if (data.is_boolean())
        {
            m_value = data.get<bool>() ? 1 : 0;
            return true;
        }
        if (data.is_string())
        {
            QByteArray aby(QByteArray::fromStdString(data.get<std::string>()));
            const char* pStart(aby.constData());
            char* pEnd(nullptr);
            if (pStart[0] == '0' && pStart[1] >= '0' && pStart[1] <= '9')
                ++pStart;
            long long llValue(strtoll(pStart, &pEnd, 0));
            while (pEnd != nullptr)
                if (*pEnd == ' ' || *pEnd == '\t' || *pEnd == '\r' || *pEnd == '\n')
                    ++pEnd;
            if (pEnd != nullptr && *pEnd == '\0')
            {
                m_value = llValue;
                return true;
            }
        }
    }
    return false;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleInt::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    return CSECoPminmaxType<long long>::exportTypeHelper(json, bArray);
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPsimpleInt::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    if (pMeMyselfAndI == nullptr)
        return nlohmann::json();
    return nlohmann::json(m_value);
}

/*****************************************************************************
 * CSECoParrayInt
 *****************************************************************************/
/**
 * \brief Standard constructor for a simple array (with min/max length) of double (numeric with min/max).
 */
CSECoParrayInt::CSECoParrayInt()
    : CSECoParrayBase(SECoP_VT_ARRAY_INTEGER)
    , CSECoParraySimple<long long>()
    , CSECoPminmaxType<long long>()
{
    setType(SECoP_VT_ARRAY_INTEGER);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoParrayInt::CSECoParrayInt(const CSECoParrayInt* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoParrayBase(static_cast<const CSECoParrayBase*>(pOther))
    , CSECoParraySimple<long long>(static_cast<const CSECoParraySimple<long long>*>(pOther))
    , CSECoPminmaxType<long long>(static_cast<const CSECoPminmaxType<long long>*>(pOther))
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoParrayInt::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoParraySimple<long long>::compareType(pOther) ||
        !CSECoPminmaxType<long long>::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoParrayInt*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoParrayInt::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPminmaxType<long long>::compareValue(pOther));
    if (iResult == SECoP_VC_EQUAL)
        iResult = CSECoParraySimple<long long>::compareValue(pOther);
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoParrayInt::isValid() const
{
    return CSECoParraySimple<long long>::isValid() && CSECoPminmaxType<long long>::isValid();
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoParrayInt::clear()
{
    return CSECoParraySimple<long long>::clear() && CSECoPminmaxType<long long>::clear();
}

/**
 * \brief This function stores the value at a given index.
 * \param[in] uIndex array index
 * \param[in] llValue new value
 * \return true: successful, false: not successful
 */
bool CSECoParrayInt::setValue(unsigned int uIndex, const long long llValue)
{
    if (!CSECoPminmaxType<long long>::isValid(llValue))
        return false;
    return CSECoParraySimple<long long>::setValue(uIndex, llValue);
}

/**
 * \brief This function (re)allocates memory to store the number of items and
 *        makes a deep copy of the data.
 * \param[in] pData  source array to copy
 * \param[in] uItems size of array
 * \return true: successful, false: not successful
 */
bool CSECoParrayInt::setArray(const long long* pData, unsigned int uItems)
{
    if (pData != nullptr)
        for (unsigned int i = 0; i < uItems; ++i)
            if (CSECoPminmaxType<long long>::isValid(pData[i]))
                return false;
    return CSECoParraySimple<long long>::setArray(pData, uItems);
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] llValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayInt::appendValue(const long long llValue)
{
    if (!CSECoPminmaxType<long long>::isValid(llValue))
        return false;
    return CSECoParraySimple<long long>::appendValue(llValue);
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] value new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayInt::appendValue(const CSECoPsimpleInt &value)
{
    long long llValue;
    if (!value.getValue(llValue))
        return false;
    return appendValue(llValue);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (boolean and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoParrayInt::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    CSECoPsimpleInt value;
    if (data.is_array())
    {
        unsigned int uSize(static_cast<unsigned int>(data.size()));
        if (!setSize(uSize))
            return false;
        for (unsigned int i = 0; i < uSize; ++i)
        {
            value.setValue(m_pData[i]);
            if (!value.importSECoP(data[i], bStrict))
                return false;
            if (!value.getValue(m_pData[i]))
                return false;
        }
        return true;
    }
    if (!bStrict)
    {
        if (!setSize(1))
            return false;
        value.setValue(m_pData[0]);
        if (!value.importSECoP(data, bStrict))
            return false;
        return value.getValue(m_pData[0]);
    }
    return false;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoParrayInt::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoParraySimple<long long>::createSECoPHelper(json, aszDelKeys))
        return false;
    nlohmann::json &m(json["members"]);
    if (QString::fromStdString(m["type"]).compare("int", Qt::CaseInsensitive) != 0)
        return false;
    return CSECoPminmaxType<long long>::createSECoPHelper(m, aszDelKeys);
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoParrayInt::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    nlohmann::json tmp;
    if (!CSECoPminmaxType<long long>::exportTypeHelper(tmp, bArray) ||
        !CSECoParraySimple<long long>::exportTypeHelper(json, true))
        return false;
    tmp["type"] = "int";
    json["members"] = tmp;
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoParrayInt::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    nlohmann::json a;
    if (pMeMyselfAndI != nullptr)
        for (unsigned int i = 0; i < m_uSize; ++i)
            a.push_back(nlohmann::json(m_pData[i]));
    return a;
}

/*****************************************************************************
 * CSECoPsimpleScaled
 *****************************************************************************/
/**
 * \brief Standard constructor for a scaled integer value (numeric with scale/min/max).
 */
CSECoPsimpleScaled::CSECoPsimpleScaled()
    : CSECoPscaledBase()
    , CSECoPsimpleInt()
{
    setType(SECoP_VT_SCALED);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPsimpleScaled::CSECoPsimpleScaled(const CSECoPsimpleScaled* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoPscaledBase(static_cast<const CSECoPscaledBase*>(pOther))
    , CSECoPsimpleType<long long>(static_cast<const CSECoPsimpleType<long long>*>(pOther))
    , CSECoPminmaxType<long long>(static_cast<const CSECoPminmaxType<long long>*>(pOther))
    , CSECoPsimpleInt(static_cast<const CSECoPsimpleInt*>(pOther))
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPsimpleScaled::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPscaledBase::compareType(pOther) ||
        !CSECoPsimpleInt::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoPsimpleScaled*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPsimpleScaled::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPscaledBase::compareValue(pOther));
    if (iResult == SECoP_VC_EQUAL)
        iResult = CSECoPsimpleInt::compareValue(pOther);
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPsimpleScaled::isValid() const
{
    return CSECoPscaledBase::isValid() && CSECoPsimpleInt::isValid();
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::clear()
{
    return CSECoPscaledBase::clear() && CSECoPsimpleInt::clear();
}

/**
 * \brief This function reads the allowed minimum and maximum values.
 * \param[out] dMinimum minimum value
 * \param[out] dMaximum maximum value
 */
void CSECoPsimpleScaled::getMinMaxValue(double &dMinimum, double &dMaximum) const
{
    long long llMin, llMax;
    CSECoPminmaxType<long long>::getMinMaxValue(llMin, llMax);
    if (std::isfinite(m_dScale) && m_dScale > 0.0)
    {
        dMinimum = llMin * m_dScale;
        dMaximum = llMax * m_dScale;
    }
    else
        dMinimum = dMaximum = std::numeric_limits<double>::quiet_NaN();
}

/**
 * \brief This overloaded function reads the allowed minimum and maximum values.
 * \param[out] llMinimum minimum value
 * \param[out] llMaximum maximum value
 */
void CSECoPsimpleScaled::getMinMaxValue(long long &llMinimum, long long &llMaximum) const
{
    return CSECoPminmaxType<long long>::getMinMaxValue(llMinimum, llMaximum);
}

/**
 * \brief This function stores new allowed minimum and maximum values.
 *        The minimum has to be lower or equal the maximum value or
 *        use NaN, which uses the limit value of the data type.
 * \param[in] dMinimum minimum value
 * \param[in] dMaximum maximum value
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::setMinMaxValue(double dMinimum, double dMaximum)
{
    long long llMin(std::numeric_limits<long long>::lowest());
    long long llMax(std::numeric_limits<long long>::max());
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0)
        return false;
    if (std::isfinite(dMinimum))
        llMin = static_cast<long long>(floor(dMinimum / m_dScale + 0.5));
    if (std::isfinite(dMaximum))
        llMax = static_cast<long long>(floor(dMaximum / m_dScale + 0.5));
    return CSECoPsimpleInt::setMinMaxValue(llMin, llMax);
}

/**
 * \brief This overloaded function returns an error.
 * \param[in] llMinimum minimum value
 * \param[in] llMaximum maximum value
 * \return false: not successful
 */
bool CSECoPsimpleScaled::setMinMaxValue(long long llMinimum, long long llMaximum)
{
    Q_UNUSED(llMinimum);
    Q_UNUSED(llMaximum);
    return false;
}

/**
 * \brief This overloaded function reads the current stored value.
 * \param[out] llValue current value output
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::getValue(long long &llValue) const
{
    return CSECoPsimpleInt::getValue(llValue);
}

/**
 * \brief This function reads the current stored value.
 * \param[out] dValue current value output
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::getValue(double &dValue) const
{
    long long llValue;
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0)
        return false;
    if (!CSECoPsimpleInt::getValue(llValue))
        return false;
    dValue = llValue * m_dScale;
    return true;
}

/**
 * \brief This overloaded function stores the value.
 * \param[in] llValue new value to store
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::setValue(long long llValue)
{
    return CSECoPsimpleInt::setValue(llValue);
}

/**
 * \brief This function stores the value.
 * \param[in] dValue new value to store
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::setValue(double dValue)
{
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0 || !std::isfinite(dValue))
        return false;
    return CSECoPsimpleInt::setValue(static_cast<long long>(dValue / m_dScale));
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoPscaledBase::createSECoPHelper(json, aszDelKeys))
        return false;
    return CSECoPminmaxType<long long>::createSECoPHelper(json, aszDelKeys);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (boolean and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::importSECoP(const nlohmann::json &data, bool bStrict)
{
    return CSECoPsimpleInt::importSECoP(data, bStrict);
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleScaled::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    return CSECoPscaledBase::exportTypeHelper(json, bArray);
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPsimpleScaled::exportSECoPjson() const
{
    return CSECoPsimpleInt::exportSECoPjson();
}

/*****************************************************************************
 * CSECoParrayScaled
 *****************************************************************************/
/**
 * \brief Standard constructor for a simple array (with min/max length) of
 *        scaled integer values (numeric with scale/min/max).
 */
CSECoParrayScaled::CSECoParrayScaled()
    : CSECoPscaledBase()
    , CSECoParrayBase(SECoP_VT_ARRAY_SCALED)
    , CSECoParrayInt()
{
    setType(SECoP_VT_ARRAY_SCALED);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoParrayScaled::CSECoParrayScaled(const CSECoParrayScaled* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoPscaledBase(static_cast<const CSECoPscaledBase*>(pOther))
    , CSECoParrayBase(static_cast<const CSECoParrayBase*>(pOther))
    , CSECoParraySimple<long long>(static_cast<const CSECoParraySimple<long long>*>(pOther))
    , CSECoPminmaxType<long long>(static_cast<const CSECoPminmaxType<long long>*>(pOther))
    , CSECoParrayInt(static_cast<const CSECoParrayInt*>(pOther))
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoParrayScaled::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPscaledBase::compareType(pOther) ||
        !CSECoParrayInt::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoParrayScaled*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoParrayScaled::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPscaledBase::compareValue(pOther));
    if (iResult == SECoP_VC_EQUAL)
        iResult = CSECoParrayInt::compareValue(pOther);
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoParrayScaled::isValid() const
{
    return CSECoPscaledBase::isValid() && CSECoParrayInt::isValid();
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::clear()
{
    return CSECoPscaledBase::clear() && CSECoParrayInt::clear();
}

/**
 * \brief This function reads the allowed minimum and maximum values.
 * \param[out] dMinimum minimum value
 * \param[out] dMaximum maximum value
 */
void CSECoParrayScaled::getMinMaxValue(double &dMinimum, double &dMaximum) const
{
    long long llMin, llMax;
    CSECoPminmaxType<long long>::getMinMaxValue(llMin, llMax);
    if (std::isfinite(m_dScale) && m_dScale > 0.0)
    {
        dMinimum = llMin * m_dScale;
        dMaximum = llMax * m_dScale;
    }
    else
        dMinimum = dMaximum = std::numeric_limits<double>::quiet_NaN();
}

/**
 * \brief This overloaded function reads the allowed minimum and maximum values.
 * \param[out] llMinimum minimum value
 * \param[out] llMaximum maximum value
 */
void CSECoParrayScaled::getMinMaxValue(long long &llMinimum, long long &llMaximum) const
{
    return CSECoPminmaxType<long long>::getMinMaxValue(llMinimum, llMaximum);
}

/**
 * \brief This function stores new allowed minimum and maximum values.
 *        The minimum has to be lower or equal the maximum value or
 *        use NaN, which uses the limit value of the data type.
 * \param[in] dMinimum minimum value
 * \param[in] dMaximum maximum value
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::setMinMaxValue(double dMinimum, double dMaximum)
{
    long long llMin(std::numeric_limits<long long>::lowest());
    long long llMax(std::numeric_limits<long long>::max());
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0)
        return false;
    if (std::isfinite(dMinimum))
        llMin = static_cast<long long>(floor(dMinimum / m_dScale + 0.5));
    if (std::isfinite(dMaximum))
        llMax = static_cast<long long>(floor(dMaximum / m_dScale + 0.5));
    return CSECoPminmaxType<long long>::setMinMaxValue(llMin, llMax);
}

/**
 * \brief This overloaded function returns an error.
 * \param[in] llMinimum minimum value
 * \param[in] llMaximum maximum value
 * \return false: not successful
 */
bool CSECoParrayScaled::setMinMaxValue(long long llMinimum, long long llMaximum)
{
    Q_UNUSED(llMinimum);
    Q_UNUSED(llMaximum);
    return false;
}

/**
 * \brief This overloaded function reads the current stored value at a given index.
 * \param[in]  uIndex array index
 * \param[out] dValue current value output
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::getValue(unsigned int uIndex, double &dValue) const
{
    long long llValue;
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0)
        return false;
    if (!CSECoParrayInt::getValue(uIndex, llValue))
        return false;
    dValue = llValue * m_dScale;
    return true;
}

/**
 * \brief This overloaded function reads the current stored value at a given index.
 * \param[in]  uIndex  array index
 * \param[out] llValue current value output
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::getValue(unsigned int uIndex, long long &llValue) const
{
    return CSECoParrayInt::getValue(uIndex, llValue);
}

/**
 * \brief This overloaded function returns a nullptr.
 * \return nullptr
 */
const long long* CSECoParrayScaled::getArray() const
{
    return nullptr;
}

/**
 * \brief This function stores the value at a given index.
 * \param[in] uIndex array index
 * \param[in] dValue  new value to store
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::setValue(unsigned int uIndex, const double dValue)
{
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0 || !std::isfinite(dValue))
        return false;
    return CSECoParrayInt::setValue(uIndex, static_cast<long long>(floor(dValue / m_dScale + 0.5)));
}

/**
 * \brief This overloaded function stores the value at a given index.
 * \param[in] uIndex array index
 * \param[in] llValue new value to store
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::setValue(unsigned int uIndex, const long long llValue)
{
    return CSECoParrayInt::setValue(uIndex, llValue);
}

bool CSECoParrayScaled::setArray(const double* pData, unsigned int uItems)
{
    unsigned int uNewMax(m_uMaxSize);
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0)
        return false;
    for (unsigned int i = 0; i < uItems; ++i)
        if (!std::isfinite(pData[i]))
            return false;
    if (uNewMax < uItems)
        uNewMax = uItems;
    if (!setSize(uItems))
        return false;
    if (!setMinMaxSize(m_uMinSize, uNewMax, false) || uItems < m_uMinSize)
        return false;
    for (unsigned int i = 0; i < uItems; ++i)
        if (!CSECoParrayInt::setValue(i, static_cast<long long>(pData[i] / m_dScale)))
            return false;
    return true;
}

/**
 * \brief This function return an error.
 * \param[in] pData  source array to copy
 * \param[in] uItems size of array
 * \return false: not successful
 */
bool CSECoParrayScaled::setArray(const long long* pData, unsigned int uItems)
{
    Q_UNUSED(pData);
    Q_UNUSED(uItems);
    return false;
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] dValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::appendValue(const double dValue)
{
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0 || !std::isfinite(dValue))
        return false;
    return CSECoParrayInt::appendValue(static_cast<long long>(dValue / m_dScale));
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] value new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::appendValue(const CSECoPsimpleScaled &value)
{
    double dValue;
    if (!value.getValue(dValue))
        return false;
    if (!std::isfinite(m_dScale) || m_dScale <= 0.0)
        m_dScale = value.m_dScale;
    return appendValue(dValue);
}

/**
 * \brief This overloaded function returns an error.
 * \param[in] llValue new value to append at end
 * \return false: not successful
 */
bool CSECoParrayScaled::appendValue(const long long llValue)
{
    Q_UNUSED(llValue);
    return false;
}

/**
 * \brief This overloaded function returns an error.
 * \param[in] value new value to append at end
 * \return false: not successful
 */
bool CSECoParrayScaled::appendValue(const CSECoPsimpleInt &value)
{
    Q_UNUSED(value);
    return false;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoParrayInt::createSECoPHelper(json, aszDelKeys))
        return false;
    nlohmann::json &m(json["members"]);
    if (QString::fromStdString(m["type"]).compare("scaled", Qt::CaseInsensitive) != 0)
        return false;
    return CSECoPscaledBase::createSECoPHelper(m, aszDelKeys) &&
           CSECoPminmaxType<long long>::createSECoPHelper(m, aszDelKeys);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (boolean and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::importSECoP(const nlohmann::json &data, bool bStrict)
{
    return CSECoParrayInt::importSECoP(data, bStrict);
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoParrayScaled::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    nlohmann::json tmp;
    if (!CSECoPscaledBase::exportTypeHelper(tmp, bArray) ||
        !CSECoParrayInt::exportTypeHelper(json, true))
        return false;
    tmp["type"] = "scaled";
    json["members"] = tmp;
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoParrayScaled::exportSECoPjson() const
{
    return CSECoParrayInt::exportSECoPjson();
}

/*****************************************************************************
 * CSECoPsimpleEnum
 *****************************************************************************/
/**
 * \brief Standard constructor for a enumeration value (integer numeric).
 */
CSECoPsimpleEnum::CSECoPsimpleEnum()
    : CSECoPenumBase()
    , CSECoPsimpleInt()
{
    setType(SECoP_VT_ENUM);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPsimpleEnum::CSECoPsimpleEnum(const CSECoPsimpleEnum* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoPenumBase(static_cast<const CSECoPenumBase*>(pOther))
    , CSECoPsimpleType<long long>(static_cast<const CSECoPsimpleType<long long>*>(pOther))
    , CSECoPminmaxType<long long>(static_cast<const CSECoPminmaxType<long long>*>(pOther))
    , CSECoPsimpleInt(static_cast<const CSECoPsimpleInt*>(pOther))
{
    setType(SECoP_VT_ENUM);
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPsimpleEnum::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPenumBase::compareType(pOther) ||
        !CSECoPsimpleInt::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoPsimpleEnum*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPsimpleEnum::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPenumBase::compareValue(pOther));
    if (iResult == SECoP_VC_EQUAL)
        iResult = CSECoPsimpleInt::compareValue(pOther);
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPsimpleEnum::isValid() const
{
    if (!CSECoPenumBase::isValid())
        return false;
    return CSECoPenumBase::isValid(m_value);
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleEnum::clear()
{
    return CSECoPenumBase::clear() && CSECoPsimpleInt::clear();
}

/**
 * \brief This function changes the stored value.
 * \param[in] llValue new value
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleEnum::setValue(const long long llValue)
{
    if (!CSECoPenumBase::isValid(llValue))
        return false;
    return CSECoPsimpleInt::setValue(llValue);
}

/**
 * \brief This function reads the allowed minimum and maximum values.
 * \param[out] llMinimum minimum value
 * \param[out] llMaximum maximum value
 */
void CSECoPsimpleEnum::getMinMaxValue(long long &llMinimum, long long &llMaximum) const
{
    llMinimum = INT64_MAX;
    llMaximum = INT64_MIN;
    for (int i = 0; i < m_aItems.size(); ++i)
    {
        const internal* pItem(&m_aItems[i]);
        if (llMinimum > pItem->llValue)
            llMinimum = pItem->llValue;
        if (llMaximum < pItem->llValue)
            llMaximum = pItem->llValue;
    }
}

/**
 * \brief This function return an error.
 * \param[in] llMinimum minimum value
 * \param[in] llMaximum maximum value
 * \return false: not successful
 */
bool CSECoPsimpleEnum::setMinMaxValue(long long llMinimum, long long llMaximum)
{
    Q_UNUSED(llMinimum);
    Q_UNUSED(llMaximum);
    return false;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleEnum::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    return CSECoPenumBase::createSECoPHelper(json, aszDelKeys);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (numbers, boolean and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleEnum::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    if (!CSECoPsimpleInt::importSECoP(data, bStrict))
        return false;
    if (bStrict)
        return CSECoPenumBase::isValid(m_value);
    else if (data.is_string())
    {
        QString sValue(QString::fromStdString(data.get<std::string>()));
        for (auto it = m_aItems.constBegin(); it != m_aItems.constEnd(); ++it)
            if (sValue.compare(it->szName, Qt::CaseInsensitive) == 0)
                return setValue(it->llValue);
    }
    return false;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleEnum::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    return CSECoPenumBase::exportTypeHelper(json, bArray);
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPsimpleEnum::exportSECoPjson() const
{
    return CSECoPsimpleInt::exportSECoPjson();
}

/**
 * \brief This function adds an enumeration.
 * \param[in] llValue value of enumeration
 * \param[in] szName  name of enumeration (zero terminated string pointer)
 * \return true: successful, false: not successful
 */
bool CSECoPsimpleEnum::addItem(long long llValue, const char* szName)
{
    bool bInit(m_aItems.isEmpty());
    if (!CSECoPenumBase::addItem(llValue, szName))
        return false;
    if (bInit)
        m_value = llValue;
    return true;
}

/*****************************************************************************
 * CSECoParrayEnum
 *****************************************************************************/
/**
 * \brief Standard constructor for a simple array (with min/max length) of
 *        enumeration values (integer numeric).
 */
CSECoParrayEnum::CSECoParrayEnum()
    : CSECoPenumBase()
    , CSECoParrayBase(SECoP_VT_ARRAY_ENUM)
    , CSECoParrayInt()
{
    setType(SECoP_VT_ARRAY_ENUM);
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoParrayEnum::CSECoParrayEnum(const CSECoParrayEnum* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoPenumBase(static_cast<const CSECoPenumBase*>(pOther))
    , CSECoParrayBase(static_cast<const CSECoParrayBase*>(pOther))
    , CSECoParraySimple<long long>(static_cast<const CSECoParraySimple<long long>*>(pOther))
    , CSECoPminmaxType<long long>(static_cast<const CSECoPminmaxType<long long>*>(pOther))
    , CSECoParrayInt(static_cast<const CSECoParrayInt*>(pOther))
{
    setType(SECoP_VT_ARRAY_ENUM);
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoParrayEnum::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPenumBase::compareType(pOther) ||
        !CSECoParraySimple<long long>::compareType(pOther))
        return false;
    return (dynamic_cast<const CSECoParrayEnum*>(pOther) != nullptr);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoParrayEnum::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPenumBase::compareValue(pOther));
    if (iResult == SECoP_VC_EQUAL)
        iResult = CSECoParraySimple<long long>::compareValue(pOther);
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoParrayEnum::isValid() const
{
    if (!CSECoPenumBase::isValid() || !CSECoParrayInt::isValid())
        return false;
    for (unsigned int i = 0; i < m_uSize; ++i)
        if (!CSECoPenumBase::isValid(m_pData[i]))
            return false;
    return true;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoParrayEnum::clear()
{
    return CSECoPenumBase::clear() && CSECoParrayInt::clear();
}

/**
 * \brief This function stores the value at a given index.
 * \param[in] uIndex array index
 * \param[in] llValue new value
 * \return true: successful, false: not successful
 */
bool CSECoParrayEnum::setValue(unsigned int uIndex, const long long llValue)
{
    if (m_aItems.isEmpty())
        return false;
    if (!CSECoPenumBase::isValid(llValue))
        return false;
    return CSECoParrayInt::setValue(uIndex, llValue);
}

/**
 * \brief This function (re)allocates memory to store the number of items and
 *        makes a deep copy of the data.
 * \param[in] pData  source array to copy
 * \param[in] uItems size of array
 * \return true: successful, false: not successful
 */
bool CSECoParrayEnum::setArray(const long long* pData, unsigned int uItems)
{
    unsigned int uNewMax(m_uMaxSize);
    if (m_aItems.isEmpty())
        return false;
    if (uNewMax < uItems)
        uNewMax = uItems;
    if (uItems < m_uMinSize)
        return false;
    if (!setSize(uItems))
        return false;
    if (!setMinMaxSize(m_uMinSize, uNewMax, false))
        return false;
    m_uSize = uItems;
    for (unsigned int i = 0; i < uItems; ++i)
        if (!setValue(i, pData[i]))
            return false;
    return true;
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] llValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayEnum::appendValue(const long long llValue)
{
    if (!CSECoPenumBase::isValid(llValue))
        return false;
    return CSECoParraySimple<long long>::appendValue(llValue);
}

/**
 * \brief This function reallocates memory and appends one value.
 * \param[in] value new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParrayEnum::appendValue(const CSECoPsimpleEnum &value)
{
    long long llValue;
    if (!value.getValue(llValue))
        return false;
    if (m_aItems.isEmpty())
        m_aItems = value.m_aItems;
    return appendValue(llValue);
}

/**
 * \brief This function returns an error.
 * \param[in] value new value to append at end
 * \return false: not successful
 */
bool CSECoParrayEnum::appendValue(const CSECoPsimpleInt &value)
{
    Q_UNUSED(value);
    return false;
}

/**
 * \brief This function reads the allowed minimum and maximum values.
 * \param[out] llMinimum minimum value
 * \param[out] llMaximum maximum value
 */
void CSECoParrayEnum::getMinMaxValue(long long &llMinimum, long long &llMaximum) const
{
    llMinimum = INT64_MAX;
    llMaximum = INT64_MIN;
    for (int i = 0; i < m_aItems.size(); ++i)
    {
        const internal* pItem(&m_aItems[i]);
        if (llMinimum > pItem->llValue)
            llMinimum = pItem->llValue;
        if (llMaximum < pItem->llValue)
            llMaximum = pItem->llValue;
    }
}

/**
 * \brief This function return an error.
 * \param[out] llMinimum minimum value
 * \param[out] llMaximum maximum value
 * \return false: not successful
 */
bool CSECoParrayEnum::setMinMaxValue(long long llMinimum, long long llMaximum)
{
    Q_UNUSED(llMinimum);
    Q_UNUSED(llMaximum);
    return false;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoParrayEnum::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoParrayInt::createSECoPHelper(json, aszDelKeys))
        return false;
    nlohmann::json &m(json["members"]);
    if (QString::fromStdString(m["type"]).compare("enum", Qt::CaseInsensitive) != 0)
        return false;
    return CSECoPenumBase::createSECoPHelper(m, aszDelKeys);
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (numbers, boolean and some strings)
 * \return true: successful, false: not successful
 */
bool CSECoParrayEnum::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    CSECoPsimpleEnum value;
    value.m_aItems = m_aItems;
    if (data.is_array())
    {
        unsigned int uSize(static_cast<unsigned int>(data.size()));
        if (!setSize(uSize))
            return false;
        for (unsigned int i = 0; i < uSize; ++i)
        {
            value.setValue(m_pData[i]);
            if (!value.importSECoP(data[i], bStrict))
                return false;
            if (!value.getValue(m_pData[i]))
                return false;
        }
        return true;
    }
    if (!bStrict)
        if (value.importSECoP(data, false) && setSize(1) && value.getValue(m_pData[0]))
            return true;
    return false;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoParrayEnum::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    nlohmann::json tmp;
    if (!CSECoPenumBase::exportTypeHelper(tmp, bArray) ||
        !CSECoParrayInt::exportTypeHelper(json, true))
        return false;
    tmp["type"] = "enum";
    json["members"] = tmp;
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoParrayEnum::exportSECoPjson() const
{
    return CSECoParrayInt::exportSECoPjson();
}

/*****************************************************************************
 * CSECoPstring
 *****************************************************************************/
/**
 * \brief Standard constructor for a string or blob. This class uses unsigned
 *        bytes and automatically adds an internal (only) NUL bytes at end.
 */
CSECoPstring::CSECoPstring(SECoP_V_type iType /* = SECoP_VT_STRING*/)
    : CSECoParrayBase(iType)
    , CSECoParraySimple<unsigned char>()
    , m_bIsUTF8(true)
{
    if (iType != SECoP_VT_STRING && iType != SECoP_VT_JSON)
        iType = SECoP_VT_BLOB;
    setType(iType);
    m_bAdditional = true;
    m_uMaxSize    = UINT_MAX - 1;
    m_uMaxChars   = UINT_MAX - 1;
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPstring::CSECoPstring(const CSECoPstring* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoParrayBase(static_cast<const CSECoParrayBase*>(pOther))
    , CSECoParraySimple<unsigned char>(static_cast<const CSECoParraySimple<unsigned char>*>(pOther))
    , m_bIsUTF8(pOther->m_bIsUTF8)
    , m_uMaxChars(pOther->m_uMaxChars)
{
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPstring::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoParraySimple<unsigned char>::compareType(pOther))
        return false;
    if (getType() == SECoP_VT_STRING)
    {
        const CSECoPstring* pOtherString(dynamic_cast<const CSECoPstring*>(pOther));
        if (pOtherString == nullptr)
            return false;
        return (pOtherString->m_bIsUTF8 == m_bIsUTF8 && pOtherString->m_uMaxChars == m_uMaxChars);
    }
    return true;
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPstring::compareValue(const CSECoPbaseType* pOther) const
{
    return CSECoParraySimple<unsigned char>::compareValue(pOther);
}

/**
 * \brief This function reads the current stored byte at a given index.
 * \param[in]  uIndex array index
 * \param[out] value  current value output
 * \return true: successful, false: not successful
 */
bool CSECoPstring::getValue(unsigned int uIndex, unsigned char &value) const
{
    return CSECoParraySimple<unsigned char>::getValue(uIndex, value);
}

/**
 * \brief The functions returns the string/blob as Qt byte array.
 * \return QByteArray of string/blob
 */
QByteArray CSECoPstring::getValue() const
{
    if (getType() == SECoP_VT_BLOB)
        return QByteArray::fromRawData(reinterpret_cast<const char*>(m_pData), static_cast<int>(m_uSize)).toBase64();
    else
        return QByteArray::fromRawData(reinterpret_cast<const char*>(m_pData), static_cast<int>(m_uSize));
}

/**
 * \brief This functions (re)allocates memory to store the string/blob and
 *        makes a deep copy of the data.
 * \param[in] abyValue source data to copy
 * \return true: successful, false: not successful
 */
bool CSECoPstring::setValue(QByteArray abyValue)
{
    switch (getType())
    {
        case SECoP_VT_BLOB:
        {
            QByteArray abyTmp(QByteArray::fromBase64(abyValue));
            return setArray(reinterpret_cast<const unsigned char*>(abyTmp.constData()), static_cast<unsigned int>(abyTmp.size()));
        }
        case SECoP_VT_JSON:
        {
            try
            {
                static_cast<void>(nlohmann::json::parse(abyValue.constData()));
            }
            catch (nlohmann::detail::exception&)
            {
                return false;
            }
            catch (...)
            {
                return false;
            }
            return setArray(reinterpret_cast<const unsigned char*>(abyValue.constData()), static_cast<unsigned int>(abyValue.size()));
        }
        default:
            if (m_bIsUTF8)
            {
                // check UTF-8 and string length
                QString sValue(QString::fromUtf8(abyValue));
                if (abyValue != sValue.toUtf8())
                    return false;
                if (static_cast<unsigned int>(sValue.size()) > m_uMaxChars)
                    return false;
            }
            else
            {
                // force ASCII
                if (static_cast<unsigned int>(abyValue.size()) > m_uMaxChars)
                    return false;
                for (int i = 0; i < abyValue.size(); ++i)
                    if (abyValue.at(i) < 0)
                        return false;
            }
            return setArray(reinterpret_cast<const unsigned char*>(abyValue.constData()), static_cast<unsigned int>(abyValue.size()));
    }
}

/**
 * \brief This function returns an error.
 * \param[in] uIndex array index
 * \param[in] byValue  new value to store
 * \return false: not successful
 */
bool CSECoPstring::setValue(unsigned int uIndex, const unsigned char byValue)
{
    Q_UNUSED(uIndex);
    Q_UNUSED(byValue);
    return false;
}

/**
 * \brief This function returns an error.
 * \param[in] byValue new value to append at end
 * \return false: not successful
 */
bool CSECoPstring::appendValue(const unsigned char byValue)
{
    Q_UNUSED(byValue);
    return false;
}

/**
 * \brief This function reads the allowed minimum and maximum string character count.
 * \param[out] uMinimum minimum string character count
 * \param[out] uMaximum maximum string character count
 */
void CSECoPstring::getMinMaxSize(unsigned int &uMinimum, unsigned int &uMaximum) const
{
    if (getType() == SECoP_VT_STRING)
    {
        uMinimum = m_uMinSize;
        uMaximum = m_uMaxChars;
    }
    else
        CSECoParraySimple<unsigned char>::getMinMaxSize(uMinimum, uMaximum);
}

/**
 * \brief This overloaded function stores new allowed minimum and maximum
 *        string character count and optionally resizes the content.
 *        The minimum count has to be lower or equal the maximum count.
 * \param[in] uMinimum      minimum string character count
 * \param[in] uMaximum      maximum string character count
 * \param[in] bForceRealloc true: force a reallocation of memory, false: no reallocation
 * \return true: successful, false: not successful
 */
bool CSECoPstring::setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum, bool bForceRealloc)
{
    if (getType() == SECoP_VT_STRING)
    {
        if (uMinimum > uMaximum)
            return false;
        if (!CSECoParraySimple<unsigned char>::setMinMaxSize(uMinimum, UINT_MAX - 1, bForceRealloc))
            return false;
        m_uMaxChars = (uMaximum < UINT_MAX) ? uMaximum : (UINT_MAX - 1);
        return true;
    }
    else
        return CSECoParraySimple<unsigned char>::setMinMaxSize(uMinimum, uMaximum, bForceRealloc);
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        This function takes the key "isUTF8".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPstring::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoParrayBase::createSECoPHelper(json, aszDelKeys))
        return false;
    m_uMaxChars = m_uMaxSize;
    m_uMaxSize  = UINT_MAX - 1;
    if (json.contains("isUTF8"))
    {
        const nlohmann::json &v(json["isUTF8"]);
        if (!v.is_boolean())
            return false;
        m_bIsUTF8 = v.get<bool>();
        if (!aszDelKeys.contains("isUTF8"))
            aszDelKeys.append("isUTF8");
    }
    else
        m_bIsUTF8 = false;
    return true;
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing
 * \return true: successful, false: not successful
 */
bool CSECoPstring::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    if (data.is_string())
        return setValue(QByteArray::fromStdString(data.get<std::string>()));
    if (!bStrict)
        return setValue(QByteArray::fromStdString(data.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace)));
    return false;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        This function handles the keys "isUTF8" and "maxchars".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPstring::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    if (!CSECoParraySimple<unsigned char>::exportTypeHelper(json, bArray))
        return false;
    if (getType() == SECoP_VT_STRING)
    {
        json["isUTF8"] = m_bIsUTF8;
        if (m_uMaxChars < (UINT_MAX - 1))
            json["maxchars"] = m_uMaxChars;
        else if (json.contains("maxchars"))
            json.erase("maxchars");
    }
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPstring::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    if (pMeMyselfAndI == nullptr)
        return nlohmann::json();
    if (getType() == SECoP_VT_JSON)
    {
        try
        {
            return nlohmann::json::parse(getValue().constData());
        }
        catch (nlohmann::json::exception&)
        {
        }
        catch (...)
        {
        }
        return nlohmann::json();
    }
    else
        return nlohmann::json(getValue().toStdString());
}

/*****************************************************************************
 * CSECoPstruct
 *****************************************************************************/
/**
 * \brief Standard constructor for structured data (key/value storage)
 */
CSECoPstruct::CSECoPstruct()
    : CSECoPbaseType(SECoP_VT_STRUCT)
{
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPstruct::CSECoPstruct(const CSECoPstruct* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
{
    m_asNames = pOther->m_asNames;
    for (auto it = pOther->m_apItems.constBegin(); it != pOther->m_apItems.constEnd(); ++it)
        m_apItems.append((*it)->duplicate());
}

/**
 * \brief Destructor for structured data. It frees the allocated memory.
 */
CSECoPstruct::~CSECoPstruct()
{
    for (int i = 0; i < m_apItems.size(); ++i)
        delete m_apItems[i];
    m_asNames.clear();
    m_apItems.clear();
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 *        All key names have to match and every data type behind a key is compared.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPstruct::compareType(const CSECoPbaseType* pOther) const
{
    if (m_asNames.size() != m_apItems.size())
        return false;
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    const CSECoPstruct* pTmp(dynamic_cast<const CSECoPstruct*>(pOther));
    if (pTmp == nullptr)
        return false;
    if (m_asNames.size() != pTmp->m_asNames.size() ||
        m_apItems.size() != pTmp->m_apItems.size())
        return false;
    for (int i = 0; i < m_asNames.size(); ++i)
    {
        int j;
        if (m_apItems[i] == nullptr)
            return false;
        for (j = 0; j < pTmp->m_asNames.size(); ++j)
        {
            if (QString::fromUtf8(m_asNames[i]).compare(pTmp->m_asNames[j], Qt::CaseInsensitive) == 0)
            {
                if (m_apItems[j] == nullptr)
                    return false;
                if (!m_apItems[i]->compareType(pTmp->m_apItems[j]))
                    return false;
                break;
            }
        }
        if (j >= pTmp->m_asNames.size())
            return false;
    }
    return true;
}

/**
 * \brief This overloaded function compares two objects.
 *        All keys and their values are compared.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPstruct::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE)
    {
        if (!compareType(pOther))
            iResult = SECoP_VC_DIFF_TYPE;
        else
        {
            const CSECoPstruct* pTmp(dynamic_cast<const CSECoPstruct*>(pOther));
            for (int i = 0; i < m_asNames.size(); ++i)
            {
                const CSECoPbaseType* pValue(pTmp->getItem(m_asNames[i].constData()));
                if (pValue == nullptr || m_apItems[i] == nullptr)
                {
                    iResult = SECoP_VC_DIFF_TYPE;
                    break;
                }
                SECoP_V_compareResult iResult2(m_apItems[i]->compareValue(pValue));
                if (iResult < iResult2)
                    iResult = iResult2;
                if (iResult == SECoP_VC_DIFF_TYPE)
                    break;
            }
        }
    }
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPstruct::isValid() const
{
    if (!CSECoPbaseType::isValid())
        return false;
    if (m_asNames.size() != m_apItems.size())
        return false;
    for (int i = 0; i < m_asNames.size(); ++i)
    {
        if (m_apItems[i] == nullptr)
            return false;
        const QString &sz1(m_asNames[i]);
        if (sz1.isEmpty())
            return false;
        for (int j = i + 1; j < m_asNames.size(); ++j)
        {
            const QString &sz2(m_asNames[j]);
            if (sz1.compare(sz2, Qt::CaseInsensitive) == 0)
                return false;
        }
    }
    return true;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::clear()
{
    if (!CSECoPbaseType::clear())
        return false;
    for (int i = 0; i < m_apItems.size(); ++i)
        delete m_apItems[i];
    m_asNames.clear();
    m_apItems.clear();
    return true;
}

/**
 * \brief This function returns the number of items.
 * \return number of items
 */
unsigned int CSECoPstruct::getItemCount() const
{
    return static_cast<unsigned int>(m_apItems.size());
}

/**
 * \brief This function searches for a key.
 * \param[in]  szName name of the key
 * \param[out] uIndex index, where this key is stored
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::findItem(const char* szName, unsigned int &uIndex) const
{
    QString szItem(szName);
    for (int i = 0; i < m_asNames.size(); ++i)
    {
        if (szItem.compare(m_asNames[i], Qt::CaseInsensitive) == 0)
        {
            uIndex = static_cast<unsigned int>(i);
            return true;
        }
    }
    uIndex = static_cast<unsigned int>(m_asNames.size());
    return false;
}

/**
 * \brief This function returns the name of a key at given index.
 * \param[in] uIndex index of key
 * \return key name as QByteArray
 */
QByteArray CSECoPstruct::getItemName(unsigned int uIndex) const
{
    QByteArray szResult;
    if (uIndex < static_cast<unsigned int>(m_apItems.size()))
        szResult = m_asNames[static_cast<int>(uIndex)];
    return szResult;
}

/**
 * \brief This function returns the value at given key or an error.
 * \param[in] szName key name
 * \return pointer to SECoP value or nullptr
 */
CSECoPbaseType* CSECoPstruct::getItem(const char* szName) const
{
    unsigned int uIndex(0U);
    if (!findItem(szName, uIndex))
        return nullptr;
    return getItem(uIndex);
}

/**
 * \brief This function returns the value at given index or an error.
 * \param[in] uIndex index of key
 * \return pointer to SECoP value or nullptr
 */
CSECoPbaseType* CSECoPstruct::getItem(unsigned int uIndex) const
{
    if (uIndex >= static_cast<unsigned int>(m_apItems.size()))
        return nullptr;
    return m_apItems[static_cast<int>(uIndex)];
}

/**
 * \brief This function removes a key with value from given name.
 * \param[in] szName key name
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::removeItem(const char* szName)
{
    unsigned int uIndex(0U);
    if (!findItem(szName, uIndex))
        return false;
    return removeItem(uIndex);
}

/**
 * \brief This function removes a key with value from given index.
 * \param[in] uIndex index of key
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::removeItem(unsigned int uIndex)
{
    int iIndex(static_cast<int>(uIndex));
    if (iIndex < 0 || iIndex >= m_apItems.size())
        return false;
    delete m_apItems[iIndex];
    m_asNames.removeAt(iIndex);
    m_apItems.removeAt(iIndex);
    return true;
}

/**
 * \brief This function reallocates memory and appends one key/value pair.
 *        This function takes ownership of the value.
 * \param[in] szName new key to append
 * \param[in] pValue new value to append
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::appendValue(const char* szName, CSECoPbaseType* pValue)
{
    if (szName == nullptr || pValue == nullptr || szName[0] == '\0' ||
        m_asNames.size() != m_apItems.size())
        return false;
    QString szNameTmp(szName);
    for (int i = 0; i < m_asNames.size(); ++i)
        if (szNameTmp.compare(m_asNames[i], Qt::CaseInsensitive) == 0)
            return false;
    for (int i = 0; i < m_apItems.size(); ++i)
        if (pValue == m_apItems[i])
            return false;
    m_asNames.append(szName);
    m_apItems.append(pValue);
    return true;
}

/**
 * \brief This overloaded function reallocates memory and appends one key/value pair.
 *        This function makes a deep copy of the value.
 * \param[in] szName new key to append
 * \param[in] pValue new value to append
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::appendValue(const char* szName, const CSECoPbaseType* pValue)
{
    CSECoPbaseType* pTmp(const_cast<CSECoPbaseType*>(pValue)->duplicate());
    if (appendValue(szName, pTmp))
        return true;
    delete pTmp;
    return false;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        It takes the key "members".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!json.contains("members"))
        return false;
    const nlohmann::json &vMembers(json["members"]);
    if (!vMembers.is_object())
        return false;
    CSECoPstruct::clear();
    for (auto &it : vMembers.items())
    {
        CSECoPbaseType* pValue(CSECoPbaseType::createSECoP(it.value(), false));
        if (pValue == nullptr)
            return false;
        if (!appendValue(it.key().c_str(), pValue))
        {
            delete pValue;
            return false;
        }
    }
    if (!aszDelKeys.contains("members"))
        aszDelKeys.append("members");
    return true;
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing (allow missing parts, imports unknown keys)
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    if (!data.is_object())
        return false;
    if (bStrict)
    {
        for (auto &it : data.items())
        {
            bool bFound(false);
            QString szKey(QString::fromStdString(it.key()));
            for (auto it2 = m_asNames.constBegin(); it2 != m_asNames.constEnd(); ++it2)
            {
                if (QString::fromUtf8(*it2).compare(szKey, Qt::CaseInsensitive) == 0)
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
                return false;
        }
        for (auto it = m_asNames.constBegin(); it != m_asNames.constEnd(); ++it)
        {
            bool bFound(false);
            QString szKey(QString::fromUtf8(*it));
            for (auto &it2 : data.items())
            {
                if (QString::fromStdString(it2.key()).compare(szKey, Qt::CaseInsensitive) == 0)
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
                return false;
        }
    }
    QVector<bool> abTouched;
    abTouched.resize(m_asNames.size());
    for (auto &it : data.items())
    {
        int iIndex;
        QString szKey(QString::fromStdString(it.key()));
        for (iIndex = m_asNames.size() - 1; iIndex >= 0; --iIndex)
            if (QString::fromUtf8(m_asNames[iIndex]).compare(szKey, Qt::CaseInsensitive) == 0)
                break;
        if (iIndex < 0)
        {
            if (bStrict)
                return false;
            CSECoPbaseType* pItem(CSECoPbaseType::importSECoP(it.value()));
            if (pItem == nullptr)
                return false;
            m_asNames.append(szKey.toUtf8());
            m_apItems.append(pItem);
            abTouched.append(true);
            continue;
        }
        if (abTouched[iIndex])
            return false;
        abTouched[iIndex] = true;
        const nlohmann::json &v(it.value());
        if (v.is_null())
            continue;
        if (m_apItems[iIndex] == nullptr)
            return false;
        if (!m_apItems[iIndex]->importSECoP(v, bStrict))
            return false;
    }
    return true;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPstruct::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    nlohmann::json m;
    if (!CSECoPbaseType::exportTypeHelper(json, bArray))
        return false;
    for (int i = 0; i < m_asNames.size(); ++i)
    {
        if (m_asNames[i].isEmpty() || m_apItems[i] == nullptr)
            return false;
        m[m_asNames[i].toStdString()] = m_apItems[i]->exportType();
    }
    json["members"] = m;
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPstruct::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    nlohmann::json o;
    if (pMeMyselfAndI != nullptr)
    {
        for (int i = 0; i < m_asNames.size(); ++i)
        {
            nlohmann::json v;
            if (m_apItems[i] != nullptr)
                v = m_apItems[i]->exportSECoPjson();
            o[m_asNames[i].toStdString()] = v;
        }
    }
    return o;
}

/*****************************************************************************
 * CSECoPtuple
 *****************************************************************************/
/**
 * \brief Standard constructor for a tuple of data (fixed number of values of
 *        different type)
 */
CSECoPtuple::CSECoPtuple()
    : CSECoPbaseType(SECoP_VT_TUPLE)
{
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPtuple::CSECoPtuple(const CSECoPtuple* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
{
    for (auto it = pOther->m_apItems.constBegin(); it != pOther->m_apItems.constEnd(); ++it)
        m_apItems.append((*it)->duplicate());
}

/**
 * \brief Destructor for data. It frees the allocated memory.
 */
CSECoPtuple::~CSECoPtuple()
{
    for (int i = 0; i < m_apItems.size(); ++i)
        delete m_apItems[i];
    m_apItems.clear();
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 *        All items have to match.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPtuple::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    const CSECoPtuple* pTmp(dynamic_cast<const CSECoPtuple*>(pOther));
    if (pTmp == nullptr)
        return false;
    if (m_apItems.size() != pTmp->m_apItems.size())
        return false;
    for (int i = 0; i < m_apItems.size(); ++i)
    {
        if (m_apItems[i] == nullptr || pTmp->m_apItems[i] == nullptr)
            return false;
        if (!m_apItems[i]->compareType(pTmp->m_apItems[i]))
            return false;
    }
    return true;
}

/**
 * \brief This overloaded function compares two objects.
 *        All items are compared.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPtuple::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE)
    {
        if (!compareType(pOther))
            iResult = SECoP_VC_DIFF_TYPE;
        else
        {
            const CSECoPtuple* pTmp(dynamic_cast<const CSECoPtuple*>(pOther));
            for (int i = 0; i < m_apItems.size(); ++i)
            {
                const CSECoPbaseType* pValue(pTmp->getValue(static_cast<unsigned int>(i)));
                if (m_apItems[i] == nullptr || pValue == nullptr)
                {
                    iResult = SECoP_VC_DIFF_TYPE;
                    break;
                }
                SECoP_V_compareResult iResult2(m_apItems[i]->compareValue(pValue));
                if (iResult < iResult2)
                    iResult = iResult2;
                if (iResult == SECoP_VC_DIFF_TYPE)
                    break;
            }
        }
    }
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPtuple::isValid() const
{
    if (!CSECoPbaseType::isValid())
        return false;
    if (m_apItems.isEmpty())
        return false;
    for (int i = 0; i < m_apItems.size(); ++i)
        if (m_apItems[i] == nullptr || !m_apItems[i]->isValid())
            return false;
    return true;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::clear()
{
    for (int i = 0; i < m_apItems.size(); ++i)
        delete m_apItems[i];
    m_apItems.clear();
    return true;
}

/**
 * \brief This function returns the number of items.
 * \return number of items
 */
unsigned int CSECoPtuple::getSize() const
{
    return static_cast<unsigned int>(m_apItems.size());
}

/**
 * \brief This function changes the allowed number of items.
 * \param[in] uNewSize new size
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::setSize(unsigned int uNewSize)
{
    unsigned int uSize(static_cast<unsigned int>(m_apItems.size()));
    if (uSize == uNewSize)
        return true;
    if (uSize < uNewSize)
    {
        while (uSize < uNewSize)
        {
            m_apItems.append(nullptr);
            ++uSize;
        }
    }
    else
    {
        while (uSize > uNewSize)
        {
            delete m_apItems.takeLast();
            --uSize;
        }
    }
    return true;
}

/**
 * \brief This function returns the value at a given index.
 * \param[in]  uIndex tuple index
 * \return pointer to SECoP value or nullptr
 */
CSECoPbaseType* CSECoPtuple::getValue(unsigned int uIndex) const
{
    if (uIndex >= static_cast<unsigned int>(m_apItems.size()))
        return nullptr;
    return m_apItems[static_cast<int>(uIndex)];
}

/**
 * \brief This function stores the value at a given index.
 *        This function takes ownership of the value.
 * \param[in] uIndex tuple index
 * \param[in] pValue new value to store
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::setValue(unsigned int uIndex, CSECoPbaseType* pValue)
{
    if (uIndex >= static_cast<unsigned int>(m_apItems.size()) || pValue == nullptr)
        return false;
    int iIndex(static_cast<int>(uIndex));
    if (m_apItems[iIndex] == pValue)
        return true;
    delete m_apItems[iIndex];
    m_apItems[iIndex] = pValue;
    return true;
}

/**
 * \brief This function stores the value at a given index.
 *        This function makes a deep copy of the value.
 * \param[in] uIndex tuple index
 * \param[in] pValue new value to store
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::setValue(unsigned int uIndex, const CSECoPbaseType* pValue)
{
    if (uIndex >= static_cast<unsigned int>(m_apItems.size()))
        return false;
    CSECoPbaseType* pTmp(const_cast<CSECoPbaseType*>(pValue)->duplicate());
    if (setValue(uIndex, pTmp))
        return true;
    delete pTmp;
    return false;
}

/**
 * \brief This function reallocates memory and appends one value.
 *        This function takes ownership of the value.
 * \param[in] pValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::appendValue(CSECoPbaseType* pValue)
{
    if (pValue == nullptr)
        return false;
    for (int i = 0; i < m_apItems.size(); ++i)
        if (m_apItems[i] == pValue)
            return false;
    m_apItems.append(pValue);
    return true;
}

/**
 * \brief This function reallocates memory and appends one value.
 *        This function makes a deep copy of the value.
 * \param[in] pValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::appendValue(const CSECoPbaseType* pValue)
{
    CSECoPbaseType* pTmp(const_cast<CSECoPbaseType*>(pValue)->duplicate());
    if (appendValue(pTmp))
        return true;
    delete pTmp;
    return false;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        It takes the key "members".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!json.contains("members"))
        return false;
    const nlohmann::json &aMembers(json["members"]);
    if (!aMembers.is_array() || aMembers.empty())
        return false;
    CSECoPtuple::clear();
    for (unsigned int i = 0; i < aMembers.size(); ++i)
    {
        CSECoPbaseType* pValue(CSECoPbaseType::createSECoP(aMembers[i], false));
        if (pValue == nullptr)
            return false;
        if (!appendValue(pValue))
        {
            delete pValue;
            return false;
        }
    }
    if (!aszDelKeys.contains("members"))
        aszDelKeys.append("members");
    return true;
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    if (!data.is_array())
        return false;
    if (bStrict)
    {
        if (data.size() != static_cast<unsigned int>(m_apItems.size()))
            return false;
    }
    else
    {
        for (;;)
        {
            unsigned int i(static_cast<unsigned int>(m_apItems.size()));
            if (i >= data.size())
                break;
            CSECoPbaseType* pItem(CSECoPbaseType::importSECoP(data[i]));
            if (pItem == nullptr)
                return false;
            m_apItems.append(pItem);
        }
    }
    for (unsigned int i = 0; i < data.size(); ++i)
    {
        const nlohmann::json &v(data[i]);
        if (v.is_null())
            continue;
        if (m_apItems[static_cast<int>(i)] == nullptr)
            return false;
        if (!m_apItems[static_cast<int>(i)]->importSECoP(v, bStrict))
            return false;
    }
    return true;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPtuple::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    nlohmann::json m;
    if (!CSECoPbaseType::exportTypeHelper(json, bArray))
        return false;
    for (int i = 0; i < m_apItems.size(); ++i)
    {
        if (m_apItems[i] == nullptr)
            return false;
        m.push_back(m_apItems[i]->exportType());
    }
    json["members"] = m;
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPtuple::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    nlohmann::json a;
    if (pMeMyselfAndI != nullptr)
    {
        for (int i = 0; i < m_apItems.size(); ++i)
        {
            nlohmann::json v;
            if (m_apItems[i] != nullptr)
                v = m_apItems[i]->exportSECoPjson();
            a.push_back(v);
        }
    }
    return a;
}

/*****************************************************************************
 * CSECoParray
 *****************************************************************************/
/**
 * \brief Standard constructor for an array of data (variable number of values of same type)
 */
CSECoParray::CSECoParray()
    : CSECoParrayBase(SECoP_VT_ARRAY)
    , m_pType(nullptr)
{
    m_uMaxSize = INT_MAX;
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoParray::CSECoParray(const CSECoParray* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , CSECoParrayBase(static_cast<const CSECoParrayBase*>(pOther))
    , m_pType(nullptr)
{
    if (pOther->m_pType != nullptr)
        m_pType = pOther->m_pType->duplicate();
    for (auto it = pOther->m_apItems.constBegin(); it != pOther->m_apItems.constEnd(); ++it)
        m_apItems.append((*it)->duplicate());
}

/**
 * \brief Destructor for data. It frees the allocated memory.
 */
CSECoParray::~CSECoParray()
{
    delete m_pType;
    for (int i = 0; i < m_apItems.size(); ++i)
        delete m_apItems[i];
    m_apItems.clear();
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 *        All items have to match.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoParray::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoParrayBase::compareType(pOther))
        return false;
    const CSECoParray* pTmp(dynamic_cast<const CSECoParray*>(pOther));
    if (pTmp == nullptr || m_pType == nullptr || pTmp->m_pType == nullptr)
        return false;
    return (m_pType->compareType(pTmp->m_pType));
}

/**
 * \brief This overloaded function compares two objects.
 *        All items are compared.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoParray::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE)
    {
        if (!compareType(pOther))
            iResult = SECoP_VC_DIFF_TYPE;
        else
        {
            const CSECoParray* pTmp(dynamic_cast<const CSECoParray*>(pOther));
            for (int i = 0; i < m_apItems.size(); ++i)
            {
                const CSECoPbaseType* pValue(pTmp->getValue(static_cast<unsigned int>(i)));
                if (m_apItems[i] == nullptr || pValue == nullptr)
                {
                    iResult = SECoP_VC_DIFF_TYPE;
                    break;
                }
                SECoP_V_compareResult iResult2(m_apItems[i]->compareValue(pValue));
                if (iResult < iResult2)
                    iResult = iResult2;
                if (iResult == SECoP_VC_DIFF_TYPE)
                    break;
            }
        }
    }
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoParray::isValid() const
{
    if (!CSECoParrayBase::isValid() || m_pType == nullptr)
        return false;
    for (int i = 0; i < m_apItems.size(); ++i)
        if (m_apItems[i] == nullptr || !m_pType->compareType(m_apItems[i]))
            return false;
    return true;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoParray::clear()
{
    for (int i = 0; i < m_apItems.size(); ++i)
        delete m_apItems[i];
    m_apItems.clear();
    return true;
}

/**
 * \brief This functions returns access to the data type of all array elements.
 * \return SECoP value or nullptr
 */
CSECoPbaseType* CSECoParray::getArrayType() const
{
    return m_pType;
}

/**
 * \brief This functions changes the data type of all array elements.
 *        This function takes ownership of the type value.
 * \param[in] pType new data type of array elements
 * \return true: successful, false: not successful
 */
bool CSECoParray::setArrayType(CSECoPbaseType* pType)
{
    if (pType != nullptr)
    {
        if (!clear())
            return false;
        delete m_pType;
        m_pType = pType;
        return true;
    }
    else
        return false;
}

/**
 * \brief This functions changes the data type of all array elements.
 *        This function makes a deep copy of the type value.
 * \param[in] pType new data type of array elements
 * \return true: successful, false: not successful
 */
bool CSECoParray::setArrayType(const CSECoPbaseType* pType)
{
    CSECoPbaseType* pTmp(const_cast<CSECoPbaseType*>(pType)->duplicate());
    if (setArrayType(pTmp))
        return true;
    delete pTmp;
    return false;
}

/**
 * \brief This function stores new allowed minimum and maximum array size and
 *        resizes the content.
 *        The minimum array size has to be lower or equal the maximum array size.
 * \param[in] uMinimum      minimum array size
 * \param[in] uMaximum      maximum array size
 * \param[in] bForceRealloc true: force a reallocation of memory, false: no reallocation (ignored)
 * \return true: successful, false: not successful
 */
bool CSECoParray::setMinMaxSize(unsigned int uMinimum, unsigned int uMaximum, bool bForceRealloc)
{
    if (uMaximum > INT_MAX)
        return false;
    if (uMinimum == m_uMinSize && uMaximum == m_uMaxSize)
        return true;
    if (!CSECoParrayBase::setMinMaxSize(uMinimum, uMaximum, bForceRealloc))
        return false;
    unsigned int uNewMax(uMaximum);
    if (uNewMax >= INT_MAX)
        uNewMax = static_cast<unsigned int>(m_apItems.size());
    while (static_cast<unsigned int>(m_apItems.size()) < uNewMax)
        m_apItems.append(nullptr);
    while (static_cast<unsigned int>(m_apItems.size()) > uNewMax)
        delete m_apItems.takeLast();
    return true;
}

/**
 * \brief This function returns the current array size.
 * \return current array size
 */
unsigned int CSECoParray::getSize() const
{
    return static_cast<unsigned int>(m_apItems.size());
}

/**
 * \brief This function changes the array size in the allowed ranges.
 * \param[in] uSize new array size
 * \return true: successful, false: not successful
 */
bool CSECoParray::setSize(unsigned int uSize)
{
    if (!CSECoParrayBase::setSize(uSize))
        return false;
    unsigned int uNewMax(m_uMaxSize);
    if (uNewMax >= INT_MAX)
        uNewMax = static_cast<unsigned int>(m_apItems.size());
    if (uNewMax < uSize)
        uNewMax = uSize;
    while (static_cast<unsigned int>(m_apItems.size()) < uNewMax)
        m_apItems.append(nullptr);
    while (static_cast<unsigned int>(m_apItems.size()) > uNewMax)
        delete m_apItems.takeLast();
    return true;
}

/**
 * \brief This function stores the value at a given index.
 *        This function takes ownership of the value.
 * \param[in] uIndex array index
 * \param[in] pValue new value to store
 * \return true: successful, false: not successful
 */
bool CSECoParray::setValue(unsigned int uIndex, CSECoPbaseType* pValue)
{
    int iPos(static_cast<int>(uIndex));
    if (iPos < 0 || iPos >= m_apItems.size())
        return false;
    if (m_pType == nullptr)
    {
        if (!setArrayType(pValue->duplicate()))
            return false;
    }
    else if (!m_pType->compareType(pValue))
        return false;
    if (m_apItems[iPos] != nullptr)
        delete m_apItems[iPos];
    m_apItems[iPos] = pValue;
    return true;
}

/**
 * \brief This function stores the value at a given index.
 *        This function makes a deep copy of the value.
 * \param[in] uIndex array index
 * \param[in] pValue new value to store
 * \return true: successful, false: not successful
 */
bool CSECoParray::setValue(unsigned int uIndex, const CSECoPbaseType* pValue)
{
    int iPos(static_cast<int>(uIndex));
    if (iPos < 0 || iPos >= m_apItems.size())
        return false;
    if (m_pType == nullptr)
    {
        if (!setArrayType(pValue->duplicate()))
            return false;
    }
    else if (!m_pType->compareType(pValue))
        return false;
    CSECoPbaseType* pTmp(const_cast<CSECoPbaseType*>(pValue)->duplicate());
    if (pTmp == nullptr)
        return false;
    if (m_apItems[iPos] != nullptr)
        delete m_apItems[iPos];
    m_apItems[iPos] = pTmp;
    return true;
}

/**
 * \brief This function returns the stored value at a given index.
 * \param[in]  uIndex array index
 * \return SECoP value or nullptr
 */
CSECoPbaseType* CSECoParray::getValue(unsigned int uIndex) const
{
    if (uIndex >= static_cast<unsigned int>(m_apItems.size()))
        return nullptr;
    return m_apItems[static_cast<int>(uIndex)];
}

/**
 * \brief This function reallocates memory and appends one value.
 *        This function takes ownership of the value.
 * \param[in] pValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParray::appendValue(CSECoPbaseType* pValue)
{
    if (static_cast<unsigned int>(m_apItems.size()) >= m_uMaxSize)
        return false;
    if (m_pType == nullptr)
    {
        if (!setArrayType(pValue->duplicate()))
            return false;
    }
    else if (!m_pType->compareType(pValue))
        return false;
    int iPos(m_apItems.size());
    if (!setSize(static_cast<unsigned int>(iPos) + 1U))
        return false;
    if (m_apItems[iPos] != nullptr)
        delete m_apItems[iPos];
    m_apItems[iPos] = pValue;
    return true;
}

/**
 * \brief This function reallocates memory and appends one value.
 *        This function makes a deep copy of the value.
 * \param[in] pValue new value to append at end
 * \return true: successful, false: not successful
 */
bool CSECoParray::appendValue(const CSECoPbaseType* pValue)
{
    if (static_cast<unsigned int>(m_apItems.size()) >= m_uMaxSize)
        return false;
    if (m_pType == nullptr)
    {
        if (!setArrayType(pValue->duplicate()))
            return false;
    }
    else if (!m_pType->compareType(pValue))
        return false;
    int iPos(m_apItems.size());
    if (!setSize(static_cast<unsigned int>(iPos) + 1U))
        return false;
    CSECoPbaseType* pTmp(const_cast<CSECoPbaseType*>(pValue)->duplicate());
    if (pTmp == nullptr)
        return false;
    if (m_apItems[iPos] != nullptr)
        delete m_apItems[iPos];
    m_apItems[iPos] = pTmp;
    return true;
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        It takes the key "members".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoParray::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!CSECoParrayBase::createSECoPHelper(json, aszDelKeys))
        return false;
    if (!json.contains("members"))
        return false;
    const nlohmann::json &v(json["members"]);
    if (!v.is_object() || !v.contains("type"))
        return false;
    CSECoParray::clear();
    if (m_pType != nullptr)
        delete m_pType;
    m_pType = CSECoPbaseType::createSECoP(v, false);
    m_uSize = 0U;
    if (m_pType == nullptr)
        return false;
    if (!aszDelKeys.contains("members"))
        aszDelKeys.append("members");
    return true;
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the object.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing
 * \return true: successful, false: not successful
 */
bool CSECoParray::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (data.is_null() && !bStrict)
        return true;
    if (!data.is_array() || m_pType == nullptr)
        return false;
    unsigned int uSize(static_cast<unsigned int>(data.size()));
    if (!setSize(uSize))
        return false;
    for (int i = 0; i < static_cast<int>(data.size()); ++i)
    {
        const nlohmann::json &v(data[static_cast<unsigned int>(i)]);
        if (v.is_null())
            continue;
        if (m_apItems[i] == nullptr)
        {
            m_apItems[i] = m_pType->duplicate();
            if (m_apItems[i] == nullptr)
                return false;
        }
        if (!m_apItems[i]->importSECoP(v, bStrict))
            return false;
    }
    return true;
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the key "members" and possibly erases the key "maxlen".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoParray::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    if (m_pType == nullptr || !CSECoParrayBase::exportTypeHelper(json, bArray))
        return false;
    if (m_uMaxSize >= INT_MAX)
        json.erase("maxlen");
    json["members"] = m_pType->exportType();
    return true;
}

/**
 * \brief This overloaded function exports the value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoParray::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    nlohmann::json a;
    if (pMeMyselfAndI != nullptr)
    {
        for (int i = 0; i < m_apItems.size(); ++i)
        {
            nlohmann::json v;
            if (m_apItems[i] != nullptr)
                v = m_apItems[i]->exportSECoPjson();
            a.push_back(v);
        }
    }
    return a;
}

/*****************************************************************************
 * CSECoPcommand
 *****************************************************************************/
/**
 * \brief Standard constructor of a SECoP command. This class is used as
 *        convenience class while using SECoP accessibles.
 */
CSECoPcommand::CSECoPcommand()
    : CSECoPbaseType(SECoP_VT_COMMAND)
    , m_pArgument(nullptr)
    , m_pResult(nullptr)
{
}

/**
 * \brief special copy constructor for "duplicate": it copyies the type and
 *        additional information and puts the newly created object into a
 *        global list of known objects.
 * \param[in] pOther existing object, which should be copied later
 */
CSECoPcommand::CSECoPcommand(const CSECoPcommand* pOther)
    : CSECoPbaseType(static_cast<const CSECoPbaseType*>(pOther))
    , m_pArgument(nullptr)
    , m_pResult(nullptr)
{
    m_pArgument = pOther->m_pArgument->duplicate();
    m_pResult   = pOther->m_pResult->duplicate();
}

/**
 * \brief Destructor for command data. It frees the allocated memory.
 */
CSECoPcommand::~CSECoPcommand()
{
    clear();
}

/**
 * \brief This overloaded function compares, if the type of both objects are the same.
 * \param[in] pOther other object to be compared
 * \return true: same object types, false: otherwise
 */
bool CSECoPcommand::compareType(const CSECoPbaseType* pOther) const
{
    if (!CSECoPbaseType::compareType(pOther))
        return false;
    const CSECoPcommand* pTmp(dynamic_cast<const CSECoPcommand*>(pOther));
    if (pTmp == nullptr)
        return false;
    if (m_pArgument == nullptr || pTmp->m_pArgument == nullptr ||
        m_pResult   == nullptr || pTmp->m_pResult   == nullptr)
        return false;
    return m_pArgument->compareType(pTmp->m_pArgument) &&
           m_pResult  ->compareType(pTmp->m_pResult);
}

/**
 * \brief This overloaded function compares two objects.
 * \param[in] pOther other object to be compared
 * \return if the objects are equal, differ (slightly) in value only, differ in type
 */
SECoP_V_compareResult CSECoPcommand::compareValue(const CSECoPbaseType* pOther) const
{
    SECoP_V_compareResult iResult(CSECoPbaseType::compareValue(pOther));
    if (iResult != SECoP_VC_DIFF_TYPE)
    {
        if (!compareType(pOther))
            iResult = SECoP_VC_DIFF_TYPE;
        else
        {
            const CSECoPcommand* pTmp(dynamic_cast<const CSECoPcommand*>(pOther));
            if (pTmp == nullptr)
                iResult = SECoP_VC_DIFF_TYPE;
            else
            {
                SECoP_V_compareResult iResult2(m_pArgument->compareValue(pTmp->m_pArgument));
                if (iResult < iResult2)
                    iResult = iResult2;
                iResult2 = m_pResult->compareValue(pTmp->m_pResult);
                if (iResult < iResult2)
                    iResult = iResult2;
            }
        }
    }
    return iResult;
}

/**
 * \brief This overloaded function checks, if the object is valid and usable.
 * \return true: valid object, false: otherwise
 */
bool CSECoPcommand::isValid() const
{
    if (!CSECoPbaseType::isValid())
        return false;
    if (m_pArgument != nullptr)
        if (!m_pArgument->isValid())
            return false;
    if (m_pResult != nullptr)
        if (!m_pResult->isValid())
            return false;
    return true;
}

/**
 * \brief This overloaded function clears the value of the object.
 * \return true: successful, false: not successful
 */
bool CSECoPcommand::clear()
{
    if (m_pArgument != nullptr)
    {
        delete m_pArgument;
        m_pArgument = nullptr;
    }
    if (m_pResult != nullptr)
    {
        delete m_pResult;
        m_pResult = nullptr;
    }
    return true;
}

/**
 * \brief This functions returns the data type of the command argument.
 * \return data type of command argument
 */
CSECoPbaseType* CSECoPcommand::getArgument() const
{
    return m_pArgument;
}

/**
 * \brief This functions returns the data type of the command result.
 * \return data type of command result
 */
CSECoPbaseType* CSECoPcommand::getResult() const
{
    return m_pResult;
}

/**
 * \brief This function stores the data type for SECoP command arguments.
 *        This function makes a deep copy of the value.
 * \param[in] pArgument new data type for command arguments
 * \return true: successful, false: not successful
 */
bool CSECoPcommand::setArgument(const CSECoPbaseType* pArgument)
{
    if (pArgument != nullptr)
    {
        if (!pArgument->isValid())
            return false;
        const CSECoPcommand* pCommand(dynamic_cast<const CSECoPcommand*>(pArgument));
        if (pCommand != nullptr)
            return false;
    }
    delete m_pArgument;
    if (pArgument == nullptr)
    {
        m_pArgument = nullptr;
        return true;
    }
    m_pArgument = pArgument->duplicate();
    return (m_pArgument != nullptr);
}

/**
 * \brief This function stores the data type for SECoP command results.
 *        This function makes a deep copy of the value.
 * \param[in] pResult new data type for command results
 * \return true: successful, false: not successful
 */
bool CSECoPcommand::setResult(const CSECoPbaseType* pResult)
{
    if (pResult != nullptr)
    {
        if (!pResult->isValid())
            return false;
        const CSECoPcommand* pCommand(dynamic_cast<const CSECoPcommand*>(pResult));
        if (pCommand != nullptr)
            return false;
    }
    delete m_pResult;
    if (pResult == nullptr)
    {
        m_pResult = nullptr;
        return true;
    }
    m_pResult = pResult->duplicate();
    return (m_pResult != nullptr);
}

/**
 * \brief This overloaded function takes SECoP-JSON and initializes the object.
 *        It takes the keys "argument" and "result".
 * \param[in] json       SECoP-JSON for newly created object
 * \param[in] aszDelKeys standard keys, which should be removed for additional information
 * \return true: successful, false: not successful
 */
bool CSECoPcommand::createSECoPHelper(nlohmann::json &json, QStringList &aszDelKeys)
{
    if (!clear())
        return false;
    struct item
    {
        const char*      szName;
        CSECoPbaseType** pVar;
    } aItems[2] = { { "argument", &m_pArgument }, { "result", &m_pResult } };
    for (int i = 0; i < 2; ++i)
    {
        item* pItem(&aItems[i]);
        if (!json.contains(pItem->szName))
            continue;
        if (!aszDelKeys.contains(pItem->szName))
            aszDelKeys.append(pItem->szName);
        const nlohmann::json &v(json[pItem->szName]);
        if (v.is_null())
            continue;
        CSECoPbaseType* pVar(CSECoPbaseType::createSECoP(v, false));
        if (pVar == nullptr)
            return false;
        *aItems[i].pVar = pVar;
    }
    return true;
}

/**
 * \brief This overloaded function validates and imports a SECoP-JSON value into the argument.
 * \param[in] data    SECoP-JSON value
 * \param[in] bStrict true: be strict, false: relax parsing
 * \return true: successful, false: not successful
 */
bool CSECoPcommand::importSECoP(const nlohmann::json &data, bool bStrict)
{
    if (m_pArgument != nullptr)
        return m_pArgument->importSECoP(data, bStrict);
    return data.is_null();
}

/**
 * \brief This overloaded helper function returns the SECoP data information of this object.
 *        It puts the keys "argument" and "result".
 * \param[out] json   type plus standard and additional information
 * \param[in]  bArray flag, if this is an array type or not
 * \return true: successful, false: not successful
 */
bool CSECoPcommand::exportTypeHelper(nlohmann::json &json, bool bArray) const
{
    if (!CSECoPbaseType::exportTypeHelper(json, bArray))
        return false;
    json["argument"] = nlohmann::json();
    json["result"] = nlohmann::json();
    if (m_pArgument != nullptr && dynamic_cast<const CSECoPnull*>(m_pArgument) == nullptr)
        json["argument"] = m_pArgument->exportType();
    if (m_pResult != nullptr && dynamic_cast<const CSECoPnull*>(m_pResult) == nullptr)
        json["result"] = m_pResult->exportType();
    return true;
}

/**
 * \brief This overloaded function exports the resulting value as SECoP value.
 * \return SECoP value
 */
nlohmann::json CSECoPcommand::exportSECoPjson() const
{
    const CSECoPbaseType* pMeMyselfAndI(this);
    if (pMeMyselfAndI != nullptr && m_pResult != nullptr)
        return m_pResult->exportSECoPjson();
    else
        return nlohmann::json();
}
