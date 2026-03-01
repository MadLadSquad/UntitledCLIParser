#include <CLIParser.hpp>
#include <span>

/**
 * This is where most of the parsing happens. The `parse` function only cleans up the name and finds the correct command
 * or flag. Here we actually deal with everything else. The syntax for arguments is like this:
 *
 * - UCLI_COMMAND_TYPE_BOOL:
 *   - `command` or short `c` - the boolean is toggled or set to true
 *   - `command=value` - the boolean is set to the value
 *   - `command=` - same as 1
 *   - `command` or `command=value` where `boolValue` is nullptr - no additional parsing, we simply push the callback
 *   - `-cC` batched short arguments - the same as cases 1 or 3
 *   - `command value` - sets the command to the value
 *   - `command --flag` - the boolean is toggled or set to true
 *
 * - UCLI_COMMAND_TYPE_STRING:
 *   - `command`, `-c`, `-cC`, `c` - uses the default arguments if set or sets the string value to null
 *   - `-c value`, `c value` - uses the next argument as the value
 *   - `-cC value` - same as 1, value is parsed as another command/flag
 *   - `command=value` - uses the assigned value
 *   - `command=` - same as 1
 *   - `command value` - will set the value to value
 *   - `command --value` - will use the default arguments if set
 *   - `command --boolean-flag --value-flag=1 --value-flag2 2 test`(but not --command!) - A special case for commands
 *      in the shape of `git clone --recursive --depth 1 https://...` where we parse the flags until we find the find
 *      the value.
 *
 * - UCLI_COMMAND_TYPE_ARRAY:
 *   - `command` - uses the default values or sets the array values to null. Only runs if there are no more arguments
 *   - `command=value,value,value` - uses the assigned values
 *   - `command=` - same as 1
 *   - `command value value value --` - interprets all following arguments as array values until `--` is found
 *   - `command value value --flag` - interprets all following arguments as array values until a flag is found
 *   - `-c` or `-cC` - same as 1
 *   - `-cC value` - same as 1, value is parsed as another command/flag
 *   - `-c value value value` - same as 2
 *   - `command --` - produces and empty string because -- is an explicit stop
 *   - `command --boolean-flag --value-flag=1 --value-flag 2 value1 value2`(but not --command!) - A special case for
 *      commands in the shape of `git clone --recursive --depth 1 https://... folder` where we parse the flags until
 *      we find the find the values.
 */
