#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int N = 1000; 
    if (argc > 1) {
        N = atoi(argv[1]);
    }

    // Diferentes tamnio de mensaje 
    int sizes[] = {1, 10, 100, 1000, 10000, 100000};
    int num_sizes = 6;

    if (rank == 0) {
        printf("Tamaño del mensaje (bytes)\tTiempo por viaje (segundos)\n");
        printf("-------------------------------------------------------\n");
    }

    for (int s = 0; s < num_sizes; s++) {
        int size = sizes[s];
        int *buffer = (int*)malloc(size * sizeof(int));
        
        // Inicializar buffer
        for (int i = 0; i < size; i++) {
            buffer[i] = i;
        }

        // Sincronizar antes de medir
        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();

        if (rank == 0) {
            for (int i = 0; i < N; i++) {
                MPI_Send(buffer, size, MPI_INT, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, size, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        } else {
            for (int i = 0; i < N; i++) {
                MPI_Recv(buffer, size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer, size, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
        }

        double end_time = MPI_Wtime();

        if (rank == 0) {
            double total_time = end_time - start_time;
            double time_per_round_trip = total_time / N;
            printf("%d\t\t\t%.8f\n", size * sizeof(int), time_per_round_trip);
        }

        free(buffer);
    }

    MPI_Finalize();
    return 0;
}