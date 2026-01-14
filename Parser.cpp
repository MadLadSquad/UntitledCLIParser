#include <CLIParser.hpp>

template<typename T>
static void executeCommand(int& i, const int argc, char** argv, T& command, const int assignmentIndex, const char arrayDelimiter, const bool bToggle) noexcept
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
        else if (argc > (i + 1))
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
            if (argc > (i + 1))
            {
                i++; // Skip to the next argument

                command.stringValues.stringValues = argv + i;
                command.stringValues.stringValuesCount = 1;
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

        // Non-variable assignment: --flag a b c d -- ...
        for (++i; i < argc; i++)
        {
            // This is a standard semantic for starting/terminating an array for the latest command
            if (strcmp(argv[i], "--") == 0)
                break;

            stringValues.push_back(argv[i]);
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

        // We're not allocating and copying the inner strings, since they were already allocated by the toolchain's code
        command.stringValues.stringValues = static_cast<char**>(malloc(stringValues.size() * sizeof(char*)));
        memcpy(command.stringValues.stringValues, stringValues.data(), stringValues.size() * sizeof(char*));

        command.stringValues.stringValuesCount = stringValues.size();

        // Inner values owned by the user, outer container owned by us
        command.stringValues._internal_._bFreeStringValues = true;
        command.stringValues._internal_._bFreeInnerStringValues = false;
    }
call_callback:
    command.callback(&command);
}

static int getAssignmentIndex(const char* str) noexcept
{
    if (str[0] == '=')
        return -1;

    int assignmentIndex = -1;
    for (int j = 0; j < INT8_MAX; j++)
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

UCLI::Parser& UCLI::Parser::parse(const int argc, char** argv) noexcept
{
    if (bUseHelp)
        pushHelp();

    if (argc <= 1)
    {
        if (defaultCommand == nullptr)
            return *this;
        int tmp = 0;
        executeCommand(tmp, argc, argv, *defaultCommand, -1, arrayDelimiter, bToggleBooleans);
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            const char* current = argv[i];
            if (current[0] == '\0')
                continue;

            // Whether we actually found a command or flag so that we run the default one if we have not done so
            bool bFound = false;

            // Flags
            if (current[0] == flagPrefix)
            {
                // Skip --
                if (current[1] == flagPrefix && current[2] == '\0')
                    continue;

                // --flag
                if (current[1] == flagPrefix)
                {
                    std::string cleanName = current + 2;
                    int assignmentIndex = getAssignmentIndex(cleanName.c_str());
                    if (assignmentIndex != -1)
                    {
                        cleanName.erase(assignmentIndex);
                        assignmentIndex += 2; // + 2 to account for removing the --
                    }

                    for (auto& flag : flags)
                    {
                        if (cleanName == flag.longName)
                        {
                            bFound = true;
                            executeCommand(
                                i,
                                argc,
                                argv,
                                flag,
                                assignmentIndex,
                                arrayDelimiter,
                                bToggleBooleans
                            );
                            break;
                        }
                    }

                    if (!bFound && defaultFlag != nullptr)
                        executeCommand(i, argc, argv, *defaultFlag, -1, arrayDelimiter, bToggleBooleans);
                }
                // -
                else if (current[1] == '\0')
                    continue;

                for (size_t f = 1; current[f] != '\0'; f++)
                {
                    for (auto& flag : flags)
                    {
                        if (current[f] == flag.shortName)
                        {
                            bFound = true;
                            executeCommand(
                                i,
                                argc,
                                argv,
                                flag,
                                -1,
                                arrayDelimiter,
                                bToggleBooleans
                            );
                            break;
                        }
                    }

                    if (!bFound && defaultFlag != nullptr)
                        executeCommand(i, argc, argv, *defaultFlag, -1, arrayDelimiter, bToggleBooleans);
                    bFound = false;
                }
            }
            else
            {
                std::string cleanName = current;
                const int assignmentIndex = getAssignmentIndex(current);
                if (assignmentIndex != -1)
                    cleanName.erase(assignmentIndex);

                for (auto& command : commands)
                {
                    if (cleanName == command.longName || cleanName[0] == command.shortName)
                    {
                        bFound = true;
                        executeCommand(i, argc, argv, command, assignmentIndex, arrayDelimiter, bToggleBooleans);
                        break;
                    }
                }

                if (!bFound && defaultCommand != nullptr)
                    executeCommand(i, argc, argv, *defaultCommand, -1, arrayDelimiter, bToggleBooleans);
            }
        }
    }

    bShowHelp = false;
    return *this;
}