template<typename T>
static void executeCommand(int& i, const int argc, char** argv, T& command, const int assignmentIndex, const char arrayDelimiter, const bool bToggle, const char flagPrefix, const bool bForcedDefault, const int64_t commandDepth, std::deque<UCLI::Parser::CallbackObject>& callbacks) noexcept
{
    if (command.type == UCLI_COMMAND_TYPE_BOOL)
    {
        // This is completely fine, since we have to support some way of accepting void without a distinct type
        if (command.boolValue == nullptr)
            goto call_callback;

        if (assignmentIndex >= 0)
        {
            const std::string_view str = argv[i] + assignmentIndex + 1;
            if (str == "false" || str == "0")
            {
                *command.boolValue = false;
                goto call_callback;
            }
        }
        else if (!bForcedDefault && argc > (i + 1) && argv[i + 1][0] != flagPrefix)
        {
            const std::string_view str = argv[i + 1];
            i++;

            if (str == "false" || str == "0")
            {
                *command.boolValue = false;
                goto call_callback;
            }
        }
        *command.boolValue = bToggle ? !*command.boolValue : true;
    }
    else if (command.type == UCLI_COMMAND_TYPE_STRING)
    {
        if (assignmentIndex >= 0)
        {
            const std::string_view str = &argv[i][assignmentIndex + 1];

            if (str.empty())
            {
                command.stringValues.stringValues = nullptr;
                command.stringValues.stringValuesCount = 0;

                command.stringValues._internal_._bFreeStringValues = false;
                command.stringValues._internal_._bFreeInnerStringValues = false;

                goto call_callback;
            }

            command.stringValues.stringValues = static_cast<char**>(malloc(sizeof(char**)));
            command.stringValues.stringValues[0] = static_cast<char*>(malloc(str.size() * sizeof(char)));

            memcpy(command.stringValues.stringValues[0], str.data(), str.size() * sizeof(char));

            command.stringValues.stringValuesCount = 1;

            // Owned by us
            command.stringValues._internal_._bFreeStringValues = true;
            command.stringValues._internal_._bFreeInnerStringValues = true;
        }
        else
        {
            // Owned by the user
            if (!bForcedDefault && argc > (i + 1) && argv[i + 1][0] != flagPrefix)
            {
                i++; // Skip to the next argument

                command.stringValues.stringValues = argv + i;
                command.stringValues.stringValuesCount = 1;
            }
            else if (!bForcedDefault && argc > (i + 1) && argv[i + 1][0] == flagPrefix && std::is_same_v<T, UCLI::Command>)
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
                // TODO:
            }
            else
            {
                if (command.defaultValues != nullptr && command.defaultValuesCount > 0)
                {
                    command.stringValues.stringValues = command.defaultValues;
                    command.stringValues.stringValuesCount = command.defaultValuesCount;
                }
                else
                {
                    command.stringValues.stringValues = nullptr;
                    command.stringValues.stringValuesCount = 0;
                }
            }

            command.stringValues._internal_._bFreeStringValues = false;
            command.stringValues._internal_._bFreeInnerStringValues = false;
        }
    }
    else if (command.type == UCLI_COMMAND_TYPE_ARRAY)
    {
        std::vector<char*> stringValues{};

        // Is a variable assignment --flag=value
        if (assignmentIndex >= 0)
        {
            size_t startIndex = assignmentIndex + 1;
            // Loop until null terminator but in a more code-efficient way
            for (size_t j = startIndex; true; j++)
            {
                if (argv[i][j] == arrayDelimiter || argv[i][j] == '\0')
                {
                    const size_t size = (j - startIndex) * sizeof(char);
                    if (size <= 1)
                        break;

                    auto* str = static_cast<char*>(malloc(size));
                    memcpy(str, argv[i] + startIndex, size - 1);
                    str[size - 1] = '\0';

                    stringValues.emplace_back(str);

                    startIndex = j + 1;

                    // Break on the end of the string or if the string ends in ,
                    if (argv[i][j] == '\0' || (argv[i][j + 1] == arrayDelimiter && argv[i][j + 2] == '\0'))
                        break;
                }
                else if (argv[i][j] == '\0')
                    break;
            }

            // Default values
            if (stringValues.empty())
            {
                command.stringValues.stringValues = command.defaultValues;
                command.stringValues.stringValuesCount = command.defaultValuesCount;

                // Owned by the user
                command.stringValues._internal_._bFreeStringValues = false;
                command.stringValues._internal_._bFreeInnerStringValues = false;
                goto call_callback;
            }
            command.stringValues.stringValues = static_cast<char**>(malloc(stringValues.size() * sizeof(char*)));
            for (size_t f = 0; f < stringValues.size(); f++)
            {
                const size_t size = strlen(stringValues[i]) * sizeof(char);
                command.stringValues.stringValues[f] = static_cast<char*>(malloc(size));
                memcpy(command.stringValues.stringValues[f], stringValues[f], size);
            }

            command.stringValues.stringValuesCount = stringValues.size();

            // Owned by us because of the string copies
            command.stringValues._internal_._bFreeStringValues = true;
            command.stringValues._internal_._bFreeInnerStringValues = true;

            goto call_callback;
        }

        if (!bForcedDefault)
        {
            // --
            const char doubleFlagPrefix[] = { flagPrefix, flagPrefix, '\0' };

            // Non-variable assignment: --flag a b c d -- ...
            for (++i; i < argc; i++)
            {
                // This is a standard semantic for starting/terminating an array for the latest command
                if (strcmp(argv[i], doubleFlagPrefix) == 0)
                {
                    // This is for cases where `--flag -- ...` since `--` is and explicit hard stop
                    if (stringValues.empty())
                    {
                        command.stringValues.stringValues = nullptr;
                        command.stringValues.stringValuesCount = 0;

                        // Owned by the user
                        command.stringValues._internal_._bFreeStringValues = false;
                        command.stringValues._internal_._bFreeInnerStringValues = false;
                        goto call_callback;
                    }
                    break;
                }

                // Stop parsing at another flag: --flag a b c d --another produces [a, b, c, d]
                if (argv[i][0] == flagPrefix)
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
                    if (std::is_same_v<T, UCLI::Command> && stringValues.empty())
                    {
                        // TODO:
                    }
                    else
                        i--; // Bump back so that we can parse it separately outside here
                    break;
                }

                stringValues.push_back(argv[i]);
            }

            if (!stringValues.empty())
            {
                // We're not allocating and copying the inner strings, since they were already allocated by the toolchain's code
                command.stringValues.stringValues = static_cast<char**>(malloc(stringValues.size() * sizeof(char*)));
                memcpy(command.stringValues.stringValues, stringValues.data(), stringValues.size() * sizeof(char*));

                command.stringValues.stringValuesCount = stringValues.size();

                // Inner values owned by the user, outer container owned by us
                command.stringValues._internal_._bFreeStringValues = true;
                command.stringValues._internal_._bFreeInnerStringValues = false;

                goto call_callback;
            }
        }
        // Default arguments, since: no assignment and forced to use default arguments or no further arguments found
        command.stringValues.stringValues = command.defaultValues;
        command.stringValues.stringValuesCount = command.defaultValuesCount;

        // Owned by the user
        command.stringValues._internal_._bFreeStringValues = false;
        command.stringValues._internal_._bFreeInnerStringValues = false;
    }

