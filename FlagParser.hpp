#pragma once
#include "Common.h"
namespace UCLI
{
    class Parser;
    namespace Internal
    {
        // The outcome of probing for flags. This is an internal C++ enum, so it doesn't follow the naming of the
        // enums in Common.h, which are part of the C ABI. The members are also deliberately not spelled in all caps,
        // since ERROR is a macro in the Windows headers
        enum class ProbeResult
        {
            // We stopped at a value that belongs to the command. It is the argument at index i
            Value,
            // We ran out of arguments before finding a value, so the command has no value of its own. The default
            // values have already been applied to it
            Exhausted,
            // We found an unrecognised flag while in strict mode
            Failure
        };

        int parseFlag(int& i, int argc, char** argv, Parser& p) noexcept;
        ProbeResult probeFlags(UCLI_Command& command, int& i, int argc, char** argv, Parser& p) noexcept;
    }
}


