#include "windows_unit_tests.h"

jmp_buf env = {};
bool all_tests_passed = true;

void on_sigabrt(int signum) {
    longjmp(env, 1);
}
