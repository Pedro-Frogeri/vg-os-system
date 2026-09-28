#ifndef PANIC_H
#define PANIC_H

#include <stdint.h>
#include "isr.h"

// Registra o framebuffer RGB de 32 bits validado pelo kernel.
// Retorna 0 se geometria/endereco forem invalidos; nesse caso panic apenas para.
int panic_configurar_video(uint32_t *fb, uint32_t pitch,
                          uint32_t largura, uint32_t altura);

// Nao retorna. regs pode ser NULL quando nao ha um quadro de excecao.
void kernel_panic(const char *message, registers_t *regs)
    __attribute__((noreturn));

#endif
