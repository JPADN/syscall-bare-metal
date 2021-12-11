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

#define SYSCALL_TABLE_LEN  4

unsigned long syscall_a(unsigned long x0, unsigned long x1, unsigned long x2, unsigned long x3,
               unsigned long x4, unsigned long x5) {
  uart_puts("I'm Syscall A!\n");
  return 1;
}

unsigned long syscall_b(unsigned long x0, unsigned long x1, unsigned long x2, unsigned long x3,
               unsigned long x4, unsigned long x5) {
    
  uart_puts("\nI'm Syscall B!\nThose were the arguments:\n");
  uart_hex(x0);
  uart_puts("\n");
  uart_hex(x1);
  uart_puts("\n");
  uart_hex(x2);
  uart_puts("\n");
  uart_hex(x3);
  uart_puts("\n");
  uart_hex(x4);
  uart_puts("\n");
  uart_hex(x5);
  uart_puts("\n");
  return x0 + x1 + x2 + x3 + x4 + x5;
}

unsigned long syscall_c(unsigned long x0, unsigned long x1, unsigned long x2, unsigned long x3,
               unsigned long x4, unsigned long x5) {
  uart_puts("I'm Syscall C!\n");
  return 3;
}

unsigned long syscall_d(unsigned long x0, unsigned long x1, unsigned long x2, unsigned long x3,
               unsigned long x4, unsigned long x5) {
  uart_puts("I'm Syscall D!\n");
  return 4;
}

const unsigned long (*syscall_table[SYSCALL_TABLE_LEN])() = {
    syscall_a,
    syscall_b,
    syscall_c,
    syscall_d,
};


unsigned long syscall_handler(unsigned long syscall_num, unsigned long x0, unsigned long x1, unsigned long x2, unsigned long x3,
                              unsigned long x4, unsigned long x5) {  // I know it's ugly passing arguments like this, but in this bare metal 
                                                                     // we can't use an array of arguments and pass a pointer to it as the argument
  
  uart_puts("\n\nSystem call Handler\n");
  
  if (syscall_num > SYSCALL_TABLE_LEN) {
    uart_puts("Couldn't find system call: ");
    uart_hex(syscall_num);
    uart_puts("\n");
    return 0;
  }

  return syscall_table[syscall_num](x0, x1, x2, x3, x4, x5);
}

/**
 * common exception handler
 */
void exc_handler()
{

    // Since we are working with 64 bit registers, we use long C variables (sizeof(long) is 8 bytes)
    unsigned long x0, x1, x2, x3, x4, x5, x8, ret, \
                  type, esr, elr, spsr, far, current_el;  // System Registers for EL1
    ret = 0;
 
    // Reading system call registers
    // Those register should be read inside the system call handler, but we lose their original values if we read them there
    // since this C function overwrite these registers in some way that we couldn't fix. Thus we are reading them inside de Exception Handler
    // and passing them as arguments to the system call handler
    __asm__ volatile (
      "mov     %0, x0\n\t"  // Reading System Call arguments (x0 to x5)  
      "mov     %1, x1\n\t"   
      "mov     %2, x2\n\t"
      "mov     %3, x3\n\t"    
      "mov     %4, x4\n\t"
      "mov     %5, x5\n\t"
      "mov     %6, x8\n\t"  // Reading System Call number
      : "=r" (x0), "=r" (x1), "=r" (x2), "=r" (x3), "=r" (x4), "=r" (x5), "=r" (x8)
    );

    // Reading EL1 System Registers and CurrentEl
    __asm__ volatile (
      "mov     %0, x6\n\t"
      "mrs     %1, esr_el1\n\t"    // Reading Exception Syndrome Register
      "mrs     %2, elr_el1\n\t"    // Reading Exception Link Register
      "mrs     %3, spsr_el1\n\t"   // Reading Saved Program Status Registers
      "mrs     %4, far_el1\n\t"    // Reading Fault Address Register
      "mrs     x7, CurrentEL\n\t"  // Reading Current Exception Level
      "and     x7, x7, #12\n\t"    // Applying mask 
      "lsr     %5, x7, #2\n\t"

      : "=r" (type), "=r" (esr), "=r" (elr), "=r" (spsr), "=r" (far), "=r" (current_el)
    );

    uart_puts("\nException Handler\n");
    uart_puts("I'm at Exception Level: ");
    uart_hex(current_el);
    uart_puts("\n");
    

    // print out interruption type
    switch(type) {
        case 0: uart_puts("\nSynchronous Exception"); break;
        case 1: uart_puts("\nIRQ"); break;
        case 2: uart_puts("\nFIQ"); break;
        case 3: uart_puts("\nSError"); break;
    }
    uart_puts(": ");
    // decode exception type (some, not all)
    switch(esr>>26) {
        case 0b000000: uart_puts("Unknown"); break;
        case 0b000001: uart_puts("Trapped WFI/WFE"); break;
        case 0b001110: uart_puts("Illegal execution"); break;
        case 0b010101: ret = syscall_handler(x8, x0, x1, x2, x3, x4, x5); break;
        case 0b100000: uart_puts("Instruction abort, lower EL"); break;
        case 0b100001: uart_puts("Instruction abort, same EL"); break;
        case 0b100010: uart_puts("Instruction alignment fault"); break;
        case 0b100100: uart_puts("Data abort, lower EL"); break;
        case 0b100101: uart_puts("Data abort, same EL"); break;
        case 0b100110: uart_puts("Stack alignment fault"); break;
        case 0b101100: uart_puts("Floating point"); break;
        default: uart_puts("Unknown"); break;
    }

    // dump registers
    uart_puts(":\n  ESR_EL1 ");
    uart_hex(esr>>32);
    uart_hex(esr);
    uart_puts(" ELR_EL1 ");
    uart_hex(elr>>32);
    uart_hex(elr);
    uart_puts("\n SPSR_EL1 ");
    uart_hex(spsr>>32);
    uart_hex(spsr);
    uart_puts(" FAR_EL1 ");
    uart_hex(far>>32);
    uart_hex(far);
    uart_puts("\n");

    // The return value of a system call should be written inside the System Call Handler, but similarly to 
    // the system call arguments, the return value would be overwritten by this C code, thus we are storing 
    // the return value in a C variable and writing to x0 (system call return value register) at the very end of this function
    __asm__ volatile (
      "mov     x0, %0\n\t"
      :: "r"(ret)
    );
    return;
}
