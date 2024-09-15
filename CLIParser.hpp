#pragma once
#include <vector>
#include <string>
#include "Common.h"

namespace UCLI
{

    // UntitledImGuiFramework Event Safety - Any time but recommended you use at Startup
    class MLS_PUBLIC_API Parser
    {
    public:
        typedef UCLI_Parser_ArrayFlag ArrayFlag;
        typedef UCLI_Parser_BooleanFlag BooleanFlag;
        typedef UCLI_Parser_BooleanFlagWithFunc BooleanFlagWithFunc;
        typedef UCLI_Parser_Pair Pair;
        typedef UCLI_Parser_PairWithFunc PairWithFunc;
        typedef UCLI_Parser_UnknownArgumentsCallback UnknownArgumentsCallback;
        typedef UCLI_Parser_ArrayFlagFunc ArrayFlagFunc;

        Parser() noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        void init() noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        void setDelimiter(char del) noexcept;
        // UntitledImGuiFramework Event Safety - Any time
        void setArgumentStyleWindows(bool bWindows) noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        void setBooleanFlipping(bool bFlip) noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        void setUnknownArgumentCallback(UnknownArgumentsCallback func, void* callbackData = nullptr) noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        void setDefaultArray(void* additionalData, ArrayFlagFunc func) noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        void parse(int argc, char** argv,
                   ArrayFlag* arrayFlags = nullptr, size_t arrayFlagsSize = 0,
                   BooleanFlag* booleanFlags = nullptr, size_t booleanFlagsSize = 0,
                   BooleanFlagWithFunc* booleanFlagsWithFunc = nullptr, size_t booleanFlagsWithFuncSize = 0,
                   Pair* pairs = nullptr, size_t pairsSize = 0,
                   PairWithFunc* pairsWithFunc = nullptr, size_t pairsWithFuncSize = 0) noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        void parse(int argc, char** argv,
                   const std::vector<ArrayFlag>& arrayFlags = {},
                   const std::vector<BooleanFlag>& booleanFlags = {},
                   const std::vector<BooleanFlagWithFunc>& booleanFlagsWithFunc = {},
                   const std::vector<Pair>& pairs = {},
                   const std::vector<PairWithFunc>& pairsWithFunc = {}) noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        static void cleanupPairs(UCLI_Parser_Pair* pairs, size_t pairsSize);
    private:
        void parseShortArgument(std::vector<char*>& args, std::string& singleChar, const std::string& tmp,
                                ArrayFlag* arrayFlags, size_t arrayFlagsSize,
                                BooleanFlag* booleanFlags, size_t booleanFlagsSize,
                                BooleanFlagWithFunc* booleanFlagsWithFunc, size_t booleanFlagsWithFuncSize) noexcept;

        void parseLongArgument(std::vector<char*>& args, uint8_t frontTruncate, const std::string& tmp,
                               ArrayFlag* arrayFlags = nullptr, size_t arrayFlagsSize = 0,
                               BooleanFlag* booleanFlags = nullptr, size_t booleanFlagsSize = 0,
                               BooleanFlagWithFunc* booleanFlagsWithFunc = nullptr, size_t booleanFlagsWithFuncSize = 0,
                               Pair* pairs = nullptr, size_t pairsSize = 0,
                               PairWithFunc* pairsWithFunc = nullptr, size_t pairsWithFuncSize = 0, bool bCheckShort = false) noexcept;

        UCLI_Parser_Data data{};
    };
}