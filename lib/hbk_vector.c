#include "hbk_vector.h"

#include "hbk_api.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define HBK_VECTOR_MIN_CAPACITY 32

void hbk_vector_ensure_capacity(void** vector_address, int64_t element_size, int64_t minimum_capacity) {
    HBK_ASSERT(vector_address != NULL, "Invalid vector address pointer");
    HBK_ASSERT(element_size > 0, "Invalid element size");
    HBK_ASSERT(minimum_capacity >= 0, "Invalid vector minimum capacity");

    if (minimum_capacity == 0) {
        return;
    }

    hbk_vector_header* header = NULL;
    if (*vector_address == NULL) {
        if (minimum_capacity < HBK_VECTOR_MIN_CAPACITY) {
            minimum_capacity = HBK_VECTOR_MIN_CAPACITY;
        }

        header = malloc((sizeof *header) + (element_size * minimum_capacity));
        HBK_ASSERT(header != NULL, "Buy more ram lol");
        header->capacity = minimum_capacity;
        header->count = 0;

        void* data = header + 1;
        memset(data, 0, (size_t)(element_size * minimum_capacity));

        *vector_address = data;
    } else {
        header = (hbk_vector_header*)(*vector_address) - 1;
        if (minimum_capacity <= header->capacity) {
            return;
        }

        int64_t original_count = header->count;

        int64_t new_capacity = header->capacity;
        while (new_capacity < minimum_capacity) {
            new_capacity *= 2;
        }

        header = realloc(header, (sizeof *header) + (element_size * new_capacity));
        HBK_ASSERT(header != NULL, "Buy more ram lol");
        header->capacity = new_capacity;

        void* data = header + 1;
        memset((char*)data + (element_size * original_count), 0, (size_t)(element_size * (new_capacity - original_count)));

        *vector_address = data;
    }

    HBK_ASSERT(*vector_address != NULL, "Failed to populate vector data");
}
