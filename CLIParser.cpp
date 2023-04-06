#include <cstring>
#include "CLIParser.hpp"
#ifdef UCLI_COMPILE_WITH_UTF_SUPPORT
    #include "utf8cpp/utf8.h"
#endif

void UCLI::Parser::parse(int argc, char** argv,
                         UCLI_Parser_ArrayFlag* arrayFlags, size_t arrayFlagsSize,
                         UCLI_Parser_BooleanFlag* booleanFlags, size_t booleanFlagsSize,
                         UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc, size_t booleanFlagsWithFuncSize,
                         UCLI_Parser_Pair* pairs, size_t pairsSize,
                         UCLI_Parser_PairWithFunc* pairsWithFunc, size_t pairsWithFuncSize) noexcept
{
    std::string tmp;

    std::u32string singleCharU32;
    singleCharU32.resize(1);

    std::string singleChar;
    singleChar.resize(1);

    std::vector<char*> currentArray;

    for (int i = 0; i < argc; i++)
    {
        tmp = argv[i];
        if (tmp.size() >= 2 && tmp[0] == data.delimiter)
        {
            // long flag
            if (tmp[1] == data.delimiter)
                parseLongArgument(currentArray, 2, tmp,
                                  arrayFlags, arrayFlagsSize,
                                  booleanFlags, booleanFlagsSize,
                                  booleanFlagsWithFunc, booleanFlagsWithFuncSize,
                                  pairs, pairsSize,
                                  pairsWithFunc, pairsWithFuncSize);
            else // short flag
            {
                if (data.bWindowsStyle)
                    parseLongArgument(currentArray, 1, tmp,
                                      arrayFlags, arrayFlagsSize,
                                      booleanFlags, booleanFlagsSize,
                                      booleanFlagsWithFunc, booleanFlagsWithFuncSize,
                                      pairs, pairsSize,
                                      pairsWithFunc, pairsWithFuncSize, true);
                else
                    parseShortArgument(currentArray, singleChar, singleCharU32, tmp, arrayFlags, arrayFlagsSize, booleanFlags, booleanFlagsSize, booleanFlagsWithFunc, booleanFlagsWithFuncSize);
            }
        }
        else
            currentArray.push_back(argv[i]);
    }

    data.currentArrayFlag->func(data.currentArrayFlag, currentArray.data(), currentArray.size());
}

void UCLI::Parser::setDefaultArray(void* additionalData, UCLI_Parser_ArrayFlagFunc func) noexcept
{
    data.defaultArrayFlag.additionalData = additionalData;
    data.defaultArrayFlag.func = func;
}

void UCLI::Parser::setDelimiter(char del) noexcept
{
    data.delimiter = del;
}

void UCLI::Parser::setArgumentStyleWindows(bool bWindows) noexcept
{
    data.bWindowsStyle = bWindows;
}

void UCLI::Parser::setUnknownArgumentCallback(UCLI_Parser_UnknownArgumentsCallback func, void* callbackData) noexcept
{
    data.unknownArgumentsCallback = func;
    data.unknownArgumentsCallbackAdditionalData = callbackData;
}

void UCLI::Parser::init() noexcept
{
    data.currentArrayFlag = &data.defaultArrayFlag;
    data.delimiter = '-';
    data.bWindowsStyle = false;
    data.bFlipBool = false;

    data.defaultArrayFlag = {
        .longType = "",
        .shortType = "",
        .additionalData = nullptr,
        .func = [](UCLI_Parser_ArrayFlag*, char**, size_t) -> void {},
    };
}

UCLI::Parser::Parser() noexcept
{
    init();
}

