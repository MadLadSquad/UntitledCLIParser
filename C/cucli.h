#pragma once
#include "../Common.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // UntitledImGuiFramework Event Safety - Any time
    UVK_PUBLIC_API void UCLI_Parser_init(UCLI_Parser_Data* data);


    // UntitledImGuiFramework Event Safety - Any time
    UVK_PUBLIC_API void UCLI_Parser_setDelimiter(UCLI_Parser_Data* data, char del);

    // UntitledImGuiFramework Event Safety - Any time
    UVK_PUBLIC_API void UCLI_Parser_setArgumentStyleWindows(UCLI_Parser_Data* data, bool bWindows);


    // UntitledImGuiFramework Event Safety - Any time
    UVK_PUBLIC_API void UCLI_Parser_setUnknownArgumentCallback(UCLI_Parser_Data* data, UCLI_Parser_UnknownArgumentsCallback func, void* callbackData);

    // UntitledImGuiFramework Event Safety - Any time
    UVK_PUBLIC_API void UCLI_Parser_setDefaultArray(UCLI_Parser_Data* data, void* additionalData, UCLI_Parser_ArrayFlagFunc func);


    // UntitledImGuiFramework Event Safety - Any time
    UVK_PUBLIC_API void UCLI_Parser_setBooleanFlipping(UCLI_Parser_Data* data, bool bFlip);


    // UntitledImGuiFramework Event Safety - Any time
    UVK_PUBLIC_API void UCLI_Parser_parse(UCLI_Parser_Data* data, int argc, char** argv,
                           UCLI_Parser_ArrayFlag* arrayFlags, size_t arrayFlagsSize,
                           UCLI_Parser_BooleanFlag* booleanFlags, size_t booleanFlagsSize,
                           UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc, size_t booleanFlagsWithFuncSize,
                           UCLI_Parser_Pair* pairs, size_t pairsSize,
                           UCLI_Parser_PairWithFunc* pairsWithFunc, size_t pairsWithFuncSize);


    // UntitledImGuiFramework Event Safety - Any time
    UVK_PUBLIC_API void UCLI_Parser_cleanupPairs(UCLI_Parser_Pair* pairs, size_t pairsSize);

#ifdef __cplusplus
}
#endif
