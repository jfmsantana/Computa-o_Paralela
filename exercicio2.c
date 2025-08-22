#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_NUMBERS 3000   // cada vetor pode ter até 3000 elementos

// ---------- Funções para os escravos fazerem ----------
double media(int *arr, int n) {
    double soma = 0;
    for (int i = 0; i < n; i++) soma += arr[i];
    return soma / n;
}

void bubble_sort(int *arr, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
}

double desvio_padrao(int *arr, int n) {
    double m = media(arr, n);
    double soma = 0;
    for (int i = 0; i < n; i++) soma += (arr[i] - m) * (arr[i] - m);
    return sqrt(soma / n);
}

void filtro_passabaixa(int *arr, int n) {
    for (int i = 1; i < n; i++) arr[i] = (arr[i - 1] + arr[i]) / 2;
}

typedef struct {
    int op;
    int size;
    int values[MAX_NUMBERS];
} Requisicao;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    srand(time(NULL) + world_rank);

    if (world_rank == 0) {
        // MASTER
        int num_requisicoes = 800 + rand() % 201;  // 800 a 1000 requisições
        Requisicao *reqs = malloc(num_requisicoes * sizeof(Requisicao));

        // gera requisições grandes
        for (int i = 0; i < num_requisicoes; i++) {
            reqs[i].op = 1 + rand() % 4;             // operação 1..4
            reqs[i].size = 2500 + rand() % 501;      // 2500 a 3000 elementos
            for (int j = 0; j < reqs[i].size; j++)
                reqs[i].values[j] = rand() % 10000; // valores aleatórios grandes
        }

        int tarefas_enviadas = 0;

        // Enviar primeira rodada de tarefas (uma para cada slave)
        for (int destino = 1; destino < world_size && tarefas_enviadas < num_requisicoes; destino++) {
            MPI_Send(reqs[tarefas_enviadas].values, reqs[tarefas_enviadas].size, MPI_INT,
                     destino, reqs[tarefas_enviadas].op, MPI_COMM_WORLD);
            tarefas_enviadas++;
        }

        int tarefas_concluidas = 0;
        while (tarefas_concluidas < num_requisicoes) {
            MPI_Status status;
            int recv_values[MAX_NUMBERS];
            MPI_Recv(recv_values, MAX_NUMBERS, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            int recv_count;
            MPI_Get_count(&status, MPI_INT, &recv_count);

            int slave = status.MPI_SOURCE;
            int tag = status.MPI_TAG;

            if (tag == 1 || tag == 4) {
                double result;
                MPI_Recv(&result, 1, MPI_DOUBLE, slave, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (tag == 1)
                    printf("Slave %d concluiu média (n=%d) -> %f\n", slave, recv_count, result);
                else
                    printf("Slave %d concluiu desvio (n=%d) -> %f\n", slave, recv_count, result);
            } else if (tag == 2) {
                printf("Slave %d concluiu filtro (n=%d)\n", slave, recv_count);
            } else if (tag == 3) {
                printf("Slave %d concluiu ordenação (n=%d)\n", slave, recv_count);
            }

            tarefas_concluidas++;

            // Enviar nova tarefa para o slave que terminou, se ainda houver
            if (tarefas_enviadas < num_requisicoes) {
                MPI_Send(reqs[tarefas_enviadas].values, reqs[tarefas_enviadas].size,
                         MPI_INT, slave, reqs[tarefas_enviadas].op, MPI_COMM_WORLD);
                tarefas_enviadas++;
            }
        }

        // Finalizar todos os slaves
        for (int destino = 1; destino < world_size; destino++)
            MPI_Send(NULL, 0, MPI_INT, destino, 0, MPI_COMM_WORLD);

        free(reqs);

    } else {
        // SLAVE
        while (1) {
            int recv_values[MAX_NUMBERS];
            MPI_Status status;
            MPI_Recv(recv_values, MAX_NUMBERS, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == 0) break;  // Finalizar

            int recv_count;
            MPI_Get_count(&status, MPI_INT, &recv_count);

            if (status.MPI_TAG == 1) {
                double m = media(recv_values, recv_count);
                MPI_Send(recv_values, recv_count, MPI_INT, 0, 1, MPI_COMM_WORLD);
                MPI_Send(&m, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
            } else if (status.MPI_TAG == 2) {
                filtro_passabaixa(recv_values, recv_count);
                MPI_Send(recv_values, recv_count, MPI_INT, 0, 2, MPI_COMM_WORLD);
            } else if (status.MPI_TAG == 3) {
                bubble_sort(recv_values, recv_count);
                MPI_Send(recv_values, recv_count, MPI_INT, 0, 3, MPI_COMM_WORLD);
            } else if (status.MPI_TAG == 4) {
                double d = desvio_padrao(recv_values, recv_count);
                MPI_Send(recv_values, recv_count, MPI_INT, 0, 4, MPI_COMM_WORLD);
                MPI_Send(&d, 1, MPI_DOUBLE, 0, 4, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Finalize();
    return 0;
}
