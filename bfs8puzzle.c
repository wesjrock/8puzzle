#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define DEFAULT_SIZE 3

typedef struct {
	int size;       /* 3x3, 4x4, ... */
	int lineSize;   /* 3, 4, ...     */
	int zeroPos;    /* posicao do 0 no vetor */
	int *element;   /* vetor com com elementos */
} Board;

typedef struct block {
    Board *b;
    struct block *next;
} Node;

typedef struct {
    Node *first;
    Node *last;
} Queue;

Queue *newQueue() {
    Queue *q = (Queue*) malloc(sizeof(Queue));
    if(q == NULL) {
        printf("Falha ao alocar memoria.");
    }

    q->first = NULL;
    q->last = NULL;

    return q;
}

int isEmptyQueue(Queue *q) {
    if(q->first == NULL) {
        return 1;
    }

    return 0;
}

void pushQueue(Queue *q, Board *b) {
    Node *n;

    n = (Node*) malloc(sizeof(Node));
    if(n == NULL) {
        printf("Falha ao alocar memoria.");
    }

    n->b = b;
    n->next = NULL;

    if(q->first == NULL) {
        q->first = n;
    } else {
        q->last->next = n;
    }

    q->last = n;
}

Board *popQueue(Queue *q) {
    Node *n;

    if(isEmptyQueue(q)) {
        printf("tentativa de pop em lista vazia\n");
        exit(1);
    }

    Board *b = q->first->b;
    n = q->first;
    q->first = q->first->next;

    if(q->first == NULL) {    /* 1 element */
        q->last = NULL;
    }

    free(n);

    return b;
}

Board *newBoard(int n) {
    Board *b = (Board*) malloc(sizeof(Board));

    b->lineSize = n;
    b->size = n*n;
    b->element = (int*) malloc(b->size * sizeof(int));

    return b;
}

void freeBoard(Board *b) {
    free(b->element);
    free(b);
}

/* embaralha um tabuleiro b em n vezes */
void shuffleBoard(Board *b, int n) {
    int i;
    int side;

    for(i = 0; i < n; i++) {
        side = rand() % 4;

        switch(side) {
            case 0:
                up(b);
                break;
            case 1:
                down(b);
                break;
            case 2:
                left(b);
                break;
            case 3:
                right(b);
                break;
        }
    }
}

/* retorna o estado final */
Board *solution(int n) {
    int i;
    Board *s = newBoard(n);

    for(i = 0; i < s->size; i++) {
        s->element[i] = i + 1;
    }

    s->element[i-1] = 0;
    s->zeroPos = i - 1;

    return s;
}

/* verifica se dois tabuleiros sao iguais */
int isBoardEqual(Board *a, Board *b) {
    int i;

    for(i = 0; i < a->size; i++) {
        if(a->element[i] != b->element[i]) {
            return 0;
        }
    }

    return 1;
}

/* faz o movimento no tabuleiro, retorna se for valido */
int up(Board *b) {
    int z = b->zeroPos;
    int l = b->lineSize;

    /* valida o movimento */
    if(z < l) {
        return 0;
    }

    int tmp = b->element[z];    /* tmp = 0 sempre */
    b->element[z] = b->element[z - l];
    b->element[z - l] = tmp;

    b->zeroPos = z - l;

    return 1;
}

int down(Board *b) {
    int z = b->zeroPos;
    int l = b->lineSize;

    /* valida o movimento */
    if(z >= b->size - l) {
        return 0;
    }

    int tmp = b->element[z];    /* tmp = 0 sempre */
    b->element[z] = b->element[z + l];
    b->element[z + l] = tmp;

    b->zeroPos = z + l;

    return 1;
}

int left(Board *b) {
    int z = b->zeroPos;
    int l = b->lineSize;

    /* valida o movimento */
    if(z % l == 0) {
        return 0;
    }

    int tmp = b->element[z];    /* tmp = 0 sempre */
    b->element[z] = b->element[z - 1];
    b->element[z - 1] = tmp;

    b->zeroPos = z - 1;

    return 1;
}

int right(Board *b) {
    int z = b->zeroPos;
    int l = b->lineSize;

    /* valida o movimento */
    if((z + 1) % l == 0) {
        return 0;
    }

    int tmp = b->element[z];    /* tmp = 0 sempre */
    b->element[z] = b->element[z + 1];
    b->element[z + 1] = tmp;

    b->zeroPos = z + 1;

    return 1;
}

Board *copyBoard(Board *b) {
    int i;
    Board *b2 = newBoard(b->lineSize);

    for(i = 0; i < b->size; i++) {
        b2->element[i] = b->element[i];
    }

    b2->zeroPos = b->zeroPos;

    return b2;
}

