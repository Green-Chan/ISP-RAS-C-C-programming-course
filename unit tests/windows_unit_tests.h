#include<windows.h>
#include<iostream>

#ifndef __UNIT_TESTS
#define __UNIT_TESTS

//!  Prints result of unit testing @c code with expected result @c expected

#define $unit_test(code, expected)                                                                                                                    \
{                                                                                                                                                     \
    const __decltype(code)     &_result   = (code);                                                                                                   \
    const __decltype(expected) &_expected = (expected);                                                                                               \
                                                                                                                                                      \
    CONSOLE_SCREEN_BUFFER_INFO con = {};                                                                                                              \
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &con);                                                                                \
    unsigned oldAttrs = con.wAttributes;                                                                                                              \
                                                                                                                                                      \
    if (_result == _expected) {                                                                                                                       \
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN);                         \
        std::cerr << "[PASSED] " << __FILE__ << " (" << __LINE__ << "):" << #code << " == " << _result << " == " << _expected << " == " << #expected; \
    } else {                                                                                                                                          \
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED   | FOREGROUND_INTENSITY | BACKGROUND_RED);                           \
        std::cerr << "[FAILED] " << __FILE__ << " (" << __LINE__ << "):" << #code << " == " << _result << " != " << _expected << " == " << #expected; \
    }                                                                                                                                                 \
                                                                                                                                                      \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), oldAttrs);                                                                               \
    std::cerr << std::endl;                                                                                                                           \
}

//! Prints result of unit testing @c code with expected result @c expected. Test is considered passed if the difference between the two values is less than @c eps


#define $f_unit_test(code, expected, eps)                                                                                                             \
{                                                                                                                                                     \
    const __decltype(code)     &_result   = (code);                                                                                                   \
    const __decltype(expected) &_expected = (expected);                                                                                               \
                                                                                                                                                      \
    CONSOLE_SCREEN_BUFFER_INFO con = {};                                                                                                              \
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &con);                                                                                \
    unsigned oldAttrs = con.wAttributes;                                                                                                              \
                                                                                                                                                      \
    if (_result - _expected > -eps && _result - _expected < eps) {                                                                                    \
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN);                         \
        std::cerr << "[PASSED] " << __FILE__ << " (" << __LINE__ << "):" << #code << " == " << _result << " ~= " << _expected << " == " << #expected; \
    } else {                                                                                                                                          \
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED   | FOREGROUND_INTENSITY | BACKGROUND_RED);                           \
        std::cerr << "[FAILED] " << __FILE__ << " (" << __LINE__ << "):" << #code << " == " << _result << " != " << _expected << " == " << #expected; \
    }                                                                                                                                                 \
                                                                                                                                                      \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), oldAttrs);                                                                               \
    std::cerr << std::endl;                                                                                                                           \
}

#endif
