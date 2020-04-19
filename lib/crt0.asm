section .text

[EXTERN main]
[EXTERN pre_main]

[EXTERN micro_init]
[EXTERN micro_fini]

[GLOBAL _start]
_start:
    pop eax

    call micro_init

    push main
    call pre_main

    call micro_fini
.end:
