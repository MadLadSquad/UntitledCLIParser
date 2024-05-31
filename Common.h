#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef UVK_LOG_EXPORT_FROM_LIBRARY
    #ifdef _WIN32
        #ifdef UVK_LIB_COMPILE
            #define UVK_PUBLIC_API __declspec(dllexport)
            #define UVK_PUBLIC_TMPL_API __declspec(dllexport)
        #else
            #define UVK_PUBLIC_API __declspec(dllimport)
            #define UVK_PUBLIC_TMPL_API
        #endif
    #else
        #define UVK_PUBLIC_API
        #define UVK_PUBLIC_TMPL_API
    #endif
#else
    #define UVK_PUBLIC_API
    #define UVK_PUBLIC_TMPL_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct UCLI_Parser_ArrayFlag UCLI_Parser_ArrayFlag;

    typedef void(*UCLI_Parser_UnknownArgumentsCallback)(const char*, void*);
    typedef void(*UCLI_Parser_ArrayFlagFunc)(UCLI_Parser_ArrayFlag*, char**, size_t);

    typedef struct UVK_PUBLIC_API UCLI_Parser_BooleanFlag
    {
        const char* longType;
        const char* shortType;
        bool* flag;
    } UCLI_Parser_BooleanFlag;

    typedef struct UVK_PUBLIC_API UCLI_Parser_ArrayFlag
    {
        const char* longType;
        const char* shortType;
        void* additionalData;
        UCLI_Parser_ArrayFlagFunc func;
    } UCLI_Parser_ArrayFlag;

    typedef struct UVK_PUBLIC_API UCLI_Parser_BooleanFlagWithFunc
    {
        const char* longType;
        const char* shortType;
        void* additionalData;
        void(*func)(struct UCLI_Parser_BooleanFlagWithFunc*);
    } UCLI_Parser_BooleanFlagWithFunc;

    typedef struct UVK_PUBLIC_API UCLI_Parser_Pair
    {
        const char* longType;

        // This is internal and should always be set to false
        bool InternalbFound;
        // Do not touch this, we will copy the data into here
        const char* data;
    } UCLI_Parser_Pair;

    typedef struct UVK_PUBLIC_API UCLI_Parser_PairWithFunc
    {
        const char* longType;
        void* additionalData;
        void(*func)(struct UCLI_Parser_PairWithFunc*, const char*);
    } UCLI_Parser_PairWithFunc;

    typedef struct UVK_PUBLIC_API UCLI_Parser_Data
    {
        UCLI_Parser_ArrayFlag defaultArrayFlag;
        UCLI_Parser_ArrayFlag* currentArrayFlag;

        void* unknownArgumentsCallbackAdditionalData;
        UCLI_Parser_UnknownArgumentsCallback unknownArgumentsCallback;

        char delimiter;
        bool bWindowsStyle;
        bool bFlipBool;
    } UCLI_Parser_Data;
#ifdef __cplusplus
}
#endif
