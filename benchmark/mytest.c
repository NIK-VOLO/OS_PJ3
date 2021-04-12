#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../my_vm.h"

#define SIZE 5
#define ARRAY_SIZE 400

int main(int argc, char **argv){

    // int val = 0b11010111;
    // printf("%s\n", print_arbitrary_bits(&val, 8));

    int test0 = 0; // Passed
    int test1 = 0; // Not passed
    int test2 = 0; // Not passed
    int test3 = 0; // Passed, but strange behavior
    int test4 = 0; // Passed

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
        int num_malloc_calls = 7;
        int i = 0;
        while (i < num_malloc_calls) {
            void *tmp = a_malloc(1);
            i += 1;
        }
    }

    // Putting an array of ints into a one-page block
    if (test2) {
        int arr_size = 10;
        int *tmp = a_malloc(arr_size * sizeof(int));

        for (int i = 0; i < arr_size; i++) {
            tmp[i] = i + 1;
        }

        for (int i = 0; i < arr_size; i++) {
            printf("Index %d: %d\n", i, arr_size);
        }
    }

    // Putting one int into a one-page block
    if (test3) {
        int *tmp = a_malloc(sizeof(int));
        printf("Initial value of memory: %d\n", *tmp); // Appears to contain junk value
        *tmp = 10;
        printf("New value of memory: %d\n", *tmp);
    }

    // Putting one int into a one-page block, then changing it twice
    if (test4) {
        int *tmp = a_malloc(sizeof(int));
        printf("Initial value of memory: %d\n", *tmp); // Appears to contain junk value
        *tmp = 10;
        printf("New value of memory: %d\n", *tmp);
        *tmp = 100;
        printf("New value of memory: %d\n", *tmp);
        *tmp = 1000;
        printf("New value of memory: %d\n", *tmp);
    }

    return 0;
}