void UCLI::Parser::parseShortArgument(std::vector<char*>& args, std::string& singleChar, std::u32string& singleCharU32, const std::string& tmp,
                                      UCLI_Parser_ArrayFlag* arrayFlags, size_t arrayFlagsSize,
                                      UCLI_Parser_BooleanFlag* booleanFlags, size_t booleanFlagsSize,
                                      UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc, size_t booleanFlagsWithFuncSize) noexcept
{
#ifdef UCLI_COMPILE_WITH_UTF_SUPPORT
    utf8::iterator begin(tmp.begin() + 1, tmp.begin() + 1, tmp.end());
    utf8::iterator end(tmp.end(), tmp.begin() + 1, tmp.end());

    for (auto f = begin; f != end; ++f)
    {
        singleCharU32[0] = *f;

        if (booleanFlags != nullptr)
        {
            for (size_t t = 0; t < booleanFlagsSize; t++)
            {
                auto& a = booleanFlags[t];
                if (a.shortType == utf8::utf32to8(singleCharU32))
                {
                    if (data.bFlipBool)
                        *a.flag = !*a.flag;
                    else
                        *a.flag = true;
                    goto continue_from_single_flag_inner_loops;
                }
            }
        }
        if (booleanFlagsWithFunc != nullptr)
        {
            for (size_t t = 0; t < booleanFlagsWithFuncSize; t++)
            {
                auto& a = booleanFlagsWithFunc[t];
                if (a.shortType == utf8::utf32to8(singleCharU32))
                {
                    a.func(&a);
                    goto continue_from_single_flag_inner_loops;
                }
            }
        }
        if (arrayFlags != nullptr)
        {
            for (size_t i = 0; i < arrayFlagsSize; i++)
            {
                if (arrayFlags[i].shortType == utf8::utf32to8(singleCharU32))
                {
                    data.currentArrayFlag->func(data.currentArrayFlag, args.data(), args.size());
                    args.clear();

                    data.currentArrayFlag = &arrayFlags[i];
                    goto continue_from_single_flag_inner_loops;
                }
            }
        }
        data.unknownArgumentsCallback(utf8::utf32to8(singleCharU32).c_str(), data.unknownArgumentsCallbackAdditionalData);
continue_from_single_flag_inner_loops:;
    }
#else
    for (size_t f = 1; f < tmp.size(); f++)
    {
        singleChar[0] = tmp[f];

        if (booleanFlags != nullptr)
        {
            for (size_t t = 0; t < booleanFlagsSize; t++)
            {
                auto& a = booleanFlags[t];
                if (a.shortType == singleChar)
                {
                    if (data.bFlipBool)
                        *a.flag = !*a.flag;
                    else
                        *a.flag = true;
                    goto continue_from_single_flag_inner_loops;
                }
            }
        }
        if (booleanFlagsWithFunc != nullptr)
        {
            for (size_t t = 0; t < booleanFlagsWithFuncSize; t++)
            {
                auto& a = booleanFlagsWithFunc[t];
                if (a.shortType == singleChar)
                {
                    a.func(&a);
                    goto continue_from_single_flag_inner_loops;
                }
            }
        }
        if (arrayFlags != nullptr)
        {
            for (size_t i = 0; i < arrayFlagsSize; i++)
            {
                if (singleChar == arrayFlags[i].shortType)
                {
                    data.currentArrayFlag->func(data.currentArrayFlag, args.data(), args.size());
                    args.clear();

                    data.currentArrayFlag = &arrayFlags[i];
                    goto continue_from_single_flag_inner_loops;
                }
            }
        }
        data.unknownArgumentsCallback(singleChar.c_str(), data.unknownArgumentsCallbackAdditionalData);
continue_from_single_flag_inner_loops:;
    }
#endif
}

void UCLI::Parser::parseLongArgument(std::vector<char*>& args, uint8_t frontTruncate, std::string& tmp,
                       UCLI_Parser_ArrayFlag* arrayFlags, size_t arrayFlagsSize,
                       UCLI_Parser_BooleanFlag* booleanFlags, size_t booleanFlagsSize,
                       UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc, size_t booleanFlagsWithFuncSize,
                       UCLI_Parser_Pair* pairs, size_t pairsSize,
                       UCLI_Parser_PairWithFunc* pairsWithFunc, size_t pairsWithFuncSize, bool bCheckShort) noexcept
{
    std::string currentTmp = tmp;
    currentTmp.erase(0, frontTruncate);

    if (booleanFlags != nullptr)
    {
        for (size_t t = 0; t < booleanFlagsSize; t++)
        {
            auto& a = booleanFlags[t];
            if (a.longType == currentTmp)
            {
                if (data.bFlipBool)
                    *a.flag = !*a.flag;
                else
                    *a.flag = true;
                return;
            }
        }
    }
    if (booleanFlagsWithFunc != nullptr)
    {
        for (size_t t = 0; t < booleanFlagsWithFuncSize; t++)
        {
            auto& a = booleanFlagsWithFunc[t];
            if (a.longType == currentTmp)
            {
                a.func(&a);
                return;
            }
        }
    }

    if (pairs != nullptr)
    {
        for (size_t i = 0; i < pairsSize; i++)
        {
            std::string it = pairs[i].longType;
            pairs[i].InternalbFound = false;
            // This is a valid pair
            if (currentTmp.starts_with(it + "="))
            {
                currentTmp.erase(0, it.length() + 1);

                // Allocate memory for this
                pairs[i].data = static_cast<const char*>(malloc(currentTmp.size()));
                pairs[i].InternalbFound = true;
                memcpy((void*)pairs[i].data, currentTmp.data(), currentTmp.size());
                return;
            }
        }
    }


    if (pairsWithFunc != nullptr)
    {
        for (size_t i = 0; i < pairsWithFuncSize; i++)
        {
            std::string it = pairsWithFunc[i].longType;
            // This is a valid pair
            if (currentTmp.starts_with(it + "="))
            {
                currentTmp.erase(0, it.length() + 1);
                pairsWithFunc[i].func(&pairsWithFunc[i], currentTmp.c_str());
                return;
            }
        }
    }

    if (arrayFlags != nullptr)
    {
        for (size_t i = 0; i < arrayFlagsSize; i++)
        {
            if (currentTmp == arrayFlags[i].longType || (bCheckShort && currentTmp == arrayFlags[i].shortType))
            {
                data.currentArrayFlag->func(data.currentArrayFlag, args.data(), args.size());
                args.clear();

                data.currentArrayFlag = &arrayFlags[i];
                return;
            }
        }
    }

    data.unknownArgumentsCallback(tmp.c_str(), data.unknownArgumentsCallbackAdditionalData);
}

void UCLI::Parser::cleanupPairs(UCLI_Parser_Pair* pairs, size_t pairsSize)
{
    if (pairs != nullptr)
    {
        for (size_t i = 0; i < pairsSize; i++)
        {
            if (pairs[i].InternalbFound)
            {
                free((void*)pairs[i].data);
                pairs[i].InternalbFound = false;
            }
        }
    }
}

void UCLI::Parser::setBooleanFlipping(bool bFlip) noexcept
{
    data.bFlipBool = bFlip;
}
