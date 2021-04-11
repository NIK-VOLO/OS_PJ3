#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../my_vm.h"

#define SIZE 5
#define ARRAY_SIZE 400

int main(int argc, char **argv){
    int arr_size = 400;
    printf("\n ALLOCATING a \n");
    int *a = a_malloc(arr_size);
    int old_a = (int)a;
    printf("\n ALLOCATING b \n");
    void *b = a_malloc(arr_size);
    printf("\n ALLOCATING c \n");
    void *c = a_malloc(arr_size);

    printf("Addresses of the allocations: %x, %x, %x\n", (int)a, (int)b, (int)c); // Segfault here

    // Testing translate():
    // Called
    

    // Testing translate()
    // Nothing here yet

    // Testing page_map()
    // Nothing here yet

    // //Integer Test
    // int x = 4;
    // int* int_ptr = &x;
    // char* dest = put_in_phys((void*) int_ptr, 0, sizeof(x));
    // printf("%d\n", (int)*dest);

    // //String Test
    // char* str = "Hello World!";
    // dest = put_in_phys((void*) str, 10, strlen(str));
    // printf("%s\n", dest);

    //page_dir* ptr;
    //page_dir_init();


    //----- TEST FOR a_malloc() -----
    //a_malloc(16000);

    //----- END TEST FOR a_malloc() -----

    return 0;
}