#include <time.h>
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

	// Get bandwidth stats
	if(world_size % 2 == 0){
		measure_bandwidth();
	}

	// Define and initialize some variables for excecution
    int* dims;
	clock_t start, end;
    double cpu_time_used;
	uint8_t kernel_size = 3;

	// Start clock 
	start = clock();

    // Only the rank 0 reads the image and distributes it
    if (world_rank == 0) {

		// We asume the first argument is the image name
		dims = get_img_dims(argv[1]); //{width, height}
		// Send the image and recieve our part
		uint8_t* img = distribute_image(argv[1], &dims);

		// Apply the filter to our partition 
		uint8_t* f = apply_filter(img, &median_filter, 
								kernel_size, dims[0], dims[1], 
								CHANNELS);
		
		//write_image("shared/imgs/rank0_img.png", img, dims[0], dims[1], CHANNELS);
		//write_image("shared/imgs/rank0_img_filtered.png", f, dims[0], dims[1], CHANNELS);
		
		// Rebuild the image and write it
		rebuild_img(&f, dims[0]*dims[1]*CHANNELS);
		write_image("shared/imgs/FilteredLenna.png", f, dims[0], dims[1]*world_size, CHANNELS);
		
		// Free the allocated memory
		free(img);
		free(f);

    } 
	// The rest of the threads are just normal workers
	else{

		// Allocate memory to recieve the dimensions
		dims = malloc(sizeof(int) * 2);
		// Recieve the dimensions
		recieve_dims(&dims);
		// Allocate the image memory
		uint8_t* img = malloc(sizeof(uint8_t) * dims[0]*dims[1]*CHANNELS);
		// Recieve the image
		recieve_img(&img, dims[0]*dims[1]*CHANNELS, 0);
		
		// Aplies the filterto the image 
		uint8_t* f = apply_filter(img, &median_filter, 
								kernel_size, dims[0], dims[1], 
								CHANNELS);
		// Send the filtered image 
		MPI_Send(f, dims[0]*dims[1]*CHANNELS, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
		//write_image("shared/imgs/rank1_img.png", img, dims[0], dims[1], CHANNELS);
		/*const char* fp = "shared/imgs/rank";
		const char* sp = "_img_filtered.png";
		char part_name[50];
		char r[10];
		sprintf(r, "%d", world_rank);
		strcpy(part_name, fp);
		strcat(part_name, r);
		strcat(part_name, sp);
		write_image(part_name, f, dims[0], dims[1], CHANNELS);*/
		// Free the allocated memory
		free(dims);
		free(img);
		free(f);
    }

	// Stop clock and measure elapsed time
	end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	// Time execution metrics
	execution_time(cpu_time_used, world_rank);
	

    MPI_Finalize();
}