/* evita processar repeticoes, se eh um tabuleiro novo retorna false
   se o tabuleiro ja foi processado retorna true */
int isBoardProcessed(Queue *q, Board *b) {
    Node *tmp;

    for(tmp = q->first; tmp != NULL; tmp = tmp->next) {
        if(isBoardEqual(b, tmp->b)) {
            return 1;
        }
    }

    return 0;
}

/* estado inicial e final, retorna o numero de movimentos */
int solve(Board *ini, Board *fin) {
    Queue *q = newQueue();   /* fila dos tabuleiros a serem processados */
    Queue *processed = newQueue();  /* lista dos tabuleiros processados */

    Board *b;
    Board *copyUp, *copyDown,
        *copyLeft, *copyRight;

    int move = 0;   /* conta quantos movimentos sao feitos */
    int pushCount;  /* auxilia na contagem de movimentos */
    int breadth;    /* largura do nivel do grafo atual */

    pushQueue(q, ini);
    pushCount = 0;
    breadth = 1;

    while(!isEmptyQueue(q)) {
        b = popQueue(q);

        /* verifica se eh a solucao */
        if(isBoardEqual(b, fin)) {
            return move;
        }

        pushQueue(processed, b);

        if(breadth > 0) {
            breadth--;
        }

        copyUp = copyBoard(b);
        copyDown = copyBoard(b);
        copyLeft = copyBoard(b);
        copyRight = copyBoard(b);

        if(up(copyUp) && !isBoardProcessed(processed, copyUp)) {
            pushQueue(q, copyUp);
            pushCount++;
        } else {
            freeBoard(copyUp);
        }

        if(down(copyDown) && !isBoardProcessed(processed, copyDown)) {
            pushQueue(q, copyDown);
            pushCount++;
        } else {
            freeBoard(copyDown);
        }

        if(left(copyLeft) && !isBoardProcessed(processed, copyLeft)) {
            pushQueue(q, copyLeft);
            pushCount++;
        } else {
            freeBoard(copyLeft);
        }

        if(right(copyRight) && !isBoardProcessed(processed, copyRight)) {
            pushQueue(q, copyRight);
            pushCount++;
        } else {
            freeBoard(copyRight);
        }

        /* desceu um nivel no grafo, conta +1 movimento */
        if(breadth == 0) {
            breadth = pushCount;    /* nova largura */
            pushCount = 0;          /* reseta numero de arestas */
            move++;                 /* incrementa numero de movimentos */
        }
    }

    return move;
}

void showBoard(Board *b) {
    int i;

    for(i = 0; i < b->size; i++) {
        printf("%d ", b->element[i]);

        if((i+1) % b->lineSize == 0) {
            printf("\n");
        }
    }

    printf("\n");
}

/* faz o usuario dar a entrada do tabuleiro */
Board *inputBoard() {
    printf("\nDigite a entrada:\n");
    printf("(exemplo de um tabuleiro:\n");
    printf("1 2 3 4 5 6 7 8 0\n");
    printf("o resultado eh o tabuleiro abaixo)\n");

    Board *a = solution(3);
    showBoard(a);
    freeBoard(a);
    printf("\n");

    int n;
    //scanf("%d", &n);
    n = DEFAULT_SIZE;

    Board *b = newBoard(n);

    int i;
    for(i = 0; i < b->size; i++) {
        scanf("%d", &b->element[i]);

        if(b->element[i] == 0) {
            b->zeroPos = i;
        }
    }

    return b;
}

/* verifica se o tabuleiro b pode ser resolvido para
   chegar no estado final padrao 1 2 3 4 5 6 7 8 0 */
int isSolvable(Board *b) {
    int invCount = 0;
    int i, j;

	for(i = 0; i < b->size - 1; i++) {
        for(j = i + 1; j < b->size; j++) {
			if(b->element[j] && b->element[i] &&
                b->element[i] > b->element[j]) {
				invCount++;
            }
        }
	}

    return (invCount%2 == 0);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));       /* semente para gerar numero aleatorio */
    clock_t begin, end;
    double timeSpent;
    char ans;

    Board *b;
    Board *s = solution(3);

    printf("Usar um tabuleiro aleatorio? (s/n)\n");
    scanf("%c", &ans);

    if(ans == 's') {
        b = solution(3);
        shuffleBoard(b, 70);
    } else {
        b = inputBoard();
    }

    printf("\nEstado inicial:\n");
    showBoard(b);

    printf("Estado final:\n");
    showBoard(s);

    if(!isSolvable(b)) {
        printf("Nao ha solucao para esse tabuleiro\n");
        exit(1);
    }

    begin = clock();
    int move = solve(b, s);
    end = clock();
    timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Numero de movimentos para a solucao: %d\n", move);
    printf("Duracao: %.3lf\n", timeSpent);

    return 0;
}
