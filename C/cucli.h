#pragma once
#include "../Common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void UCLI_Parser;

    MLS_PUBLIC_API UCLI_Parser* UCLI_Parser_init();
    MLS_PUBLIC_API void UCLI_Parser_free(UCLI_Parser* parser);

    MLS_PUBLIC_API void UCLI_Parser_setHelpHeader(UCLI_Parser* parser, const char* header);
    MLS_PUBLIC_API void UCLI_Parser_setHelpFooter(UCLI_Parser* parser, const char* footer);
    // Set to true by default
    MLS_PUBLIC_API void UCLI_Parser_setUseGeneratedHelp(UCLI_Parser* parser, bool bUseGeneratedHelp);
    // Set to 2 by default
    MLS_PUBLIC_API void UCLI_Parser_setHelpSubcommandIndentationSpaces(UCLI_Parser* parser, size_t indentSpaces);

    // The default is `-`
    MLS_PUBLIC_API void UCLI_Parser_setFlagPrefix(UCLI_Parser* parser, char prefix);
    // The default is `,`
    MLS_PUBLIC_API void UCLI_Parser_setArrayDelimiter(UCLI_Parser* parser, char delimiter);

    // By default, we use strict mode where the default command/flag is called and directly exits. Lenient mode replaces
    // calls to invalid commands/flags with the default argument/flag command instead without exiting.
    MLS_PUBLIC_API void UCLI_Parser_useLenientMode(UCLI_Parser* parser, bool bUseLenientMode);

    // Whether to toggle boolean arguments or to set them to true. The default behaviour is to set them to true
    MLS_PUBLIC_API void UCLI_Parser_setBoolToggle(UCLI_Parser* parser, bool bToggle);

    MLS_PUBLIC_API void UCLI_Parser_pushCommand(UCLI_Parser* parser, UCLI_Command command);
    MLS_PUBLIC_API void UCLI_Parser_pushFlag(UCLI_Parser* parser, UCLI_Flag flag);

    MLS_PUBLIC_API void UCLI_Parser_pushDefaultCommand(UCLI_Parser* parser, UCLI_Command command);
    MLS_PUBLIC_API void UCLI_Parser_pushDefaultFlag(UCLI_Parser* parser, UCLI_Flag flag);

    MLS_PUBLIC_API void UCLI_Parser_parse(UCLI_Parser* parser, int argc, char** argv);

    MLS_PUBLIC_API void UCLI_Parser_release(UCLI_Parser* parser);
#ifdef __cplusplus
}
#endif
