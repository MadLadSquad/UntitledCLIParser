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

    #ifdef __cplusplus
        #define CLITERAL(x, ...)                \
            []() {                              \
                static x b[] = { __VA_ARGS__ }; \
                return b;                       \
            }()
    #else
        #define CLITERAL(x, ...) (x){ __VA_ARGS__ }
    #endif


    // Initialise a pointer with an array inline
    #define UCLI_MAKE_FLAG_ARRAY(...) CLITERAL(UCLI_Flag, __VA_ARGS__)
    // Initialise a pointer with an array inline
    #define UCLI_MAKE_COMMAND_ARRAY(...) CLITERAL(UCLI_Command, __VA_ARGS__)
    // Initialise a pointer with an array inline
    #define UCLI_MAKE_STRING_ARRAY(...) CLITERAL(char*, __VA_ARGS__)


    typedef enum UCLI_CommandType
    {
        UCLI_COMMAND_TYPE_VOID = 0,
        UCLI_COMMAND_TYPE_BOOL = 0,
        UCLI_COMMAND_TYPE_STRING = 1,
        UCLI_COMMAND_TYPE_ARRAY = 2
    } UCLI_CommandType;

    typedef struct UCLI_Command UCLI_Command;
    typedef struct UCLI_Flag UCLI_Flag;

    typedef void(*UCLI_CommandEvent)(const UCLI_Command*);
    typedef void(*UCLI_FlagEvent)(const UCLI_Flag*);

    typedef struct MLS_PUBLIC_API UCLI_Flag
    {
        const char* longName;
        char shortName;
        const char* description;

        char** defaultValues;
        size_t defaultValuesCount;

        union
        {
            struct
            {
                char** stringValues;
                size_t stringValuesCount;

                struct
                {
                    bool _bFreeStringValues;
                    bool _bFreeInnerStringValues;
                } _internal_;
            } stringValues;
            bool* boolValue;
        };
        UCLI_CommandType type;

        UCLI_FlagEvent callback;
        void* context;
    } UCLI_Flag;

    typedef struct MLS_PUBLIC_API UCLI_Command
    {
        const char* longName;
        char shortName;
        const char* description;

        char** defaultValues;
        size_t defaultValuesCount;

        union
        {
            struct
            {
                char** stringValues;
                size_t stringValuesCount;
                struct
                {
                    bool _bFreeStringValues;
                    bool _bFreeInnerStringValues;
                } _internal_;
            } stringValues;
            bool* boolValue;
        };
        UCLI_CommandType type;

        UCLI_Command* subcommands;
        size_t subcommandsCount;

        UCLI_Flag* flags;
        size_t flagsCount;

        UCLI_CommandEvent callback;
        void* context;
    } UCLI_Command;

#ifdef __cplusplus
}
#endif
