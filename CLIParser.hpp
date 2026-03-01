#pragma once
#include <vector>
#include <deque>
#include "Common.h"

namespace UCLI
{
    typedef UCLI_Command Command;
    typedef UCLI_Flag Flag;
    typedef UCLI_CommandType CommandType;
    typedef UCLI_CallbackResult CallbackResult;

    class MLS_PUBLIC_API Parser
    {
    public:
        Parser() noexcept = default;

        Parser& setHelpHeader(const char* header) noexcept;
        Parser& setHelpFooter(const char* footer) noexcept;
        // Set to true by default
        Parser& setUseGeneratedHelp(bool bUseGeneratedHelp) noexcept;
        // Set to 2 by default
        Parser& setHelpSubcommandIndentationSpaces(size_t indentSpaces) noexcept;

        // The default is `-`
        Parser& setFlagPrefix(char prefix) noexcept;
        // The default is `,`
        Parser& setArrayDelimiter(char delimiter) noexcept;

        // Whether to toggle boolean arguments or to set them to true. The default behaviour is to set them to true
        Parser& setBoolToggle(bool bToggle) noexcept;

        Parser& pushCommand(const Command& command) noexcept;
        Parser& pushFlag(const Flag& flag) noexcept;

        Parser& pushDefaultCommand(const Command& command) noexcept;
        Parser& pushDefaultFlag(const Flag& flag) noexcept;

        Parser& parse(int argc, char** argv) noexcept;

        Parser& release() noexcept;
        ~Parser() noexcept;
    private:
        std::string helpHeader;
        std::string helpFooter;

        std::string indentationString = "  ";

        char arrayDelimiter = ',';
        char flagPrefix = '-';

        bool bToggleBooleans = false;

        bool bUseHelp = true;
        bool bShowHelp = false;

        std::vector<Command> commands{};
        std::vector<Flag> flags{};

        Command* currentCommand = nullptr;

        Command* defaultCommand = nullptr;
        Flag* defaultFlag = nullptr;

        void pushHelp() noexcept;

        static void printCommands(const Command* commands, size_t size, const std::string& indentationString, size_t indentation) noexcept;
        static void printFlags(const Flag* flags, size_t size, const std::string& indentationString, size_t indentation) noexcept;

        static int64_t getAssignmentIndex(const char* str) noexcept;

        bool findFlagsRecursive(int& i, int argc, char** argv, int64_t assignmentIndex, int64_t depth, const Command* command, const std::string& cleanName) noexcept;
        bool findFlagsRecursive(int& i, int argc, char** argv, int64_t depth, const Command* command, char shortName, bool bBatched) noexcept;

        static void freeCommands(Command& command) noexcept;
        static void freeFlags(Flag& command) noexcept;

    public:
        struct CallbackObject
        {
            void* ptr = nullptr;
            bool bCommand = false;
        };

        static CallbackResult helpCommand(const Flag* command) noexcept;
        static CallbackResult helpCommand(const Command* command) noexcept;
    private:
        std::deque<CallbackObject> callbacks;
    };
}