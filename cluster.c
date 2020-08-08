#include <stdio.h>
#include <stdlib.h>

#include "ImageProcessing/ImageProcessing.h"
#include "ClusterManagement/Manager.c"


int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Find out rank
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	// Get the world size 
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int* dims;
	uint8_t kernel_size = 3;
    // Only the rank 0 reads the image and distributes it
    if (world_rank == 0) {
		printf("Starting rank0\n");
		// We asume the first argument is the image name
		dims = get_img_dims(argv[1]); //{width, height}
		// Send the image and recieve our part
		uint8_t* img = distribute_image(argv[1], &dims);

		uint8_t* f = apply_filter(img, &median_filter, 
								kernel_size, dims[0], dims[1], 
								CHANNELS);
		
		write_image("rank0_img.png", img, dims[0], dims[1], CHANNELS);
		write_image("rank0_img_filtered.png", f, dims[0], dims[1], CHANNELS);
		
		// Rebuild the image
		rebuild_img(&f, dims[0]*dims[1]*CHANNELS);
		write_image("filtered.png", f, dims[0], dims[1]*world_size, CHANNELS);
		
		free(img);
		free(f);
		
      
    } else if (world_rank == 1) {
		printf("Starting rank1\n");
		dims = malloc(sizeof(int) * 2);
		recieve_dims(&dims);
		printf("Dims (%i,%i)\n", dims[0], dims[1]);
		uint8_t* img = malloc(sizeof(uint8_t) * dims[0]*dims[1]*CHANNELS);
		recieve_img(&img, dims[0]*dims[1]*CHANNELS, 0);
		
		
		uint8_t* f = apply_filter(img, &median_filter, 
								kernel_size, dims[0], dims[1], 
								CHANNELS);

		MPI_Send(f, dims[0]*dims[1]*CHANNELS, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
		write_image("rank1_img.png", img, dims[0], dims[1], CHANNELS);
		write_image("rank1_img_filtered.png", f, dims[0], dims[1], CHANNELS);
		free(dims);
		free(img);
		free(f);
    }
    MPI_Finalize();
}