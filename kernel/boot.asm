; SentinelOS - Boot entry point
extern kernel_main

MAGIC    equ 0x1BADB002
FLAGS    equ 0x0
CHECKSUM equ -(MAGIC + FLAGS)

section .text
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

global _start
_start:
    call kernel_main
    hlt