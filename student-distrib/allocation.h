#ifndef ALLOCATION_H
#define ALLOCATION_H
#include "types.h"
#include "lib.h"
#include "page.h"
#define FREE 0  //the memory is not used
#define USED 1  //the memory is used
#define SLAB_SIZE 4 * 1024 //the size of each slab

#define SLAB_ADDRESS 1024 * 1024 * 144 // 144mb is the beginning of the slab memory
#define SLAB_CACHE_MANAGE_ADDR 1024*1024*140 //144mb is the beginning of the management address
#define CACHE_NUMBER 100            //the maximum number of cache
#define SLAB_NUMBER 1024            //the maximum number of slab

typedef struct kmem_unit kmem_unit;
typedef struct kmem_slab kmem_slab;
typedef struct kmem_cache kmem_cache;

/*the struct of the unit*/
struct kmem_unit
{
    void *add_ptr;
    kmem_unit *next;
    kmem_unit *prev;
    int32_t p;
};

/*the struct of the slab*/
struct kmem_slab
{
    kmem_unit *unit_head;
    kmem_unit *free_head;
    kmem_slab *page_next;   //the difference between page_next and array_next is 
    kmem_slab *page_prev;   //page_next points to the next slan in the whole slab list
    kmem_slab *array_next;  //array_next points to the next slab in the cache
    kmem_slab *array_prev;
    void *page_ptr;
    uint32_t size;
    uint32_t unit_number;
    uint32_t used_unit_number;
    uint32_t p;
    uint32_t pid;
};

/*the struct of the cache*/
struct kmem_cache
{
    uint32_t size;
    kmem_slab *slab_array;
    kmem_cache *next;
    kmem_cache *prev;
    int32_t p;
    // int32_t extend;
};

/*it contains the information fo the cache list and slab list*/
typedef struct slab_cache_info
{
    kmem_cache *free_cache_head;
    kmem_cache *cache_head;
    kmem_slab *free_slab_head;
    kmem_slab *slab_head;
} slab_cache_info;

/*initialize the memory allocation*/
void init_memory_allocation();
/*initialize paging for the memory allocation*/
void init_allocation_paging();
/*initialize one slab*/
void init_one_slab(kmem_cache *cache, kmem_slab *slab);
/*create a new cache*/
extern kmem_cache *kmem_cache_create(
    uint32_t size);
/*alloc memory from the memory page*/
extern void *kmem_cache_alloc(kmem_cache *kmem_cache);
/*free the allocated memory*/
void kmem_cache_free(void *ptr);
/*move the cache to the free list*/
void kmem_cache_destroy(kmem_cache *kmem_cache);
/*see the allocation situation*/
void info_allocation();
/*move the slab to the free list*/
void delete_slab(kmem_cache* kmem_cache,kmem_slab* kmem_slab);
/*free all allocated memory within one program */
void free_one_program(uint32_t pid);
// /*allocate large memory*/
// void alloc_large(kmem_cache * kmem_cache);
extern slab_cache_info slab_cache;
extern kmem_cache *cache_array;
extern kmem_slab *slab_array;

#endif
