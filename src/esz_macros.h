// SPDX-License-Identifier: MIT
/**
 * @file  esz_macros.h
 * @brief eszFW macros
 */

#ifndef ESZ_MACROS_H
#define ESZ_MACROS_H

#define CLR_STATE(number, bit) number &= ~(1UL << bit)
#define SET_STATE(number, bit) number |=   1UL << bit
#define IS_STATE_SET(number, bit) ((0U == (number & (1 << bit))) ? 0U : 1U)

/* Based on
 * https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/
 */
#if defined(_MSC_VER)
    #define DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define DISABLE_WARNING_POP            __pragma(warning( pop ))
    #define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

    #define DISABLE_WARNING_PADDING            DISABLE_WARNING(4820)
    #define DISABLE_WARNING_SPECTRE_MITIGATION DISABLE_WARNING(5045)
    #define DISABLE_WARNING_SYMBOL_NOT_DEFINED DISABLE_WARNING(4668)

#elif defined(__GNUC__) || defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
    #define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

    #define DISABLE_WARNING_PADDING            DISABLE_WARNING(-Wpadded)
    #define DISABLE_WARNING_SPECTRE_MITIGATION
    #define DISABLE_WARNING_SYMBOL_NOT_DEFINED

#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING_PADDING
    #define DISABLE_WARNING_SPECTRE_MITIGATION
    #define DISABLE_WARNING_SYMBOL_NOT_DEFINED

#endif

#endif // ESZ_MACROS_H
