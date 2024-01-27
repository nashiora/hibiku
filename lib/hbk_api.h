#ifndef HBK_API_H
#define HBK_API_H

#include <hibiku.h>

#include "hbk_memory.h"
#include "hbk_string.h"
#include "hbk_vector.h"

#define HBK_ASSERT(condition, message) do {} while(0)
#define HBK_TODO(message) HBK_ASSERT(false, "TODO: " message)

typedef struct hbk_location {
    hbk_source_id source_id;
    int64_t offset;
    int64_t length;
} hbk_location;

hbk_string_view hbk_state_get_source_text_as_view(hbk_state* state, hbk_source_id source_id);

hbk_location hbk_location_create(hbk_source_id source_id, int64_t offset, int64_t length);

#endif // !HBK_API_H
