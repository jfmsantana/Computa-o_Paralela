#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define N 1024  

int main(int argc, char* argv[]) {
    int rank, size;
    int *A, *B, *C = NULL;
    int *subA, *subB, *subC = NULL;
    int elementos_por_proc;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    elementos_por_proc = (N * N) / size;

    // processo 0 inicializa A e B
    if (rank == 0) {
        A = (int*) malloc(N * N * sizeof(int));
        B = (int*) malloc(N * N * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < N * N; i++) {
            A[i] = rand() % 10;  // valores de 0 a 9
            B[i] = rand() % 10;
        }
    }

    // cada processo recebe pedaços de A e B
    subA = (int*) malloc(elementos_por_proc * sizeof(int));
    subB = (int*) malloc(elementos_por_proc * sizeof(int));
    subC = (int*) malloc(elementos_por_proc * sizeof(int));

    MPI_Scatter(A, elementos_por_proc, MPI_INT,
                subA, elementos_por_proc, MPI_INT,
                0, MPI_COMM_WORLD);

    MPI_Scatter(B, elementos_por_proc, MPI_INT,
                subB, elementos_por_proc, MPI_INT,
                0, MPI_COMM_WORLD);

    // cada processo faz sua parte: soma A + B
    for (int i = 0; i < elementos_por_proc; i++) {
        subC[i] = subA[i] + subB[i];
    }

    // junta os pedaços da matriz resultante C
    if (rank == 0) {
        C = (int*) malloc(N * N * sizeof(int));
    }
    MPI_Gather(subC, elementos_por_proc, MPI_INT,
               C, elementos_por_proc, MPI_INT,
               0, MPI_COMM_WORLD);

    // processo 0 pode imprimir uma amostra da matriz C
    if (rank == 0) {
        printf("Primeiros 10 elementos da matriz C:\n");
        for (int i = 0; i < 10; i++) {
            printf("%d ", C[i]);
        }
        printf("\n");
    }

    // libera memória
    free(subA);
    free(subB);
    free(subC);
    if (rank == 0) {
        free(A);
        free(B);
        free(C);
    }

    MPI_Finalize();
    return 0;
}
