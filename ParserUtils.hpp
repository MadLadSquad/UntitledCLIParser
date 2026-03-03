#pragma once

// Cast a const T* of anything to void* for use with standard library functions
#define UCLI_VOID_CAST(x)                       \
    static_cast<void*>(                         \
        const_cast<                             \
            std::add_pointer_t<                 \
                std::remove_const_t<            \
                    std::remove_pointer_t<      \
                        std::decay_t<           \
                            decltype(x)         \
                        >                       \
                    >                           \
                >                               \
            >                                   \
        >                                       \
        (x)                                     \
    )

template<typename T>
static void useDefaultArguments(T& command) noexcept
{
    command.stringValues.stringValues = command.defaultValues;
    command.stringValues.stringValuesCount = command.defaultValuesCount;

    command.stringValues._internal_._bFreeStringValues = false;
    command.stringValues._internal_._bFreeInnerStringValues = false;
}

template<typename T>
static void useNullArguments(T& command) noexcept
{
    command.stringValues.stringValues = nullptr;
    command.stringValues.stringValuesCount = 0;

    command.stringValues._internal_._bFreeStringValues = false;
    command.stringValues._internal_._bFreeInnerStringValues = false;
}