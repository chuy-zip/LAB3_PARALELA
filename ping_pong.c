#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int numero = 5;
    MPI_Status status;
    int N = 3;

    if (argc > 1) {
        N = atoi(argv[1]);
    }

    // Sincronizar antes de empezar
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
        {
            MPI_Send(&numero, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("Proceso 0 envió el número %d, iteración (%d/%d)\n", numero, i + 1, N);
            fflush(stdout); 

            MPI_Recv(&numero, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            printf("Proceso 0 recibió el número %d, iteración (%d/%d)\n", numero, i + 1, N);
            fflush(stdout); 
        }
    }
    else if (rank == 1)
    {
        for (int j = 0; j < N; j++)
        {
            MPI_Recv(&numero, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            printf("Proceso 1 recibió el número %d, iteración (%d/%d)\n", numero, j + 1, N);
            fflush(stdout); 
            // pausa para que se vea la secuencia como "ping pong"
            usleep(100000);
            
            MPI_Send(&numero, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("Proceso 1 envió el número %d, iteración (%d/%d)\n", numero, j + 1, N);
            fflush(stdout); 
        }
    }

    MPI_Finalize();
    return 0;
}