call_callback:
    // When inserting flags we need to sort them in a specific manner
    if constexpr (std::is_same_v<T, UCLI::Flag>)
    {
        // If the flag is with a priority higher than SIZE_MAX / 2, then insert at the front of the queue.
        // These are sorted after the fact
        if (command.priority > (SIZE_MAX / 2))
            callbacks.emplace_front(&command, false);
        else
        {
            // Iterate call callbacks and count up until we hit commandDepth.
            for (size_t a = 0, b = 0; a < callbacks.size(); a++)
            {
                // Skip other flags
                if (!callbacks[a].bCommand)
                    continue;

                // We're in a command, and we found the command at our required depth
                if (b == commandDepth)
                {
                    // Insert just after the current command
                    callbacks.insert(
                        callbacks.begin() + static_cast<std::remove_cvref_t<decltype(callbacks)>::difference_type>(a + 1),
                        {
                            .ptr = &command,
                            .bCommand = false
                        }
                    );

                    // Find the length of the flag sequence until the next command
                    size_t f = a + 1;
                    for (; f < callbacks.size() && !callbacks[f].bCommand; f++)
                        ;

                    // Sort flags in descending order based on their priority
                    std::ranges::sort(
                        callbacks.begin() + static_cast<std::remove_cvref_t<decltype(callbacks)>::difference_type>(a + 1),
                        callbacks.begin() + static_cast<std::remove_cvref_t<decltype(callbacks)>::difference_type>(f),
                        [](const UCLI::Parser::CallbackObject& x, const UCLI::Parser::CallbackObject& y) -> bool
                        {
                            return static_cast<UCLI::Flag*>(x.ptr)->priority > static_cast<UCLI::Flag*>(y.ptr)->priority;
                        }
                    );
                    break;
                }

                b++;
            }
        }
    }
    else
        callbacks.emplace_back(&command, true);
}

