#include <stdio.h>
#include <string.h>  /* For strlen             */
#include <mpi.h>     /* For MPI functions, etc */ 

const int MAX_STRING = 100;

int main(void) {
   char     greeting[MAX_STRING];  /* String storing message */
   int        size;               /* Number of processes    */
   int        rank;               /* My process rank        */

   /* Start up MPI */
   MPI_Init(NULL, NULL); 
   /* Get the number of processes */
   MPI_Comm_size(MPI_COMM_WORLD, &size); 
   /* Get my rank among all the processes */
   MPI_Comm_rank(MPI_COMM_WORLD, &rank); 

   /* Create message */
   sprintf(greeting, "Greetings from process %d of %d!",  rank, size);
   /* Send message to process 0 */
   MPI_Send(greeting, strlen(greeting) + 1, MPI_CHAR, (rank + 1) % size, 0, MPI_COMM_WORLD);
   /* Receive comm_smessage from process q */
   MPI_Recv(greeting, MAX_STRING, MPI_CHAR, (rank - 1 + size) % size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   /* Print message from process q */
   printf("%s\n", greeting);

   /* Shut down MPI */
   MPI_Finalize(); 

   return 0;
}  /* main */
