#include "my_vm.h"


#define handle_mmap_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

void* create_dir_entry();

//GLOBALS
/* 
* Simulated Physical address space
* First level page table (inside sim-phys)
*/

int page_dir_created = 0;

char* phys;
int num_phys_pages;
int num_virt_pages;

char* phys_map;
char* dir_map;
pde_t* entries;

int num_offset_bits;
int num_dir_bits;
int num_table_bits;

int num_pd_entries;



/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating 
    unsigned long long length = MEMSIZE;
    phys = mmap(NULL,length, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_NORESERVE, -1, 0); //DOUBLE CHECK THESE PARAMETERS LATER
    
    if (phys == MAP_FAILED){
        handle_mmap_error("mmap");
    }else{
        printf("|-- mmap() successful\n");
        printf("\t|-- Physical Address:  %p \n", phys);
    }
       
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

    num_offset_bits = log2(PGSIZE);
    num_dir_bits = (int) floor((ADDR_BITS - num_offset_bits)/2);
    num_table_bits = ADDR_BITS - (num_dir_bits+num_offset_bits);
    printf("Offset bits: %d\nDirectory bits: %d\nPage Table bits: %d\n", num_offset_bits, num_dir_bits, num_table_bits);

    double higher_bits = (double) (ADDR_BITS - num_offset_bits);
    num_phys_pages = scalbn(1, higher_bits)/PGSIZE; // (1*2^n)/Page size
    printf("Number of physical Pages: %d\n", num_phys_pages);

    phys_map = (char*) malloc(4); // For now allocating 32 bits for bit map, even though physical space is 20 bits
    memset(phys_map,0, 4);

    //munmap(phys, length); //DELETE LATER
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */

    return -1;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */

}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */


    //If translation not successfull
    return NULL; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    return -1;
}

//Gets the starting address of a page based on page_num ("index" for the start of the page's memory block)
void* get_addr(int page_num){
    //Address = phys + (size of page * page_num)
    int shift = PGSIZE * page_num;
    //printf("Shift by %d bytes\n", shift);
    void* ptr = phys + shift;
    //printf("Address of page: %p\n", ptr);

    return ptr;
}

/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
    //Use virtual address bitmap to find the next free page
    //Iteratively check each bit
    //Find the first unset bit
    //Use the position (index) of that bit to pass to get_addr(int page_num) to get the address itself

    int i = 0;
    int value = get_bit_at_index(phys_map, i);
    while(value == 1 && i < num_pages){
        i++;
        value = get_bit_at_index(phys_map, i);
        //printf("Bit: %d at index: %d\n", value, i);
        if(value == 0){
            break;
        }
    }
    //printf("INDEX: %d\n", i);
    return get_addr(i);
}

/*Function that gets the next available page directory entry slot
Returns the index of the next open spot. Each 4 Bytes
*/
int get_next_pde() {
    
    void* address;
    int i = 0;
    int value = get_bit_at_index(dir_map, i);
    if(value == 0){
        printf("OPEN INDEX: %d\n", i);
        //Get the address of that position
        //PD address + bytes    
        address = &entries[i];
        printf("Dir: %p -- Entry: %p\n", entries, address);
        return i;
        //return address;
    }
    
    while(value == 1 && i < sizeof(dir_map)){
        i++;
        value = get_bit_at_index(dir_map, i);
        printf("Bit: %d at index: %d\n", value, i);
        if(value == 0){
            printf("OPEN INDEX: %d\n", i);
            //Get the address of that position
            //PD address + bytes    
            address = &entries[i];
            printf("Dir: %p -- Entry: %p\n", entries, address);
            return i;
           //return address;
        }
    }
    //Directory full
    return -1;
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *a_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

    return NULL;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void a_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
     
    
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void put_value(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */




}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */




}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */

       
}

/*
This function creates second level page table (directory) in the simulated
physical memory; 
*/
void page_table_init(page_table* ptr){
    ptr->bitmap = (char*) malloc(4);
    memset(ptr->bitmap, 0, sizeof(ptr->bitmap));
    //Allocate mem in "physical memory"
}

