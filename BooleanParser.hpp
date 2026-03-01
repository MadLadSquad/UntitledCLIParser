#pragma once
#include <string_view>

/**
 * This is a parser for arguments of type UCLI_COMMAND_TYPE_BOOL. The syntax is as follows:
 * - `command` or short `c` - the boolean is toggled or set to true
 * - `command=value` - the boolean is set to the value
 * - `command=` - same as 1
 * - `command` or `command=value` where `boolValue` is nullptr - no additional parsing, we simply push the callback
 * - `-cC` batched short arguments - the same as cases 1 or 3
 * - `command value` - sets the command to the value
 * - `command --flag` - the boolean is toggled or set to true
 */
template<typename T>
static void loadBooleanCommand(int& i, const int argc, char** argv, T& command, const int64_t assignmentIndex, const bool bToggle, const char flagPrefix, const bool bForcedDefault) noexcept
{
    // This is completely fine, since we have to support some way of accepting void without a distinct type
    if (command.boolValue == nullptr)
        return;

    if (assignmentIndex >= 0)
    {
        const std::string_view str = argv[i] + assignmentIndex + 1;
        if (str == "false" || str == "0")
        {
            *command.boolValue = false;
            return;
        }
    }
    else if (!bForcedDefault && argc > (i + 1) && argv[i + 1][0] != flagPrefix)
    {
        const std::string_view str = argv[i + 1];
        i++;

        if (str == "false" || str == "0")
        {
            *command.boolValue = false;
            return;
        }
    }
    *command.boolValue = bToggle ? !*command.boolValue : true;
}