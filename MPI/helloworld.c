//
// Prints 'Hello World' from rank 0 and 
// prints what processor it is out of the total number of processors from all ranks
//

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main( int argc, char *argv[]){


 int rank, size;

  MPI_Init (&argc, &argv);	/* starts MPI */
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */
  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, rank, size);
  MPI_Finalize();
  return 0;
}