#pragma once
#include <vector>
#include <string>
#include "Common.h"

namespace UCLI
{
    class UVK_PUBLIC_API Parser
    {
    public:
        Parser() noexcept;

        void init() noexcept;

        void setDelimiter(char del) noexcept;
        void setArgumentStyleWindows(bool bWindows) noexcept;

        void setBooleanFlipping(bool bFlip) noexcept;

        void setUnknownArgumentCallback(UCLI_Parser_UnknownArgumentsCallback func, void* callbackData = nullptr) noexcept;

        void setDefaultArray(void* additionalData, UCLI_Parser_ArrayFlagFunc func) noexcept;

        void parse(int argc, char** argv,
                   UCLI_Parser_ArrayFlag* arrayFlags = nullptr, size_t arrayFlagsSize = 0,
                   UCLI_Parser_BooleanFlag* booleanFlags = nullptr, size_t booleanFlagsSize = 0,
                   UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc = nullptr, size_t booleanFlagsWithFuncSize = 0,
                   UCLI_Parser_Pair* pairs = nullptr, size_t pairsSize = 0,
                   UCLI_Parser_PairWithFunc* pairsWithFunc = nullptr, size_t pairsWithFuncSize = 0) noexcept;

        static void cleanupPairs(UCLI_Parser_Pair* pairs, size_t pairsSize);
    private:
        void parseShortArgument(std::vector<char*>& args, std::string& singleChar, const std::string& tmp,
                                UCLI_Parser_ArrayFlag* arrayFlags, size_t arrayFlagsSize,
                                UCLI_Parser_BooleanFlag* booleanFlags, size_t booleanFlagsSize,
                                UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc, size_t booleanFlagsWithFuncSize) noexcept;

        void parseLongArgument(std::vector<char*>& args, uint8_t frontTruncate, std::string& tmp,
                               UCLI_Parser_ArrayFlag* arrayFlags = nullptr, size_t arrayFlagsSize = 0,
                               UCLI_Parser_BooleanFlag* booleanFlags = nullptr, size_t booleanFlagsSize = 0,
                               UCLI_Parser_BooleanFlagWithFunc* booleanFlagsWithFunc = nullptr, size_t booleanFlagsWithFuncSize = 0,
                               UCLI_Parser_Pair* pairs = nullptr, size_t pairsSize = 0,
                               UCLI_Parser_PairWithFunc* pairsWithFunc = nullptr, size_t pairsWithFuncSize = 0, bool bCheckShort = false) noexcept;

        UCLI_Parser_Data data{};
    };
}