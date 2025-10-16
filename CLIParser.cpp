#include <cstring>
#include "CLIParser.hpp"
#include <span>
#include <iostream>

void UCLI::Parser::parse(const int argc, char** argv,
                         ArrayFlag* arrayFlags, const size_t arrayFlagsSize,
                         BooleanFlag* booleanFlags, const size_t booleanFlagsSize,
                         BooleanFlagWithFunc* booleanFlagsWithFunc, const size_t booleanFlagsWithFuncSize,
                         Pair* pairs, const size_t pairsSize,
                         PairWithFunc* pairsWithFunc, const size_t pairsWithFuncSize) noexcept
{
    std::string singleChar;
    singleChar.resize(1);

    std::vector<char*> currentArray;

    for (int i = 0; i < argc; i++)
    {
        std::string tmp = argv[i];
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
                    parseShortArgument(currentArray, singleChar, tmp,
                                       arrayFlags, arrayFlagsSize,
                                       booleanFlags, booleanFlagsSize,
                                       booleanFlagsWithFunc, booleanFlagsWithFuncSize);
            }
        }
        else
            currentArray.push_back(argv[i]);
    }

    data.currentArrayFlag->func(data.currentArrayFlag, currentArray.data(), currentArray.size());
}

void UCLI::Parser::setDefaultArray(void* additionalData, const ArrayFlagFunc func) noexcept
{
    data.defaultArrayFlag.additionalData = additionalData;
    data.defaultArrayFlag.func = func;
}

void UCLI::Parser::setDelimiter(const char del) noexcept
{
    data.delimiter = del;
}

void UCLI::Parser::setArgumentStyleWindows(const bool bWindows) noexcept
{
    data.bWindowsStyle = bWindows;
}

void UCLI::Parser::setUnknownArgumentCallback(const UnknownArgumentsCallback func, void* callbackData) noexcept
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
        .func = [](ArrayFlag*, char**, size_t) -> void {},
    };
}

UCLI::Parser::Parser() noexcept
{
    init();
}

#define FOR_EACH_WITH_CHECK(x, y, z, w) if ((x) != nullptr) for (auto& z : std::span(x, y)) w

void UCLI::Parser::parseShortArgument(std::vector<char*>& args, std::string& singleChar, const std::string& tmp,
                                      ArrayFlag* arrayFlags, const size_t arrayFlagsSize,
                                      BooleanFlag* booleanFlags, const size_t booleanFlagsSize,
                                      BooleanFlagWithFunc* booleanFlagsWithFunc, const size_t booleanFlagsWithFuncSize) noexcept
{
    for (size_t f = 1; f < tmp.size(); f++)
    {
        singleChar[0] = tmp[f];

        FOR_EACH_WITH_CHECK(booleanFlags, booleanFlagsSize, a, {
            if (a.shortType == singleChar)
            {
                if (data.bFlipBool)
                    *a.flag = !*a.flag;
                else
                    *a.flag = true;
                goto continue_from_single_flag_inner_loops;
            }
        });
        FOR_EACH_WITH_CHECK(booleanFlagsWithFunc, booleanFlagsWithFuncSize, a, {
            if (a.shortType == singleChar)
            {
                a.func(&a);
                goto continue_from_single_flag_inner_loops;
            }
        });
        FOR_EACH_WITH_CHECK(arrayFlags, arrayFlagsSize, a, {
            if (singleChar == a.shortType)
            {
                data.currentArrayFlag->func(data.currentArrayFlag, args.data(), args.size());
                args.clear();

                data.currentArrayFlag = &a;
                goto continue_from_single_flag_inner_loops;
            }
        });
        data.unknownArgumentsCallback(singleChar.c_str(), data.unknownArgumentsCallbackAdditionalData);
continue_from_single_flag_inner_loops:;
    }
}

void UCLI::Parser::parseLongArgument(std::vector<char*>& args, const uint8_t frontTruncate, const std::string& tmp,
                       ArrayFlag* arrayFlags, const size_t arrayFlagsSize,
                       BooleanFlag* booleanFlags, const size_t booleanFlagsSize,
                       BooleanFlagWithFunc* booleanFlagsWithFunc, const size_t booleanFlagsWithFuncSize,
                       Pair* pairs, const size_t pairsSize,
                       PairWithFunc* pairsWithFunc, const size_t pairsWithFuncSize, const bool bCheckShort) noexcept
{
    std::string currentTmp = tmp.substr(frontTruncate);

    FOR_EACH_WITH_CHECK(booleanFlags, booleanFlagsSize, a, {
        if (a.longType == currentTmp)
        {
            if (data.bFlipBool)
                *a.flag = !*a.flag;
            else
                *a.flag = true;
            return;
        }
    });
    FOR_EACH_WITH_CHECK(booleanFlagsWithFunc, booleanFlagsWithFuncSize, a, {
        if (a.longType == currentTmp)
        {
            a.func(&a);
            return;
        }
    });
    FOR_EACH_WITH_CHECK(pairs, pairsSize, a, {
        std::string it = a.longType;
        a.InternalbFound = false;
        // This is a valid pair
        if (currentTmp.starts_with(it + "="))
        {
            currentTmp.erase(0, it.length() + 1);

            // Allocate memory for this
            a.data = static_cast<const char*>(malloc(currentTmp.size()));
            a.InternalbFound = true;
            memcpy((void*)a.data, currentTmp.data(), currentTmp.size());
            return;
        }
    });
    FOR_EACH_WITH_CHECK(pairsWithFunc, pairsWithFuncSize, a, {
        std::string it = a.longType;
        // This is a valid pair
        if (currentTmp.starts_with(it + "="))
        {
            currentTmp.erase(0, it.length() + 1);
            a.func(&a, currentTmp.c_str());
            return;
        }
    });
    FOR_EACH_WITH_CHECK(arrayFlags, arrayFlagsSize, a, {
        if (currentTmp == a.longType || (bCheckShort && currentTmp == a.shortType))
        {
            data.currentArrayFlag->func(data.currentArrayFlag, args.data(), args.size());
            args.clear();

            data.currentArrayFlag = &a;
            return;
        }
    });

    data.unknownArgumentsCallback(tmp.c_str(), data.unknownArgumentsCallbackAdditionalData);
}

void UCLI::Parser::cleanupPairs(Pair* pairs, const size_t pairsSize)
{
    FOR_EACH_WITH_CHECK(pairs, pairsSize, a, {
        if (a.InternalbFound)
        {
            free((void*)a.data);
            a.InternalbFound = false;
        }
    })
}

void UCLI::Parser::setBooleanFlipping(const bool bFlip) noexcept
{
    data.bFlipBool = bFlip;
}

#define PASS_VECTOR(x, y) const_cast<x*>((y).data()), (y).size()

void UCLI::Parser::parse(const int argc, char** argv, const std::vector<ArrayFlag>& arrayFlags,
                         const std::vector<BooleanFlag>& booleanFlags,
                         const std::vector<BooleanFlagWithFunc>& booleanFlagsWithFunc,
                         const std::vector<Pair>& pairs,
                         const std::vector<PairWithFunc>& pairsWithFunc) noexcept
{
    parse(argc, argv,
          PASS_VECTOR(ArrayFlag, arrayFlags),
          PASS_VECTOR(BooleanFlag, booleanFlags),
          PASS_VECTOR(BooleanFlagWithFunc, booleanFlagsWithFunc),
          PASS_VECTOR(Pair, pairs),
          PASS_VECTOR(PairWithFunc, pairsWithFunc));
}
