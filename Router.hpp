#pragma once
#include "BooleanParser.hpp"
#include "StringParser.hpp"
#include "ArrayParser.hpp"

#include "CLIParser.hpp"

template<typename T>
static void pushCallback(T& command, const int64_t commandDepth, std::deque<UCLI::Parser::CallbackObject>& callbacks) noexcept
{
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

// This function routes the actual parsing of arguments and handling of syntax features to the command's corresponding
// parser
template<typename T>
static void executeCommand(int& i, const int argc, char** argv, T& command, const int64_t assignmentIndex, const char arrayDelimiter, const bool bToggle, const char flagPrefix, const bool bForcedDefault, const int64_t commandDepth, std::deque<UCLI::Parser::CallbackObject>& callbacks, UCLI::Parser& p) noexcept
{
    if (command.type == UCLI_COMMAND_TYPE_BOOL)
        loadBooleanCommand(i, argc, argv, command, assignmentIndex, bToggle, flagPrefix, bForcedDefault);
    else if (command.type == UCLI_COMMAND_TYPE_STRING)
    {
        if (!loadStringCommand(i, argc, argv, command, assignmentIndex, bForcedDefault, flagPrefix, p))
            return;
    }
    else if (command.type == UCLI_COMMAND_TYPE_ARRAY)
        if (!loadArrayCommand(i, argc, argv, command, assignmentIndex, arrayDelimiter, flagPrefix, bForcedDefault, p))
            return;

    pushCallback(command, commandDepth, callbacks);
}