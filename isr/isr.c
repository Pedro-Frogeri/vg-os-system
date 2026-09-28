#include "isr.h"
#include "panic.h"
#include "idt.h"
#include "gdt.h"
#include <stddef.h>

_Static_assert(sizeof(registers_t) == 56, "Quadro ISR deve ter 56 bytes");
_Static_assert(offsetof(registers_t, esp_dummy) == 16, "Offset de PUSHA incorreto");
_Static_assert(offsetof(registers_t, eip) == 44, "Offset de EIP incorreto");

// Descricoes textuais das 32 excecoes padrao da CPU x86 (Intel SDM vol. 3,
// cap. 6). Exibidas pela rotina de kernel panic.
static const char *excecoes[32] = {
    "Divisao por Zero",
    "Depuracao (Debug)",
    "Interrupcao Nao Mascaravel (NMI)",
    "Breakpoint",
    "Overflow",
    "Limite de Intervalo Excedido (BOUND)",
    "Opcode Invalido",
    "Dispositivo Nao Disponivel (No Math Coprocessor)",
    "Dupla Falha (Double Fault)",
    "Overrun de Coprocessador (legado)",
    "TSS Invalida",
    "Segmento Nao Presente",
    "Falha de Pilha (Stack-Segment Fault)",
    "Falha de Protecao Geral (General Protection Fault)",
    "Falha de Pagina (Page Fault)",
    "Reservada",
    "Excecao de Ponto Flutuante (x87 FPU)",
    "Verificacao de Alinhamento",
    "Falha de Maquina (Machine Check)",
    "Excecao SIMD de Ponto Flutuante",
    "Excecao de Virtualizacao",
    "Excecao de Protecao de Controle",
    "Reservada",
    "Reservada",
    "Reservada",
    "Reservada",
    "Reservada",
    "Reservada",
    "Injecao de Hypervisor",
    "Excecao de Comunicacao com VMM",
    "Excecao de Seguranca",
    "Reservada"
};

// Handler generico: todos os 32 stubs de excecao (interrupts.s) caem aqui.
//
// Mantem o argumento por valor, conforme o contrato do stub Assembly existente.
// Seu endereco aponta para o quadro salvo, nao para registradores lidos depois.
void isr_handler(registers_t regs) {
    // O stub atual nao limpa DF; garanta a direcao esperada pelas rotinas C.
    // O valor original de EFLAGS permanece salvo em regs.eflags.
    asm volatile("cld" ::: "cc");
    const char *nome = (regs.int_no < 32) ? excecoes[regs.int_no] : "Excecao desconhecida";
    kernel_panic(nome, &regs);
}

// Registra as 32 excecoes da CPU na IDT.
void isr_install(void) {
    void (*isr_stubs[32])(void) = {
        isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
        isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };

    for (int i = 0; i < 32; i++) {
        // Seletor 0x08: segmento de codigo do kernel (definido na GDT).
        // Flags 0x8E: presente, DPL = 0, gate de interrupcao de 32 bits.
        idt_set_gate((uint8_t)i, (uint32_t)isr_stubs[i], SELETOR_CODIGO_KERNEL, 0x8E);
    }
}
