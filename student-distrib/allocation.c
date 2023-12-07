#include "allocation.h"

/*There is a hierarchy of three
        ————————————
        |   cache  | (it contains one or more slabs)
        ————————————
              |
        ————————————
        |   slab   | (one slab is 4kb)(it contains one or more units)
        ————————————
              |
        ————————————
        |   unit   | (the size depends on allocation)
        ————————————
*/
PT_t slab_pageTable __attribute__((aligned(1024 * 4)));
// PT_t slab_cache_manage_pageTable __attribute__((aligned(1024 * 4)));
kmem_cache *cache_array = NULL; // all caches
kmem_slab *slab_array = NULL;   // all slabs
slab_cache_info slab_cache;

/**
 * init_allocation_paging
 * set the paging used for memory allocation
 * INPUT: none
 * OUTPUT: none
 */
void init_allocation_paging()
{
    int32_t i;

    /*the whole page table is used for memory allocation*/
    for (i = 0; i < SLAB_NUMBER; i++)
    {
        slab_pageTable[i].val = 0;
        set_pte(&slab_pageTable, (SLAB_ADDRESS + i * SLAB_SIZE) >> 12 & 0x3ff, 1, (SLAB_ADDRESS + i * SLAB_SIZE) >> 12);
    }
    set_pde(&pageDirectory, (SLAB_ADDRESS >> 22) & 0x3FF, 1, 1, 0, (((uint32_t)&slab_pageTable) >> 12));
    set_pde(&pageDirectory, (SLAB_CACHE_MANAGE_ADDR >> 22) & 0x3FF, 0, 1, 1, (SLAB_CACHE_MANAGE_ADDR >> 22) & 0x3FF);
    update_cr3();
    return;
}

/**
 * init_memory_allocation
 * initialize the paging and linked-list for memory allocation
 * INPUT: none
 * OUTPUT: none
 */
void init_memory_allocation()
{
    int i;

    /*initialize slab page table*/
    init_allocation_paging();

    /*initialize the management pointer*/
    cache_array = (kmem_cache *)(SLAB_CACHE_MANAGE_ADDR);
    slab_array = (kmem_slab *)(SLAB_CACHE_MANAGE_ADDR + CACHE_NUMBER * sizeof(kmem_cache));

    /*initialize caches*/
    for (i = 0; i < CACHE_NUMBER; i++)
    {
        cache_array[i].next = &cache_array[i + 1];
        cache_array[i].prev = &cache_array[i - 1];
        cache_array[i].p = FREE;
        // cache_array[i].extend = 0;
    }
    cache_array[0].prev = NULL;
    cache_array[CACHE_NUMBER - 1].next = NULL;

    /*initialize slabs*/
    for (i = 0; i < SLAB_NUMBER; i++)
    {
        slab_array[i].page_next = &slab_array[i + 1];
        slab_array[i].page_prev = &slab_array[i - 1];
        slab_array[i].page_ptr = (void *)(SLAB_ADDRESS + i * SLAB_SIZE);
        slab_array[i].p = FREE;
    }
    slab_array[0].page_prev = NULL;
    slab_array[SLAB_NUMBER - 1].page_next = NULL;

    /*initialize slab_cache information*/
    slab_cache.cache_head = cache_array;
    slab_cache.free_cache_head = cache_array;
    slab_cache.free_slab_head = slab_array;
    slab_cache.slab_head = slab_array;
}

/**
 * kmem_cache_create
 * assign a new cache based on the size
 * INPUT: size: the object size of the memory required
 * OUTPUT: the new assigned cache for the size
 */
kmem_cache *kmem_cache_create(
    uint32_t size)
{
    uint32_t cache_index;
    kmem_cache *new_cache = NULL;

    /*sanity check*/
    if (size == 0)
        return NULL;

    /*get a free cache*/
    if (slab_cache.free_cache_head)
    {
        new_cache = slab_cache.free_cache_head;
        /*initialize other members*/
        new_cache->size = size;
        cache_index = (new_cache - slab_cache.cache_head) / sizeof(kmem_cache);
    }
    else
    {
        return NULL;
    }

    /*get a free slab*/
    if (slab_cache.free_slab_head)
    {
        new_cache->slab_array = slab_cache.free_slab_head;
        new_cache->slab_array->array_prev = NULL;
        init_one_slab(new_cache, new_cache->slab_array);
    }
    else
    {
        return NULL;
    }
    new_cache->p = USED;

    /*update slab_cache information*/
    slab_cache.free_cache_head = slab_cache.free_cache_head->next;
    slab_cache.free_slab_head = slab_cache.free_slab_head->page_next;

    return new_cache;
}

