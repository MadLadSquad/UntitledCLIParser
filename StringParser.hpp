#pragma once
#include "FlagParser.hpp"
#include "ParserUtils.hpp"
#include <iostream>

template<typename T>
static void setStringValue(T& command, const int i, char** argv) noexcept
{
    command.stringValues.stringValues = const_cast<const char**>(argv + i);
    command.stringValues.stringValuesCount = 1;

    // Owned by the user
    command.stringValues._internal_._bFreeStringValues = false;
    command.stringValues._internal_._bFreeInnerStringValues = false;
}

/**
 * This is a parser for arguments of type UCLI_COMMAND_TYPE_STRING. The syntax is as follows:
 * - `command`, `-c`, `-cC`, `c` - uses the default arguments if set or sets the string value to null
 * - `-c value`, `c value` - uses the next argument as the value
 * - `-cC value` - same as 1, value is parsed as another command/flag
 * - `command=value` - uses the assigned value
 * - `command=` - same as 1
 * - `command value` - will set the value to value
 * - `command --value` - will use the default arguments if set
 * - `command --boolean-flag --value-flag=1 --value-flag2 2 test`(but not --command!) - A special case for commands
 *    in the shape of `git clone --recursive --depth 1 https://...` where we parse the flags until we find the value.
 */
template<typename T>
static bool loadStringCommand(int& i, const int argc, char** argv, T& command, const int64_t assignmentIndex, const bool bForcedDefault, const char flagPrefix, UCLI::Parser& p) noexcept
{
    if (assignmentIndex >= 0)
    {
        const std::string_view str = &argv[i][assignmentIndex + 1];

        if (str.empty())
        {
            useNullArguments(command);
            return true;
        }

        command.stringValues.stringValues = static_cast<const char**>(malloc(sizeof(char**)));
        command.stringValues.stringValues[0] = static_cast<const char*>(malloc(str.size() * sizeof(char)));

        memcpy(UCLI_VOID_CAST(command.stringValues.stringValues[0]), str.data(), str.size() * sizeof(char));

        command.stringValues.stringValuesCount = 1;

        // Owned by us
        command.stringValues._internal_._bFreeStringValues = true;
        command.stringValues._internal_._bFreeInnerStringValues = true;

        return true;
    }

    // String value from the next argument
    if (!bForcedDefault && argc > (i + 1))
    {
        if (argv[i + 1][0] != flagPrefix || (argv[i + 1][0] == flagPrefix && command.useLiteralFlags))
        {
            i++; // Skip to the next argument
            setStringValue(command, i, argv);
            return true;
        }

        // Special case: we have encountered a situation like this: `git clone --recursive https://...`
        // where we're at `clone`.
        //
        // To fix this issue, we start parsing flags forward until we find a non-flag value. If the value is
        // associated with a flag that will consume it, it is set for the flag, otherwise it's set as the
        // command's value
        //
        // For example: `git clone --recursive --depth=1 --depth 2 https://...` will be interpreted as if
        // recursive = true, depth = 1, depth = 2, clone="https://..."
        if constexpr (std::is_same_v<T, UCLI::Command>)
        {
            if (argv[i + 1][0] == flagPrefix)
            {
                i++; // Skip to the next argument
                if (UCLI::Internal::probeFlags(command, i, argc, argv, p))
                {
                    setStringValue(command, i, argv);
                    return true;
                }
                return false;
            }
        }
    }

    if (command.defaultValues != nullptr && command.defaultValuesCount > 0)
        useDefaultArguments(command);
    else
        useNullArguments(command);

    return true;
}