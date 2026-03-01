#include "CLIParser.hpp"
#include <span>
#include <iostream>

UCLI::Parser& UCLI::Parser::setArrayDelimiter(const char delimiter) noexcept
{
    arrayDelimiter = delimiter;
    return *this;
}

UCLI::Parser& UCLI::Parser::setBoolToggle(const bool bToggle) noexcept
{
    bToggleBooleans = bToggle;
    return *this;
}

UCLI::Parser& UCLI::Parser::setFlagPrefix(const char prefix) noexcept
{
    flagPrefix = prefix;
    return *this;
}

UCLI::Parser& UCLI::Parser::pushCommand(const Command& command) noexcept
{
    commands.push_back(command);
    return *this;
}

UCLI::Parser& UCLI::Parser::pushFlag(const Flag& flag) noexcept
{
    flags.push_back(flag);
    return *this;
}

UCLI::Parser& UCLI::Parser::pushDefaultCommand(const Command& command) noexcept
{
    commands.push_back(command);
    defaultCommand = &commands.back();
    return *this;
}

UCLI::Parser& UCLI::Parser::pushDefaultFlag(const Flag& flag) noexcept
{
    flags.push_back(flag);
    defaultFlag = &flags.back();
    return *this;
}

void UCLI::Parser::freeCommands(Command& command) noexcept
{
    if ((command.type == UCLI_COMMAND_TYPE_ARRAY || command.type == UCLI_COMMAND_TYPE_STRING) && !(command.stringValues.stringValues == nullptr || command.stringValues.stringValuesCount == 0))
    {
        if (command.stringValues._internal_._bFreeInnerStringValues)
            for (size_t i = 0; i < command.stringValues.stringValuesCount; i++)
                free(command.stringValues.stringValues[i]);
        if (command.stringValues._internal_._bFreeStringValues)
            free(command.stringValues.stringValues);
        memset(&command.stringValues, 0, sizeof(command.stringValues));

        command.stringValues._internal_._bFreeInnerStringValues = false;
        command.stringValues._internal_._bFreeStringValues = false;
    }

    if (command.subcommands != nullptr)
        for (size_t i = 0; i < command.subcommandsCount; i++)
            freeCommands(command.subcommands[i]);

    if (command.flags != nullptr)
        for (size_t i = 0; i < command.flagsCount; i++)
            freeFlags(command.flags[i]);
}

void UCLI::Parser::freeFlags(Flag& command) noexcept
{
    if ((command.type == UCLI_COMMAND_TYPE_ARRAY || command.type == UCLI_COMMAND_TYPE_STRING) && !(command.stringValues.stringValues == nullptr || command.stringValues.stringValuesCount == 0))
    {
        if (command.stringValues._internal_._bFreeInnerStringValues)
            for (size_t i = 0; i < command.stringValues.stringValuesCount; i++)
                free(command.stringValues.stringValues[i]);
        if (command.stringValues._internal_._bFreeStringValues)
            free(command.stringValues.stringValues);
        memset(&command.stringValues, 0, sizeof(command.stringValues));
    }
}

UCLI::Parser& UCLI::Parser::release() noexcept
{
    for (auto& command : commands)
        freeCommands(command);
    for (auto& flag : flags)
        freeFlags(flag);

    defaultCommand = nullptr;
    defaultFlag = nullptr;

    return *this;
}

UCLI::Parser::~Parser() noexcept
{
    release();
}

UCLI_CallbackResult UCLI_EMPTY_FLAG_CALLBACK(const UCLI_Flag* flag)
{
    // TODO: remove
    std::cout << flag->longName << std::endl;
    std::cout << flag->description << std::endl;
    return UCLI_CALLBACK_RESULT_OK;
}

UCLI_CallbackResult UCLI_EMPTY_COMMAND_CALLBACK(const UCLI_Command* flag)
{
    // TODO: remove
    std::cout << flag->longName << std::endl;
    std::cout << flag->description << std::endl;
    return UCLI_CALLBACK_RESULT_OK;
}