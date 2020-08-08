//
// Created by erick on 5/5/20.
//

#ifndef HPC_CLUSTER_TOOLS_H
#define HPC_CLUSTER_TOOLS_H

#include <png.h>
#include "constants.h"


uint8_t* get_values(uint8_t* image, size_t* current_pos, uint8_t kernel_size){

    // Calc the amount of steps to move in each direction
    // Multiply by the CHANNELS to take them into account when
    // moving through the image
    uint8_t half_kernel = (kernel_size / 2);
    uint8_t* values = calloc(kernel_size*kernel_size, sizeof(uint8_t));
    size_t v = 0;


    // Define the limits of the kernel
    uint8_t steps_back = half_kernel;
    uint8_t steps_fwd = half_kernel;
    // If the kernel size if even
    if(kernel_size % 2 == 0){
        steps_back = half_kernel - 1;
    }

    // Define the upper limit
    size_t upper_lim_h = HEIGHT*WIDTH*CHANNELS;
    size_t upper_lim_w = WIDTH*CHANNELS;

    // Set limmits as integers since size_t gives sign problems
    int l_h = current_pos[1] - steps_back*upper_lim_w;
    int u_h = current_pos[1] + steps_fwd*upper_lim_w;
    int l_w = current_pos[0] - steps_back*CHANNELS;
    int u_w = current_pos[0] + steps_fwd*CHANNELS;

    // Get the values
    for (int h = l_h; h <= u_h; h += upper_lim_w)
    {

        for (int w = l_w; w <= u_w; w += CHANNELS)
        {

            if(pad == ZERO){
                // If we are in a invalid position for zero padding, break
                if((h < 0) && (h >= upper_lim_h)){
                    break;
                }
                // If we are in a valid position for zero padding, search for the value
                else if((w >= 0) && (w < upper_lim_w)){
                    *(values + v++) =  *(image + h + w);
                }
            }
            // if we have symmetric padding
            else if(pad == SYMMETRIC){
                // Define the new indexes, offsets and steps
                int new_w = w;
                int new_h = h;
                int offset_w = 0;
                int offset_h = 0;
                int step_w = CHANNELS;
                int step_h = upper_lim_w;

                /* Calculate the width */
                // If we are below 0 in w, then increment until we are over it
                // count the number of increments and do them again minus one time
                if(new_w < 0){
                    while(new_w < 0){
                        new_w += step_w;
                        offset_w += step_w;
                    }
                    // Add the offset to get to the reflection
                    new_w += offset_w - step_w;
                }
                
                // If we are over the upperlimit in w, then decrement until we are under it
                // count the number of decrements and do them again minus one time
                else if(new_w >= upper_lim_w){
                    while(new_w >= upper_lim_w){
                        new_w -= step_w;
                        offset_w -= step_w;
                    }
                    // Add the offset to get to the reflection
                    new_w -= offset_w + step_w;
                }

                /* Calculate the height */
                // If we are below 0 in h, then increment until we are over it
                // count the number of increments and do them again minus one time
                if(new_h < 0){
                    while(new_h < 0){
                        new_h += step_h;
                        offset_h += step_h;
                    }
                    // Add the offset to get to the reflection
                    new_h += offset_h - step_h;
                }
                
                // If we are over the upperlimit in h, then decrement until we are under it
                // count the number of decrements and do them again minus one time
                else if(new_h >= upper_lim_h){
                    while(new_h >= upper_lim_h){
                        new_h -= step_h;
                        offset_h -= step_h;
                    }
                    // Add the offset to get to the reflection
                    new_h -= offset_h + step_h;
                }
                // Get the values
                *(values + v++) =  *(image + new_h + new_w);
            }
            
        }

    }

    return values;

}


void print_array(uint8_t* array , int len){

    printf("[ " );

    for (int i = 0; i < len; ++i) {
        printf("%i, ", array[i]);
    }

    printf("]\n" );

}

