#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>


// Definicoes para gerar um numero aleatorio maximo transformado pra int
#define RANDSTART() srand(time(NULL))
#define RANDMAX(x) (int)((float)(x)*rand()/(RAND_MAX+1.0))


#define TAM_BOARD 9
#define ALPHA  (double)1.0 /* Peso da Profundidade */
#define BETA  (double)2.0 /* Peso dos blocos trocados no Board */
#define PROFUNDIDADE_MAXIMA 30



struct s_board;

/* estrutura do tabuleiro do jogo dos 8 */
typedef struct s_board {
    struct s_board *predecessor; //armazena o predecessor do board atual
    double f; // funcao de avaliacao
    double g; // profundidade da funcao de avaliacao
    double h; // numero de blocos fora de ordem na funcao de avaliacao
    char array[TAM_BOARD];
    char vazio;
    int profundidade;
} Board;

void imprimeBoard(Board *board);

/* Alocacao de nodes */

Board *novoNode() {
    Board *p_board;

    p_board = (Board *) malloc(sizeof (Board));
    assert(p_board);

    p_board->predecessor = NULL;
    p_board->f = p_board->g = p_board->h = (double) 0.0;

    return p_board;
}

void freeNode(Board *p_board) {
    assert(p_board);
    free(p_board);
    return;
}



/* Numero maximo de elementos nas listas open e closed */
#define MAX_LIST_ELEM 100000

/* estrutura para armazenar lista de Boards */
typedef struct {
    int numElements;
    Board *elements[MAX_LIST_ELEM];
} List;

/* passar um List* como parametro */
#define countList(x) ((x)->numElements)

List openList;
List closedList;

void iniciaLista(List *p_list) {
    int i;

    assert(p_list);

    p_list->numElements = 0;

    for (i = 0; i < MAX_LIST_ELEM; i++) {
        p_list->elements[i] = NULL;
    }

    return;
}

/* Verifica se um Board (p_board) esta na lista e retorna
 * implicitamente sua posicao na lista na variavel pos*/
int estaNaLista(List *p_list, char *p_board, int *pos) {
    int i, j;

    assert(p_list);
    assert(p_board);

    for (i = 0; i < p_list->numElements; i++) {

        if (p_list->elements[i] != NULL) {

            for (j = 0; j < TAM_BOARD; j++) {

                if (p_list->elements[i]->array[j] != p_board[j]) break;

            }

            if (j == TAM_BOARD) {
                if (pos) {
                    *pos = i;
                }
                return 1;
            }

        }

    }

    return 0;
}

/* Obtem o melhor board e tambem o remove da lista */
Board *getMelhorBoardDaLista(List *p_list) {
    int i;
    int melhor_board = -1;
    double melhor_l = 9999999.0;
    Board *p_board;

    for (i = 0; i < p_list->numElements; i++)
        if (p_list->elements[i])
            //se o valor de f for melhor que melhor_l encontrado ate agora
            if (p_list->elements[i]->f < melhor_l) {
                //o indice do melhor board fica na variavel 'melhor_board'
                melhor_board = i;

                //o valor de f do melhor board encontrado ate agora
                melhor_l = p_list->elements[melhor_board]->f;
            }

    assert(melhor_board != -1);

    p_board = p_list->elements[melhor_board];
    p_list->numElements--;
    p_list->elements[melhor_board] = p_list->elements[p_list->numElements];
    //remove o board da lista
    p_list->elements[p_list->numElements] = NULL;

    return p_board;
}

/* Remove o Board na posicao pos e o retorna */
Board *delBoardDaLista(List *p_list, int pos) {
    Board *board;

    assert(p_list);

    board = p_list->elements[pos];
    p_list->numElements--;
    p_list->elements[pos] = p_list->elements[p_list->numElements];
    p_list->elements[p_list->numElements] = NULL;

    return board;
}

/* Retorna um board p_board da lista de boards*/
Board *getBoardDaLista(List *p_list, char *p_board) {
    int pos, ret;
    Board *board = NULL;

    assert(p_list);
    assert(p_board);

    ret = estaNaLista(p_list, p_board, &pos);

    if (ret) {
        board = p_list->elements[pos];
        p_list->numElements--;
        p_list->elements[pos] = p_list->elements[p_list->numElements];
        p_list->elements[p_list->numElements] = NULL;
    }

    return board;
}

/* Coloca um Board na lista */
void poeNaLista(List *p_list, Board *p_board) {
    assert(p_list);
    assert(p_board);

    if (p_list->numElements >= MAX_LIST_ELEM) {
        printf("Lista de Boards esta cheia (%d)\n", MAX_LIST_ELEM);
        exit(1);
    }

    p_list->elements[p_list->numElements++] = p_board;
}

