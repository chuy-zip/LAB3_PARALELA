#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Diferentes tamaños de mensaje a probar
    int sizes[] = {1, 10, 100, 1000, 10000, 100000};
    int num_sizes = 6;

    if (rank == 1) {
        printf("Tamaño mensaje (bytes)\tTiempo (s)\tIteraciones\tSuma acumulada\n");
        printf("------------------------------------------------------------------------\n");
    }

    for (int s = 0; s < num_sizes; s++) {
        int message_size = sizes[s];
        int *dato = (int*)malloc(message_size * sizeof(int));
        MPI_Request req;
        int listo = 0;

        if (rank == 0) {
            // Inicializar datos
            for (int i = 0; i < message_size; i++) {
                dato[i] = i + 42;
            }

            MPI_Isend(dato, message_size, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);
            
            int iteraciones = 0;
            long suma = 0;
            
            while (!listo) {
                suma += iteraciones;  
                iteraciones++;
                MPI_Test(&req, &listo, MPI_STATUS_IGNORE);
            }
            
        } else if (rank == 1) {
            
            for (int i = 0; i < message_size; i++) {
                dato[i] = 0;
            }

            double start_time = MPI_Wtime();
            MPI_Irecv(dato, message_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &req);

            int iteraciones = 0;
            long suma = 0;
            
            while (!listo) {
                suma += iteraciones; 
                iteraciones++;
                MPI_Test(&req, &listo, MPI_STATUS_IGNORE);
            }
            double end_time = MPI_Wtime();

            printf("%d\t\t\t%.6f\t\t%d\t\t%ld\n", 
                   message_size * sizeof(int),
                   end_time - start_time, 
                   iteraciones,
                   suma);
        }

        free(dato);
        MPI_Barrier(MPI_COMM_WORLD); //sincroniizar las pruebas
    }

    MPI_Finalize();
    return 0;
}