#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int token;
    int prev = (rank - 1 + size) % size;
    int next = (rank + 1) % size;
    
    if (rank == 0) {
        token = 100;
        printf("Proceso %d: Token Ring iniciado. Token inicial: %d\n", rank, token);
        
        // Usa Sendrecv para enviar y recibir
        int send_token = token;
        MPI_Sendrecv(&send_token, 1, MPI_INT, next, 0,
                    &token, 1, MPI_INT, prev, 0,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("Proceso %d: Recibió token %d del proceso %d (anillo completado)\n", rank, token, prev);
    } else {
        // Los otros procesos reciben y reenvían
        MPI_Recv(&token, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Proceso %d: Recibió token %d del proceso %d\n", rank, token, prev);
        
        token += rank;  // Cada proceso suma su rank al token
        printf("Proceso %d: Modificó token a %d\n", rank, token);
        
        MPI_Send(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
    return 0;
}