#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef MLS_EXPORT_LIBRARY
    #ifdef _WIN32
        #ifdef MLS_LIB_COMPILE
            #define MLS_PUBLIC_API __declspec(dllexport)
        #else
            #define MLS_PUBLIC_API __declspec(dllimport)
        #endif
    #else
        #define MLS_PUBLIC_API
    #endif
#else
    #define MLS_PUBLIC_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct UCLI_Parser_ArrayFlag UCLI_Parser_ArrayFlag;

    typedef void(*UCLI_Parser_UnknownArgumentsCallback)(const char*, void*);
    typedef void(*UCLI_Parser_ArrayFlagFunc)(UCLI_Parser_ArrayFlag*, char**, size_t);

    typedef struct MLS_PUBLIC_API UCLI_Parser_BooleanFlag
    {
        const char* longType;
        const char* shortType;
        bool* flag;
    } UCLI_Parser_BooleanFlag;

    typedef struct MLS_PUBLIC_API UCLI_Parser_ArrayFlag
    {
        const char* longType;
        const char* shortType;
        void* additionalData;
        UCLI_Parser_ArrayFlagFunc func;
    } UCLI_Parser_ArrayFlag;

    typedef struct MLS_PUBLIC_API UCLI_Parser_BooleanFlagWithFunc
    {
        const char* longType;
        const char* shortType;
        void* additionalData;
        void(*func)(struct UCLI_Parser_BooleanFlagWithFunc*);
    } UCLI_Parser_BooleanFlagWithFunc;

    typedef struct MLS_PUBLIC_API UCLI_Parser_Pair
    {
        const char* longType;

        // This is internal and should always be set to false
        bool InternalbFound;
        // Do not touch this, we will copy the data into here
        const char* data;
    } UCLI_Parser_Pair;

    typedef struct MLS_PUBLIC_API UCLI_Parser_PairWithFunc
    {
        const char* longType;
        void* additionalData;
        void(*func)(struct UCLI_Parser_PairWithFunc*, const char*);
    } UCLI_Parser_PairWithFunc;

    typedef struct MLS_PUBLIC_API UCLI_Parser_Data
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