/**
 * kmem_cache_alloc
 * allocate a memory from the cache
 * INPUT: kmem_cache: the momory cache that is used to alloc
 * OUTPUT: the allocated memory address
 */
void *kmem_cache_alloc(kmem_cache *kmem_cache)
{
    /*check whether the current slab has enough space*/
    void *ret_add;
    kmem_slab **cur_slab = &(kmem_cache->slab_array);
    kmem_slab *last_slab = NULL;
    int32_t slab_exist = 0;

    /*find whether there is slab can be used*/
    while (*cur_slab)
    {
        if ((*cur_slab)->free_head && ((*cur_slab)->pid == sche_array[sche_index]))
        {
            slab_exist = 1;
            break;
        }
        else
        {
            last_slab = *cur_slab;
            cur_slab = &((*cur_slab)->array_next);
        }
    }
    /*if no slab is usable, create one*/
    if (!slab_exist)
    {
        if (slab_cache.free_slab_head)
        {
            *cur_slab = slab_cache.free_slab_head;
            (*cur_slab)->array_prev = last_slab;
            last_slab->array_next = (*cur_slab);
            init_one_slab(kmem_cache, *cur_slab);
        }
        else
        {
            return NULL;
        }
    }

    /*get the usable memory address*/
    slab_cache.free_slab_head = slab_cache.free_slab_head->page_next;
    ret_add = (*cur_slab)->free_head->add_ptr;
    (*cur_slab)->free_head->p = USED;
    (*cur_slab)->free_head = (*cur_slab)->free_head->next;
    (*cur_slab)->used_unit_number++;

    return ret_add;
}

/**
 * kmem_cache_free
 * free the allocated memory
 * INPUT: ptr:the address of the allocated memory
 * OUTPUT: none
 */
void kmem_cache_free(void *ptr)
{
    int32_t slab_index, unit_index;
    int32_t unit_mem_start;
    int32_t cache_exist = 0;
    kmem_cache *this_cache = slab_cache.cache_head;
    kmem_slab *this_slab = NULL;
    kmem_unit *this_unit = NULL;

    /*get the corresponding slab*/
    slab_index = ((int32_t)ptr - SLAB_ADDRESS) / (SLAB_SIZE);
    if (slab_index < 0 || slab_index >= PAGE_TABLE_ENTRY_NUM)
    {

        printf("Can't free an invalid pointer!\n");
        return;
    }
    this_slab = &(slab_array[slab_index]);
    if (this_slab->p == FREE)
    {
        printf("Can't free an invalid pointer!\n");
        return;
    }
    if (this_slab->pid != sche_array[sche_index])
    {
        printf("Can't free an invalid pointer!\n");
        return;
    }

    /*get the corresponding cache*/
    while (this_cache && this_cache->p)
    {
        if (this_slab->size == this_cache->size)
        {
            cache_exist = 1;
            break;
        }
        this_cache = this_cache->next;
    }
    if (!cache_exist)
    {
        printf("Can't free an invalid pointer!\n");
        return;
    }

    /*get the corresponding unit*/
    unit_mem_start = sizeof(kmem_unit) * this_slab->unit_number + (int32_t)this_slab->page_ptr;
    unit_index = ((int32_t)ptr - unit_mem_start) / this_slab->size;
    if ((((int32_t)ptr - unit_mem_start) % this_slab->size) != 0)
    {
        printf("Can't free an invalid pointer!\n");
        return;
    }
    this_unit = (kmem_unit *)((int32_t)(this_slab->page_ptr) + sizeof(kmem_unit) * unit_index);
    if (!this_unit->p)
    {
        printf("Can't free an invalid pointer!\n");
        return;
    }
    /*reshape the linked-list*/
    this_slab->used_unit_number--;
    this_unit->p = FREE;
    if (this_unit->prev == NULL) // the case when the unit is in the head
    {
        if (this_slab->used_unit_number == 0) // the case when there is only 1 used_unit
            delete_slab(this_cache, this_slab);
        else
        {
            this_slab->unit_head = this_unit->next;
            this_unit->next->prev = NULL;
            this_unit->next = this_slab->free_head;
            this_unit->prev = this_slab->free_head->prev;
            this_slab->free_head->prev->next = this_unit;
            this_slab->free_head->prev = this_unit;
            this_slab->free_head = this_unit;
        }
    }
    else if (this_unit->next == NULL) // the case when all units are used
    {
        this_slab->free_head = this_unit;
    }
    else // the normal case
    {
        this_unit->prev->next = this_unit->next;
        this_unit->next->prev = this_unit->prev;
        this_unit->next = this_slab->free_head;
        this_unit->prev = this_slab->free_head->prev;
        this_slab->free_head->prev->next = this_unit;
        this_slab->free_head->prev = this_unit;
        this_slab->free_head = this_unit;
    }
}

