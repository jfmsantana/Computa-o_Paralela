#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define N 1024  // dimensão da matriz (N x N)

int main(int argc, char* argv[]) {
    int rank, size, k;
    int *matriz = NULL;
    int *submatriz = NULL;
    int *resultado = NULL;
    int elementos_por_proc;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    elementos_por_proc = (N * N) / size; // quantos elementos cada processo recebe

    // processo 0 inicializa matriz e k
    if (rank == 0) {
        matriz = (int*) malloc(N * N * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < N * N; i++) {
            matriz[i] = rand() % 100;  // valores de 0 a 99
        }
        k = rand() % 10 + 1;  // k aleatório entre 1 e 10
        printf("Valor de k = %d\n", k);
    }

    // cada processo aloca sua parte
    submatriz = (int*) malloc(elementos_por_proc * sizeof(int));

    // 1) Broadcast de k
    MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 2) Scatter da matriz
    MPI_Scatter(matriz, elementos_por_proc, MPI_INT,
                submatriz, elementos_por_proc, MPI_INT,
                0, MPI_COMM_WORLD);

    // cada processo multiplica sua parte
    for (int i = 0; i < elementos_por_proc; i++) {
    submatriz[i] = submatriz[i] * k;  
}

    // 3) Gather para juntar resultado
    if (rank == 0) {
        resultado = (int*) malloc(N * N * sizeof(int));
    }
    MPI_Gather(submatriz, elementos_por_proc, MPI_INT,
               resultado, elementos_por_proc, MPI_INT,
               0, MPI_COMM_WORLD);

    // processo 0 pode imprimir parte da matriz
    if (rank == 0) {
        printf("Primeiros 10 elementos da matriz resultante:\n");
        for (int i = 0; i < 10; i++) {
            printf("%d ", resultado[i]);
        }
        printf("\n");
    }

    free(submatriz);
    if (rank == 0) {
        free(matriz);
        free(resultado);
    }

    MPI_Finalize();
    return 0;
}
