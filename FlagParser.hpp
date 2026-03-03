#pragma once
namespace UCLI
{
    class Parser;
    namespace Internal
    {
        int parseFlag(int& i, int argc, char** argv, Parser& p) noexcept;
        bool probeFlags(int& i, int argc, char** argv, Parser& p) noexcept;
    }
}


