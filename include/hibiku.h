#ifndef HIBIKU_H
#define HIBIKU_H

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

/// Fancy C macro trickery to get strings when we need them.
/// If you don't know what this means, don't worry too much about it.
#define HBK_STR_HELPER(x) #x
#define HBK_STR(x) HBK_STR_HELPER(x)

/// Hibiku follows a simplified version of Semantic Versioning (https://semver.org/).
/// This library provides the Major, Minor and Patch version numbers as macros.
/// Also provided are macros to convert a version to a single integer for the purposes of comparison,
/// as well as a concatenated version string.

/// For the purposes of explaining these version numbers:
/// -  X refers to the Major version,
/// -  Y refers to the Minor version, and
/// -  Z refers to the Patch version

#define HBK_VERSION_MAJOR 1
#define HBK_VERSION_MINOR 0
#define HBK_VERSION_PATCH 0

/// The version number is the Major and Minor version numbers in the form XYY.
/// This means that the Minor version takes the lower two digits, and the Major
/// fills out the higest.
#define HBK_VERSION_NUMBER ((HBK_VERSION_MAJOR * 100) + HBK_VERSION_MINOR)
/// The release version number includes the Patch version by multiplying
/// the base version by 100 so that the Patch version number can take the lowest two
/// digits. This version number takes the form XYYZZ.
#define HBK_VERSION_RELEASE_NUMBER ((HBK_VERSION_NUMBER * 100) + HBK_VERSION_PATCH)
/// The Hibiku version as a string, including the name "Hibiku" at the start.
/// This string is in the form of "Hibiku X.Y"
#define HBK_VERSION "Hibiku " HBK_STR(HBK_VERSION_MAJOR) "." HBK_STR(HBK_VERSION_MINOR)
/// The Hibiku release version as a string, including the name "Hibiku" at the start.
/// This includes the Patch version, so this string is in the form of "Hibiku X.Y.Z".
#define HBK_VERSION_RELEASE HBK_VERSION "." HBK_STR(HBK_VERSION_PATCH)

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
#define hbk_vector_free(V) do { if (V) free(hbk_vector_get_header(V)); (V) = NULL; } while (0)
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

/// @brief A string which owns its data and is growable.
typedef hbk_vector(char) hbk_string;

/// @brief An immutable view into an existing string.
/// A string view cannot be deallocated, as it does not
/// own its memory, it merely allows you to look but not touch.
typedef struct hbk_string_view {
    const char* data;
    int64_t count;
} hbk_string_view;

#define HBK_SV_EXPAND(SV) (int)(SV).count, (SV).data

typedef int64_t hbk_source_id;
typedef struct hbk_state hbk_state;

typedef struct hbk_location {
    hbk_source_id source_id;
    int64_t offset;
    int64_t length;
} hbk_location;

typedef enum  {
    HBK_DIAG_VERBOSE,
    HBK_DIAG_DEBUG,
    HBK_DIAG_INFO,
    HBK_DIAG_WARNING,
    HBK_DIAG_ERROR,
    HBK_DIAG_FATAL,
} hbk_diagnostic_kind;

typedef struct hbk_diagnostic hbk_diagnostic;
struct hbk_diagnostic {
    hbk_diagnostic_kind kind;
    hbk_location location;
    hbk_string_view message;
    hbk_diagnostic* related_diagnostics;
};

hbk_string_view hbk_cstring_as_view(const char* string);
hbk_string_view hbk_string_as_view(hbk_string string);

void hbk_string_append_format(hbk_string* string, const char* format, ...);
void hbk_string_append_formatv(hbk_string* string, const char* format, va_list v);

hbk_state* hbk_state_create();
void hbk_state_destroy(hbk_state* state);
void hbk_state_set_enable_color(hbk_state* state, bool use_color);
hbk_source_id hbk_state_add_source_from_file(hbk_state* state, const char* file_path);
hbk_string_view hbk_state_get_source_name(hbk_state* state, hbk_source_id source_id);
hbk_string_view hbk_state_get_source_text(hbk_state* state, hbk_source_id source_id);

hbk_location hbk_location_create(hbk_source_id source_id, int64_t offset, int64_t length);

hbk_diagnostic hbk_diagnostic_create(hbk_state* state, hbk_diagnostic_kind kind, hbk_location location, const char* message);
hbk_diagnostic hbk_diagnostic_create_format(hbk_state* state, hbk_diagnostic_kind kind, hbk_location location, const char* format, ...);
hbk_diagnostic hbk_diagnostic_create_formatv(hbk_state* state, hbk_diagnostic_kind kind, hbk_location location, const char* format, va_list v);
void hbk_diagnostic_add_related(hbk_diagnostic* diag, hbk_diagnostic related);

int64_t hbk_diagnostic_get_related_count(hbk_diagnostic diag);
//hbk_diagnostic hbk_diagnostic_get_related_at_index(hbk_diagnostic diag, int64_t index);

void hbk_diagnostic_render_to_string(hbk_state* state, hbk_diagnostic diag, hbk_string* string);

#endif // !HIBIKU_H
