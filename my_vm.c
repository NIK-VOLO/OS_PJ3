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

int num_table_entries;
int num_dir_entries;

int num_tlb_bits;
tlb* tlb_store;

char** table_maps;

pthread_mutex_t lock;

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating 
    unsigned long long length = MEMSIZE;
    phys = mmap(NULL,length, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_NORESERVE, -1, 0); //DOUBLE CHECK THESE PARAMETERS LATER
    memset(phys, 0, length);

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

    // Initialize TLB
    int num_tlb_bits = log2(TLB_ENTRIES);
    tlb_store = malloc(sizeof(tlb));
    memset(tlb_store, 0, sizeof(tlb));
    tlb_store->phys = malloc(TLB_ENTRIES * sizeof(pte_t));
    memset(tlb_store->phys, 0, TLB_ENTRIES * sizeof(pte_t));
    tlb_store->virt = malloc(TLB_ENTRIES * sizeof(pte_t));
    memset(tlb_store->virt, 0, TLB_ENTRIES * sizeof(pte_t));

    // Initialize mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        // Mutex init failed
        return;
    }

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
    set_bit_at_index(phys_map, num_phys_pages,39); // PET: Why?

    num_table_entries = scalbn(1, num_table_bits);
    num_dir_entries = scalbn(1, num_dir_bits);
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

    unsigned long tlb_index = get_tlb_index(va);

    //evict
    if (tlb_store->phys[tlb_index] != 0) {
        if (DEBUG) printf("Evicting TLB mapping: ");
        if (DEBUG) printf("virtual 0x%lx to ", (unsigned long int) tlb_store->phys[tlb_index]);
        if (DEBUG) printf("phys 0x%lx\n", (unsigned long int) tlb_store->virt[tlb_index]);
    }

    if (DEBUG) printf("Adding TLB mapping: ");
    tlb_store->phys[tlb_index] = (pte_t) pa;
    tlb_store->virt[tlb_index] = (pte_t) va;
    if (DEBUG) printf("virt 0x%lx to ", *(unsigned long int*) va);
    if (DEBUG) printf("phys 0x%lx\n", (unsigned long int) pa);

    return 0;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */
    // TLB index:
    // (((unsigned int)va)&tlb_bitmask) >> (numOffsetBits)

    unsigned long tlb_index = get_tlb_index(va);
    unsigned long offset = get_mid_bits(*(unsigned int*)va, num_offset_bits, 0);

    pte_t mapping = tlb_store->phys[tlb_index];

    if (mapping != 0) {
        
        if (DEBUG) printf("TLB hit\n");

        // TLB hit
        pte_t* pa = (pte_t*) tlb_store->phys[tlb_index];

        // Increment TLB hits
        tlb_store->hits += 1;

        return (pte_t*) pa + offset;
    }
    else {
        
        if (DEBUG) printf("TLB miss\n");

        // TLB miss

        pte_t *pa = translate(entries, (char*) va + offset);
        
        // Update TLB
        add_TLB(va, pa);

        // Increment TLB misses
        tlb_store->misses += 1;

        return (pte_t*) pa + offset;
    }

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

    double hits_double = (double) tlb_store->hits;
    double misses_double = (double) tlb_store->misses;
    miss_rate = misses_double / (hits_double + misses_double);


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

    unsigned int top = get_top_bits(*(unsigned int*)va, num_dir_bits);
    unsigned int mid = get_mid_bits(*(unsigned int*)va, num_table_bits, num_offset_bits);
    unsigned int offset = get_mid_bits(*(unsigned int*)va, num_offset_bits, 0);

    // Going to PDE
    pde_t *pde_addr = pgdir + top;
    pthread_mutex_lock(&lock);
    if (*pde_addr == (pde_t) NULL) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    // Going to page table
    pte_t *pt_addr = (pte_t*) *pde_addr;
    // Going to PTE
    pte_t *pte_addr = pt_addr + mid;
    if (*pte_addr == (pde_t) NULL) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    // Going to physical page
    pte_t *pp_addr = (pte_t*)(*pte_addr + offset);
    pthread_mutex_unlock(&lock);
    return pp_addr;
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

    if (DEBUG) printf("SETTING PAGE TABLE ENTRY\n");
    
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
    int value_dir = get_bit_at_index(dir_map, num_dir_entries, top_bits);
    if (value_dir) {

        // Page table already exists
        if (DEBUG) printf("Attempting to place PTE in PDE %d of %d.\n", top_bits+1, num_dir_entries);

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
        page_table[mid_bits] = (pte_t) pa;
        if (DEBUG) printf("Mapping Physical address 0x%lx to PTE %d of %d.\n", (unsigned long int) pa, mid_bits+1, num_table_entries);

        // Setting table bitmap
        char* table_start = (char*)&table_maps[top_bits*32];
        char* mb_str = print_arbitrary_bits(&mid_bits, 10);
        char* tb_str = print_arbitrary_bits(&top_bits, 10);
        set_bit_at_index((char*)&table_maps[top_bits*32], num_table_entries, mid_bits);

        // Setting physical bitmap
        int bit_to_set = ((char*)pa - phys) / PGSIZE;
        set_bit_at_index(phys_map, num_table_entries, bit_to_set);

        if (DEBUG) printf("DONE SETTING PAGE TABLE ENTRY\n");
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

    if (DEBUG) printf("DONE SETTING PAGE TABLE ENTRY (WITH NEW PAGE TABLE)\n");
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
    if (DEBUG) printf("Next available index: %d\n", index);
    return get_addr(index);
}

