#include "cucli.h"
#include "../CLIParser.hpp"
#ifdef __cplusplus
extern "C"
{
#endif
#define to_class(data) ((UCLI::Parser*)data)

    void UCLI_Parser_init(struct UCLI_Parser_Data* data)
    {
        to_class(data)->init();
    }

    void UCLI_Parser_setDelimiter(struct UCLI_Parser_Data* data, char del)
    {
        to_class(data)->setDelimiter(del);
    }
    void UCLI_Parser_setArgumentStyleWindows(struct UCLI_Parser_Data* data, bool bWindows)
    {
        to_class(data)->setArgumentStyleWindows(bWindows);
    }

    void UCLI_Parser_setUnknownArgumentCallback(struct UCLI_Parser_Data* data, UCLI_Parser_UnknownArgumentsCallback func, void* callbackData)
    {
        to_class(data)->setUnknownArgumentCallback(func, callbackData);
    }

    void UCLI_Parser_setDefaultArray(struct UCLI_Parser_Data* data, void* additionalData, UCLI_Parser_ArrayFlagFunc func)
    {
        to_class(data)->setDefaultArray(additionalData, func);
    }

    void UCLI_Parser_parse(struct UCLI_Parser_Data* data, int argc, char** argv,
                           struct UCLI_Parser_ArrayFlag* arrayFlags, size_t arrayFlagsSize,
                           struct UCLI_Parser_BooleanFlag* booleanFlags, size_t booleanFlagsSize,
                           struct UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc, size_t booleanFlagsWithFuncSize,
                           struct UCLI_Parser_Pair* pairs, size_t pairsSize,
                           struct UCLI_Parser_PairWithFunc* pairsWithFunc, size_t pairsWithFuncSize)
    {
        to_class(data)->parse(argc, argv,
                              arrayFlags, arrayFlagsSize,
                              booleanFlags, booleanFlagsSize,
                              booleanFlagsWithFunc, booleanFlagsWithFuncSize,
                              pairs, pairsSize,
                              pairsWithFunc, pairsWithFuncSize);
    }

    void UCLI_Parser_cleanupPairs(struct UCLI_Parser_Pair* pairs, size_t pairsSize)
    {
        UCLI::Parser::cleanupPairs(pairs, pairsSize);
    }

    void UCLI_Parser_setBooleanFlipping(struct UCLI_Parser_Data* data, bool bFlip)
    {
        to_class(data)->setBooleanFlipping(bFlip);
    }
#ifdef __cplusplus
}
#endif