#pragma once
#include <string>
#include <algorithm>

// Lowercases the ASCII characters of the string. We deliberately don't use std::tolower here: it is locale-dependent,
// and passing it a char with a negative value - which any non-ASCII byte has on platforms with a signed char - is
// undefined behaviour
static void toLower(std::string& str) noexcept
{
    std::ranges::transform(str, str.begin(), [](const char c) -> char
    {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    });
}

// Parses a boolean literal. Returns whether the string is one at all, and writes the value it holds to bOut
static bool parseBooleanLiteral(std::string str, bool& bOut) noexcept
{
    toLower(str);

    if (str == "false" || str == "0" || str == "no" || str == "n")
    {
        bOut = false;
        return true;
    }
    bOut = true;
    return true;
}

/**
 * This is a parser for arguments of type UCLI_COMMAND_TYPE_BOOL. The syntax is as follows:
 * - `command` or short `c` - the boolean is toggled or set to true
 * - `command=value` - the boolean is set to the value
 * - `command=` - same as 1
 * - `command` or `command=value` where `boolValue` is nullptr - no additional parsing, we simply push the callback
 * - `-cC` batched short arguments - the same as cases 1 or 3
 * - `command value` - sets the command to the value, but only when the value is a boolean literal. Anything else is
 *   left alone, so that it can be parsed as a command or flag of its own
 * - `command --flag` - the boolean is toggled or set to true
 *
 * The accepted boolean literals are `true`/`1`/`yes`/`y` and `false`/`0`/`no`/`n`, in any casing
 */
template<typename T>
static void loadBooleanCommand(int& i, const int argc, char** argv, T& command, const int64_t assignmentIndex, const bool bToggle, const char flagPrefix, const bool bForcedDefault) noexcept
{
    // This is completely fine, since we have to support some way of accepting void without a distinct type
    if (command.boolValue == nullptr)
        return;

    bool value = false;

    if (assignmentIndex >= 0)
    {
        if (parseBooleanLiteral(argv[i] + assignmentIndex + 1, value))
        {
            *command.boolValue = value;
            return;
        }
    }
    else if (!bForcedDefault && argc > (i + 1) && argv[i + 1][0] != flagPrefix)
    {
        // Only consume the next argument when it really is a boolean value. Otherwise `command build` would eat the
        // `build` command instead of leaving it for the parser to handle on its own
        if (parseBooleanLiteral(argv[i + 1], value))
        {
            i++;
            *command.boolValue = value;
            return;
        }
    }
    *command.boolValue = bToggle ? !*command.boolValue : true;
}