/**
 * kmem_cache_destroy
 * free the cache
 * INPUT: kmem_cache: the cache that is destroyed
 * OUTPUT: none
 */
void kmem_cache_destroy(kmem_cache *kmem_cache)
{
    kmem_cache->p = FREE;

    /*move it to free cache list*/
    if (kmem_cache->prev == NULL) // the case when it is in the head
    {
        if (kmem_cache->next == slab_cache.free_cache_head) // the case these is only 1 cache used
        {
            slab_cache.free_cache_head = kmem_cache;
        }
        else
        {
            slab_cache.cache_head = kmem_cache->next;
            kmem_cache->next->prev = NULL;
            kmem_cache->next = slab_cache.free_cache_head;
            kmem_cache->prev = slab_cache.free_cache_head->prev;
            slab_cache.free_cache_head->prev->next = kmem_cache;
            slab_cache.free_cache_head->prev = kmem_cache;
            slab_cache.free_cache_head = kmem_cache;
        }
    }
    else if (kmem_cache->next == NULL) // the case that all caches are used
    {
        slab_cache.free_cache_head = kmem_cache;
    }
    else // the normal case
    {
        kmem_cache->prev->next = kmem_cache->next;
        kmem_cache->next->prev = kmem_cache->prev;
        kmem_cache->next = slab_cache.free_cache_head;
        kmem_cache->prev = slab_cache.free_cache_head->prev;
        slab_cache.free_cache_head->prev->next = kmem_cache;
        slab_cache.free_cache_head->prev = kmem_cache;
        slab_cache.free_cache_head = kmem_cache;
    }
}

/**
 * free_one_program
 * free all allocated memory within one program
 * INPUT: pid:process id
 * OUTPUT: none
 */
void free_one_program(uint32_t pid)
{
    kmem_slab *cur_slab = NULL;
    kmem_cache *cur_cache = slab_cache.cache_head;
    kmem_cache *next_cache = NULL;
    while (cur_cache && cur_cache->p)
    {
        next_cache = cur_cache->next;
        cur_slab = cur_cache->slab_array;
        /*free all slab of the cache belongs to the program*/
        while (cur_slab && cur_slab->p)
        {
            if (cur_slab->pid == pid)
            {
                delete_slab(cur_cache, cur_slab);
            }
            cur_slab = cur_slab->array_next;
        }

        /*update the current cache*/
        cur_cache = next_cache;
    }
}
// /*allocate large memory*/
// void alloc_large(kmem_cache *kmem_cache)
// {

// }
/*------------------------------helper-------------------------------------*/

/**
 * init_one_slab
 * initialize the new slab
 * INPUT: cache:the cache that the slab belongs to
 *        slab: the slab that will be initialized
 * OUTPUT: none
 */
void init_one_slab(kmem_cache *cache, kmem_slab *slab)
{
    int32_t i;
    int32_t size = cache->size;
    int32_t unit_number = SLAB_SIZE / (sizeof(kmem_unit) + size);
    void *unit_mem_start = (void *)((uint32_t)(slab->page_ptr) + (uint32_t)(unit_number * sizeof(kmem_unit)));
    kmem_unit *unit;

    /*init members of the slab*/
    slab->array_next = NULL;
    slab->size = size;
    slab->unit_head = (kmem_unit *)slab->page_ptr;
    slab->free_head = slab->unit_head;
    slab->unit_number = unit_number;
    slab->used_unit_number = 0;
    slab->p = USED;

    /*initialize all units of this slab*/
    for (i = 0; i < unit_number; i++)
    {

        unit = (kmem_unit *)((uint32_t)(slab->unit_head) + i * sizeof(kmem_unit));
        unit->p = FREE;
        unit->add_ptr = (void *)((int32_t)unit_mem_start + i * size);
        unit->next = (kmem_unit *)((int32_t)unit + sizeof(kmem_unit));
        unit->prev = (kmem_unit *)((int32_t)unit - sizeof(kmem_unit));
    }
    ((kmem_unit *)((uint32_t)(slab->unit_head)))->prev = NULL;
    unit->next = NULL;

    /*associate it with the program*/
    slab->pid = sche_array[sche_index];
}

