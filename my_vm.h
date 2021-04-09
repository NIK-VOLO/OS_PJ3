#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need
#include <sys/mman.h>
#include <errno.h>


#define PGSIZE 4096

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024

// Represents a page table entry
typedef unsigned long pte_t;

// Represents a page directory entry
typedef unsigned long pde_t;

#define TLB_ENTRIES 512

#define ADDR_BITS 32

//Structure to store information for every tlb entry. 
struct tlb_entry {
    
};

//Structure to represents TLB
struct tlb {
    /*Assume your TLB is a direct mapped TLB with number of entries as TLB_ENTRIES
    * Think about the size of each TLB entry that performs virtual to physical
    * address translation.
    */

};
struct tlb tlb_store;

//Struct for Page table
typedef struct page_table_t{
   char* bitmap;
   pte_t* entries[];
} page_table;

//Struct for directory table
// typedef struct page_directory_t{
//    char* bitmap;
//    pde_t* entries;
// } page_dir;


void set_physical_mem();
pte_t* translate(pde_t *pgdir, void *va);
int page_map(pde_t *pgdir, void *va, void* pa);
bool check_in_tlb(void *va);
void put_in_tlb(void *va, void *pa);
void *a_malloc(unsigned int num_bytes);
void a_free(void *va, int size);
void put_value(void *va, void *val, int size);
void get_value(void *va, void *val, int size);
void mat_mult(void *mat1, void *mat2, int size, void *answer);
void print_TLB_missrate();

void page_table_init();
void page_dir_init();
unsigned long long create_virt_addr();
char* put_in_phys(void* val, int offset, int size);
// Function from HW3
static unsigned int get_top_bits(unsigned int value,  int num_bits);
static unsigned int get_mid_bits (unsigned int value, int num_middle_bits, int num_lower_bits);
static void set_bit_at_index(char *bitmap, int num_entries, int index);
static int get_bit_at_index(char *bitmap, int num_entries, int index);
static void free_bit_at_index(char *bitmap, int num_entries, int index);
void print_bitmap(char* bitmap, int chunk);

#endif
