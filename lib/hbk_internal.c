#include "hbk_internal.h"

#include <stdalign.h>
#include <stddef.h>

#define HBK_POOL_BLOCK_CAPACITY (1024*64)

typedef struct hbk_pool_block {
    void* memory;
    size_t memory_capacity;
    size_t total_allocated;
} hbk_pool_block;

struct hbk_pool {
    hbk_vector(hbk_pool_block) blocks;
};

void hbk_pool_add_block(hbk_pool* pool) {
    void* memory = calloc(HBK_POOL_BLOCK_CAPACITY, 1);
    hbk_pool_block block = {
        .memory = memory,
        .memory_capacity = HBK_POOL_BLOCK_CAPACITY,
    };
    hbk_vector_push(pool->blocks, block);
}

hbk_pool* hbk_pool_create() {
    hbk_pool* pool = calloc(1, sizeof *pool);
    HBK_ASSERT(pool != NULL, "Buy more ram");
    hbk_pool_add_block(pool);
    return pool;
}

void hbk_pool_destroy(hbk_pool* pool) {
    if (pool == NULL) return;
    for (int64_t i = 0; i < hbk_vector_count(pool->blocks); i++) {
        free(pool->blocks[i].memory);
        pool->blocks[i].memory = NULL;
    }

    hbk_vector_free(pool->blocks);
    free(pool);
}

void* hbk_pool_alloc(hbk_pool* pool, size_t count) {
    HBK_ASSERT(count <= HBK_POOL_BLOCK_CAPACITY, "Attempted to allocate something WAY too big, please don't");

    // TODO(local): vector_back?
    hbk_pool_block* block = &pool->blocks[hbk_vector_count(pool->blocks) - 1];
    HBK_ASSERT(block != NULL, "what");

    // align the count
    size_t align = alignof(max_align_t);
    count += (align - (count % align)) % align;

    if (count > block->memory_capacity - block->total_allocated) {
        hbk_pool_add_block(pool);
        block = &pool->blocks[hbk_vector_count(pool->blocks) - 1];
        HBK_ASSERT(block != NULL, "what");
    }

    void* result = (char*)block->memory + block->total_allocated;
    block->total_allocated += count;

    return result;
}
