#include "cucli.h"
#include "../CLIParser.hpp"
#ifdef __cplusplus
extern "C"
{
#endif
#define to_class(data) ((UCLI::Parser*)(data))

    void UCLI_Parser_init(UCLI_Parser_Data* data)
    {
        to_class(data)->init();
    }

    void UCLI_Parser_setDelimiter(UCLI_Parser_Data* data, const char del)
    {
        to_class(data)->setDelimiter(del);
    }

    void UCLI_Parser_setArgumentStyleWindows(UCLI_Parser_Data* data, const bool bWindows)
    {
        to_class(data)->setArgumentStyleWindows(bWindows);
    }

    void UCLI_Parser_setUnknownArgumentCallback(UCLI_Parser_Data* data, const UCLI_Parser_UnknownArgumentsCallback func, void* callbackData)
    {
        to_class(data)->setUnknownArgumentCallback(func, callbackData);
    }

    void UCLI_Parser_setDefaultArray(UCLI_Parser_Data* data, void* additionalData, const UCLI_Parser_ArrayFlagFunc func)
    {
        to_class(data)->setDefaultArray(additionalData, func);
    }

    void UCLI_Parser_parse(UCLI_Parser_Data* data, const int argc, char** argv,
                           UCLI_Parser_ArrayFlag* arrayFlags, const size_t arrayFlagsSize,
                           UCLI_Parser_BooleanFlag* booleanFlags, const size_t booleanFlagsSize,
                           UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc, const size_t booleanFlagsWithFuncSize,
                           UCLI_Parser_Pair* pairs, const size_t pairsSize,
                           UCLI_Parser_PairWithFunc* pairsWithFunc, const size_t pairsWithFuncSize)
    {
        to_class(data)->parse(argc, argv,
                              arrayFlags, arrayFlagsSize,
                              booleanFlags, booleanFlagsSize,
                              booleanFlagsWithFunc, booleanFlagsWithFuncSize,
                              pairs, pairsSize,
                              pairsWithFunc, pairsWithFuncSize);
    }

    void UCLI_Parser_cleanupPairs(UCLI_Parser_Pair* pairs, const size_t pairsSize)
    {
        UCLI::Parser::cleanupPairs(pairs, pairsSize);
    }

    void UCLI_Parser_setBooleanFlipping(UCLI_Parser_Data* data, const bool bFlip)
    {
        to_class(data)->setBooleanFlipping(bFlip);
    }
#ifdef __cplusplus
}
#endif
