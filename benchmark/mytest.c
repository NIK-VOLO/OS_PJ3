#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../my_vm.h"

#define SIZE 5
#define ARRAY_SIZE 400

int main(int argc, char **argv){
    //TEST set_physical_mem()
    //set_physical_mem();

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
    a_malloc(1024);

    //----- END TEST FOR a_malloc() -----


    return 0;
}