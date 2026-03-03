#include <CLIParser.hpp>
#include "Router.hpp"

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
 *    callbacks to the command queue. More information in Router.hpp.
 * 1. Once we have built a command queue, we execute every callback until the queue is empty
 */
UCLI::Parser& UCLI::Parser::parse(const int argc, char** argv) noexcept
{
    if (bUseHelp)
        pushHelp();

    if (argc <= 1)
    {
        if (defaultCommand == nullptr)
            goto finish_parse;
        defaultCommand->callback(defaultCommand);
    }
    else
    {
        bool bHitDefault = false;

        for (int i = 1; i < argc; i++)
        {
            const char* current = argv[i];
            if (current[0] == '\0')
                continue;

            // Flags
            if (current[0] == flagPrefix)
            {
                if (Internal::parseFlag(i, argc, argv, *this) == 2)
                {
                    bHitDefault = true;
                    break;
                }
            }
            else
            {
                // Clean up the command name so that we can search for the command
                std::string cleanName = current;
                const int64_t assignmentIndex = getAssignmentIndex(current);
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

                        currentCommandsArray[f]._internal_parent = currentCommand;
                        currentCommand = &currentCommandsArray[f];

                        executeCommand(
                            i,
                            argc,
                            argv,
                            currentCommandsArray[f],
                            assignmentIndex,
                            arrayDelimiter,
                            bToggleBooleans,
                            flagPrefix,
                            false,
                            0,
                            callbacks,
                            *this
                        );
                        break;
                    }
                }

                if (!bFound)
                {
                    if (defaultCommand != nullptr)
                    {
                        // This is for diagnostic commands like --help
                        defaultCommand->_internal_ctx_ = argv[i];
                        // Do not run executeCommand, since it contains additional parsing logic. Default commands
                        // should not take advantage of features such as default arguments and values
                        pushCallback(*defaultCommand, 0, callbacks);
                    }

                    // In lenient mode we just add the default and continue forward
                    if (bStrictMode)
                    {
                        bHitDefault = true;
                        break;
                    }
                }
            }
        }

        // In lenient mode we simply run everything
        if (bHitDefault && bStrictMode)
        {
            // In strict mode we only run the default command
            const auto& last = callbacks.back();
            if (last.bCommand && defaultCommand != nullptr)
            {
                const auto* ptr = static_cast<Command*>(last.ptr);
                ptr->callback(ptr);
            }
            else if (defaultFlag != nullptr)
            {
                const auto* ptr = static_cast<Flag*>(last.ptr);
                ptr->callback(ptr);
            }
        }
        else
        {
            // Find the length of the flag sequence until the next command
            size_t f = 0;
            for (; f < callbacks.size() && !callbacks[f].bCommand; f++)
                ;

            // Sort flags in descending order based on their priority
            std::ranges::sort(
                callbacks.begin(),
                callbacks.begin() + static_cast<std::remove_cvref_t<decltype(callbacks)>::difference_type>(f),
                [](const CallbackObject& x, const CallbackObject& y) -> bool
                {
                    return static_cast<Flag*>(x.ptr)->priority > static_cast<Flag*>(y.ptr)->priority;
                }
            );

            for (const auto& a : callbacks)
            {
                if (a.bCommand)
                {
                    const auto* command = static_cast<Command*>(a.ptr);
                    if (command->callback(command) == UCLI_CALLBACK_RESULT_PREMATURE_EXIT)
                        break;
                }
                else
                {
                    const auto* flag = static_cast<Flag*>(a.ptr);
                    if (flag->callback(flag) == UCLI_CALLBACK_RESULT_PREMATURE_EXIT)
                        break;
                }
            }
        }

        callbacks.clear();
    }

finish_parse:
    bShowHelp = false;
    return *this;
}