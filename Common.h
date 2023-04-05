#pragma once
#include <stdio.h>
#include <stdbool.h>
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
    struct UCLI_Parser_ArrayFlag;

    typedef void(*UCLI_Parser_UnknownArgumentsCallback)(const char*, void*);
    typedef void(*UCLI_Parser_ArrayFlagFunc)(struct UCLI_Parser_ArrayFlag*, char**, size_t);

    struct UVK_PUBLIC_API UCLI_Parser_BooleanFlag
    {
        const char* longType;
        const char* shortType;
        bool* flag;
    };

    struct UVK_PUBLIC_API UCLI_Parser_ArrayFlag
    {
        const char* longType;
        const char* shortType;
        void* additionalData;
        UCLI_Parser_ArrayFlagFunc func;
    };

    struct UVK_PUBLIC_API UCLI_Parser_BooleanFlagWithFunc
    {
        const char* longType;
        const char* shortType;
        void* additionalData;
        void(*func)(struct UCLI_Parser_BooleanFlagWithFunc*);
    };

    struct UVK_PUBLIC_API UCLI_Parser_Pair
    {
        const char* longType;

        // This is internal and should always be set to false
        bool InternalbFound;
        // Do not touch this, we will copy the data into here
        const char* data;
    };

    struct UVK_PUBLIC_API UCLI_Parser_PairWithFunc
    {
        const char* longType;
        void* additionalData;
        void(*func)(struct UCLI_Parser_PairWithFunc*, const char*);
    };

    struct UVK_PUBLIC_API UCLI_Parser_Data
    {
        struct UCLI_Parser_ArrayFlag defaultArrayFlag;
        struct UCLI_Parser_ArrayFlag* currentArrayFlag;

        void* unknownArgumentsCallbackAdditionalData;
        UCLI_Parser_UnknownArgumentsCallback unknownArgumentsCallback;

        char delimiter;
        bool bWindowsStyle;
        bool bFlipBool;
    };
#ifdef __cplusplus
}
#endif