static int getAssignmentIndex(const char* str) noexcept
{
    if (str[0] == '=')
        return -1;

    int assignmentIndex = -1;
    for (int j = 0; j < INT32_MAX; j++)
    {
        if (str[j] == '\0')
            break;
        if (str[j] == '=')
        {
            assignmentIndex = j;
            break;
        }
    }
    return assignmentIndex;
}

bool UCLI::Parser::findFlagsRecursive(int& i, const int argc, char** argv, const int assignmentIndex, int64_t& depth, const Command* command, const std::string& cleanName) noexcept
{
    for (size_t j = 0; j < command->flagsCount; j++)
    {
        if (cleanName == command->flags[j].longName)
        {
            executeCommand(
                i,
                argc,
                argv,
                command->flags[j],
                assignmentIndex,
                arrayDelimiter,
                bToggleBooleans,
                flagPrefix,
                false,
                depth,
                callbacks
            );
            return true;
        }
    }

    if (command->_internal_parent != nullptr)
    {
        depth++;
        return findFlagsRecursive(i, argc, argv, assignmentIndex, depth, command->_internal_parent, cleanName);
    }
    return false;
}

bool UCLI::Parser::findFlagsRecursive(int& i, const int argc, char** argv, int64_t& depth, const Command* command, const char shortName, const bool bBatched) noexcept
{
    for (size_t j = 0; j < command->flagsCount; j++)
    {
        if (command->flags[j].shortName == shortName)
        {
            executeCommand(
                i,
                argc,
                argv,
                command->flags[j],
                -1,
                arrayDelimiter,
                bToggleBooleans,
                flagPrefix,
                bBatched,
                depth,
                callbacks
            );
            return true;
        }
    }

    if (command->_internal_parent != nullptr)
    {
        depth++;
        return findFlagsRecursive(i, argc, argv, depth, command->_internal_parent, shortName, bBatched);
    }
    return false;
}

/**
 * How this works:
 * 1. We add a default help command if the user wants default help/--help arguments
 * 1. If we have less than 1 terminal arguments we directly exit
 * 1. For every argument, we check if it's a flag or command. Flags start with - or -- unless they are - or -- which are
 *    reserved commands
 * 1. For flags, we clean up the name and run findFlagsRecursive. This function looks for a flag in the current chain
 *    of commands.
 * 1. For commands, we iterate the current command's subcommand list to find the matching command
 * 1. Once we found the correct command or flag, we run the executeCommand function on it, which parses the command
 *    further. It handles argument types, default values, setting and allocating the values, and adding + sorting
 *    callbacks to the command queue. More information in the comment above this function.
 * 1. Once we have built a command queue, we execute every callback until the queue is empty
 */
