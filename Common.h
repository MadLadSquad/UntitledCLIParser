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

    typedef enum UCLI_CallbackResult
    {
        UCLI_CALLBACK_RESULT_OK = 0,
        UCLI_CALLBACK_RESULT_PREMATURE_EXIT = 1,
    } UCLI_CallbackResult;

    typedef struct UCLI_Command UCLI_Command;
    typedef struct UCLI_Flag UCLI_Flag;

    typedef UCLI_CallbackResult(*UCLI_CommandEvent)(const UCLI_Command*);
    typedef UCLI_CallbackResult(*UCLI_FlagEvent)(const UCLI_Flag*);

    /**
     * @var longName - the long name of the flag. Example: "my-flag" -> `--my-flag`
     * @var shortName - the short name of the flag. Example: "f" -> `-f`
     * @var description - the help description of the flag. Only used if using the auto-generated help command
     *
     * @var defaultValues - A list of default string values for the flag
     * @var defaultValuesCount - The size of the list of default values for the flag
     *
     * @var type - The data type of the flag(void, bool, string or array)
     *
     * @var boolValue - Used if the type of the flag is bool
     * @var stringValues - An array of strings. Used if the type is string or array.
     * @var stringValuesCount - The size of the strings array
     *
     * @var callback - A callback that will be called if the flag is encountered
     * @var context - A user-defined context pointer
     *
     * @var priority - If the flag's priority is below (SIZE_MAX / 2), then the flag is pushed before any other flags
     * with a lower priority for the flag's command depth. If the priority is higher, the flag is moved to the front of
     * the command queue. SIZE_MAX is the highest priority. These commands are guaranteed to be called first, with the
     * default help command being set to this exact priority if used.
     */
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

        size_t priority;

        char* _internal_ctx_;
    } UCLI_Flag;

    /**
      * @var longName - the long name of the command. Example: "my-command" -> `my-command`
      * @var shortName - the short name of the command. Example: "f" -> `f`
      * @var description - the help description of the command. Only used if using the auto-generated help command
      *
      * @var defaultValues - A list of default string values for the command
      * @var defaultValuesCount - The size of the list of default values for the command
      *
      * @var type - The data type of the command(void, bool, string or array)
      *
      * @var boolValue - Used if the type of the command is bool
      * @var stringValues - An array of strings. Used if the type is string or array.
      * @var stringValuesCount - The size of the strings array
      *
      * @var callback - A callback that will be called if the command is encountered
      * @var context - A user-defined context pointer
      */
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

        UCLI_Command* _internal_parent;
        char* _internal_ctx_;
    } UCLI_Command;

    MLS_PUBLIC_API UCLI_CallbackResult UCLI_EMPTY_FLAG_CALLBACK(const UCLI_Flag* flag);
    MLS_PUBLIC_API UCLI_CallbackResult UCLI_EMPTY_COMMAND_CALLBACK(const UCLI_Command* flag);

    MLS_PUBLIC_API UCLI_CallbackResult UCLI_Parser_helpCommandC(const UCLI_Command* command);
    MLS_PUBLIC_API UCLI_CallbackResult UCLI_Parser_helpCommandF(const UCLI_Flag* flag);
#ifdef __cplusplus
}
#endif
