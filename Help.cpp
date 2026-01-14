#include "CLIParser.hpp"
#include <iostream>
#include <string.h>

static void printIndentation(const std::string& indentString, const size_t count) noexcept
{
    for (size_t i = 0; i < count; i++)
        std::cout << indentString;
}

static void printDefaultValues(char** values, const size_t count, const UCLI::CommandType type) noexcept
{
    if (values != nullptr && count > 0)
    {
        if (type == UCLI_COMMAND_TYPE_ARRAY)
        {
            std::cout << "=[";
            for (size_t i = 0; i < count; i++)
            {
                std::cout << values[i];
                if ((i + 1) != count)
                    std::cout << ",";
            }
            std::cout << "]";
        }
        else if (type == UCLI_COMMAND_TYPE_STRING)
            std::cout << "=\"" << values[0] << "\"";
        else
            std::cout << "=" << values[0];
    }
}

void UCLI::Parser::printFlags(UCLI::Flag* flags, const size_t size, const std::string& indentationString, const size_t indentation) noexcept
{
    printIndentation(indentationString, indentation);
    std::cout << "Flags:" << std::endl;

    for (size_t i = 0; i < size; i++)
    {
        auto& flag = flags[i];

        const bool bHasLongName = flag.longName != nullptr && strlen(flag.longName) > 0;
        const bool bHasShortName = flag.shortName != '\0';

        printIndentation(indentationString, indentation + 1);

        if (bHasLongName)
            std::cout << "--" << flag.longName;
        if (bHasShortName && bHasLongName)
            std::cout << "/";
        if (bHasShortName)
            std::cout << "-" << flag.shortName;

        printDefaultValues(flag.defaultValues, flag.defaultValuesCount, flag.type);

        if (flag.description != nullptr && strlen(flag.description) > 0)
            std::cout << " - " << flag.description;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void UCLI::Parser::printCommands(const UCLI::Command* commands, const size_t size, const std::string& indentationString, const size_t indentation) noexcept
{
    printIndentation(indentationString, indentation);
    std::cout << "Commands:" << std::endl;

    for (size_t i = 0; i < size; i++)
    {
        const auto& command = commands[i];

        printIndentation(indentationString, indentation + 1);

        const bool bHasLongName = command.longName != nullptr && strlen(command.longName) > 0;
        const bool bHasShortName = command.shortName != '\0';

        if (!bHasLongName && !bHasShortName)
            continue;
        if (bHasLongName)
            std::cout << command.longName;
        if (bHasLongName && bHasShortName)
            std::cout << "/";
        if (bHasShortName)
            std::cout << command.shortName;

        printDefaultValues(command.defaultValues, command.defaultValuesCount, command.type);

        if (command.description != nullptr && strlen(command.description) > 0)
            std::cout << " - " << command.description;
        std::cout << std::endl;

        if (command.subcommands != nullptr && command.subcommandsCount > 0)
            printCommands(command.subcommands, command.subcommandsCount, indentationString, indentation + 1);
        if (command.flags != nullptr && command.flagsCount > 0)
            printFlags(command.flags, command.flagsCount, indentationString, indentation + 1);
    }
    std::cout << std::endl;
}

void UCLI::Parser::helpCommand(const Flag* command) noexcept
{
    auto& p = *static_cast<Parser*>(command->context);
    if (!p.helpHeader.empty())
        std::cout << p.helpHeader << std::endl;

    printCommands(p.commands.data(), p.commands.size(), p.indentationString, 0);
    printFlags(p.flags.data(), p.flags.size(), p.indentationString, 0);

    if (!p.helpFooter.empty())
        std::cout << p.helpFooter << std::endl;
}

void UCLI::Parser::helpCommand(const Command* command) noexcept
{
    auto& p = *static_cast<Parser*>(command->context);
    if (!p.helpHeader.empty())
        std::cout << p.helpHeader << std::endl;

    printCommands(p.commands.data(), p.commands.size(), p.indentationString, 0);
    printFlags(p.flags.data(), p.flags.size(), p.indentationString, 0);

    if (!p.helpFooter.empty())
        std::cout << p.helpFooter << std::endl;
}

void UCLI::Parser::pushHelp() noexcept
{
    std::string header = helpHeader;

    pushCommand(Command{
        .longName = "help",
        .shortName = 'h',
        .description = "Prints a help message",

        .boolValue = &bShowHelp,
        .type = UCLI_COMMAND_TYPE_BOOL,

        .callback = helpCommand,
        .context = this,
    });

    pushFlag(Flag{
        .longName = "help",
        .shortName = 'h',
        .description = "Prints a help message",

        .boolValue = &bShowHelp,
        .type = UCLI_COMMAND_TYPE_BOOL,

        .callback = helpCommand,
        .context = this,
    });
    if (defaultCommand == nullptr)
        defaultCommand = &commands.back();
    if (defaultFlag == nullptr)
        defaultFlag = &flags.back();
}

UCLI::Parser& UCLI::Parser::setHelpHeader(const char* header) noexcept
{
    helpHeader = header;
    return *this;
}

UCLI::Parser& UCLI::Parser::setHelpFooter(const char* footer) noexcept
{
    helpFooter = footer;
    return *this;
}

UCLI::Parser& UCLI::Parser::setUseGeneratedHelp(const bool bUseGeneratedHelp) noexcept
{
    bUseHelp = bUseGeneratedHelp;
    return *this;
}

UCLI::Parser& UCLI::Parser::setHelpSubcommandIndentationSpaces(const size_t indentSpaces) noexcept
{
    indentationString.clear();
    for (size_t i = 0; i < indentSpaces; i++)
        indentationString += " ";
    return *this;
}
