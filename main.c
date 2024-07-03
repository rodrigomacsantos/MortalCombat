#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LIFE 1000
#define MAX_STAMINA 1000

typedef struct Nodo {
    char ataque[10];
    struct Nodo *prox;
} Nodo;

typedef struct Jogador {
    int vida;
    int estamina;
    char historico[100];
    int historico_tamanho;
    struct Nodo *head;
    int multiplicador_jogada;
} Jogador;

typedef struct {
    char nome[20];
    char sequencia[10];
    int dano;
    int custo_estamina;
} Combo;

Combo combos[] = {
    {"Arrozão", "ARROZAO", 500, 500},
    {"Dad Bad", "DADBAD", 400, 400},
    {"Bife Wellington", "STTEACC", 300, 300},
    {"Furacão Thiago", "TATAPAAA", 200, 200}
};

const int NUM_COMBOS = sizeof(combos) / sizeof(Combo);

void inicializaJogador(Jogador *jogador) {
    jogador->vida = MAX_LIFE;
    jogador->estamina = MAX_STAMINA;
    jogador->historico[0] = '\0';
    jogador->historico_tamanho = 0;
    jogador->head = NULL;
    jogador->multiplicador_jogada = 1;
}

int calcularMultiplicadorEstamina(int estamina) {
    if (estamina > 750) return 1;
    if (estamina > 500) return 2;
    if (estamina > 250) return 3;
    return 4;
}

void imprimeStatusJogador(Jogador *jogador, int numero) {
    int multiplicador = calcularMultiplicadorEstamina(jogador->estamina);
    printf("P#%d [%d|%d] (x%d)\nH: %s\nI: ", numero, jogador->vida, jogador->estamina, multiplicador, jogador->historico);
}

void adicionarAoHistorico(Jogador *jogador, char ataque) {
    if (jogador->historico_tamanho < 20) {
        jogador->historico[jogador->historico_tamanho++] = ataque;
        jogador->historico[jogador->historico_tamanho] = '\0';
    } else {
        memmove(jogador->historico, jogador->historico + 1, 19);
        jogador->historico[19] = ataque;
        jogador->historico[20] = '\0';
    }
}

void adicionarListaLigada(Jogador *jogador, const char *ataque) {
    Nodo *novo = (Nodo *)malloc(sizeof(Nodo));
    strcpy(novo->ataque, ataque);
    novo->prox = NULL;

    if (jogador->head == NULL) {
        jogador->head = novo;
    } else {
        Nodo *atual = jogador->head;
        while (atual->prox != NULL) {
            atual = atual->prox;
        }
        atual->prox = novo;
    }
}

int obterDano(char ataque1, char ataque2) {
    static const int tabelaDano[12][12] = {
        {  0,  4,  8, 12, 16,-20,-16, 12,  -8, -4, 0, 20}, 
        { -4,  0,  4,  8, 12, 16,-20,-16, -12, 16, 0, 20}, 
        { -8, -4,  0,  4,  8, 12, 16,-20, -16, 12, 0, 20}, 
        {-12, -8, -4,  0,  4, -8, 12, 16,  20,-16, 0, 20}, 
        {-16,-12, -8, -4,  0,  4, -8, 12,  16, 20, 0, 20}, 
        { 20,-16,-12,  8, -4,  0,  4,  8,  12,-16, 0, 20}, 
        { 16, 20,-16,-12,  8, -4,  0, -8,  -4, -8, 0, 20}, 
        {-12, 16, 20,-16,-12, -8,  8,  0,  16,-20, 0, 20}, 
        {  8, 12, 16,-20,-16,-12,  4,-16,   0, 20, 0, 20}, 
        {  4,-16,-12, 16,-20, 16,  8, 20, -20,  0, 0, 20}, 
        {  0,  0,  0,  0,  0,  0,  0,  0,   0,  0, 0,  0}, 
        {-20,-20,-20,-20,-20,-20,-20,-20, -20,-20, 0,  0}
    };
    static const char ataques[] = "ZPAETRCBOMD ";
    int idx1 = strchr(ataques, ataque1) - ataques;
    int idx2 = strchr(ataques, ataque2) - ataques;
    return tabelaDano[idx1][idx2];
}

