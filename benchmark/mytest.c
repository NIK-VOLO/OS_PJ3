#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "../my_vm.h"

#define SIZE 10
#define ARRAY_SIZE 400

int main(int argc, char **argv){

    // int val = 0b11010111;
    // printf("%s\n", print_arbitrary_bits(&val, 8));

    int test0 = 0; // Passed
    int test1 = 0; // Not passed
    int test2 = 0; // Not passed
    int test3 = 0; // 
    int test4 = 1; // 

    // Allocating three one-page blocks
    if (test0) {
        printf("-------- ALLOCATING a -------- \n\n");
        void *tmp = a_malloc(1);
        unsigned long int a = *(unsigned long int*)tmp;
        printf("-------- ALLOCATING b -------- \n\n");
        tmp = a_malloc(1);
        unsigned long int b = *(unsigned long int*)tmp;
        printf("-------- ALLOCATING c -------- \n\n");
        tmp = a_malloc(1);
        unsigned long int c = *(unsigned long int*)tmp;

        printf("Virtual addresses of the allocations: 0x%lx, 0x%lx, 0x%lx\n", a, b, c);
    }

    // Allocating a large number of one-page blocks
    if (test1) {
        int num_malloc_calls = 1025;
        int i = 0;
        while (i < num_malloc_calls) {
            void *tmp = a_malloc(1);
            i += 1;
        }
    }

    // Putting an array of ints into a one-page block
    if (test2) {
        int arr_size = 2;
        void *tmp = a_malloc(arr_size * sizeof(int));
        

        int val[arr_size];
        for (int i = 0; i < arr_size; i++) {
            val[i] = i + 1;
        }
        unsigned long tmp_va = *(unsigned long*)tmp;
        printf("a_malloc returned VA: %lx\n", *(unsigned long*)tmp);
        printf("Putting value into virtual address.\n");
        put_value((void*)&tmp_va, val, arr_size*sizeof(int));

        printf("Expected: ");
        for (int i = 0; i < arr_size; i++) {
            printf("%d ", val[i]);
        }
        printf("\n");

        int print_arr[arr_size];
        get_value((void*)&tmp_va, print_arr, arr_size*sizeof(int));

        printf("Actual: ");
        for (int i = 0; i < arr_size; i++) {
            printf("%d ", print_arr[i]);
        }
        printf("\n");
    }

    // Putting one int into a one-page block
    if (test3) {
        void *tmp = a_malloc(sizeof(int));
        //unsigned long tmp_va = *(unsigned long*)tmp;
        int val;
        //int new_val = 10;
        get_value((void*)tmp, &val, sizeof(int));
        
        printf("Initial value of memory: %d\n", val); 
        
        val = 10;
        put_value((void*)tmp, &val, sizeof(int));
        get_value((void*)tmp, &val, sizeof(int));
        printf("New value of memory: %d\n", val);
    }

    // Allocate large amount of data
    if (test4) {
       void *tmp = a_malloc(16000);
       void* tmp2 = a_malloc(16000);
    }

    print_TLB_missrate();

    return 0;
}