/*
This function creates the first level page table (directory) in the simulated
physical memory at the very start of the address space
*/
void page_dir_init(){
    dir_map = (char*) malloc(4);
    memset(dir_map, 0, sizeof(dir_map));

    //Allocate mem in "physical memory"
    //Check how many physical pages need to be allocated for the directory
    //  Each entry is 4 bytes (unsigned long)
    //  Number of entries = 2^higher order bits (ex: 2^10  = 1024)
    //  Total Size of table = num entries * 4 bytes (1024 * 4 = 4KB)
    // = 1 Physical Page

    //NOTE: 
    //  NEED TO ADD CHECKS TO SEE IF THE DIR SHOULD SPAN MULTIPLE PAGES (POSSIBLE)
    printf("Size of pde_t: %d Bytes\n", sizeof(pde_t));
    num_pd_entries = scalbn(1, num_dir_bits);
    printf("Number of entries in directory: %d\n", num_pd_entries);
    int dir_size = num_pd_entries * sizeof(pde_t);
    printf("Total Size of directory: %d\n", dir_size);
    int num_pages_needed = dir_size/PGSIZE + ((dir_size % PGSIZE) != 0);
    printf("Number of pages needed to store directory: %d\n", num_pages_needed);


    //Set required bits in bitmap to 1
    //Loop using num_pages_needed
    int i;
    for(i = 0; i < num_pages_needed; i++){
        printf("Setting bit index %d\n", i);
        set_bit_at_index(phys_map, i);
    }
    
    //ptr = (page_dir*) phys;

    // ----- TEST FOR ADDRESS MAPPING -----
    void* addr = phys;
    entries = (pde_t*) addr;
    printf("Phys: %p -- Directory: %p\n", phys, entries);
    // entries[0] = 0xb7d9f800;
    // printf("|-- Directory entry 0: %lx\n", entries[0]);
    // int* val = (int*) entries[0];
    // *val = 2;
    // printf("\t|-- Value at address: %d\n", *val);
    // printf("|-- Directory address of index 0: %p\n", &entries[0]);
    // printf("\t|-- Size of Entry: %d\n", sizeof(entries[0]));
    // printf("|-- Directory address of index 1: %p\n", &entries[1]);
    // printf("\t|-- Size of Entry: %d\n", sizeof(entries[1]));
    // ----- END TEST FOR ADDRESS MAPPING -----

    // ----- TEST FOR get_next_avail() ------
    void* ptr = get_next_avail(num_phys_pages);
    printf("Next open page: %p\n", ptr);
    // ----- END TEST FOR get_next_avail() ------

    // ----- TEST FOR create_dir_entry() ------
    //set_bit_at_index(dir_map, 0);
    create_dir_entry();
    printf("|-- Directory entry 0: %lx\n", entries[0]);
    // ----- END TEST FOR create_dir_entry() ------

}

//When needed, create a new entry (new page table) in the page directory.
//Allocates physical space for the memory and stores the reference to the address as an entry. 
void* create_dir_entry(){
    //Find an available slot in page Directory
    //Find available page address
    //Set the pointer of 'entry' to that address

    int index = get_next_pde();
    if(index == -1){
        printf("--------------\nERROR: Page Directory is Full!\n--------------\n");
    }
    //Allocate Space for Page Table
    //CHECKS TO SEE IF THE DIR SHOULD SPAN MULTIPLE PAGES
    printf("\n--------------------------------------\n");
    printf("Size of pte_t: %d Bytes\n", sizeof(pte_t));
    int num_pt_entries = scalbn(1, num_table_bits);
    printf("Number of entries in Table: %d\n", num_pt_entries);
    int tab_size = num_pt_entries * sizeof(pte_t);
    //int tab_size = 8192;
    printf("Total Size of Table: %d\n", tab_size);
    int num_pages_needed = tab_size/PGSIZE + ((tab_size % PGSIZE) != 0);
    printf("Number of pages needed to store Table: %d\n", num_pages_needed);

    //void* page = get_next_avail(num_phys_pages);
    void* page;

    //Check to see if there are enough contiguous pages when table size exceeds 1 page
    //Set required bits in bitmap to 1
    //Loop using num_pages_needed
    int i = 0;
    int count = 0;
    int bits_for_map = sizeof(phys_map) * 8;
    int start;
    int bit;
    print_bitmap(phys_map);
    //Loop to find the start index of avaiable pages
    while(i < bits_for_map || count < num_pages_needed-1){
        bit = get_bit_at_index(phys_map, i);
        if(bit == 1){
            count = 0;
        }
        if(bit == 0){
            if(count == 0){
                start = i;
            }
            count++;
        }
        if(count >= num_pages_needed){
            break;
        }
        // printf("Setting bit index %d\n", i);
        // set_bit_at_index(phys_map, i);
        i++;
    }
    //Check if there enough pages available (If count > 0)
    if(count != num_pages_needed){
        printf("Not enough pages available!\n");
        printf("\n--------------------------------------\n");
        return NULL;
    }else{
        //printf("Found contiguous pages starting from: %d\n", start);
    }

    //Loop to set the bits from start, until n iterations
    for(i = start; i <= count; i++){
        //printf("Setting bit index %d\n", i);
        set_bit_at_index(phys_map, i);
    }
    print_bitmap(phys_map);

    page = get_addr(start);
    printf("Start Address of Allocated Page(s): %p\n", page);
    entries[index] = (int) page;
    printf("\n--------------------------------------\n");
    return page;
    
}

