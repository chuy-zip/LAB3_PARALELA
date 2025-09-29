#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 100000  // mas mediciones

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Diferentes valores pa probar
    int chunk_sizes[] = {100, 500, 1000, 5000, 10000, 50000};
    int num_sizes = 6;

    if (rank == 0) {
        printf("Tamaño chunk\tTiempo total (s)\tChunks totales\n");
        printf("------------------------------------------------\n");
    }

    for (int s = 0; s < num_sizes; s++) {
        int chunk_size = chunk_sizes[s];
        int total_chunks = ARRAY_SIZE / chunk_size;
        if (ARRAY_SIZE % chunk_size != 0) total_chunks++;

        // Sincronizar antes de empezar la medición
        MPI_Barrier(MPI_COMM_WORLD);
        double start_time = MPI_Wtime();

        if (rank == 0) {
            int *big_array = (int *)malloc(ARRAY_SIZE * sizeof(int));
            for (int i = 0; i < ARRAY_SIZE; i++) {
                big_array[i] = i + 1;
            }

            MPI_Request send_request;
            int listo;
            double trabajo = 0.0;

            for (int chunk = 0; chunk < total_chunks; chunk++) {
                int start_index = chunk * chunk_size;
                int elements_in_chunk = (chunk == total_chunks - 1) ? 
                                       (ARRAY_SIZE - start_index) : chunk_size;

                MPI_Isend(&big_array[start_index], elements_in_chunk, MPI_INT, 1, 0, 
                         MPI_COMM_WORLD, &send_request);

                // Trabajar mientras se transfiere
                listo = 0;
                while (!listo) {
                    trabajo += 0.1;  
                    MPI_Test(&send_request, &listo, MPI_STATUS_IGNORE);
                }
            }

            int stop_signal = -1;
            MPI_Send(&stop_signal, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
            free(big_array);

        } else if (rank == 1) {
            int *chunk_buffer = (int *)malloc(chunk_size * sizeof(int));
            MPI_Request recv_request;
            MPI_Status status;
            int stop_received = 0;
            int chunks_procesados = 0;
            double trabajo_util = 0.0;

            MPI_Irecv(chunk_buffer, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &recv_request);

            while (!stop_received) {
                int listo = 0;
                MPI_Test(&recv_request, &listo, &status);

                if (listo) {
                    int elements_received;
                    MPI_Get_count(&status, MPI_INT, &elements_received);

                    // Procesar el chunk 
                    long suma_chunk = 0;
                    for (int i = 0; i < elements_received; i++) {
                        suma_chunk += chunk_buffer[i];
                    }
                    chunks_procesados++;

                    // Preparar siguiente recepción
                    if (chunks_procesados < total_chunks) {
                        MPI_Irecv(chunk_buffer, chunk_size, MPI_INT, 0, 0, 
                                 MPI_COMM_WORLD, &recv_request);
                    }
                }

                // trabjo espera
                trabajo_util += 0.1;

                // Ver stop
                int stop_message_waiting;
                MPI_Iprobe(0, 1, MPI_COMM_WORLD, &stop_message_waiting, MPI_STATUS_IGNORE);
                if (stop_message_waiting) {
                    MPI_Recv(&stop_received, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }

            free(chunk_buffer);
        }

        double end_time = MPI_Wtime();

        if (rank == 0) {
            printf("%d\t\t%.6f\t\t%d\n", 
                   chunk_size, end_time - start_time, total_chunks);
        }

        // Sincronizar antes del siguiente tamaño
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}