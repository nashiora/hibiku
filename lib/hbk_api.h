#ifndef HBK_API_H
#define HBK_API_H

#include <hibiku.h>

#include "hbk_memory.h"

#define HBK_ASSERT(condition, message) do {} while(0)
#define HBK_TODO(message) HBK_ASSERT(false, "TODO: " message)

typedef struct hbk_pool hbk_pool;

hbk_string_view hbk_state_get_source_text_as_view(hbk_state* state, hbk_source_id source_id);
/// @brief "Intern"s the data of the given string. To intern a string,
/// the state creates a copy of it that it owns and returns to you a view.
/// This way, all instances of that string can point to the same memory.
/// An interned string is immutable.
hbk_string_view hbk_state_intern_string(hbk_state* state, hbk_string string);
/// @brief "Intern"s the data of the given string. To intern a string,
/// the state creates a copy of it that it owns and returns to you a view.
/// This way, all instances of that string can point to the same memory.
/// An interned string is immutable.
hbk_string_view hbk_state_intern_string_view(hbk_state* state, hbk_string_view sv);
/// @brief "Intern"s the data of the given string. To intern a string,
/// the state creates a copy of it that it owns and returns to you a view.
/// This way, all instances of that string can point to the same memory.
/// An interned string is immutable.
hbk_string_view hbk_state_intern_cstring(hbk_state* state, const char* string);

hbk_pool* hbk_pool_create();
void hbk_pool_destroy(hbk_pool* pool);
void* hbk_pool_alloc(hbk_pool* pool, size_t count);

#endif // !HBK_API_H
