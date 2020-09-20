#ifndef __UNIT_TESTS
#define __UNIT_TESTS

#include<windows.h>
#include<iostream>
#include<csetjmp>
#include<csignal>
#include<cstdlib>

extern jmp_buf env;
extern bool all_tests_passed;

void on_sigabrt(int signum);

#define $try_catch_sigabrt(code)        \
{                                       \
    if (setjmp(env) == 0) {             \
        got_sigabrt = false;            \
        signal(SIGABRT, &on_sigabrt);   \
        code                            \
    } else {                            \
        got_sigabrt = true;             \
    }                                   \
}

#define $set_passed_color() SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN)
#define $set_failed_color() SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED   | FOREGROUND_INTENSITY | BACKGROUND_RED)

#define __FILE_LINE__ __FILE__ << " (" << __LINE__ << "): "

#define $begin_testing() all_tests_passed = true
#define $testing_result()                                              \
{                                                                      \
    CONSOLE_SCREEN_BUFFER_INFO con = {};                               \
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &con); \
    unsigned oldAttrs = con.wAttributes;                               \
    if (all_tests_passed) {                                            \
        $set_passed_color();                                           \
        std::cerr << "ALL TESTS PASSED";                               \
    } else {                                                           \
        $set_failed_color();                                           \
        std::cerr << "THERE IS FAILED TEST";                           \
    }                                                                  \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), oldAttrs);\
    std::cerr << std::endl;                                            \
}

#define $unit_test_inside(code, expected_result, expected_sigabort, eps, equ_simb)                                                             \
{                                                                                                                                              \
    unsigned oldAttrs = {};                                                                                                                    \
                                                                                                                                               \
    if (setjmp(env) == 0) {                                                                                                                    \
        const __decltype(expected_result) &_expected = (expected_result);                                                                      \
        auto old_handler = signal(SIGABRT, &on_sigabrt);                                                                                       \
        const __decltype(code) &_result = (code);                                                                                              \
        signal(SIGABRT, old_handler);                                                                                                          \
        /*did not get sigabrt*/                                                                                                                \
        CONSOLE_SCREEN_BUFFER_INFO con = {};                                                                                                   \
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &con);                                                                     \
        oldAttrs = con.wAttributes;                                                                                                            \
        if (expected_sigabort) {                                                                                                               \
            $set_failed_color();                                                                                                               \
            all_tests_passed = false;                                                                                                          \
            std::cerr << "[FAILED] " << __FILE_LINE__ << "did not get SIGABRT running " << #code;                                              \
        } else {                                                                                                                               \
            if (_result - _expected >= -eps && _result - _expected <= eps) {                                                                   \
                $set_passed_color();                                                                                                           \
                std::cerr << "[PASSED] " << __FILE_LINE__ << #code << " == " << _result << equ_simb << _expected << " == " << #expected_result;\
            } else {                                                                                                                           \
                $set_failed_color();                                                                                                           \
                all_tests_passed = false;                                                                                                      \
                std::cerr << "[FAILED] " << __FILE_LINE__ << #code << " == " << _result << equ_simb << _expected << " == " << #expected_result;\
            }                                                                                                                                  \
        }                                                                                                                                      \
    } else {                                                                                                                                   \
        /*got sigabort*/                                                                                                                       \
        CONSOLE_SCREEN_BUFFER_INFO con = {};                                                                                                   \
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &con);                                                                     \
        oldAttrs = con.wAttributes;                                                                                                            \
        if (expected_sigabort) {                                                                                                               \
            $set_passed_color();                                                                                                               \
            std::cerr << "[PASSED] " << __FILE_LINE__ << "got SIGABRT running " << #code;                                                      \
        } else {                                                                                                                               \
            $set_failed_color();                                                                                                               \
            all_tests_passed = false;                                                                                                          \
            std::cerr << "[FAILED] " << __FILE_LINE__ << "got SIGABRT running " << #code;                                                      \
        }                                                                                                                                      \
    }                                                                                                                                          \
                                                                                                                                               \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), oldAttrs);                                                                        \
    std::cerr << std::endl;                                                                                                                    \
}


//!  Prints result of unit testing @c code with expected result @c expected
#define $unit_test(code, expected) $unit_test_inside(code, expected, false, 0, " == ")


//! Prints result of unit testing @c code with expected result @c expected. Test is considered passed if the difference between the two values is less than @c eps

#define $f_unit_test(code, expected, eps) $unit_test_inside(code, expected, false, eps, " ~= ")


//! Prints result of unit testing @c code which is expected to raise SIGABRT

#define $unit_test_sigabrt(code) $unit_test_inside(code, 0, true, 0, "")

#endif
