#pragma once
#include "FlagParser.hpp"
#include "ParserUtils.hpp"
#include <iostream>

/**
 * This is a parser for arguments of type UCLI_COMMAND_TYPE_ARRAY. The syntax is as follows:
 * - `command` - uses the default values or sets the array values to null. Only runs if there are no more arguments
 * - `command=value,value,value` - uses the assigned values
 * - `command=` - same as 1
 * - `command=a,,b`, `command=,a`, `command=a,` - empty values are skipped, so these give [a, b], [a] and [a]
 * - `command value value value --` - interprets all following arguments as array values until `--` is found
 * - `command value value --flag` - interprets all following arguments as array values until a flag is found
 * - `-c` or `-cC` - same as 1
 * - `-cC value` - same as 1, value is parsed as another command/flag
 * - `-c value value value` - same as 2
 * - `command --` - produces and empty string because -- is an explicit stop
 * - `command --boolean-flag --value-flag=1 --value-flag 2 value1 value2`(but not --command!) - A special case for
 *    commands in the shape of `git clone --recursive --depth 1 https://... folder` where we parse the flags until
 *    we find the values.
 */
template<typename T>
static bool loadArrayCommand(int& i, const int argc, char** argv, T& command, const int64_t assignmentIndex, const char arrayDelimiter, const char flagPrefix, const bool bForcedDefault, UCLI::Parser& p) noexcept
{
    std::vector<char*> stringValues{};

    // Is a variable assignment --flag=value
    if (assignmentIndex >= 0)
    {
        size_t startIndex = assignmentIndex + 1;

        // Loop until null terminator but in a more code-efficient way
        for (size_t j = startIndex; true; j++)
        {
            if (argv[i][j] != arrayDelimiter && argv[i][j] != '\0')
                continue;

            const size_t length = j - startIndex;

            // Empty segments are skipped instead of ending the list, so that `command=a,,b` gives [a, b] and
            // `command=,a` gives [a] rather than silently cutting the values short
            if (length > 0)
            {
                auto* str = static_cast<char*>(malloc((length + 1) * sizeof(char)));
                memcpy(str, argv[i] + startIndex, length);
                str[length] = '\0';

                stringValues.emplace_back(str);
            }

            if (argv[i][j] == '\0')
                break;

            startIndex = j + 1;
        }

        // Default values
        if (stringValues.empty())
        {
            useDefaultArguments(command);
            return true;
        }

        // The segments above are already null-terminated copies that we own, so we hand their ownership over to the
        // command instead of copying every one of them a second time
        command.stringValues.stringValues = static_cast<const char**>(malloc(stringValues.size() * sizeof(char*)));
        memcpy(UCLI_VOID_CAST(command.stringValues.stringValues), stringValues.data(), stringValues.size() * sizeof(char*));

        command.stringValues.stringValuesCount = stringValues.size();

        // Owned by us because of the string copies
        command.stringValues._internal_._bFreeStringValues = true;
        command.stringValues._internal_._bFreeInnerStringValues = true;

        return true;
    }

    // Nothing after the command
    if (argc == (i + 1))
    {
        useDefaultArguments(command);
        return true;
    }

    if (!bForcedDefault)
    {
        // --
        const char doubleFlagPrefix[] = { flagPrefix, flagPrefix, '\0' };

        // Whether probing left us on the command's first value, in which case the value sequence starts at the
        // current argument instead of the one after it
        bool bProbedValue = false;

        // Terminate directly if the next argument is a flag or probe for flags if the current argument is a command
        if (!command.useLiteralFlags && argv[i + 1][0] == flagPrefix)
        {
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
                i++;

                const auto result = UCLI::Internal::probeFlags(command, i, argc, argv, p);
                if (result == UCLI::Internal::ProbeResult::Failure)
                    return false;

                // We ran out of arguments while probing, so the command has no values of its own. probeFlags has
                // already applied its default values
                if (result == UCLI::Internal::ProbeResult::Exhausted)
                    return true;

                // Probing stops on the command's first value, so we must not skip over it below
                bProbedValue = true;
            }
            else
                goto use_defaults;
        }

        if (!bProbedValue)
            i++;

        // Non-variable assignment: --flag a b c d -- ...
        for (; i < argc; i++)
        {
            // This is a standard semantic for starting/terminating an array for the latest command
            if (strcmp(argv[i], doubleFlagPrefix) == 0)
            {
                // This is for cases where `--flag -- ...` since `--` is and explicit hard stop
                if (stringValues.empty())
                {
                    useNullArguments(command);
                    return true;
                }
                break;
            }

            // Stop parsing at another flag: --flag a b c d --another produces [a, b, c, d]
            if (!command.useLiteralFlags && argv[i][0] == flagPrefix)
            {
                // Bump back so that we can parse it separately outside here. Every caller - both the main parse loop
                // and the probing loop - increments i afterwards, so this always leaves us on the flag
                i--;
                break;
            }

            stringValues.push_back(argv[i]);
        }

        if (!stringValues.empty())
        {
            // We're not allocating and copying the inner strings, since they were already allocated by the toolchain's code
            command.stringValues.stringValues = static_cast<const char**>(malloc(stringValues.size() * sizeof(char*)));
            memcpy(UCLI_VOID_CAST(command.stringValues.stringValues), stringValues.data(), stringValues.size() * sizeof(char*));

            command.stringValues.stringValuesCount = stringValues.size();

            // Inner values owned by the user, outer container owned by us
            command.stringValues._internal_._bFreeStringValues = true;
            command.stringValues._internal_._bFreeInnerStringValues = false;

            return true;
        }
    }

    // Batched short arguments never parse a value of their own, so they end up here too
use_defaults:
    if (command.defaultValues != nullptr && command.defaultValuesCount > 0)
        useDefaultArguments(command);
    else
        useNullArguments(command);

    return true;
}