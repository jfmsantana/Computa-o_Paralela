#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define N 1024  // dimensão da matriz NxN

int main(int argc, char* argv[]) {
    int rank, size;
    int *A = NULL, *B = NULL, *C = NULL;
    int *subA = NULL, *subC = NULL;
    int linhas_por_proc;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (N % size != 0) {
        if (rank == 0) printf("N deve ser múltiplo do número de processos!\n");
        MPI_Finalize();
        return 0;
    }

    linhas_por_proc = N / size;  // quantas linhas cada processo recebe

    // Processo 0 inicializa A, B e C
    if (rank == 0) {
        A = (int*) malloc(N * N * sizeof(int));
        B = (int*) malloc(N * N * sizeof(int));
        C = (int*) malloc(N * N * sizeof(int));

        srand(time(NULL));
        for (int i = 0; i < N * N; i++) {
            A[i] = rand() % 10;
            B[i] = rand() % 10;
        }

        printf("Matriz A e B inicializadas.\n");
    } else {
        B = (int*) malloc(N * N * sizeof(int)); // todos precisam de B
    }

    // Cada processo aloca sua fatia de linhas de A e C
    subA = (int*) malloc(linhas_por_proc * N * sizeof(int));
    subC = (int*) malloc(linhas_por_proc * N * sizeof(int));

    // 1) Scatter: distribui linhas de A
    MPI_Scatter(A, linhas_por_proc * N, MPI_INT,
                subA, linhas_por_proc * N, MPI_INT,
                0, MPI_COMM_WORLD);

    // 2) Broadcast: envia B inteiro para todos os processos
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    // 3) Cada processo multiplica sua parte
    for (int i = 0; i < linhas_por_proc; i++) {
        for (int j = 0; j < N; j++) {
            subC[i * N + j] = 0;
            for (int k = 0; k < N; k++) {
                subC[i * N + j] += subA[i * N + k] * B[k * N + j];
            }
        }
    }

    // 4) Gather: reúne as linhas de C no processo 0
    MPI_Gather(subC, linhas_por_proc * N, MPI_INT,
               C, linhas_por_proc * N, MPI_INT,
               0, MPI_COMM_WORLD);

    // Processo 0 imprime parte da matriz resultante
    if (rank == 0) {
        printf("Primeiros 10 elementos da matriz C:\n");
        for (int i = 0; i < 10; i++) {
            printf("%d ", C[i]);
        }
        printf("\n");
    }

    // Libera memória
    free(subA);
    free(subC);
    free(B);
    if (rank == 0) {
        free(A);
        free(C);
    }

    MPI_Finalize();
    return 0;
}