void processaAtaque(Jogador *jogador1, Jogador *jogador2, char ataque1, char ataque2) {
    int dano = obterDano(ataque1, ataque2);
    char atq1[2], atq2[2];

    // Processa descanso (espaço)
    if (ataque1 == ' ') {
        jogador1->estamina = (jogador1->estamina + 25 > MAX_STAMINA) ? MAX_STAMINA : jogador1->estamina + 25;
    }
    if (ataque2 == ' ') {
        jogador2->estamina = (jogador2->estamina + 25 > MAX_STAMINA) ? MAX_STAMINA : jogador2->estamina + 25;
    }

    if (dano > 0) {
        jogador2->vida -= dano * jogador2->multiplicador_jogada;
    } else if (dano < 0) {
        jogador1->vida += dano * jogador1->multiplicador_jogada;
    }

    // Processa 'D' imediatamente
    if (ataque1 == 'D') {
        int cura = 10 * jogador1->multiplicador_jogada;
        jogador1->vida = (jogador1->vida + cura > MAX_LIFE) ? MAX_LIFE : jogador1->vida + cura;
        jogador1->estamina = (jogador1->estamina + 15 > MAX_STAMINA) ? MAX_STAMINA : jogador1->estamina + 15;
    }
    if (ataque2 == 'D') {
        int cura = 10 * jogador2->multiplicador_jogada;
        jogador2->vida = (jogador2->vida + cura > MAX_LIFE) ? MAX_LIFE : jogador2->vida + cura;
        jogador2->estamina = (jogador2->estamina + 15 > MAX_STAMINA) ? MAX_STAMINA : jogador2->estamina + 15;
    }

    // Reduz estamina apenas se não for um descanso
    if (ataque1 != ' ') {
        jogador1->estamina = (jogador1->estamina > 25) ? jogador1->estamina - 25 : 0;
    }
    if (ataque2 != ' ') {
        jogador2->estamina = (jogador2->estamina > 25) ? jogador2->estamina - 25 : 0;
    }

    adicionarAoHistorico(jogador1, ataque1);
    adicionarAoHistorico(jogador2, ataque2);

    atq1[0] = ataque1; atq1[1] = '\0';
    atq2[0] = ataque2; atq2[1] = '\0';
    adicionarListaLigada(jogador1, atq1);
    adicionarListaLigada(jogador2, atq2);
}

void limpaListaLigada(Nodo **head) {
    Nodo *atual = *head;
    Nodo *prox;

    while (atual != NULL) {
        prox = atual->prox;
        free(atual);
        atual = prox;
    }

    *head = NULL;
}

void imprimeListaLigadaFormatada(Nodo *nodo1, Nodo *nodo2) {
    while (nodo1 || nodo2) {
        printf("[%s,%s]", nodo1 ? nodo1->ataque : " ", nodo2 ? nodo2->ataque : " ");
        nodo1 = nodo1 ? nodo1->prox : NULL;
        nodo2 = nodo2 ? nodo2->prox : NULL;
    }
}

void processaJogada(Jogador *jogador1, Jogador *jogador2, const char *jogada1, const char *jogada2) {
    int len1 = strlen(jogada1);
    int len2 = strlen(jogada2);
    int max_len = len1 > len2 ? len1 : len2;
    int i;

    jogador1->multiplicador_jogada = calcularMultiplicadorEstamina(jogador1->estamina);
    jogador2->multiplicador_jogada = calcularMultiplicadorEstamina(jogador2->estamina);

    limpaListaLigada(&(jogador1->head));
    limpaListaLigada(&(jogador2->head));

    for (i = 0; i < max_len; i++) {
        char ataque1 = i < len1 ? jogada1[i] : ' ';
        char ataque2 = i < len2 ? jogada2[i] : ' ';
        if (ataque1 == ' ' && ataque2 == ' ') break;
        processaAtaque(jogador1, jogador2, ataque1, ataque2);
    }

    imprimeListaLigadaFormatada(jogador1->head, jogador2->head);
    printf("\n");

    limpaListaLigada(&(jogador1->head));
    limpaListaLigada(&(jogador2->head));
}

