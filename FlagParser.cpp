#include "CLIParser.hpp"
#include "Router.hpp"
#include "FlagParser.hpp"

int64_t UCLI::Parser::getAssignmentIndex(const char* str) noexcept
{
    if (str[0] == '=')
        return -1;

    int64_t assignmentIndex = -1;
    for (int64_t j = 0; j < INT64_MAX; j++)
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

bool UCLI::Parser::findFlagsRecursive(int& i, const int argc, char** argv, const int64_t assignmentIndex, const int64_t depth, const Command* command, const std::string& cleanName) noexcept
{
    Flag* array = command == nullptr ? flags.data() : command->flags;
    const size_t count = command == nullptr ? flags.size() : command->flagsCount;

    for (size_t j = 0; j < count; j++)
    {
        if (cleanName == array[j].longName)
        {
            executeCommand(
                i,
                argc,
                argv,
                array[j],
                assignmentIndex,
                arrayDelimiter,
                bToggleBooleans,
                flagPrefix,
                false,
                bProbingFlags,
                depth,
                callbacks,
                *this
            );
            return true;
        }
    }

    if (command != nullptr && command->_internal_parent != nullptr)
        return findFlagsRecursive(i, argc, argv, assignmentIndex, depth + 1, command->_internal_parent, cleanName);
    return false;
}

bool UCLI::Parser::findFlagsRecursive(int& i, const int argc, char** argv, const int64_t depth, const Command* command, const char shortName, const bool bBatched) noexcept
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
                bProbingFlags,
                depth,
                callbacks,
                *this
            );
            return true;
        }
    }

    if (command->_internal_parent != nullptr)
        return findFlagsRecursive(i, argc, argv, depth + 1, command->_internal_parent, shortName, bBatched);
    return false;
}

// Returns 0 on success, 1 when skipping -- or - and 2 only in strict mode when an unrecognized command is found
int UCLI::Internal::parseFlag(int& i, const int argc, char** argv, UCLI::Parser& p) noexcept
{
    const char* current = argv[i];

    // Skip -- and -
    if (current[1] == '\0' || (current[1] == p.flagPrefix && current[2] == '\0'))
        return 1;

    // --flag
    if (current[1] == p.flagPrefix)
    {
        // Clean up the command name so that we can search for the command
        std::string cleanName = current + 2;
        int64_t assignmentIndex = UCLI::Parser::getAssignmentIndex(cleanName.c_str());
        if (assignmentIndex != -1)
        {
            cleanName.erase(assignmentIndex);
            assignmentIndex += 2; // + 2 to account for removing the --
        }

        if (!p.findFlagsRecursive(i, argc, argv, assignmentIndex, 0, p.currentCommand, cleanName))
        {
            if (p.defaultFlag != nullptr)
            {
                // For diagnostic commands like --help
                p.defaultFlag->_internal_ctx_ = argv[i];
                // Do not run executeCommand, since it contains additional parsing logic. Default commands should not
                // take advantage of features such as default arguments and values
                pushCallback(p.callbacks.size(), *p.defaultFlag, 0, p.callbacks);
            }
            return p.bStrictMode ? 2 : 0;
        }
    }
    else // -f or -fF
    {
        for (size_t f = 1; current[f] != '\0'; f++)
        {
            if (
                !p.findFlagsRecursive(
                    i,
                    argc,
                    argv,
                    0,
                    p.currentCommand,
                    current[f],
                    f > 1 || (f == 1 && current[f + 1] != '\0')
                )
            )
            {
                if (p.defaultFlag != nullptr)
                {
                    // For diagnostic commands like --help
                    p.defaultFlag->_internal_ctx_ = argv[i];
                    // Do not run executeCommand, since it contains additional parsing logic. Default commands should not
                    // take advantage of features such as default arguments and values
                    pushCallback(p.callbacks.size(), *p.defaultFlag, 0, p.callbacks);
                }
                return p.bStrictMode ? 2 : 0;
            }
        }
    }
    return 0;
}

// Add all flags before the argument to a command in syntax such as: git clone --recursive https://...
bool UCLI::Internal::probeFlags(UCLI_Command& command, int& i, const int argc, char** argv, UCLI::Parser& p) noexcept
{
    p.bProbingFlags = true;

    for (; i < argc; i++)
    {
        // Is a flag and errors when false
        if (argv[i][0] == p.flagPrefix && parseFlag(i, argc, argv, p) == 2)
            return false;

        // A value
        if (argv[i][0] != p.flagPrefix)
            return true;
    }

    if (command.defaultValues != nullptr && command.defaultValuesCount > 0)
        useDefaultArguments(command);
    else
        useNullArguments(command);

    p.bProbingFlags = false;
    return true;
}
