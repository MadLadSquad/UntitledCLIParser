#pragma once
#include "Common.h"
namespace UCLI
{
    class Parser;
    namespace Internal
    {
        int parseFlag(int& i, int argc, char** argv, Parser& p) noexcept;
        bool probeFlags(UCLI_Command& command, int& i, int argc, char** argv, Parser& p) noexcept;
    }
}