void merge_sort(uint8_t* array,int lower_limit, int upper_limit, size_t arr_len){

    // Calc the lenght of the array
    int len = upper_limit - lower_limit;

    // If we have just two elements left then sort them and return
    if(len <= 2){
        if(array[lower_limit] > array[upper_limit - 1]){
            uint8_t temp = array[upper_limit - 1];
            array[upper_limit - 1] = array[lower_limit];
            array[lower_limit] = temp;
        }
    }
    // If we have a bigger array then cut it and sort
    else{
        // Calc the middle point in the array using absolute indexes
        // Absolute against array
        int mid_limit = lower_limit + (len / 2);
        uint8_t copy[arr_len];

        // Call merge sort again for each part
        merge_sort(array, lower_limit, mid_limit, arr_len);
        merge_sort(array, mid_limit, upper_limit, arr_len);

        // Copy the almost sorted array
        memcpy(copy, array, arr_len*sizeof(uint8_t));

        // Create indexes for resorting
        int lower_half_index = lower_limit;
        int upper_half_index = mid_limit;
        int array_index = lower_limit;

        // Make sure each half is sorted correctly
        while((lower_half_index < mid_limit) && (upper_half_index < upper_limit)){

            // If the current lower half value is smaller then the current upper half
            if(copy[lower_half_index] < copy[upper_half_index]){
                int a = copy[lower_half_index];
                // Then set the new value to the array and increment the lower half index
                array[array_index] = copy[lower_half_index++];
            }
            // Else add the upper one
            else{
                int a = copy[upper_half_index];
                array[array_index] = copy[upper_half_index++];
            }

            // Increment the array current index
            ++array_index;
        }


        // Check if we have any half unfinished
        while(lower_half_index < mid_limit){
            array[array_index++] = copy[lower_half_index++];
        }
        while(upper_half_index < upper_limit){
            array[array_index++] = copy[upper_half_index++];
        }
    }

}


int* get_jpg_dim(const char* file){

    int* dims = malloc(2*sizeof(int)); //{width, height}
    int iPos, i;

    // Open the file in binary mode
    FILE *fp = fopen(file,"rb");
    // Jump to the end of the file
    fseek(fp,0,SEEK_END);
    // Get the current byte offset in the file
    long len = ftell(fp);
    // Jump back to the beginning of the file
    fseek(fp,0,SEEK_SET);

    // Enough memory for the file
    unsigned char *ucpImageBuffer = (unsigned char*) malloc (len+1);
    // Read in the entire file
    fread(ucpImageBuffer,1,len,fp);
    // Close the file
    fclose(fp);

    /*Extract start of frame marker(FFCO) of width and hight and get the position*/
    for(i=0;i<len;i++)
    {
        if((ucpImageBuffer[i]==0xFF) && (ucpImageBuffer[i+1]==0xC0) )
        {
            iPos=i;
            break;
        }
    }

    /*Moving to the particular byte position and assign byte value to pointer variable*/
    iPos = iPos + 5;
    *(dims + 1) = ucpImageBuffer[iPos]<<8|ucpImageBuffer[iPos+1];
    *dims = ucpImageBuffer[iPos+2]<<8|ucpImageBuffer[iPos+3];

    //printf("\nWxH = %dx%d\n\n", *dims, *(dims + 1));

    free(ucpImageBuffer);

    return dims;
}


int* get_png_dim(const char* file){

    int* dims = malloc(2*sizeof(int)); //{width, height}
    int iPos, i;
    int bit_depth;
    int color_type;
    int interlace_method;
    int compression_method;
    int filter_method;
    int j;
    png_structp	png_ptr;
    png_infop info_ptr;

    // Open the file in binary mode
    FILE *fp = fopen(file,"rb");
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct (png_ptr);
    png_init_io (png_ptr, fp);
    png_read_png (png_ptr, info_ptr, 0, 0);
    png_get_IHDR (png_ptr, info_ptr, dims, dims+1, & bit_depth,
                  & color_type, & interlace_method, & compression_method,
                  & filter_method);

    //printf("\nWxH = %dx%d\n\n", *dims, *(dims + 1));

    return dims;
}

char *get_filename_ext(const char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}



/*  Takes the name of and image and get its dimentions

    originalName:    Name of the image

*/
int* get_img_dims(const char *originalName){

    // Check image dimensions
    char* ext = get_filename_ext(originalName);
    int* dims;  //{width, height}

    //If the extension starts with
    if(strcmp(ext, "png") == 0){
        return get_png_dim(originalName);
    }
    else if((strcmp(ext, "jpg") == 0) || (strcmp(ext, "jpeg") == 0)){
        return get_jpg_dim(originalName);
    }
    else{
        perror("get_img_dims unsupported img extension");
        exit(EXIT_FAILURE);
    }
}



#endif //HPC_CLUSTER_TOOLS_H
