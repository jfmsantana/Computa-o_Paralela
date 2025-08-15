#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  const int PING_PONG_LIMIT = 100;

  // Initialize the MPI environment
  MPI_Init(NULL, NULL);
  // Find out rank, size
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  //MPI_Request request;

  int ping_pong_count = 0;
  int partner_rank = (world_rank + 1) % world_size;
  if (world_rank == 0)
  {
     MPI_Send(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);
  }
  
  while (ping_pong_count < PING_PONG_LIMIT) {
    MPI_Recv(&ping_pong_count, 1, MPI_INT, (world_rank + world_size - 1) % world_size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Processo %d recebido %d\n", world_rank, ping_pong_count);
    ping_pong_count++;
    MPI_Send(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);


  }
  MPI_Finalize();
}