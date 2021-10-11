# User-level Memory Management
The goal of this project is to develop a fully functional virtualized memory in the user-space and have it completely separate from the OS. Implementation of our own malloc() function allows use to use this virtual memory just as we would the OS managed memory. 

Mimics the the memory management unit (MMU) to map the virtualized memory to physical memory. 


## In Review
What I learned from this project in hindsight
- Focus more on keeping the code clean as you work, don't wait until later to go back and clean it
- Don't use print statements as much for logging
- Structure your functions more and test extensively
- Do more research on best practices

<!-- # OS_PJ3
User-level Memory Management

## Grade components

- 70 points for virtual memory system
- 20 points for TLB
- 10 points for report

## Virtual memory system

- set_physical_mem()
  - Implemented
  - Works in all tests
- translate()
  - Implemented
  - Untested
- page_map()
  - Implemented
  - Tested as part of a_malloc()
- a_malloc()
  - Implemented
  - Breaks when more than six pages are allocated
- a_free()
  - Incomplete
- put_value()
  - Implemented
  - Untested
- get_value()
  - Implemented
  - Untested
- mat_mult()
  - Implemented
  - Untested

## TLB

- add_TLB()
  - Implemented with simple replacement policy
  - Untested, likely functional
- check_TLB()
  - Implemented
  - Untested
- print_TLB_missrate()
  - Implemented
  - Untested, likely functional

## Report

- Detailed logic for how you implemented each virtual memory function.
- Benchmark output for part 1 and the observed TLB miss rate in part 2.
- Support for different page sizes (in multiples of 4K).
- Possible issues in your code (if any).
 -->
