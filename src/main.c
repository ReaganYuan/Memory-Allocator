#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "sfmm.h"
#include "debug.h"

// Define 20 megabytes as the max heap siz
#define MAX_HEAP_SIZE (20 * (1 << 20))
#define VALUE1_VALUE 320
#define VALUE2_VALUE 0xDEADBEEFF00D

#define press_to_cont()                    \
    do {                                   \
        printf("Press Enter to Continue"); \
        while (getchar() != '\n')          \
            ;                              \
        printf("\n");                      \
    } while (0)

#define null_check(ptr, size)                                                                  \
    do {                                                                                       \
        if ((ptr) == NULL) {                                                                   \
            error("Failed to allocate %lu byte(s) for an integer using sf_malloc.\n", (size)); \
            error("%s\n","Aborting...");                                                            \
            assert(false);                                                                     \
        } else {                                                                               \
            success("sf_malloc returned a non-null address: %p\n", (void*)(ptr));                     \
        }                                                                                      \
    } while (0)

#define payload_check(ptr)                                                                   \
    do {                                                                                     \
        if ((unsigned long)(ptr) % 16 != 0) {                                                \
            warn("Returned payload address is not divisble by a quadword. %p %% 16 = %lu\n", \
                 (void*)(ptr), (unsigned long)(ptr) % 16);                                          \
        }                                                                                    \
    } while (0)

#define check_prim_contents(actual_value, expected_value, fmt_spec, name)                          \
    do {                                                                                           \
        if (*(actual_value) != (expected_value)) {                                                 \
            error("Expected " name " to be " fmt_spec " but got " fmt_spec "\n", (expected_value), \
                  *(actual_value));                                                                \
            error("%s\n","Aborting...");                                                                \
            assert(false);                                                                         \
        } else {                                                                                   \
            success(name " retained the value of " fmt_spec " after assignment\n",                 \
                    (expected_value));                                                             \
        }                                                                                          \
    } while (0)
    
int main(int argc, char *argv[]) {
    // Initialize the custom allocator
    sf_mem_init(MAX_HEAP_SIZE);

    info k;


    // Tell the user about the fields
    info("Initialized heap with %dmb of heap space.\n", MAX_HEAP_SIZE >> 20);
    //press_to_cont();

    // Print out title for first test
    printf("=== Test1: Allocation test ===\n");
    // Test #1: Allocate an integer

    int* a = sf_malloc(100);
    a = sf_realloc(a,100);

    sf_varprint(a);
    sf_snapshot(true);


    sf_info(&k);
    printf("allocatedBlocks: %lu\n",k.allocatedBlocks);
    printf("splinterblocks: %lu\n",k.splinterBlocks);
    printf("padding: %lu\n",k.padding);
    printf("splintering: %lu\n",k.splintering);
    printf("coalesces: %lu\n",k.coalesces);
    printf("memorypeak: %.10f",k.peakMemoryUtilization);



    sf_mem_fini();
 

    return EXIT_SUCCESS;
}