UCLI::Parser& UCLI::Parser::parse(const int argc, char** argv) noexcept
{
    if (bUseHelp)
        pushHelp();

    if (argc <= 1)
    {
        if (defaultCommand == nullptr)
            return *this;
        int tmp = 0;
        executeCommand(
            tmp,
            argc,
            argv,
            *defaultCommand,
            -1,
            arrayDelimiter,
            bToggleBooleans,
            flagPrefix,
            false,
            0,
            callbacks
        );
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            const char* current = argv[i];
            if (current[0] == '\0')
                continue;

            // Flags
            if (current[0] == flagPrefix)
            {
                // Skip -- and -
                if (current[1] == '\0' || (current[1] == flagPrefix && current[2] == '\0'))
                    continue;

                int64_t depth = 0;

                // --flag
                if (current[1] == flagPrefix)
                {
                    // Clean up the command name so that we can search for the command
                    std::string cleanName = current + 2;
                    int assignmentIndex = getAssignmentIndex(cleanName.c_str());
                    if (assignmentIndex != -1)
                    {
                        cleanName.erase(assignmentIndex);
                        assignmentIndex += 2; // + 2 to account for removing the --
                    }

                    if (!findFlagsRecursive(i, argc, argv, assignmentIndex, depth, currentCommand, cleanName) && defaultFlag != nullptr)
                    {
                        defaultFlag->_internal_ctx_ = argv[i];
                        executeCommand(i, argc, argv, *defaultFlag, -1, arrayDelimiter, bToggleBooleans, flagPrefix, false, 0, callbacks);
                        goto default_skip;
                    }
                }
                else
                {
                    for (size_t f = 1; current[f] != '\0'; f++)
                    {
                        if (
                            !findFlagsRecursive(
                                i,
                                argc,
                                argv,
                                depth,
                                currentCommand,
                                current[f],
                                f > 1 || (f == 1 && current[f + 1] != '\0')
                            )
                            && defaultFlag != nullptr
                        )
                        {
                            defaultFlag->_internal_ctx_ = argv[i];
                            executeCommand(i, argc, argv, *defaultFlag, -1, arrayDelimiter, bToggleBooleans, flagPrefix, false, 0, callbacks);
                            goto default_skip;
                        }
                    }
                }
            }
            else
            {
                // Clean up the command name so that we can search for the command
                std::string cleanName = current;
                const int assignmentIndex = getAssignmentIndex(current);
                if (assignmentIndex != -1)
                    cleanName.erase(assignmentIndex);

                // If inside a command, search for subcommands, otherwise look inside the layer 1 array.
                Command* currentCommandsArray = nullptr;
                size_t currentCommandsSize = 0;

                if (currentCommand == nullptr)
                {
                    currentCommandsArray = commands.data();
                    currentCommandsSize = commands.size();
                }
                else
                {
                    currentCommandsArray = currentCommand->subcommands;
                    currentCommandsSize = currentCommand->subcommandsCount;
                }

                // Execute the default command if false
                bool bFound = false;
                for (size_t f = 0; f < currentCommandsSize; f++)
                {
                    if (cleanName == currentCommandsArray[f].longName || (cleanName.length() == 1 && cleanName[0] == currentCommandsArray[f].shortName))
                    {
                        bFound = true;
                        executeCommand(i, argc, argv, currentCommandsArray[f], assignmentIndex, arrayDelimiter, bToggleBooleans, flagPrefix, false, 0, callbacks);

                        currentCommandsArray[f]._internal_parent = currentCommand;
                        currentCommand = &currentCommandsArray[f];
                        break;
                    }
                }

                if (!bFound && defaultCommand != nullptr)
                {
                    defaultCommand->_internal_ctx_ = argv[i];
                    executeCommand(i, argc, argv, *defaultCommand, -1, arrayDelimiter, bToggleBooleans, flagPrefix, false, 0, callbacks);
                    goto default_skip;
                }
            }
        }
default_skip:

        // Find the length of the flag sequence until the next command
        size_t f = 0;
        for (; f < callbacks.size() && !callbacks[f].bCommand; f++)
            ;

        // Sort flags in descending order based on their priority
        std::ranges::sort(
            callbacks.begin(),
            callbacks.begin() + static_cast<std::remove_cvref_t<decltype(callbacks)>::difference_type>(f),
            [](const UCLI::Parser::CallbackObject& x, const UCLI::Parser::CallbackObject& y) -> bool
            {
                return static_cast<UCLI::Flag*>(x.ptr)->priority > static_cast<UCLI::Flag*>(y.ptr)->priority;
            }
        );


        for (const auto& a : callbacks)
        {
            if (a.bCommand)
            {
                auto* command = static_cast<Command*>(a.ptr);
                if (command->callback(command) == UCLI_CALLBACK_RESULT_PREMATURE_EXIT)
                    break;
            }
            else
            {
                auto* flag = static_cast<Flag*>(a.ptr);
                if (flag->callback(flag) == UCLI_CALLBACK_RESULT_PREMATURE_EXIT)
                    break;
            }
        }
        callbacks.clear();
    }

    bShowHelp = false;
    return *this;
}