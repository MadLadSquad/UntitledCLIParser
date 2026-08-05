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
        // The long name may be null when the user only gave the flag a short name, so we can't hand it to
        // std::string's comparison operator unchecked
        if (array[j].longName != nullptr && cleanName == array[j].longName)
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
                depth,
                callbacks,
                *this
            );
            return true;
        }
    }

    // Walk up to the parent command, and once we run past the root of the chain search the parser's own flags, so
    // that a flag declared on an ancestor command - or globally - still resolves from inside a subcommand
    if (command != nullptr)
        return findFlagsRecursive(i, argc, argv, assignmentIndex, depth + 1, command->_internal_parent, cleanName);
    return false;
}

bool UCLI::Parser::findFlagsRecursive(int& i, const int argc, char** argv, const int64_t depth, const Command* command, const char shortName, const bool bBatched) noexcept
{
    // The command is null when we haven't entered one yet, and when we've walked past the root of the command chain.
    // In both cases we search the parser's own flags
    Flag* array = command == nullptr ? flags.data() : command->flags;
    const size_t count = command == nullptr ? flags.size() : command->flagsCount;

    for (size_t j = 0; j < count; j++)
    {
        if (array[j].shortName == shortName)
        {
            executeCommand(
                i,
                argc,
                argv,
                array[j],
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

    // Same as above: walk up the chain of parents and finally fall back to the parser's own flags
    if (command != nullptr)
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
            if (p.bHasDefaultFlag)
            {
                // For diagnostic commands like --help
                p.defaultFlag._internal_ctx_ = argv[i];
                // Do not run executeCommand, since it contains additional parsing logic. Default commands should not
                // take advantage of features such as default arguments and values
                pushCallback(p.callbacks.size(), p.defaultFlag, 0, p.callbacks);
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
                if (p.bHasDefaultFlag)
                {
                    // For diagnostic commands like --help
                    p.defaultFlag._internal_ctx_ = argv[i];
                    // Do not run executeCommand, since it contains additional parsing logic. Default commands should not
                    // take advantage of features such as default arguments and values
                    pushCallback(p.callbacks.size(), p.defaultFlag, 0, p.callbacks);
                }
                return p.bStrictMode ? 2 : 0;
            }
        }
    }
    return 0;
}

// Add all flags before the argument to a command in syntax such as: git clone --recursive https://...
UCLI::Internal::ProbeResult UCLI::Internal::probeFlags(UCLI_Command& command, int& i, const int argc, char** argv, UCLI::Parser& p) noexcept
{
    ProbeResult result = ProbeResult::Exhausted;

    for (; i < argc; i++)
    {
        // Not a flag, so this is a value that belongs to the command. We check this before parsing a flag, since every
        // caller of this function increments i past the last argument that a flag consumed
        if (argv[i][0] != p.flagPrefix)
        {
            result = ProbeResult::Value;
            break;
        }

        // Is a flag and errors when false
        if (parseFlag(i, argc, argv, p) == 2)
        {
            result = ProbeResult::Failure;
            break;
        }
    }

    // We never found a value for the command, so it only gets whatever the user set as its default
    if (result == ProbeResult::Exhausted)
    {
        if (command.defaultValues != nullptr && command.defaultValuesCount > 0)
            useDefaultArguments(command);
        else
            useNullArguments(command);
    }

    return result;
}
