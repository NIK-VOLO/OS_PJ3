# OS_PJ3
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
