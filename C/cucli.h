#pragma once
#include "../Common.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
    // UntitledImGuiFramework Event Safety - Any time
    void UCLI_Parser_init(struct UCLI_Parser_Data* data);


    // UntitledImGuiFramework Event Safety - Any time
    void UCLI_Parser_setDelimiter(struct UCLI_Parser_Data* data, char del);

    // UntitledImGuiFramework Event Safety - Any time
    void UCLI_Parser_setArgumentStyleWindows(struct UCLI_Parser_Data* data, bool bWindows);


    // UntitledImGuiFramework Event Safety - Any time
    void UCLI_Parser_setUnknownArgumentCallback(struct UCLI_Parser_Data* data, UCLI_Parser_UnknownArgumentsCallback func, void* callbackData);

    // UntitledImGuiFramework Event Safety - Any time
    void UCLI_Parser_setDefaultArray(struct UCLI_Parser_Data* data, void* additionalData, UCLI_Parser_ArrayFlagFunc func);


    // UntitledImGuiFramework Event Safety - Any time
    void UCLI_Parser_setBooleanFlipping(struct UCLI_Parser_Data* data, bool bFlip);


    // UntitledImGuiFramework Event Safety - Any time
    void UCLI_Parser_parse(struct UCLI_Parser_Data* data, int argc, char** argv,
                           struct UCLI_Parser_ArrayFlag* arrayFlags, size_t arrayFlagsSize,
                           struct UCLI_Parser_BooleanFlag* booleanFlags, size_t booleanFlagsSize,
                           struct UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc, size_t booleanFlagsWithFuncSize,
                           struct UCLI_Parser_Pair* pairs, size_t pairsSize,
                           struct UCLI_Parser_PairWithFunc* pairsWithFunc, size_t pairsWithFuncSize);


    // UntitledImGuiFramework Event Safety - Any time
    void UCLI_Parser_cleanupPairs(struct UCLI_Parser_Pair* pairs, size_t pairsSize);

#ifdef __cplusplus
}
#endif