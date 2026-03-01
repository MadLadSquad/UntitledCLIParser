#include "CLIParser.hpp"
#include "Router.hpp"

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
                callbacks
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
                callbacks
            );
            return true;
        }
    }

    if (command->_internal_parent != nullptr)
        return findFlagsRecursive(i, argc, argv, depth + 1, command->_internal_parent, shortName, bBatched);
    return false;
}