/*
* This function finds the next available virtual address to be used by a_malloc()
*/
void* create_virt_addr(){

}

// HW3 functions

// Example 1 EXTRACTING OUTER (TOP-ORDER) BITS

static unsigned int get_top_bits(unsigned int value,  int num_bits)
{
    //Assume you would require just the higher order (outer)  bits, 
    //that is first few bits from a number (e.g., virtual address) 
    //So given an  unsigned int value, to extract just the higher order (outer)  “num_bits”
    int num_bits_to_prune = 32 - num_bits; //32 assuming we are using 32-bit address 
    return (value >> num_bits_to_prune);
}


//Example 2 EXTRACTING BITS FROM THE MIDDLE
//Now to extract some bits from the middle from a 32 bit number, 
//assuming you know the number of lower_bits (for example, offset bits in a virtual address)

static unsigned int get_mid_bits (unsigned int value, int num_middle_bits, int num_lower_bits)
{

   //value corresponding to middle order bits we will returning.
   unsigned int mid_bits_value = 0;   
    
   // First you need to remove the lower order bits (e.g. PAGE offset bits). 
   value =    value >> num_lower_bits; 


   // Next, you need to build a mask to prune the outer bits. How do we build a mask?   

   // Step1: First, take a power of 2 for “num_middle_bits”  or simply,  a left shift of number 1.  
   // You could try this in your calculator too.
   unsigned int outer_bits_mask =   (1 << num_middle_bits);  

   // Step 2: Now subtract 1, which would set a total of  “num_middle_bits”  to 1 
   outer_bits_mask = outer_bits_mask-1;

   // Now time to get rid of the outer bits too. Because we have already set all the bits corresponding 
   // to middle order bits to 1, simply perform an AND operation. 
   mid_bits_value =  value &  outer_bits_mask;

  return mid_bits_value;

}

//Function to set a bit at "index"
// bitmap is a region where were store bitmap 
static void set_bit_at_index(char *bitmap, int index)
{
    // We first find the location in the bitmap array where we want to set a bit
    // Because each character can store 8 bits, using the "index", we find which 
    // location in the character array should we set the bit to.
    char *region = ((char *) bitmap) + (index / 8);
    
    // Now, we cannot just write one bit, but we can only write one character. 
    // So, when we set the bit, we should not distrub other bits. 
    // So, we create a mask and OR with existing values
    char bit = 1 << (index % 8);

    // just set the bit to 1. NOTE: If we want to free a bit (*bitmap_region &= ~bit;)
    *region |= bit;
   
    return;
}

static void free_bit_at_index(char *bitmap, int index){
    char *region = ((char *) bitmap) + (index / 8);
    char bit = 1 << (index % 8);
    *region &= ~bit;
}


//Example 3
//Function to get a bit at "index"
static int get_bit_at_index(char *bitmap, int index)
{
    //Same as example 3, get to the location in the character bitmap array
    char *region = ((char *) bitmap) + (index / 8);
    
    //Create a value mask that we are going to check if bit is set or not
    char bit = 1 << (index % 8);
    
    return (int)(*region >> (index % 8)) & 0x1;
}

/*
* Will copy the data from val to simulated physical memory starting from offset
*
* Make sure val is passed as a pointer. Ex: int x = 4; 
*   --> first make a pointer to this variable, then pass it to put_in_phys()
*
* Returns   Pointer to the start location of the copied mem in the physical space
*/
char* put_in_phys(void* val, int offset, int size){
    if(offset+size > MEMSIZE){
        return NULL;
    }
    char* dest = phys+offset;
    //printf("Phys: %p -- Dest: %p\n", phys, dest);
    
    memcpy(dest, val, size);
    
    return dest;
}

void print_bitmap(char* bitmap){
    printf("Bitmap: ");
    int bits_for_map = sizeof(bitmap) * 8;
    int k = 0;
    for(k = 0; k < bits_for_map; k++){
        printf("%d",get_bit_at_index(phys_map,k));
    }
    printf("\n");
}

