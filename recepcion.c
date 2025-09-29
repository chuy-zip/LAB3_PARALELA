#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int dato;
    MPI_Request req;
    int listo = 0;

    if (rank == 0) {
        dato = 42;
        MPI_Isend(&dato, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &req);
        
        int trabajo = 0;
        while (!listo) {
            trabajo++;  // Solo contar, sin multiplicaciones
            MPI_Test(&req, &listo, MPI_STATUS_IGNORE);
        }
        
        printf("[Proceso 0] Envío completado. ");
        printf("Ciclos de trabajo: %d\n", trabajo);

    } else if (rank == 1) {
        dato = 0;
        MPI_Irecv(&dato, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &req);

        int trabajo = 0;
        while (!listo) {
            trabajo++;  // Solo contar, sin overflow
            MPI_Test(&req, &listo, MPI_STATUS_IGNORE);
        }

        printf("[Proceso 1] Recepción completada.\n");
        printf("  - Dato recibido: %d\n", dato);
        printf("  - Ciclos de trabajo: %d\n", trabajo);
    }

    MPI_Finalize();
    return 0;
}