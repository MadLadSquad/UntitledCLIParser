#include "cucli.h"
#include "../CLIParser.hpp"

#define cast(x) static_cast<UCLI::Parser*>(x)

UCLI_Parser* UCLI_Parser_init()
{
    return new UCLI::Parser{};
}

void UCLI_Parser_free(UCLI_Parser* parser)
{
    delete cast(parser);
}

void UCLI_Parser_setHelpHeader(UCLI_Parser* parser, const char* header)
{
    cast(parser)->setHelpHeader(header);
}

void UCLI_Parser_setHelpFooter(UCLI_Parser* parser, const char* footer)
{
    cast(parser)->setHelpFooter(footer);
}

void UCLI_Parser_setUseGeneratedHelp(UCLI_Parser* parser, const bool bUseGeneratedHelp)
{
    cast(parser)->setUseGeneratedHelp(bUseGeneratedHelp);
}

void UCLI_Parser_setHelpSubcommandIndentationSpaces(UCLI_Parser* parser, const size_t indentSpaces)
{
    cast(parser)->setHelpSubcommandIndentationSpaces(indentSpaces);
}

void UCLI_Parser_setFlagPrefix(UCLI_Parser* parser, const char prefix)
{
    cast(parser)->setFlagPrefix(prefix);
}

void UCLI_Parser_setArrayDelimiter(UCLI_Parser* parser, const char delimiter)
{
    cast(parser)->setArrayDelimiter(delimiter);
}

void UCLI_Parser_useLenientMode(UCLI_Parser* parser, const bool bUseLenientMode)
{
    cast(parser)->useLenientMode(bUseLenientMode);
}

void UCLI_Parser_setBoolToggle(UCLI_Parser* parser, const bool bToggle)
{
    cast(parser)->setBoolToggle(bToggle);
}

void UCLI_Parser_pushCommand(UCLI_Parser* parser, UCLI_Command command)
{
    cast(parser)->pushCommand(command);
}

void UCLI_Parser_pushFlag(UCLI_Parser* parser, UCLI_Flag flag)
{
    cast(parser)->pushFlag(flag);
}

void UCLI_Parser_pushDefaultCommand(UCLI_Parser* parser, UCLI_Command command)
{
    cast(parser)->pushDefaultCommand(command);
}

void UCLI_Parser_pushDefaultFlag(UCLI_Parser* parser, UCLI_Flag flag)
{
    cast(parser)->pushDefaultFlag(flag);
}

void UCLI_Parser_parse(UCLI_Parser* parser, const int argc, char** argv)
{
    cast(parser)->parse(argc, argv);
}

void UCLI_Parser_release(UCLI_Parser* parser)
{
    cast(parser)->release();
}