/* Remove todos os boards da lista */
void limpaLista(List *p_list) {
    int i;

    assert(p_list);

    for (i = 0; i < MAX_LIST_ELEM; i++) {

        if (p_list->elements[i] != (Board *) 0) {
            freeNode(p_list->elements[i]);
            p_list->numElements--;
        }

    }

    return;
}

/* Calcula quantos blocos estao fora de ordem no Board em relacao ao Board ordenado*/
double scoreBoard(Board *p_board) {
    int i;
    const int test[TAM_BOARD - 1] = {1, 2, 3, 4, 5, 6, 7, 8};
    int score = 0;

    for (i = 0; i < TAM_BOARD - 1; i++) {
        score += (p_board->array[i] != test[i]);
    }

    return (double) score;
}

/* Conta o numero de inversoes no array de um board */
int countInversoes(char *array) {
    int invCount = 0;
    int i, j;

	for(i = 0; i < TAM_BOARD - 1; i++) {
        for(j = i + 1; j < TAM_BOARD; j++) {
			if(array[j] && array[i] &&
                array[i] > array[j]) {
				invCount++;
            }
        }
	}

    return (invCount%2 == 0);
}

/* Inicializa um Jogo */
Board *initGame(void) {
    int i, inversoes;
    Board *p_board;
    int input_numero;
    char resposta;

    p_board = novoNode();

    printf("Deseja gerar um jogo aleatorio (s)im ou (n)ao?\n");
    scanf("%c", &resposta);

    if (resposta == 'n') {
        printf("Entre com o estado incial do jogo, use zero (0) para representar o espaco vazio:\n\n");
        for (i = 0; i <= TAM_BOARD - 1; i++) {
            scanf("%d", &input_numero);
            p_board->array[i] = input_numero;
        }

        //p_board->array[TAM_BOARD - 1] = 0;
        inversoes = countInversoes(p_board->array);

        if (!inversoes) {
            printf("Este estado inicial nao possui solucao!\n");
            imprimeBoard(p_board);
            exit(1);
        }

    } else {

        for (i = 0; i < TAM_BOARD - 1; i++) {
            p_board->array[i] = i + 1;
        }

        p_board->array[TAM_BOARD - 1] = 0;

        do {

            /* Randomize the board */
            for (i = 0; i < TAM_BOARD; i++) {

                int x = RANDMAX(TAM_BOARD);
                int y = RANDMAX(TAM_BOARD);
                int temp = p_board->array[x];

                p_board->array[x] = p_board->array[y];
                p_board->array[y] = temp;

            }

            inversoes = countInversoes(p_board->array);

        } while (!inversoes);

        imprimeBoard(p_board);
    }

    int index;

    /* Encontra o espaco vazio e marca seu indice no Board */
    for (index = 0; index < TAM_BOARD; index++) {
        if (p_board->array[index] == 0) {
            p_board->vazio = index;
            break;
        }
    }

    p_board->f = p_board->h = scoreBoard(p_board);

    /* Profundidade eh zero = topo da arvore */
    p_board->profundidade = 0;

    return p_board;
}

/* Imprime um board do jogo */
void imprimeBoard(Board *board) {
    int i;

    assert(board);

    for (i = 0; i < TAM_BOARD; i++) {
        if ((i % 3) == 0) printf("\n");
        if (board->array[i] == 0) printf("  ");
        else printf(" %d", board->array[i]);
    }

    printf("\n");

    return;
}


#define MAX_VETOR 4
//estrutura para armazenar os movimentos possiveis no board
typedef struct {
    int len;
    unsigned int vetor[MAX_VETOR];
} Move;

/* Aqui sao os movimentos possiveis dependendo do indice do vetor.
   Por exemplo no indice 0 temos 2 movimentos possiveis no tabuleiro
 que sao para os indices 1 e 3 do tabuleiro

 0 1 2
 3 4 5
 6 7 8

 */
const Move movimentos[TAM_BOARD] = {
    /* 0 */
    { 2,
        {1, 3}},
    /* 1 */
    { 3,
        {0, 2, 4}},
    /* 2 */
    { 2,
        {1, 5}},
    /* 3 */
    { 3,
        {0, 4, 6}},
    /* 4 */
    { 4,
        {1, 3, 5, 7}},
    /* 5 */
    { 3,
        {2, 4, 8}},
    /* 6 */
    { 2,
        {3, 7}},
    /* 7 */
    { 3,
        {4, 6, 8}},
    /* 8 */
    { 2,
        {5, 7}}
};