/**
 * delete_slab
 * remove it from the cache-slab-list and move it to the free slab list
 * INPUT: kmem_cache:the cache that the slab belongs to
 *        kmem_slab: the slab that will be deleted
 * OUTPUT: none
 */
void delete_slab(kmem_cache *kmem_cache, kmem_slab *kmem_slab)
{
    kmem_slab->p = FREE;

    /*put it in the free slab list*/
    if (kmem_slab->page_prev == NULL) // the case when it is in the head
    {
        if (kmem_slab->page_next == slab_cache.free_slab_head) // the case when there is only 1 slab used
        {
            slab_cache.free_slab_head = kmem_slab;
        }
        else
        {
            slab_cache.slab_head = kmem_slab->page_next;
            kmem_slab->page_next->page_prev = NULL;
            kmem_slab->page_next = slab_cache.free_slab_head;
            kmem_slab->page_prev = slab_cache.free_slab_head->page_prev;
            slab_cache.free_slab_head->page_prev->page_next = kmem_slab;
            slab_cache.free_slab_head->page_prev = kmem_slab;
            slab_cache.free_slab_head = kmem_slab;
        }
    }
    else if (kmem_slab->page_next == NULL) // the case that all slabs are used
    {
        slab_cache.free_slab_head = kmem_slab;
    }
    else // the normal case
    {
        kmem_slab->page_prev->page_next = kmem_slab->page_next;
        kmem_slab->page_next->page_prev = kmem_slab->page_prev;
        kmem_slab->page_next = slab_cache.free_slab_head;
        kmem_slab->page_prev = slab_cache.free_slab_head->page_prev;
        slab_cache.free_slab_head->page_prev->page_next = kmem_slab;
        slab_cache.free_slab_head->page_prev = kmem_slab;
        slab_cache.free_slab_head = kmem_slab;
    }

    /*remove it from the cache-slab_array*/
    if (kmem_slab->array_prev == NULL && kmem_slab->array_next == NULL)
    {
        kmem_cache_destroy(kmem_cache);
    }
    else if (kmem_slab->array_prev == NULL)
    {
        kmem_cache->slab_array = kmem_slab->array_next;
        kmem_cache->slab_array->array_prev = NULL;
    }
    else if (kmem_slab->array_next == NULL)
    {
        kmem_slab->array_prev->array_next = NULL;
    }
    else
    {
        kmem_slab->array_prev->array_next = kmem_slab->array_next;
    }
}


/**
 * info_allocation
 * see the allocation situation
 * INPUT: none
 * OUTPUT: none
 */
void info_allocation()
{
    kmem_cache *cur_cache = slab_cache.cache_head;
    kmem_slab *cur_slab = NULL;
    kmem_unit *cur_unit = NULL;
    int32_t index;
    int32_t i;
    printf("\n-----------------------------------------\n");
    /*print all caches information*/
    if (!(cur_cache && cur_cache->p))
    {
        printf("No memory has been allocated!\n");
    }
    while (cur_cache && cur_cache->p)
    {
        cur_slab = cur_cache->slab_array;
        printf("cahce with size %d:\n", cur_cache->size);
        index = 0;

        /*print all slabs information*/
        while (cur_slab && cur_slab->p)
        {
            printf("    slab%d(pid=%d) : total units: %d, used: %d. \n", index, cur_slab->pid, cur_slab->unit_number, cur_slab->used_unit_number);
            printf("    memory ptrs:");
            
            cur_unit = cur_slab->unit_head;
            for (i = 0; i < cur_slab->used_unit_number; i++)
            {
                printf(" %d", cur_unit->add_ptr);
                cur_unit = cur_unit->next;
            }
            printf("\n");
            index++;
            cur_slab = cur_slab->array_next;
        }
        cur_cache = cur_cache->next;
    }
    printf("-----------------------------------------\n");
}
