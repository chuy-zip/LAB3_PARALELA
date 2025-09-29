#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int token;
    MPI_Status status;
    
    // Calcular vecinos en el anillo
    int prev = (rank - 1 + size) % size;  // el de antes
    int next = (rank + 1) % size;         // el de despues
    
    if (rank == 0) {
        // Proceso 0 inicia el token
        token = 100;
        printf("* Proceso %d: Iniciando token con valor %d\n", rank, token);
        
        // envair token al siguiente
        MPI_Send(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        printf("  Proceso %d: Envió token %d al proceso %d\n", rank, token, next);
        
        // Espera token que viene del último proceso
        MPI_Recv(&token, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
        printf("* Proceso %d: Recibió token %d completando el anillo\n", rank, token);
    } else {
        // recibe token del anterior
        MPI_Recv(&token, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
        printf("  Proceso %d: Recibió token %d del proceso %d\n", rank, token, prev);
        
        // Procesa el token y lo aumenta
        token += rank;
        usleep(100000); // Simula tiempo
        
        // siguiente
        MPI_Send(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        printf("  Proceso %d: Envió token %d al proceso %d\n", rank, token, next);
    }
    
    MPI_Finalize();
    return 0;
}