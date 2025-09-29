#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 10000

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double trabajo = 0.0;

    int chunk_size = 1000;
    if (argc > 1)
    {
        chunk_size = atoi(argv[1]);
    }

    int total_chunks = ARRAY_SIZE / chunk_size;
    if (ARRAY_SIZE % chunk_size != 0)
        total_chunks++;

    if (rank == 0)
    {
        printf("Pipeline No Bloqueante - Tamaño chunk: %d, Total chunks: %d\n", chunk_size, total_chunks);
    }

    double start_time = MPI_Wtime();

    if (rank == 0)
    {
        int *big_array = (int *)malloc(ARRAY_SIZE * sizeof(int));
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            big_array[i] = i + 1;
        }
        MPI_Request send_request;
        int listo;
        for (int chunk = 0; chunk < total_chunks; chunk++)
        {
            int start_index = chunk * chunk_size;
            int elements_in_chunk = (chunk == total_chunks - 1) ? (ARRAY_SIZE - start_index) : chunk_size;

            MPI_Isend(&big_array[start_index], elements_in_chunk, MPI_INT, 1, 0, MPI_COMM_WORLD, &send_request);
            // Esperar a que se complete el envío antes de enviar el siguiente chunk
            listo = 0;
            while (!listo)
            {
                trabajo += 0.1;
                MPI_Test(&send_request, &listo, MPI_STATUS_IGNORE);
            }

            printf("Proceso 0: Enviado chunk %d/%d\n", chunk + 1, total_chunks);
        }
        int stop_signal = -1;
        MPI_Send(&stop_signal, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
        printf("Proceso 0: STOP enviado, trabajo realizado al final: %f\n", trabajo);

        free(big_array);
    }
    else if (rank == 1)
    {
        int *chunk_buffer = (int *)malloc(chunk_size * sizeof(int));
        MPI_Request recv_request;
        MPI_Status status;
        int stop_received = 0;
        int chunks_procesados = 0;

        // Iniciar la primera recepción no bloqueante
        MPI_Irecv(chunk_buffer, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &recv_request);

        while (!stop_received)
        {
            int listo = 0;
            MPI_Test(&recv_request, &listo, &status);

            if (listo)
            {
                // Se recibió un chunk
                int elements_received;
                MPI_Get_count(&status, MPI_INT, &elements_received);

                long suma_chunk = 0;
                for (int i = 0; i < elements_received; i++)
                {
                    suma_chunk += chunk_buffer[i];
                }

                chunks_procesados++;
                printf("Proceso 1: Procesado chunk %d, elementos: %d, suma: %ld\n", chunks_procesados, elements_received, suma_chunk);

                // Preparar la siguiente recepción, a menos que sea el final
                if (chunks_procesados < total_chunks)
                {
                    MPI_Irecv(chunk_buffer, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &recv_request);
                }
            }

            // Verificar si hay señal de STOP, usando probe para no bloquear
            int stop_message_waiting;
            MPI_Iprobe(0, 1, MPI_COMM_WORLD, &stop_message_waiting, MPI_STATUS_IGNORE);
            if (stop_message_waiting)
            {
                MPI_Recv(&stop_received, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Proceso 1: STOP recibido. Total chunks procesados: %d\n", chunks_procesados);
            }
        }

        free(chunk_buffer);
    }

    double end_time = MPI_Wtime();

    if (rank == 0)
    {
        printf("Tiempo total: %.6f segundos\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}