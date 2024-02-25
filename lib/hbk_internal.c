#include "hbk_internal.h"

#include <stdarg.h>
#include <stdalign.h>
#include <stddef.h>

[[noreturn]]
void hbk_internal_error(hbk_error_kind kind, const char* file_name, int line_number, const char* assert_expression, const char* format, ...) {
    fprintf(stderr, "%s%s:%d: ", ANSI_COLOR_RESET, file_name, line_number);

    switch (kind) {
        default: fprintf(stderr, "%s???", ANSI_COLOR_CYAN); break;
        case HBK_ERROR_ASSERT: fprintf(stderr, "%sAssertion Failed", ANSI_COLOR_RED); break;
        case HBK_ERROR_ICE: fprintf(stderr, "%sInternal Compiler Error", ANSI_COLOR_MAGENTA); break;
        case HBK_ERROR_TODO: fprintf(stderr, "%sTODO", ANSI_COLOR_CYAN); break;
        case HBK_ERROR_UNREACHABLE: fprintf(stderr, "%sUnreachable", ANSI_COLOR_YELLOW); break;
    }
    fprintf(stderr, "%s", ANSI_COLOR_RESET);

    if (format != NULL) {
        fprintf(stderr, ": ");
        va_list v;
        va_start(v, format);
        vfprintf(stderr, format, v);
        va_end(v);
    }

    fprintf(stderr, "\n");
    if (assert_expression != NULL) {
        fprintf(stderr, "assert condition: %s", assert_expression);
    }

    fprintf(stderr, "\n");
    exit(1);
}

#define HBK_ARENA_BLOCK_CAPACITY (1024*64)

typedef struct hbk_arena_block {
    void* memory;
    size_t memory_capacity;
    size_t total_allocated;
} hbk_arena_block;

struct hbk_arena {
    hbk_vector(hbk_arena_block) blocks;
};

void hbk_arena_add_block(hbk_arena* arena) {
    void* memory = calloc(HBK_ARENA_BLOCK_CAPACITY, 1);
    hbk_arena_block block = {
        .memory = memory,
        .memory_capacity = HBK_ARENA_BLOCK_CAPACITY,
    };
    hbk_vector_push(arena->blocks, block);
}

hbk_arena* hbk_arena_create() {
    hbk_arena* arena = calloc(1, sizeof *arena);
    HBK_ASSERT(arena != NULL, "Buy more ram");
    hbk_arena_add_block(arena);
    return arena;
}

void hbk_arena_destroy(hbk_arena* arena) {
    if (arena == NULL) return;
    for (int64_t i = 0; i < hbk_vector_count(arena->blocks); i++) {
        free(arena->blocks[i].memory);
        arena->blocks[i].memory = NULL;
    }

    hbk_vector_free(arena->blocks);
    free(arena);
}

void* hbk_arena_alloc(hbk_arena* arena, size_t count) {
    HBK_ASSERT(count <= HBK_ARENA_BLOCK_CAPACITY, "Attempted to allocate something WAY too big, please don't");

    // TODO(local): vector_back?
    hbk_arena_block* block = &arena->blocks[hbk_vector_count(arena->blocks) - 1];
    HBK_ASSERT(block != NULL, "what");

    // align the count
    size_t align = alignof(max_align_t);
    count += (align - (count % align)) % align;

    if (count > block->memory_capacity - block->total_allocated) {
        hbk_arena_add_block(arena);
        block = &arena->blocks[hbk_vector_count(arena->blocks) - 1];
        HBK_ASSERT(block != NULL, "what");
    }

    void* result = (char*)block->memory + block->total_allocated;
    block->total_allocated += count;

    return result;
}
