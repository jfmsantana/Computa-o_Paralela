#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int rank, size;
    int N = 16;  // tamanho do vetor total
    int *vetor = NULL;
    int *subvetor;
    int local_max, global_max;
    int i, n_local;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Cada processo vai receber N/size elementos
    if (N % size != 0) {
        if (rank == 0) {
            printf("O tamanho do vetor deve ser múltiplo do número de processos.\n");
        }
        MPI_Finalize();
        return 0;
    }

    n_local = N / size;
    subvetor = (int*) malloc(n_local * sizeof(int));

    if (rank == 0) {
        // Gera vetor com números aleatórios
        vetor = (int*) malloc(N * sizeof(int));
        srand(time(NULL));
        printf("Vetor gerado:\n");
        for (i = 0; i < N; i++) {
            vetor[i] = rand() % 100;  // entre 0 e 99
            printf("%d ", vetor[i]);
        }
        printf("\n");
    }

    // Distribui os dados para os processos
    MPI_Scatter(vetor, n_local, MPI_INT,
                subvetor, n_local, MPI_INT,
                0, MPI_COMM_WORLD);

    // Calcula máximo local
    local_max = subvetor[0];
    for (i = 1; i < n_local; i++) {
        if (subvetor[i] > local_max) {
            local_max = subvetor[i];
        }
    }

    // Reduz para achar máximo global
    MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Maior valor encontrado: %d\n", global_max);
        free(vetor);
    }

    free(subvetor);
    MPI_Finalize();
    return 0;
}
