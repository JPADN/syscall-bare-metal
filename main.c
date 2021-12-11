/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */


#include "uart.h"
#include "mmu.h"

void main()
{
    unsigned long ret;

    // set up serial console
    uart_init();
    
    // set up paging
    mmu_init();

    // System call calling convention:
    // x0 - return
    // x8 - syscall number
    // x0, x1, x2, x3, x4, x5 - syscall arguments

    // Perform a system call here
    __asm__ volatile (
    "str x8, [sp, #-8]!\n\t"  // Saving registers, since they will be used by the Exception Handler and Syscall Handler
    "str x0, [sp, #-8]!\n\t"
    "str x1, [sp, #-8]!\n\t"
    "str x2, [sp, #-8]!\n\t"
    "str x3, [sp, #-8]!\n\t"
    "str x4, [sp, #-8]!\n\t"
    "str x5, [sp, #-8]!\n\t"
    
    "mov x0, #0x1\n\t"   // Passing system call arguments
    "mov x1, #0x2\n\t"
    "mov x2, #0x3\n\t"
    "mov x3, #0x4\n\t"
    "mov x4, #0x5\n\t"
    "mov x5, #0x6\n\t"
    "mov x8, #0x1\n\t"   // Passing system call number
    
    "svc 0\n\t"  

    "mov %0, x0\n\t"  // Reading System Call return value into C variable
    
    "ldr x5, [sp], #8\n\t"  // Restoring saved registers, do not work, illustrative only
    "ldr x4, [sp], #8\n\t"
    "ldr x3, [sp], #8\n\t"
    "ldr x2, [sp], #8\n\t"
    "ldr x1, [sp], #8\n\t"
    "ldr x0, [sp], #8\n\t"
    "ldr x8, [sp], #8\n\t"
    : "=r" (ret)
    );


    uart_puts("\nReturned from System Call!\n\n");
    uart_puts("Return Value: ");
    uart_hex(ret);
    uart_puts("\n");

    // echo everything back
    while(1) {
        uart_send(uart_getc());
    }
}


