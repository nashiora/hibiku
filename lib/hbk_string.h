#ifndef HBK_STRING_H
#define HBK_STRING_H

#include "hbk_vector.h"

#include <stdint.h>

/// @brief A string which owns its data and is growable.
typedef hbk_vector(char) hbk_string;

/// @brief An immutable view into an existing string.
/// A string view cannot be deallocated, as it does not
/// own its memory, it merely allows you to look but not touch.
typedef struct hbk_string_view {
    const char* data;
    int64_t count;
} hbk_string_view;

#endif // !HBK_STRING_H
