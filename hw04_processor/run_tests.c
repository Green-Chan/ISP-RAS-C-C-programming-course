#include "processor.h"
#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    int err_line;
    assert(assemble("test1.kasm", "test1.kexe", &err_line) == 0);
    assert(process("test1.kexe") == 0);
    assert(disassemble("test1.kexe", "test1_dis.kasm") == 0);
    assert(assemble("test2.kasm", "test2.kexe", &err_line) == UNKNOWN_CMD_ASM_ERR);
    assert(err_line = 4);
    assert(assemble("test3.kasm", "test3.kexe", &err_line) == NO_HALT_ASM_ERR);
    assert(assemble("test4.kasm", "test4.kexe", &err_line) == 0);
    assert(process("test4.kexe") == 0);
    assert(disassemble("test4.kexe", "test4_dis.kasm") == 0);
    printf("ALL TESTS PASSED\n");
}
