#include "hbk_internal.h"
#include "hbk_parse.h"

#include <hibiku.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct hbk_source {
    hbk_string_view name;
    hbk_string text;
} hbk_source;

struct hbk_state {
    bool use_color;
    hbk_vector(hbk_source) sources;
    hbk_vector(hbk_string_view) interned_strings;
    hbk_vector(hbk_diagnostic*) diagnostics;
    hbk_pool* misc_pool;
    hbk_pool* string_pool;
};

hbk_string_view hbk_cstring_as_view(const char* string) {
    int64_t count = (int64_t)strlen(string);
    return (hbk_string_view){
        .data = string,
        .count = count,
    };
}

void hbk_string_append_format(hbk_string* string, const char* format, ...) {
    va_list v;
    va_start(v, format);
    hbk_string_append_formatv(string, format, v);
    va_end(v);
}

void hbk_string_append_formatv(hbk_string* string, const char* format, va_list v) {
    va_list vcopy;
    va_copy(vcopy, v);
    int64_t message_length = (int64_t)vsnprintf(NULL, 0, format, vcopy);
    va_end(vcopy);

    int64_t count = hbk_vector_count(*string);
    hbk_vector_set_capacity(*string, count + message_length + 1);
    hbk_vector_set_count(*string, count + message_length);
    vsnprintf(*string + count, message_length + 1, format, v);
}

hbk_string_view hbk_string_as_view(hbk_string string) {
    int64_t count = hbk_vector_count(string);
    return (hbk_string_view){
        .data = string,
        .count = count,
    };
}

hbk_state* hbk_state_create() {
    hbk_state* state = calloc(sizeof *state, 1);
    HBK_ASSERT(state != NULL, "Buy more ram lol");
    state->misc_pool = hbk_pool_create();
    state->string_pool = hbk_pool_create();
    return state;
}

void hbk_state_destroy(hbk_state* state) {
    if (state == NULL) return;
    for (int64_t i = 0; i < hbk_vector_count(state->sources); i++) {
        hbk_vector_free(state->sources[i].text);
    }
    hbk_vector_free(state->sources);
    hbk_vector_free(state->interned_strings);
    hbk_vector_free(state->diagnostics);
    hbk_pool_destroy(state->misc_pool);
    hbk_pool_destroy(state->string_pool);
    free(state);
}

void hbk_state_set_enable_color(hbk_state* state, bool use_color) {
    state->use_color = use_color;
}

