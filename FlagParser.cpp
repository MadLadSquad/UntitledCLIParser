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
                callbacks,
                *this
            );
            return true;
        }
    }

    if (command->_internal_parent != nullptr)
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

// Returns 0 on success, 1 when skipping -- or - and 2 when skipping arguments because we hit the default
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
                pushCallback(*p.defaultFlag, 0, p.callbacks);
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
                    pushCallback(*p.defaultFlag, 0, p.callbacks);
                }
                return p.bStrictMode ? 2 : 0;
            }
        }
    }
    return 0;
}

bool UCLI::Internal::probeFlags(int& i, const int argc, char** argv, UCLI::Parser& p) noexcept
{
    for (; i < argc; i++)
    {
        // Is a flag
        if (argv[i][0] == p.flagPrefix)
        {
            auto result = parseFlag(i, argc, argv, p);

            switch (result)
            {
            // Unrecognized argument. We return false so that we can skip adding the current command to the
            case 2:
                return false;

            // When we hit -- or -. Move the iterator forward to the next argument so we can deal with anything else
            case 1:
                i++;
                return true;
            case 0:
                break;
            default:
                break;
            }
        }
        else
            break;
    }
    return true;
}
