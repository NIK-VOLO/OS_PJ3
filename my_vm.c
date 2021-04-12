#include "my_vm.h"


#define handle_mmap_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

void* create_dir_entry();
int sizeof_bitmap(char* bitmap, int num_chunks);
int set_bitmap(char* bitmap, int value, int num_pages_needed, int num_positions);

//GLOBALS
/* 
* Simulated Physical address space
* First level page table (inside sim-phys)
*/

int page_dir_created = 0;
int phys_created = 0;

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

char** table_maps;

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
        if (DEBUG) printf("|-- mmap() successful\n");
        if (DEBUG) printf("\t|-- Physical Address:  %p \n", phys);
    }
       
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

    num_offset_bits = log2(PGSIZE);
    num_dir_bits = (int) floor((ADDR_BITS - num_offset_bits)/2);
    num_table_bits = ADDR_BITS - (num_dir_bits+num_offset_bits);
    if (DEBUG) printf("Offset bits: %d\nDirectory bits: %d\nPage Table bits: %d\n", num_offset_bits, num_dir_bits, num_table_bits);

    double higher_bits = (double) (ADDR_BITS - num_offset_bits);

    num_phys_pages = MEMSIZE/PGSIZE;
    if (DEBUG) printf("Number of physical Pages: %d\n", num_phys_pages);

    //Max size of bitmap: 32 bits
    //Separate num_phys_pages into multiple bitmaps
    //num_phys_pages / 32 = number of Bitmaps required to represent the number of pages
    
    int size_of_bitmap = num_phys_pages/8;
    if (DEBUG) printf("Size of BITMAP: %d Bytes\n", size_of_bitmap);
    if (DEBUG) printf("Number of bitmap sections: %d\n", size_of_bitmap/4);
    //phys_map = (char*) malloc(4); // For now allocating 32 bits for bit map, even though physical space is 20 bits
    phys_map = (char*) malloc(size_of_bitmap);
    memset(phys_map,0, size_of_bitmap);
    set_bit_at_index(phys_map, num_phys_pages,39);

    int num_table_entries = scalbn(1, num_table_bits);
    int num_dir_entries = scalbn(1, num_dir_bits);
    int table_map_bytes = (num_table_entries * num_dir_entries)/8;
    if (DEBUG) printf("Bytes needed for every page table bitmap: %d\n", table_map_bytes);
    table_maps = malloc(table_map_bytes);
    memset(table_maps,0,table_map_bytes);

    phys_created = 1;
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



    // Check TLB
    //TODO: TLB search code

    // TLB miss, decompose virtual address
    unsigned int pde_index = get_top_bits(*(unsigned int*)va, num_dir_bits);
    unsigned int pte_index = get_mid_bits(*(unsigned int*)va, num_table_bits, num_offset_bits);
    unsigned int offset = get_mid_bits(*(unsigned int*)va, num_offset_bits, num_dir_bits + num_table_bits);

    // Go to page table page from page directory
    
    if (&pgdir[pde_index] == NULL) {
        //If translation not successfull
        return NULL; 
    }
    pde_t pt_address = pgdir[pde_index];
    // Go to physical memory from page table
    pte_t* pt = (pte_t*) pt_address;
    
    if (&pt[pte_index] == NULL) {
        //If translation not successfull
        return NULL; 
    }
    // Go to physical address from page table entry
    pte_t phys_mem_loc = pt[pte_index + offset];
    pte_t* phys_mem_with_offset = (pte_t*) phys_mem_loc;
    // Return physical address
    return phys_mem_with_offset;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int page_map(pde_t *pgdir, void *va, void *pa)
{
    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    int top_bits;
    int mid_bits;

    if(entries == NULL){
        if (DEBUG) printf("ERROR: PAGE DIRECTORY NOT INITIALIZED\n");
        return -1;
    }

    if (DEBUG) printf("\nSETTING PAGE TABLE ENTRY\n");
    
    // Convert top bits to an index for Directory
    top_bits = get_top_bits(*(unsigned int*)va, num_dir_bits);
    mid_bits = get_mid_bits(*(unsigned int*)va, num_table_bits, num_offset_bits);

    // Decrement these indexes (because they are incremented when creating virtual address)
    // PET: I don't fully understand this
    top_bits--;
    mid_bits--;

    // Get page table address
    pde_t pt_addr = pgdir[top_bits];

    // Check if directory contains page table at index
    int value_dir = get_bit_at_index(dir_map, num_pd_entries, top_bits);
    if (value_dir) {

        // Page table already exists
        if (DEBUG) printf("Attempting to place PTE in table %d of %d.\n", top_bits+1, num_pd_entries);

        // Get page table and physical page address
        pde_t* page_table = &pt_addr;
        pte_t page_addr = page_table[mid_bits];

        // Check if page table contains entry at index
        int num_table_entries = scalbn(1, num_table_bits);
        int value_table = get_bit_at_index((char*)&table_maps[top_bits*32], num_table_entries, mid_bits);
        if (value_table) {

            // PTE already exists
            if (DEBUG) printf("Physical page mapping already exists in page table. No action.\n");

            return PGMAP_NOACTION;
        }

        // Page table exists but PTE does not        
        // Mapping PTE to physical page
        page_table[mid_bits] = (pte_t) &pa;
        if (DEBUG) printf("Mapping Physical address 0x%lx to PTE %d of %d.\n", (unsigned long int) pa, mid_bits+1, num_table_entries);

        // Setting table bitmap
        char* table_start = (char*)&table_maps[top_bits*num_table_entries/8];
        //printf("%d\n", table_start);
        set_bit_at_index(table_start, 1, mid_bits);

        // Setting physical bitmap
        int bit_to_set = ((char*)pa - phys) / PGSIZE;
        set_bit_at_index(phys_map, num_table_entries, bit_to_set);

        return PGMAP_NEWPTE;
    }

    // Page table does not exist, create a new one
    if (DEBUG) printf("Directory mapping not found. Need to make a new page table.\n");
    void* new_pt = create_dir_entry();
    if (new_pt == NULL) {

        // No space for a new page table
        if (DEBUG) printf("No space for a new page table. No action.\n");

        return PGMAP_NOACTION;
    }

    // Create new PTE to put into first entry of new table
    pte_t* temp = (pte_t*) new_pt;
    
    // Mapping PTE to physical page
    temp[mid_bits] = (pte_t) &pa;

    return PGMAP_NEWTABLEANDPTE;
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
void *get_next_avail(int num_pages, int num_needed) {
    //Use virtual address bitmap to find the next free page
    //Iteratively check each bit
    //Find the first unset bit
    //Use the position (index) of that bit to pass to get_addr(int page_num) to get the address itself

    // int i = 0;
    // int value = get_bit_at_index(phys_map, num_phys_pages, i);
    // while(value == 1 && i < num_pages){
    //     i++;
    //     value = get_bit_at_index(phys_map, num_phys_pages, i);
    //     //printf("Bit: %d at index: %d\n", value, i);
    //     if(value == 0){
    //         return get_addr(i);
    //     }
    // }
    if(num_needed == 0){
        num_needed = 1;
    }
    int index = set_bitmap(phys_map, 1, num_needed, num_pages);
    if(index == -1){
        return NULL;
    }
    if (DEBUG) printf("NEXT AVAILABLE INDEX: %d\n", index);
    return get_addr(index);
}

/*Function that gets the next available page directory entry slot
Returns the index of the next open spot. Each 4 Bytes

parameter int open    0 if you are looking for unallocated (open) page dir slots, 1 if allocated (closed).
*/
int get_next_pde(int open) {
    
    void* address;
    int i = 0;
    int value = get_bit_at_index(dir_map, num_pd_entries, i);
    //printf("Bit: %d at index: %d\n", value, i);
    if(value == open){
        if (DEBUG) printf("(1) OPEN INDEX: %d Value: %d\n", i, value);
        //Get the address of that position
        //PD address + bytes    
        address = &entries[i];
        if (DEBUG) printf("Dir: %p -- Entry: %p\n", entries, address);
        return i;
        //return address;
    }
    
    while(value == !open && i < num_pd_entries){
        i++;
        value = get_bit_at_index(dir_map, num_pd_entries, i);
        //printf("Bit: %d at index: %d\n", value, i);
        if(value == open){
            if (DEBUG) printf("(2) OPEN INDEX: %d Value: %d\n", i, value);
            //Get the address of that position
            //PD address + bytes    
            address = &entries[i];
            if (DEBUG) printf("Dir: %p -- Entry: %p\n", entries, address);
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

    if(DEBUG) printf("\nMALLOC CALL\n");

    int num_pages_needed = num_bytes/PGSIZE + ((num_bytes % PGSIZE) != 0);

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */
    if(phys_created == 0 ){
        if (DEBUG) printf("\nINITIALIZING SIMULATED PHYSICAL MEMORY\n");
        set_physical_mem();
        phys_created = 1;
    }

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

   if(page_dir_created == 0 ){
       if (DEBUG) printf("\nINITIALIZING PAGE DIRECTORY\n");
       page_dir_init();
       page_dir_created = 1;
   }

   

   if (DEBUG) printf("Number of Pages needed: %d\n", num_pages_needed);
   //print_bitmap(phys_map, 0);
   void* free_page = get_next_avail(num_phys_pages, num_pages_needed);
   //print_bitmap(phys_map, 0);
   if(free_page == NULL){
       if (DEBUG) printf("ERROR: No available pages were found. . . \n");
       return NULL;
   }
    if (DEBUG) printf("Next open page: %p\n", free_page);

    unsigned long long virt = create_virt_addr();
    if(virt == 0){
        if (DEBUG) printf("PDE Not found. Double checking . . . \n");
        int t = get_next_pde(0);
        if(t == -1){
            if (DEBUG) printf("--> PDE is full. . .");
            return NULL;
        }else{
            create_dir_entry();
            virt = create_virt_addr();
            if(virt == 0){
                if (DEBUG) printf("\n **** NO PDE FOUND AGAIN --> BUG ****\n");
            }
        }
    }
    if (DEBUG) printf("Generated Virtual Address: %llx\n", virt);
    void* virt_addr = (void*) &virt;
    
    page_map(entries, virt_addr, free_page);
    
    return virt_addr;
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

    int num_pages = size / PGSIZE;
    int num_pages_freed = 0;

    while (num_pages_freed < num_pages) {

        // Clear TLB entries
        //TODO

        // Clear PTE
        //TODO

        // Clear dir entry if page table now empty
        //TODO

        num_pages_freed += 1;
    }
    
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

    int num_pages_needed = (size / PGSIZE) + 1;
    //TODO: Test to see if it's possible to fit this in physical memory

    //set_bitmap(phys_map, 1, num_pages_needed, num_phys_pages);

    // Do the actual memory write
    int num_pages_used = 0;
    while (num_pages_used < num_pages_needed) {
        int num_bytes_remaining = size - (num_pages_used * PGSIZE);
        pte_t* next_phys_addr = get_next_avail(num_phys_pages, num_pages_needed);
        // Make virtual address
        //TODO: make virtual address for start of a new page
        unsigned int temppleasechangeme = 1000000000;
        page_map(entries, &temppleasechangeme, next_phys_addr);
        char* val_as_char = (char*)val;
        unsigned int* int_val = (unsigned int*) val;
        int i = 0;
        if (num_bytes_remaining >= PGSIZE) {
            unsigned int* region_to_write = (unsigned int*) get_mid_bits(*int_val, PGSIZE, num_pages_used * PGSIZE);
            for (i = 0; i < PGSIZE; i++) {
                next_phys_addr[i] = region_to_write[i];
            }
        } else {
            // Need to pad with zeroes
             unsigned int* region_to_write = (unsigned int*) get_mid_bits(*int_val, num_bytes_remaining, num_pages_used * PGSIZE);
            for (i = 0; i < num_bytes_remaining; i++) {
                next_phys_addr[i] = region_to_write[i];
            }
            for (i = num_bytes_remaining; i < PGSIZE; i++) {
                next_phys_addr[i] = 0;
            }
        }
        num_pages_used += 1;
    }
}


// Given number of pages altered, sets the bitmap to value (0 or 1) in each location
int set_bitmap(char* bitmap, int value, int num_pages_needed, int num_positions) {
    // Below code copied from create_dir_entry()

    //Check to see if there are enough contiguous pages when table size exceeds 1 page
    //Set required bits in bitmap to 1
    //Loop using num_pages_needed
    int i = 0;
    int count = 0;
    //TODO: Change this so it's not always 32
    int bits_for_map = num_positions;
    int start;
    int bit;
    // if (DEBUG) printf("Bits for Bitmap: %d\n", bits_for_map);

    // if (DEBUG) printf("Before ");
    // if (DEBUG) print_bitmap(bitmap,0);

    // printf("Directory ");
    // print_bitmap(dir_map);
    //Loop to find the start index of avaiable pages
    while(i < bits_for_map || count < num_pages_needed-1){
        bit = get_bit_at_index(bitmap, num_positions, i);
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
        if (DEBUG) printf("Not enough pages available!\n");
        if (DEBUG) printf("\n--------------------------------------\n");
        return -1;
    }else{
        //printf("Found contiguous pages starting from: %d\n", start);
    }

    //Loop to set the bits from start, until n iterations
    for(i = 0; i < count; i++){
        //printf("Setting bit index %d\n", start+i);
        if (value == 1) {
            set_bit_at_index(bitmap, num_positions, start+i);
        } else {
            free_bit_at_index(bitmap, num_positions, start+i);
        }
    }

    // if (DEBUG) printf("After ");
    // if (DEBUG) print_bitmap(bitmap,0);
    // printf("Result: ");
    // print_bitmap(bitmap, 0);
    return start;
    
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
    

    //Allocate mem in "physical memory"
    //Check how many physical pages need to be allocated for the directory
    //  Each entry is 4 bytes (unsigned long)
    //  Number of entries = 2^higher order bits (ex: 2^10  = 1024)
    //  Total Size of table = num entries * 4 bytes (1024 * 4 = 4KB)
    // = 1 Physical Page

    //NOTE: 
    //  NEED TO ADD CHECKS TO SEE IF THE DIR SHOULD SPAN MULTIPLE PAGES (POSSIBLE)
    if (DEBUG) printf("Size of pde_t: %d Bytes\n", sizeof(pde_t));
    num_pd_entries = scalbn(1, num_dir_bits);
    if (DEBUG) printf("Number of entries in directory: %d\n", num_pd_entries);
    dir_map = (char*) malloc(num_pd_entries/8);
    memset(dir_map, 0, (num_pd_entries/8));
    if (DEBUG) printf("Size of DIRECTORY Bitmap: %d\n", (num_pd_entries/8));
    int dir_size = num_pd_entries * sizeof(pde_t);
    if (DEBUG) printf("Total Size of directory: %d\n", dir_size);
    int num_pages_needed = dir_size/PGSIZE + ((dir_size % PGSIZE) != 0);
    if (DEBUG) printf("Number of pages needed to store directory: %d\n", num_pages_needed);


    //Set required bits in bitmap to 1
    //Loop using num_pages_needed
    int i;
    for(i = 0; i < num_pages_needed; i++){
        if (DEBUG) printf("Setting bit index %d\n", i);
        set_bit_at_index(phys_map, num_phys_pages, i);
    }
    
    //ptr = (page_dir*) phys;

    // ----- TEST FOR ADDRESS MAPPING -----
    void* addr = phys;
    entries = (pde_t*) addr;
    if (DEBUG) printf("Phys: %p -- Directory: %p\n", phys, entries);
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
    // void* ptr = get_next_avail(num_phys_pages);
    // printf("Next open page: %p\n", ptr);
    // ----- END TEST FOR get_next_avail() ------

    // ----- TEST FOR create_dir_entry() ------
    //set_bit_at_index(dir_map, num_pd_entries, 0);
    create_dir_entry();
    // create_dir_entry();
    // create_dir_entry();
    // printf("|-- Directory entry 0: %lx\n", entries[0]);
    // printf("|-- Directory entry 1: %lx\n", entries[1]);
    // printf("|-- Directory entry 2: %lx\n", entries[2]);
    // ----- END TEST FOR create_dir_entry() ------

}

//When needed, create a new entry (new page table) in the page directory.
//Allocates physical space for the memory and stores the reference to the address as an entry. 
void* create_dir_entry(){
    //Find an available slot in page Directory
    //Find available page address
    //Set the pointer of 'entry' to that address

    int index = get_next_pde(0);
    if(index == -1){
        if (DEBUG) printf("--------------\nERROR: Page Directory is Full!\n--------------\n");
    }
    //Allocate Space for Page Table
    //CHECKS TO SEE IF THE DIR SHOULD SPAN MULTIPLE PAGES
    if (DEBUG) printf("\nALLOCATING SPACE FOR PAGE TABLE\n");
    if (DEBUG) printf("Size of pte_t: %d Bytes\n", sizeof(pte_t));
    int num_pt_entries = scalbn(1, num_table_bits);
    if (DEBUG) printf("Number of entries in Table: %d\n", num_pt_entries);
    int tab_size = num_pt_entries * sizeof(pte_t);
    //int tab_size = 8192;
    if (DEBUG) printf("Total Size of Table: %d\n", tab_size);
    int num_pages_needed = tab_size/PGSIZE + ((tab_size % PGSIZE) != 0);
    if (DEBUG) printf("Number of pages needed to store Table: %d\n", num_pages_needed);

    //void* page = get_next_avail(num_phys_pages);
    void* page;

    //Check to see if there are enough contiguous pages when table size exceeds 1 page
    //Set required bits in bitmap to 1
    //Loop using num_pages_needed
    int i = 0;
    int count = 0;
    int bits_for_map = num_phys_pages;
    int start;
    int bit;
    if (DEBUG) printf("Bits for Physical Bitmap: %d\n", bits_for_map);
    if (DEBUG) printf("Before physical ");
    if (DEBUG) print_bitmap(phys_map, 0);

    if (DEBUG) printf("Before directory ");
    if (DEBUG) print_bitmap(dir_map,0);
    //Loop to find the start index of avaiable pages
    while(i < bits_for_map || count < num_pages_needed-1){
        bit = get_bit_at_index(phys_map, num_phys_pages, i);
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
        if (DEBUG) printf("Not enough pages available!\n");
        if (DEBUG) printf("\n--------------------------------------\n");
        return NULL;
    }else{
        //printf("Found contiguous pages starting from: %d\n", start);
    }

    //Loop to set the bits from start, until n iterations
    for(i = 0; i < count; i++){
        if (DEBUG) printf("Setting bit index %d\n", i);
        set_bit_at_index(phys_map, num_phys_pages, start+i);
    }

    //Set bit in the Directory Map 
    if (DEBUG) printf("NEXT PDE INDEX: %d\n", index);
    set_bit_at_index(dir_map, num_pd_entries, index);
    
    if (DEBUG) printf("After physical ");
    if (DEBUG) print_bitmap(phys_map,0);

    
    if (DEBUG) printf("After directory ");
    if (DEBUG) print_bitmap(dir_map,0);

    page = get_addr(start);
    if (DEBUG) printf("Start Address of Allocated Page(s): %p\n", page);
    entries[index] = (int) page;
    //if (DEBUG) printf("END PAGE TABLE ALLOCATION\n");
    return page;
    
}

//This function finds the next available virtual address to be used by a_malloc()
//The indexes are incremented by 1 in order to avoid creating a virtual address = 0
//When decoding the virtual address you must decrement by 1 (for the directory and table bits)
unsigned long long create_virt_addr(){
    //Find available directory slot (Allocated directory slot)
    //  Use bitmap to find if there is an allocated slot->table
    //  Find available page table slot (using array of bitmaps storing bitmaps for each table)
    //      Offset bits will all be 0 because we will always be mapping to the start of a page

    //set_bit_at_index(dir_map, num_pd_entries, 0);
    // print_bitmap(dir_map,0);
    // int pde_slot = get_next_pde(1); //Finds the first index where the dirmap is set to 1
    // printf("Index of next availabe pde: %d\n", pde_slot);
    // if(pde_slot == -1){
    //     return NULL;
    // }
    int num_table_entries = scalbn(1, num_table_bits);
    //void* address;
    int i;
    int k;
    int value_dir;
    int value_table;
    unsigned long long result = 0;
    int start = 0;

    //TESTS 
    //free_bit_at_index(dir_map, num_pd_entries, 0);
    //set_bit_at_index(dir_map, num_pd_entries, 1);
    
    // int n = 0;
    // for(n = 0; n < 1024; n++){
    //     set_bit_at_index((char*)&table_maps[0], num_table_entries, n);
    // }
    
    // print_bitmap((char*)&table_maps[0], 0);
    // print_bitmap((char*)&table_maps[0], 1);
    // print_bitmap((char*)&table_maps[0], 2);
    // print_bitmap((char*)&table_maps[0], 31);
    // print_bitmap((char*)&table_maps[31], 0);
    // print_bitmap((char*)&table_maps[0], 32);

    // print_bitmap((char*)&table_maps[1], 2);
    // print_bitmap((char*)&table_maps[2], 0);

    //set_bit_at_index((char*) &table_maps[1], num_table_entries, 0);
    //print_bitmap((char*)&table_maps[0]+4, 0);
    //print_bitmap((char*)&table_maps[1], 0);

    // printf("Table map 0: %p -- %p\n", &table_maps[0], &table_maps[0]+0);
    // printf("Table map 1: %p -- %p\n", &table_maps[4], &table_maps[0]+4);
    // printf("Table map 2: %p -- %p\n", &table_maps[8], &table_maps[0]+8);
    
    //END TESTS

    //printf("Directory ");
    //print_bitmap(dir_map, 0);
    // set_bit_at_index((char*)&table_maps[0], num_table_entries, 0);
    // print_bitmap((char*)&table_maps[0], 0);
    for(i = 0; i < num_pd_entries; i++){
        value_dir = get_bit_at_index(dir_map, num_pd_entries, i);
        if(value_dir == 1){ //Found an allocated/available directory slot
            for(k = 0; k < num_table_entries; k++){
                //Find the first UNallocated slot in the table
                
                //The Index [i * 32] is to reach the next chunk of bits representing the bitmap for the ith table
                value_table = get_bit_at_index((char*)&table_maps[i*32], num_table_entries, k); 
                //printf("I = %d K = %d -- BIT in table: %d\n", i, k, value_table);
                if(value_table == 0){
                    //Found slot --> Create virtual address from this and return
                    //printf("Index in Directory: %d -- In table: %d\n", i, k);
                    //MAKE THE START INDEX 1 NOT 0 ==> add +1 to the index and make virtual address from that
                    //  Translate this back by -1 when break down a VA
                    int dir_index = i + 1;
                    int tab_index = k + 1;
                    //printf("dir_index: %d -- tab_index: %d\n", dir_index, tab_index);
                    // printf("Result before: %llx\n", result);
                    int bit;
                    
                    // printf("Start = %d\n", start);
                    
                    printf("Virtual address (flipped): ");

                    //Loop to set offset bits to 0
                    for(i = 0; i < num_offset_bits; i++){
                        if (DEBUG) printf("%d", 0);
                        free_bit_at_index((char*) &result, sizeof(unsigned long long)*8, i);
                    }

                    start = start+i; //Save the last position
                    // printf("\nStart = %d\n", start);
                    if (DEBUG) printf(" ");
                    //Loop to get middle bits table index
                    for(i = 0; i < num_table_bits; i++){
                        bit = get_bit_at_index((char*) &tab_index, num_table_bits, i);
                        if (DEBUG) printf("%d", bit);
                        if(bit == 1){
                            set_bit_at_index((char*) &result, sizeof(unsigned long long)*8, start+i);
                        }
                    }

                    start = start+i;
                    // printf("\nStart = %d\n", start);
                    if (DEBUG) printf(" ");
                    //Loop to get bits for higher bits from dir index
                    for(i = 0; i < num_dir_bits; i++){
                        bit = get_bit_at_index((char*) &dir_index, num_dir_bits, i);
                        if (DEBUG) printf("%d", bit);
                        if(bit == 1){
                            set_bit_at_index((char*) &result, sizeof(unsigned long long)*8, start+i);
                        }
                    }
                    if (DEBUG) printf("\n");
                    //printf("Result: ");
                    //print_bitmap((char*) &result, 0);
                    //if (DEBUG) printf("Result VA: %llx\n", result);
                    return result;
                }
            }
        }
        
    }

    if (DEBUG) printf("No memory currently available. . .\n ");
    return 0;
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
static void set_bit_at_index(char *bitmap, int num_entries, int index)
{
    if(index > num_entries){
        if (DEBUG) printf("ERROR: Trying to set an index larger than number of entries. . . \n");
        return;
    }

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

static void free_bit_at_index(char *bitmap, int num_entries, int index){
    if(index > num_entries){
        if (DEBUG) printf("ERROR: Trying to free an index larger than number of entries. . . \n");
        return;
    }
    char *region = ((char *) bitmap) + (index / 8);
    char bit = 1 << (index % 8);
    *region &= ~bit;
}


//Example 3
//Function to get a bit at "index"
static int get_bit_at_index(char *bitmap, int num_entries, int index)
{
    //Same as example 3, get to the location in the character bitmap array
    // printf("part 1\n");
    char *region = ((char *) bitmap) + (index / 8);
    
    //Create a value mask that we are going to check if bit is set or not
    // printf("part 2\n");
    char bit = 1 << (index % 8);
    
    // printf("part 3\n");
    // printf("region: %lx, index: %d\n", region, index);
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

char* print_arbitrary_bits(void* location, int num_bits) {
    char* output_str = malloc((num_bits + 1) * sizeof(char));
    int ptr = num_bits;
    output_str[ptr] = '\0';
    ptr -= 1;
    char* arb = (char*) location;
    int num_full_chars = num_bits / 8;
    //printf("num full chars %d\n", num_full_chars);
    int num_leftovers = num_bits % 8;
    //printf("num leftovers %d\n", num_leftovers);
    for (int i = 0; i < num_full_chars; i++) {
        char cur = arb[i];
        for (int j = 0; j < 8; j++) {
            int bit = cur >> j & 1;
            output_str[ptr] = '0' + bit;
            ptr -= 1;
        }
    }
    for (int j = 0; j < num_leftovers; j++) {
        char cur = arb[num_full_chars];
        int bit = cur >> j & 1;
        output_str[ptr] = '0' + bit;
        ptr -= 1;
    }
    return output_str;
}

void print_bitmap(char* bitmap, int chunk){
    printf("bitmap Chunk %d: ", chunk);
    int index = chunk*4;
    int bits_for_map = sizeof(&bitmap[index]) * 8;
    int k;
    for(k = 0; k < bits_for_map; k++){
        printf("%d",get_bit_at_index(bitmap, bits_for_map*(chunk+1), k+(index*8)));
    }
    printf("\n");
}

//Get the number of bytes that a bit map uses
//num_chunks is the number of 32 Bit sections needed to represent the entire bitmap Ex: 32 Bit map = 1 chunk, 262144 pages = 262144 Bits = 8192 chunks
//NOT FINSIHED YET
int sizeof_bitmap(char* bitmap, int num_chunks){
    int chunk;
    int bytes = 0;
    for(chunk = 0; chunk < num_chunks; chunk++){
        
    }

    return 0;
}

