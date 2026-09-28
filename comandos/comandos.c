#include "comandos.h"
#include "../strutil.h"
#include "render.h"
#include "version.h"
#include "gdt.h"

// Escreve exatamente 8 digitos hexadecimais (sem depender de printf/libc).
static void escrever_hex(char *destino, uint32_t valor) {
  const char *digitos = "0123456789ABCDEF";
  for (int i = 0; i < 8; i++)
    destino[i] = digitos[(valor >> (28 - i * 4)) & 0xF];
}

static void mostrar_gdt(uint32_t *fb, uint32_t pitch, int *cursor_y) {
  gdt_estado_t estado;
  gdt_obter_estado(&estado);
  *cursor_y += 24;
  desenhar_string(estado.valido ? "GDT/TSS OK - 6 entradas, limite 47" :
                                  "GDT/TSS FALHA - conferir registradores",
                  50, *cursor_y, estado.valido ? 0x0000FF00 : 0x00FF0000, fb, pitch);
  char registradores[] = "CS=00000000 DS=00000000 TR=00000000";
  escrever_hex(registradores + 3, estado.cs);
  escrever_hex(registradores + 15, estado.ds);
  escrever_hex(registradores + 27, estado.tr);
  *cursor_y += 24;
  desenhar_string(registradores, 50, *cursor_y, 0x00FFFFFF, fb, pitch);
  char pilha[] = "SS0=00000000 ESP0=00000000";
  escrever_hex(pilha + 4, estado.ss0);
  escrever_hex(pilha + 18, estado.esp0);
  *cursor_y += 24;
  desenhar_string(pilha, 50, *cursor_y, 0x00FFFFFF, fb, pitch);
  char acesso[] = "TSS access=00000000 IOMAP=00000000";
  escrever_hex(acesso + 11, estado.tss_access);
  escrever_hex(acesso + 26, estado.iomap_base);
  *cursor_y += 24;
  desenhar_string(acesso, 50, *cursor_y, 0x00FFFFFF, fb, pitch);
}

static int comando_igual(const char *linha, int len, const char *cmd) {
  int cmd_len = minha_strlen(cmd);
  if (len != cmd_len)
    return 0;
  for (int i = 0; i < len; i++)
    if (linha[i] != cmd[i])
      return 0;
  return 1;
}

comando_resultado_t comandos_executar(const char *linha, int len, uint32_t *fb,
                                      uint32_t pitch, uint32_t width,
                                      int *cursor_y) {
  (void)width;
  if (len == 0)
    return COMANDO_OK; // Enter sem digitar nada

  if (comando_igual(linha, len, "help") ||
      comando_igual(linha, len, "helpet")) {
    *cursor_y += 24;
    desenhar_string("------Comandos disponiveis:------", 50, *cursor_y, 0x00FFFFFF, fb,
                    pitch);
    *cursor_y += 24;
    desenhar_string("help    - mostra esta lista", 50, *cursor_y, 0x00AAAAAA,
                    fb, pitch);
    *cursor_y += 24;
    desenhar_string("version - mostra a versao do sistema", 50, *cursor_y,
                    0x00AAAAAA, fb, pitch);
    *cursor_y += 24;
    desenhar_string("gdt     - verifica a GDT e o TSS", 50, *cursor_y,
                    0x00AAAAAA, fb, pitch);
    *cursor_y += 24;
    desenhar_string("panic   - teste fatal; exige reinicio", 50, *cursor_y,
                    0x00AAAAAA, fb, pitch);
    *cursor_y += 24;
    desenhar_string("clear   - limpa o terminal (tambem Ctrl+L)", 50, *cursor_y,
                    0x00AAAAAA, fb, pitch);
    *cursor_y += 24;
    desenhar_string("exit    - sai do sistema / desliga", 50, *cursor_y,
                    0x00AAAAAA, fb, pitch);
    *cursor_y += 24;
    desenhar_string("Ctrl+C  - copia a linha atual", 50, *cursor_y, 0x00AAAAAA,
                    fb, pitch);
    *cursor_y += 24;
    desenhar_string("Ctrl+V  - cola o que foi copiado", 50, *cursor_y,
                    0x00AAAAAA, fb, pitch);
    return COMANDO_OK;
  }

  if (comando_igual(linha, len, "version")) {
    *cursor_y += 24;
    desenhar_string(OS_BANNER, 50, *cursor_y, 0x00FFFFFF, fb, pitch);
    return COMANDO_OK;
  }

  if (comando_igual(linha, len, "gdt")) {
    mostrar_gdt(fb, pitch, cursor_y);
    return COMANDO_OK;
  }

  if (comando_igual(linha, len, "panic")) {
    // UD2 gera a excecao real #UD (vetor 6), passando por IDT e ISR.
    asm volatile("ud2" ::: "memory");
    return COMANDO_OK; // O handler fatal nunca retorna a este ponto.
  }

  if (comando_igual(linha, len, "clear")) {
    return COMANDO_LIMPAR;
  }

  if (comando_igual(linha, len, "exit") || comando_igual(linha, len, "sair")) {
    return COMANDO_EXIT;
  }

  // Entrada nao reconhecida: nao mostra erro, kernel avanca o prompt
  // normalmente
  return COMANDO_OK;
}