static hbk_string read_file_as_string(const char* file_path) {
    FILE* f = fopen(file_path, "r");
    // TODO(local): handle errors for file not existing, or being unopenable for other reasons
    HBK_ASSERT(f != NULL, "Could not open source files (TODO: error handling)");
    fseek(f, 0, SEEK_END);
    int64_t file_length = (int64_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    hbk_string source_text = NULL;
    hbk_vector_set_capacity(source_text, file_length + 1);
    HBK_ASSERT(source_text != NULL, "Failed to allocate string data");
    HBK_ASSERT(hbk_vector_count(source_text) == file_length + 1, "Failed to allocate correct count for string data");
    fread(source_text, 1, (size_t)file_length, f);
    hbk_vector_set_count(source_text, file_length);
    HBK_ASSERT(source_text[file_length] == 0, "Source text string was not NUL-terminated");

    fclose(f);
    return source_text;
}

hbk_source_id hbk_state_add_source_from_file(hbk_state* state, const char* file_path) {
    HBK_ASSERT(state != NULL, "Invalid state pointer");
    HBK_ASSERT(file_path != NULL, "Invalid file_path pointer");

    hbk_source source_file = {};

    for (int64_t i = 0; i < hbk_vector_count(state->sources); i++) {
        hbk_source source_file = state->sources[i];
        if (0 == strncmp(file_path, source_file.name.data, source_file.name.count)) {
            return (hbk_source_id)i;
        }
    }

    hbk_string source_text = read_file_as_string(file_path);
    source_file = (hbk_source){
        .name = hbk_cstring_as_view(file_path),
        .text = source_text,
    };

    hbk_vector_push(state->sources, source_file);
    hbk_source_id source_id = (hbk_source_id)(hbk_vector_count(state->sources) - 1);

#if 0
    state->sources[source_id].tokens = hbk_lex(state, source_id);
    for (int64_t i = 0; i < hbk_vector_count(state->sources[source_id].tokens); i++) {
        hbk_token token = state->sources[source_id].tokens[i];
        fprintf(stderr, "%s [%ld,%ld]", hbk_token_kind_to_cstring(token.kind), token.location.offset, token.location.length);
        if (token.kind == HBK_TOKEN_INTEGER_LITERAL) {
            fprintf(stderr, " %ld", token.integer_value);
        } else if (token.kind == HBK_TOKEN_CHARACTER_LITERAL) {
            fprintf(stderr, " %c", (char)token.integer_value);
        } else if (token.kind == HBK_TOKEN_STRING_LITERAL) {
            fprintf(stderr, " \"%.*s\"", (int)token.string_value.count, token.string_value.data);
        } else if (token.string_value.count != 0) {
            fprintf(stderr, " [%.*s]", (int)token.string_value.count, token.string_value.data);
        }
        fprintf(stderr, "\n");
    }
    hbk_vector_free(state->sources[source_id].tokens);
#endif

    hbk_syntax_tree* tree = hbk_parse(state, source_id);
    HBK_ASSERT(tree != NULL, "parser did not return a tree");

    hbk_string debug_output = NULL;
    hbk_syntax_tree_print_to_string(tree, &debug_output, state->use_color);
    fprintf(stderr, "%s\n", debug_output);
    hbk_vector_free(debug_output);

    return source_id;
}

hbk_string_view hbk_state_get_source_name(hbk_state* state, hbk_source_id source_id) {
    HBK_ASSERT(state != NULL, "Invalid state pointer");
    HBK_ASSERT(source_id >= 0, "Invalid source id");
    return state->sources[source_id].name;
}

hbk_string_view hbk_state_get_source_text(hbk_state* state, hbk_source_id source_id) {
    HBK_ASSERT(state != NULL, "Invalid state pointer");
    HBK_ASSERT(source_id >= 0, "Invalid source id");
    return hbk_string_as_view(state->sources[source_id].text);
}

void hbk_state_render_diagnostics_to_file(hbk_state* state, FILE* file) {
    hbk_string render_target = NULL;
    for (int64_t i = 0; i < hbk_vector_count(state->diagnostics); i++) {
        auto diag = state->diagnostics[i];
        if (diag->kind == HBK_DIAG_RELATED) {
            continue;
        }

        hbk_diagnostic_render_to_string(state, diag, &render_target);
    }

    if (render_target != NULL) {
        fprintf(file, "%s", render_target);
    }

    hbk_vector_free(render_target);
}

hbk_string_view hbk_state_intern_string_data(hbk_state* state, const char* string, int64_t length) {
    for (int64_t i = 0; i < hbk_vector_count(state->interned_strings); i++) {
        if (state->interned_strings[i].count != length) {
            continue;
        }

        if (0 == strncmp(state->interned_strings[i].data, string, length)) {
            return state->interned_strings[i];
        }
    }

    char* data = hbk_pool_alloc(state->string_pool, length + 1);
    memcpy(data, string, (size_t)length);

    return (hbk_string_view){
        .data = data,
        .count = length,
    };
}

hbk_string_view hbk_state_intern_string(hbk_state* state, hbk_string string) {
    int64_t length = hbk_vector_count(string);
    return hbk_state_intern_string_data(state, string, length);
}

hbk_string_view hbk_state_intern_string_view(hbk_state* state, hbk_string_view sv) {
    return hbk_state_intern_string_data(state, sv.data, sv.count);
}

hbk_string_view hbk_state_intern_cstring(hbk_state* state, const char* string) {
    int64_t length = (int64_t)strlen(string);
    return hbk_state_intern_string_data(state, string, length);
}

hbk_location hbk_location_create(hbk_source_id source_id, int64_t offset, int64_t length) {
    return (hbk_location){
        .source_id = source_id,
        .offset = offset,
        .length = length,
    };
}

hbk_diagnostic* hbk_diagnostic_create(hbk_state* state, hbk_diagnostic_kind kind, hbk_location location, const char* message) {
    hbk_string_view message_view = hbk_state_intern_cstring(state, message);

    hbk_diagnostic* result = hbk_pool_alloc(state->misc_pool, sizeof *result);
    hbk_vector_push(state->diagnostics, result);
    *result = (hbk_diagnostic){
        .kind = kind,
        .location = location,
        .message = message_view,
    };

    return result;
}

hbk_diagnostic* hbk_diagnostic_create_format(hbk_state* state, hbk_diagnostic_kind kind, hbk_location location, const char* format, ...) {
    va_list v;
    va_start(v, format);
    hbk_diagnostic* result = hbk_diagnostic_create_formatv(state, kind, location, format, v);
    va_end(v);
    return result;
}

hbk_diagnostic* hbk_diagnostic_create_formatv(hbk_state* state, hbk_diagnostic_kind kind, hbk_location location, const char* format, va_list v) {
    va_list vcopy;
    va_copy(vcopy, v);
    int64_t message_length = (int64_t)vsnprintf(NULL, 0, format, vcopy);
    va_end(vcopy);

    char* message_data = calloc(message_length + 1, 1);
    vsnprintf(message_data, message_length + 1, format, v);

    hbk_string_view message_view = hbk_state_intern_string_data(state, message_data, message_length);
    free(message_data);

    hbk_diagnostic* result = hbk_pool_alloc(state->misc_pool, sizeof *result);
    hbk_vector_push(state->diagnostics, result);
    *result = (hbk_diagnostic){
        .kind = kind,
        .location = location,
        .message = message_view,
    };

    return result;
}

void hbk_diagnostic_add_related(hbk_diagnostic* diag, hbk_diagnostic* related) {
    HBK_ASSERT(diag != NULL, "Invalid diagnostic pointer");
    related->kind = HBK_DIAG_RELATED;
    hbk_vector_push(diag->related_diagnostics, related);
}

void hbk_diagnostic_render_to_string(hbk_state* state, hbk_diagnostic* diag, hbk_string* string) {
    HBK_ASSERT(state != NULL, "Invalid state pointer");
    HBK_ASSERT(string != NULL, "Invalid string pointer");
    HBK_ASSERT(*string != NULL, "Invalid string pointer");

    hbk_source_id source_id = diag->location.source_id;
    hbk_string_view source_name = hbk_state_get_source_name(state, source_id);
    // hbk_string_view source_text = hbk_state_get_source_text(state, source_id);

    const char* diag_kind_color = "";
    const char* diag_kind_text = "";

    bool use_color = state->use_color;

    switch (diag->kind) {
        default: HBK_UNREACHABLE;

        case HBK_DIAG_VERBOSE: {
            diag_kind_color = COL(CYAN);
            diag_kind_text = "verbose";
        } break;

        case HBK_DIAG_DEBUG: {
            diag_kind_color = COL(YELLOW);
            diag_kind_text = "debug";
        } break;

        case HBK_DIAG_INFO: {
            diag_kind_color = COL(GREEN);
            diag_kind_text = "note";
        } break;

        case HBK_DIAG_WARNING: {
            diag_kind_color = COL(MAGENTA);
            diag_kind_text = "warning";
        } break;

        case HBK_DIAG_ERROR: {
            diag_kind_color = COL(RED);
            diag_kind_text = "error";
        } break;

        case HBK_DIAG_FATAL: {
            diag_kind_color = COL(BRIGHT_RED);
            diag_kind_text = "fatal";
        } break;
    }

    hbk_string_append_format(
        string,
        "%.*s[%lld:%lld]: %s%s:%s %.*s\n",
        HBK_SV_EXPAND(source_name),
        (long long)diag->location.offset,
        (long long)diag->location.length,
        diag_kind_color,
        diag_kind_text,
        COL(RESET),
        HBK_SV_EXPAND(diag->message)
    );
}
