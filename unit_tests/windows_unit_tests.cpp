#include "windows_unit_tests.h"

jmp_buf env;
bool got_sigabrt;

void on_sigabrt(int signum) {
    longjmp(env, 1);
}