/* Retorna um board filho dependendo do board p_board e do indice index */
Board *getBoardFilho(Board *p_board, int index) {
    Board *p_child_board = NULL;
    int indexEspacoVazio;
    int i;

    //indice da onde esta o espaco vazio no board
    indexEspacoVazio = p_board->vazio;

    //verifica se eh possivel fazer o movimento do espaco vazio na lista de movimentos
    if (index < movimentos[indexEspacoVazio].len) {
        int moveFrom;

        /* cria um novo board filho */
        p_child_board = novoNode();

        /* Copia o board do pai para o filho */
        for (i = 0; i < TAM_BOARD; i++) {
            p_child_board->array[i] = p_board->array[i];
        }

        //Copia o indice do espaco vazio do board pai para o filho
        p_child_board->vazio = p_board->vazio;

        //indice do movimento a ser feito e que pode ser feito pelo vetor de movimentos em index.
        moveFrom = movimentos[indexEspacoVazio].vetor[index];

        //move para o espaco vazio o valor no indice moveFrom
        p_child_board->array[ (int) p_child_board->vazio ] = p_child_board->array[ moveFrom ];

        //o espaco vazio agora esta no lugar do valor movido
        p_child_board->array[ moveFrom ] = 0;
        p_child_board->vazio = moveFrom;

    }

    return p_child_board;
}

/* Mostra a solucao do jogo */
void showSolucao(Board *board) {
    Board * revList[MAX_LIST_ELEM];
    int i = 0, j;

    printf("\nSolucao:");

    //Monta a solucao em ordem sequencial
    while (board) {
        revList[i++] = board;
        board = board->predecessor;
    }

    //Imprime a solucao
    for (j = i - 1; j >= 0; j--) {
        imprimeBoard(revList[j]);
    }

    printf("\nNumero de movimentos ate chegar a solucao = %d\n", i - 1);
    //printf("Tamanho da openList = %d\n", openList.numElements);
    //printf("Tamanho da closedList = %d\n", closedList.numElements);
    return;
}


/* Algoritmo a* com as listas openList e closedList sendo variaveis globais*/
void astar(void) {
    Board *p_board_corrente, *p_child_board, *temp;
    int i;

    /* Enquanto houver boards na openList */
    while (countList(&openList)) {

        /* Obtem o melhor board da openList */
        p_board_corrente = getMelhorBoardDaLista(&openList);

        /* coloca o board na lista closedList */
        poeNaLista(&closedList, p_board_corrente);

        /* Temos uma solucao?  */
        if (p_board_corrente->h == (double) 0.0) {

            showSolucao(p_board_corrente);
            return;

        } else {

            // Heuristica para se passar da profundidade maxima (em media 22 para o jogo 3x3)
            if (p_board_corrente->profundidade > PROFUNDIDADE_MAXIMA) continue;

            /* Enumera os estados adjacentes (no maximo 4 possiveis) */
            for (i = 0; i < 4; i++) {

                p_child_board = getBoardFilho(p_board_corrente, i);

                if (p_child_board != NULL) {
                    int pos;

                    /* Se o board filho ja estava na lista closedList entao
                     elimina-o da lista e continua no loop passando para o proximo board filho*/
                    if (estaNaLista(&closedList, p_child_board->array, NULL)) {
                        freeNode(p_child_board);
                        continue;
                    }

                    p_child_board->profundidade = p_board_corrente->profundidade + 1;
                    p_child_board->h = scoreBoard(p_child_board);
                    p_child_board->g = (double) p_child_board->profundidade;
                    p_child_board->f = (p_child_board->g * ALPHA) + (p_child_board->h * BETA);

                    /* O board filho esta na openList? retorna sua posicao na lista em &pos*/
                    if (estaNaLista(&openList, p_child_board->array, &pos)) {

                        //deleta o board da openList na posicao pos e o retorna
                        temp = delBoardDaLista(&openList, pos);

                        /* compara se a profundidade do board removido
                         * eh menor que a do board filho se for coloca-o
                         * novamente na openList */
                        if (temp->g < p_child_board->g) {

                            freeNode(p_child_board);
                            poeNaLista(&openList, temp);
                            continue;

                        }
                        freeNode(temp);
                    }

                    /* Board corrente eh solucao entao coloca ele como
                     predecessor do filho gerado.  */
                    p_child_board->predecessor = p_board_corrente;

					/* Poe o board filho na openList pois este pode ser promissor */
                    poeNaLista(&openList, p_child_board);
                }
            }
        }
    }

    return;
}

int main() {
    Board *board_inicial;
    clock_t begin, end;
    double timeSpent;

    RANDSTART();

    iniciaLista(&openList);
    iniciaLista(&closedList);

    board_inicial = initGame();
    printf("\nJogo inicial:");
    imprimeBoard(board_inicial);
    printf("\nBuscando solucao ...\n");

    poeNaLista(&openList, board_inicial);

    begin = clock();
    astar();
    end = clock();
    timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Duracao: %.3lf\n", timeSpent);

    limpaLista(&openList);
    limpaLista(&closedList);

    return 0;
}