/*Function that gets the next available page directory entry slot
Returns the index of the next open spot. Each 4 Bytes

parameter int open    0 if you are looking for unallocated (open) page dir slots, 1 if allocated (closed).
*/
int get_next_pde(int open) {
    
    void* address;
    int i = 0;
    int value = get_bit_at_index(dir_map, num_dir_entries, i);
    //printf("Bit: %d at index: %d\n", value, i);
    if(value == open){
        //if (DEBUG) printf("(1) OPEN INDEX: %d Value: %d\n", i, value);
        //Get the address of that position
        //PD address + bytes    
        address = &entries[i];
        //if (DEBUG) printf("Dir: %p -- Entry: %p\n", entries, address);
        return i;
        //return address;
    }
    
    while(value == !open && i < num_dir_entries){
        i++;
        value = get_bit_at_index(dir_map, num_dir_entries, i);
        //printf("Bit: %d at index: %d\n", value, i);
        if(value == open){
            //if (DEBUG) printf("(2) OPEN INDEX: %d Value: %d\n", i, value);
            //Get the address of that position
            //PD address + bytes    
            address = &entries[i];
            //if (DEBUG) printf("Dir: %p -- Entry: %p\n", entries, address);
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

    if(DEBUG) printf("MALLOC CALL\n");

    int num_pages_needed = num_bytes/PGSIZE + ((num_bytes % PGSIZE) != 0);
    pthread_mutex_lock(&lock);

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */
    if(phys_created == 0 ){
        if (DEBUG) printf("INITIALIZING SIMULATED PHYSICAL MEMORY\n");
        set_physical_mem();
        phys_created = 1;
        printf("DONE INITIALIZING SIMULATED PHYSICAL MEMORY\n");
    }

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

   if(page_dir_created == 0 ){
       if (DEBUG) printf("INITIALIZING PAGE DIRECTORY\n");
       page_dir_init();
       page_dir_created = 1;
       printf("DONE INITIALIZING PAGE DIRECTORY.\n");
   }

   

   if (DEBUG) printf("Number of Pages needed: %d\n", num_pages_needed);
   //print_bitmap(phys_map, 0);
   void* free_page = get_next_avail(num_phys_pages, num_pages_needed);
   //print_bitmap(phys_map, 0);
   if(free_page == NULL){
       if (DEBUG) printf("ERROR: No available pages were found. . . \n");
       pthread_mutex_unlock(&lock);
       return NULL;
   }
    if (DEBUG) printf("Next open physical page: %p\n", free_page);

    unsigned long long virt = create_virt_addr();
    if(virt == 0){
        if (DEBUG) printf("PDE Not found. Double checking . . . \n");
        int t = get_next_pde(0);
        if(t == -1){
            if (DEBUG) printf("--> PDE is full. . .");
            pthread_mutex_unlock(&lock);
            return NULL;
        }else{
            create_dir_entry();
            virt = create_virt_addr();
            if(virt == 0){
                if (DEBUG) printf("\n **** NO PDE FOUND AGAIN --> BUG ****\n");
            }
        }
    }
    if (DEBUG) printf("Generated Virtual Address: 0x%llx\n", virt);
    void* virt_addr = (void*) &virt;
    
    page_map(entries, virt_addr, free_page);

    if (1) {
        printf("Bitmaps after a_malloc():\n");
        printf("Directory ");
        print_bitmap(dir_map, 0);
        printf("Page table ");
        print_bitmap((char*)&table_maps[0], 0);
        printf("Physical ");
        print_bitmap(phys_map, 0);
    }
    
    if(DEBUG) printf("DONE MALLOC CALL\n\n");

    // if (0) {
    //     printf("First Kilobyte of physical memory:\n");
    //     int lu_printed = 0;
    //     int lu_wanted = 128;
    //     while (lu_printed < lu_wanted) {
    //         int block_start = lu_printed * 4;
    //         long unsigned int lu1 = *((long unsigned int*)phys+block_start);
    //         long unsigned int lu2 = *((long unsigned int*)phys+block_start+4);
    //         long unsigned int lu3 = *((long unsigned int*)phys+block_start+8);
    //         long unsigned int lu4 = *((long unsigned int*)phys+block_start+12);
    //         printf("%lu %lu %lu %lu\n", lu1, lu2, lu3, lu4);
    //         lu_printed += 4;
    //     }
    // }
    pthread_mutex_unlock(&lock);
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

    int top_bits = get_top_bits(*(unsigned int*)va, num_dir_bits);
    int mid_bits = get_mid_bits(*(unsigned int*)va, num_table_bits, num_offset_bits);    
    top_bits--;
    mid_bits--;

    int num_pages = size / PGSIZE;
    int num_pages_freed = 0;
    pthread_mutex_lock(&lock);
    while (num_pages_freed < num_pages) {

        // Clear TLB entries
        //TODO

        // Clear table bitmap
        char* table_start = (char*)&table_maps[top_bits*32];
        char* mb_str = print_arbitrary_bits(&mid_bits, 10);
        char* tb_str = print_arbitrary_bits(&top_bits, 10);
        free_bit_at_index((char*)&table_maps[top_bits*32], num_table_entries, mid_bits);

        // Clear physical bitmap
        pte_t *pa = translate(entries, va);
        int bit_to_set = ((char*)pa - phys) / PGSIZE;
        free_bit_at_index(phys_map, num_table_entries, bit_to_set);

        // Clear dir bitmap if PDE now empty
        int pde_empty = 1;
        for (int i = 0; i < num_table_entries; i++) {
            int value = get_bit_at_index((char*)&table_maps[i*32], num_table_entries, i);
            if (value) pde_empty = 0;
        }
        if (pde_empty) {
            free_bit_at_index(dir_map, num_dir_entries, top_bits);
        }

        num_pages_freed += 1;
    }

    pthread_mutex_unlock(&lock);
    
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

    printf("put_value(): Parameter VA: %lx\n", *(unsigned long int*)va);
    char *val_ptr = (char*)val;

    // Does offset matter?
    // Check to see if space is allocated
    int space_ok = check_size(va, size);
    if(space_ok != 0) {
        return;
    }

    // Check TLB for phys address
    char *pa = (char*)check_TLB(va);
    if (pa == NULL) {
        // Something's wrong with the virtual address
        if (DEBUG) printf("put_value() error: invalid virtual address.\n");
        return;
    }

    int i = 0;
    pthread_mutex_lock(&lock);
    for (i = 0; i < size; i++) {
        // Pages will always be contiguous, so you can do this
        *(pa + i) = *(val_ptr + i);
    }
    pthread_mutex_unlock(&lock);
    return;
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

    // Does offset matter?

    char* val_ptr = (char*) val;

    // Check if size is ok
    int too_big = check_size(va, size);
    if (too_big != 0) {
        return;
    }

    pte_t *pa = check_TLB(va);
    if (pa == NULL) {
        return;
    }

    pthread_mutex_lock(&lock);
    char* pa_ptr = (char*) pa;
    int i = 0;
    for (i = 0; i < size; i++) {
        *(val_ptr + i) = *(pa_ptr + i);
    }
    pthread_mutex_unlock(&lock);
    return;
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

    int* mat1_copy = malloc(size * size * sizeof(int));
    int* mat2_copy = malloc(size * size * sizeof(int));

    get_value(mat1, mat1_copy, size * size);
    get_value(mat2, mat2_copy, size * size);

    
    int i, j, k, index, mat1_index, mat2_index;
    int product, product_sum;
    int* result = malloc(size * size * sizeof(int));
    // Init result matrix
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            index = (i * size) + j;
            result[index] = 0;
        }
    }
    // Doing the actual matrix math
    // Rows of mat1
    for (i = 0; i < size; i++) {
        // Cols of mat2
        for (j = 0; j < size; j++) {
            // Cols of mat1
            for (k = 0; k < size; k++) {
                index = (i * size) + j;
                mat1_index = (i * size) + k;
                mat2_index = (k * size) + j;
                result[index] += mat1_copy[mat1_index] * mat2_copy[mat2_index];
            }            
        }
    }

    answer = result;

    return;
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
    // if (DEBUG) printf("Size of pde_t: %d Bytes\n", sizeof(pde_t));
    num_dir_entries = scalbn(1, num_dir_bits);
    // if (DEBUG) printf("Number of entries in directory: %d\n", num_dir_entries);
    dir_map = (char*) malloc(num_dir_entries/8);
    memset(dir_map, 0, (num_dir_entries/8));
    // if (DEBUG) printf("Size of DIRECTORY Bitmap: %d\n", (num_dir_entries/8));
    int dir_size = num_dir_entries * sizeof(pde_t);
    // if (DEBUG) printf("Total Size of directory: %d\n", dir_size);
    int num_pages_needed = dir_size/PGSIZE + ((dir_size % PGSIZE) != 0);
    // if (DEBUG) printf("Number of pages needed to store directory: %d\n", num_pages_needed);


    //Set required bits in bitmap to 1
    //Loop using num_pages_needed
    int i;
    for(i = 0; i < num_pages_needed; i++){
        // if (DEBUG) printf("Setting bit index %d\n", i);
        set_bit_at_index(phys_map, num_phys_pages, i);
    }
    
    //ptr = (page_dir*) phys;

    // ----- TEST FOR ADDRESS MAPPING -----
    void* addr = phys;
    entries = (pde_t*) addr;
    // if (DEBUG) printf("Phys: %p -- Directory: %p\n", phys, entries);
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
    //set_bit_at_index(dir_map, num_dir_entries, 0);
    if (DEBUG) printf("Creating first page table. \n");
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
    if (DEBUG) printf("ALLOCATING SPACE FOR PAGE TABLE\n");
    // if (DEBUG) printf("Size of pte_t: %d Bytes\n", sizeof(pte_t));
    int num_pt_entries = scalbn(1, num_table_bits);
    // if (DEBUG) printf("Number of entries in Table: %d\n", num_pt_entries);
    int tab_size = num_pt_entries * sizeof(pte_t);
    //int tab_size = 8192;
    // if (DEBUG) printf("Total Size of Table: %d\n", tab_size);
    int num_pages_needed = tab_size/PGSIZE + ((tab_size % PGSIZE) != 0);
    // if (DEBUG) printf("Number of pages needed to store Table: %d\n", num_pages_needed);

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
    // if (DEBUG) printf("Bits for Physical Bitmap: %d\n", bits_for_map);
    // if (DEBUG) printf("Before physical ");
    // if (DEBUG) print_bitmap(phys_map, 0);

    // if (DEBUG) printf("Before directory ");
    // if (DEBUG) print_bitmap(dir_map,0);
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
    if (DEBUG) printf("Next directory index: %d\n", index);
    set_bit_at_index(dir_map, num_dir_entries, index);
    
    // if (DEBUG) printf("After physical ");
    // if (DEBUG) print_bitmap(phys_map,0);

    
    // if (DEBUG) printf("After directory ");
    // if (DEBUG) print_bitmap(dir_map,0);

    page = get_addr(start);
    if (DEBUG) printf("Physical address of page table: %p\n", page);
    entries[index] = (int) page;
    //if (DEBUG) printf("END PAGE TABLE ALLOCATION\n");
    if (DEBUG) printf("DONE ALLOCATING SPACE FOR PAGE TABLE\n");
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

    //set_bit_at_index(dir_map, num_dir_entries, 0);
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

 
    for(i = 0; i < num_dir_entries; i++){
        value_dir = get_bit_at_index(dir_map, num_dir_entries, i);
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

                    int bit;

                    

                    //Loop to set offset bits to 0
                    for(i = 0; i < num_offset_bits; i++){

                        free_bit_at_index((char*) &result, sizeof(unsigned long long)*8, i);
                    }

                    start = start+i; //Save the last position

                    //Loop to get middle bits table index
                    for(i = 0; i < num_table_bits; i++){
                        bit = get_bit_at_index((char*) &tab_index, num_table_bits, i);
                        if(bit == 1){
                            set_bit_at_index((char*) &result, sizeof(unsigned long long)*8, start+i);
                        }
                    }

                    start = start+i;
                    // printf("\nStart = %d\n", start);

                    //Loop to get bits for higher bits from dir index
                    for(i = 0; i < num_dir_bits; i++){
                        bit = get_bit_at_index((char*) &dir_index, num_dir_bits, i);

                        if(bit == 1){
                            set_bit_at_index((char*) &result, sizeof(unsigned long long)*8, start+i);
                        }
                    }

                    //printf("Result: ");
                    //print_bitmap((char*) &result, 0);
                    //if (DEBUG) printf("Result VA: %llx\n", result);
                    // printf("Mid bits: ");
                    // char* va_str = print_arbitrary_bits((char*)&result, 32);
                    // for (i = 0; i < 32; i++) {
                    //     if (i >= 10 && i < 20) {
                    //         printf("%c", va_str[i]);
                    //     }
                    // }
                    // printf("\n");
                    return result;
                }
            }
        }
        
    }

    if (DEBUG) printf("No memory currently available. . .\n ");
    return 0;
}

