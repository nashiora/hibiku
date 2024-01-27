#ifndef HKB_VECTOR_H
#define HKB_VECTOR_H

#include <stdint.h>

/// A vector/list/dynamir array in Hibiku is represented simply as a pointer to
/// the underlying type. This gives us type the ability to access the type info
/// of the underlying type implicitly rather than requiring the user specify it
/// for every function call, since C does not support generic types or type
/// introspection.
/// Since all of the API for the vector type is implemented through C macros,
/// we can use things like `sizeof(*vector)` to query the size of the underlying type.
#define hbk_vector(T) T*

typedef struct hbk_vector_header {
    int64_t count;
    int64_t capacity;
} hbk_vector_header;

void hbk_vector_ensure_capacity(void** vector_address, int64_t element_size, int64_t minimum_capacity);

#define hbk_vector_get_header(V) ((hbk_vector_header*)(V)-1)
#define hbk_vector_count(V)      ((V) ? hbk_vector_get_header(V)->count : 0)
#define hbk_vector_capacity(V)   ((V) ? hbk_vector_get_header(V)->capacity : 0)
#define hbk_vector_set_count(V, C)                                  \
    do {                                                            \
        hbk_vector_ensure_capacity((void**)&(V), sizeof *(V), (C)); \
        hbk_vector_get_header(V)->count = (C);                      \
    } while (0)
#define hbk_vector_set_capacity(V, C)                                  \
    do {                                                            \
        hbk_vector_ensure_capacity((void**)&(V), sizeof *(V), (C)); \
    } while (0)
#define hbk_vector_push(V, E)                                                           \
    do {                                                                                \
        hbk_vector_ensure_capacity((void**)&(V), sizeof *(V), hbk_vector_count(V) + 1); \
        (V)[hbk_vector_count(V)] = (E);                                                 \
        hbk_vector_get_header(V)->count++;                                              \
    } while (0)

#endif // !HKB_VECTOR_H