void processaCombo(Jogador *jogador1, Jogador *jogador2, const char *combo) {
    for (int i = 0; i < NUM_COMBOS; i++) {
        if (strcmp(combos[i].sequencia, combo) == 0) {
            if (jogador1->estamina >= combos[i].custo_estamina) {
                jogador2->vida -= combos[i].dano;
                jogador1->estamina -= combos[i].custo_estamina;
                limpaListaLigada(&(jogador1->head));
                adicionarListaLigada(jogador1, combos[i].sequencia);

                // Adiciona o combo ao histórico
                for (int j = 0; combo[j] != '\0' && jogador1->historico_tamanho < 20; j++) {
                    adicionarAoHistorico(jogador1, combo[j]);
                }

                break;
            }
        }
    }
}

int jogadaValida(const char *jogada) {
    const char *validos = "ZPAETRCOBMDS ";  // Adicionado espaço como caractere válido
    int len = strlen(jogada);
    int i;

    if (len > 4) return 0;
    for (i = 0; i < len; i++) {
        if (!strchr(validos, jogada[i])) return 0;
    }
    return 1;
}

int comboValido(const char *combo) {
    for (int i = 0; i < NUM_COMBOS; i++) {
        if (strcmp(combos[i].sequencia, combo) == 0) {
            return 1;
        }
    }
    return 0;
}

void processaComboEDescanso(Jogador *jogadorCombo, Jogador *jogadorDescanso, const char *combo) {
    processaCombo(jogadorCombo, jogadorDescanso, combo);

    // Aplica o descanso para o jogador que não usou o combo
    jogadorDescanso->estamina = (jogadorDescanso->estamina + 25 > MAX_STAMINA) ? MAX_STAMINA : jogadorDescanso->estamina + 25;

    // Adiciona o descanso à lista ligada e ao histórico do jogador que não usou o combo
    adicionarListaLigada(jogadorDescanso, " ");
    adicionarAoHistorico(jogadorDescanso, ' ');
}

int main() {
    Jogador jogador1, jogador2;
    char jogada1[50], jogada2[50];

    inicializaJogador(&jogador1);
    inicializaJogador(&jogador2);

    while (jogador1.vida > 0 && jogador2.vida > 0) {
        imprimeStatusJogador(&jogador1, 1);
        scanf("%s", jogada1);
        if (!jogadaValida(jogada1) && !comboValido(jogada1)) {
            printf("Entrada invalida");
            return 0;
        }

        imprimeStatusJogador(&jogador2, 2);
        scanf("%s", jogada2);
        if (!jogadaValida(jogada2) && !comboValido(jogada2)) {
            printf("Entrada invalida");
            return 0;
        }

        limpaListaLigada(&(jogador1.head));
        limpaListaLigada(&(jogador2.head));

        if (comboValido(jogada1) && !comboValido(jogada2)) {
            processaComboEDescanso(&jogador1, &jogador2, jogada1);
        } else if (comboValido(jogada2) && !comboValido(jogada1)) {
            processaComboEDescanso(&jogador2, &jogador1, jogada2);
        } else if (comboValido(jogada1) && comboValido(jogada2)) {
            processaCombo(&jogador1, &jogador2, jogada1);
            processaCombo(&jogador2, &jogador1, jogada2);
        } else {
            processaJogada(&jogador1, &jogador2, jogada1, jogada2);
        }

        imprimeListaLigadaFormatada(jogador1.head, jogador2.head);
        printf("\n");
    }
  if (jogador1.vida <= 0 && jogador2.vida <= 0) {
          printf("Empate!\n");
      } else if (jogador1.vida <= 0) {
          printf("Jogador 2 venceu o jogo!\n");
      } else {
          printf("Jogador 1 venceu o jogo!\n");
      }

      limpaListaLigada(&(jogador1.head));
      limpaListaLigada(&(jogador2.head));

      return 0;
  }
