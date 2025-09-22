#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  const int PING_PONG_LIMIT = 100;

  MPI_Init(NULL, NULL);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_size < 2)
  {
    fprintf(stderr, "World size must be greater than 1 for this example\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int count = 0;
  int partner_rank = (world_rank + 1) % world_size;
  int source_rank = (world_rank + world_size - 1) % world_size;

  if (world_rank == 0)
  {
    count++;
    printf("Processo %d iniciando com %d e enviando para %d\n", world_rank, count, partner_rank);
    MPI_Send(&count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);
  }

  while (count < PING_PONG_LIMIT)
  {
    MPI_Recv(&count, 1, MPI_INT, source_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Processo %d recebeu %d do processo %d\n", world_rank, count, source_rank);

    if (count < PING_PONG_LIMIT)
    {
      count++;
      MPI_Send(&count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);
    }
  }

  if (world_rank == 0)
  {
    printf("Processo %d finalizou com a contagem em %d.\n", world_rank, count);
  }

  MPI_Finalize();
  return 0;
}