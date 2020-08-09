#ifndef HPC_CLUSTER_MANAGER_C
#define HPC_CLUSTER_MANAGER_C


#include <mpi.h>
#include <stdint.h>

#include "../ImageProcessing/ImageProcessing.h"
#include "Network.c"


/*  Loads an image from memory and divides it into equal sized parts,
 *  then this parts are distributed along the world and aone part is 
 *  returned to the caller to be proccesed, this function must be 
 *  called by the rank 0 thread since its part isnt sent here
 *
 *  params:
 *      filename:   Path to the image
 *  
 *  return:
 *      uint8_t* piece of the image for the caller process
 */
uint8_t* distribute_image(char* filename, int** img_shape){


    // {width, height}
    int* dims = *img_shape;
    // Get the world size 
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Read the image 
    uint8_t* img = read_image(filename, dims[0], dims[1], CHANNELS, CHANNELS);
    // Calc the size of the partition
    dims[1] = dims[1] / world_size; 
    // Define the partition size 
    int partition_size = dims[1]*dims[0]*CHANNELS;
    
    // Send the image piece to every one
    for (size_t i = 1; i < world_size; ++i)
    {
        // Send the image piece
        MPI_Send(
        /* data         = */ img + i*partition_size, 
        /* count        = */ partition_size, 
        /* datatype     = */ MPI_UNSIGNED_CHAR, 
        /* destination  = */ i, 
        /* tag          = */ IMAGE, 
        /* communicator = */ MPI_COMM_WORLD);
        
        // Send the dimensions of the image
        MPI_Send(
        /* data         = */ dims, 
        /* count        = */ 2, 
        /* datatype     = */ MPI_INT, 
        /* destination  = */ i, 
        /* tag          = */ DIMENSIONS, 
        /* communicator = */ MPI_COMM_WORLD);
    }

    return img;  
}

void recieve_dims(int** img_shape){

    MPI_Recv(
    /* data         = */ *img_shape, 
    /* count        = */ 2, 
    /* datatype     = */ MPI_INT, 
    /* source       = */ 0, 
    /* tag          = */ DIMENSIONS, 
    /* communicator = */ MPI_COMM_WORLD, 
    /* status       = */ MPI_STATUS_IGNORE);

}

void recieve_img(uint8_t** img, int img_len, int source){

    MPI_Recv(
    /* data         = */ *img, 
    /* count        = */ img_len, 
    /* datatype     = */ MPI_UNSIGNED_CHAR, 
    /* source       = */ source, 
    /* tag          = */ IMAGE, 
    /* communicator = */ MPI_COMM_WORLD, 
    /* status       = */ MPI_STATUS_IGNORE);

}

void rebuild_img(uint8_t** img, int partition_size){

    // Get the world size 
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Realloc the current image
    uint8_t* filtered_img = realloc(*img, sizeof(uint8_t) * partition_size*world_size);
    if (filtered_img) {
        *img = filtered_img;
    } else {
        perror("realloc failed in rebuild_img");
        exit(EXIT_FAILURE);
    }

    // Recieve everyone's message
    for (size_t i = 1; i < world_size; ++i)
    {
        // Update filtered img pointer
        filtered_img += partition_size;
        recieve_img(&filtered_img, partition_size, i);
    }
    
}

void execution_time(double rank_time, int rank){

    // Get the world size 
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Only the main rank makes the summary
    if(rank == 0){

        printf("\n******************** MPI Execution time ********************\n");
        printf("Rank %i had an execution time of %e seconds\n", rank, rank_time);
        // For each worker in the world
        for (int i = 1; i < world_size; ++i)
        {   
            // Recieve its time 
            double recieved_time;
            MPI_Recv(
            /* data         = */ &recieved_time, 
            /* count        = */ 1, 
            /* datatype     = */ MPI_DOUBLE, 
            /* source       = */ i, 
            /* tag          = */ TIME, 
            /* communicator = */ MPI_COMM_WORLD, 
            /* status       = */ MPI_STATUS_IGNORE);

            // Print it and add it to the total
            printf("Rank %i had an execution time of %e seconds\n", i, recieved_time);
            rank_time += recieved_time;
        }

        printf("For a total of %e of distributed execution time\n", rank_time);
    }
    else
    {
        // Send the time
        MPI_Send(
        /* data         = */ &rank_time, 
        /* count        = */ 1, 
        /* datatype     = */ MPI_DOUBLE, 
        /* destination  = */ 0, 
        /* tag          = */ TIME, 
        /* communicator = */ MPI_COMM_WORLD);
    }
}


#endif  //  HPC_CLUSTER_MANAGER_C