unsigned long long create_va_from_bits(int top, int mid, int offset) {
    unsigned long long result = 0;

    int pos = 0;
    int bit;
    int i;
    int start = 0;

    // Set offset bits to 0
    for(i = 0; i < num_offset_bits; i++){
        free_bit_at_index((char*) &result, sizeof(unsigned long long)*8, i);
    }
    
    // Set middle bits
    start = start + i;
    for(i = 0; i < num_table_bits; i++){
        bit = get_bit_at_index((char*) &mid, num_table_bits, i);
        if(bit == 1){
            set_bit_at_index((char*) &result, sizeof(unsigned long long)*8, start+i);
        }
    }

    // Set top bits
    start = start + i;
    for(i = 0; i < num_dir_bits; i++){
        bit = get_bit_at_index((char*) &top, num_dir_bits, i);
        if(bit == 1){
            set_bit_at_index((char*) &result, sizeof(unsigned long long)*8, start+i);
        }
    }

    return result;
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

int check_size(void* va, int size) {

    if (va < 0) {
        if (DEBUG) printf("check_size() error: invalid virtual address.\n");
        return 1;
    }
    
    int num_pages = size / PGSIZE;
    if (size % PGSIZE != 0) num_pages += 1;

    unsigned int top_bits = get_top_bits(*(unsigned int*)va, num_dir_bits);
    unsigned int mid_bits = get_mid_bits(*(unsigned int*)va, num_table_bits, num_offset_bits);
    unsigned int offset_index = get_mid_bits(*(unsigned int*)va, num_offset_bits, 0);
    top_bits--;
    mid_bits--;

    int index = 0;
    int value_table;
    while (index < num_pages) {
        value_table = get_bit_at_index((char*)&table_maps[top_bits*32], num_table_entries, mid_bits + index);
        if (value_table == 0) {
            // Not allocated
            if (DEBUG) printf("check_size() error: not entirely allocated.\n");
            return 2;
        }
        index += 1;
    }
    

    // All good
    return 0;
}

unsigned long get_tlb_index(void* va) {
    
    unsigned long result = get_mid_bits(*(unsigned int*)va, num_tlb_bits, num_offset_bits);

    return 0;
}

