#include <hibiku.h>

#include "hbk_api.h"
#include "hbk_lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct hbk_source_file {
    const char* name;
    hbk_string text;
    hbk_vector(hbk_token) tokens;
} hbk_source_file;

struct hbk_state {
    hbk_vector(hbk_source_file) source_files;
};

hbk_state* hbk_state_create(void) {
    hbk_state* state = calloc(sizeof *state, 1);
    HBK_ASSERT(state != NULL, "Buy more ram lol");
    return state;
}

static hbk_string read_file_as_string(const char* file_path) {
    FILE* f = fopen(file_path, "r");
    // TODO(local): handle errors for file not existing, or being unopenable for other reasons
    HBK_ASSERT(f != NULL, "Could not open source files (TODO: error handling)");
    fseek(f, 0, SEEK_END);
    int64_t file_length = (int64_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    hbk_string source_text = NULL;
    hbk_vector_set_count(source_text, file_length + 1);
    HBK_ASSERT(source_text != NULL, "Failed to allocate string data");
    HBK_ASSERT(hbk_vector_count(source_text) == file_length + 1, "Failed to allocate correct count for string data");
    fread(source_text, 1, (size_t)file_length, f);
    HBK_ASSERT(source_text[file_length] == 0, "Source text string was not NUL-terminated");
    
    fclose(f);
    return source_text;
}

hbk_source_id hbk_state_add_source_from_file(hbk_state* state, const char* file_path) {
    HBK_ASSERT(state != NULL, "Invalid state pointer");
    HBK_ASSERT(file_path != NULL, "Invalid file_path pointer");

    hbk_source_file source_file = {};

    for (int64_t i = 0; i < hbk_vector_count(state->source_files); i++) {
        hbk_source_file source_file = state->source_files[i];
        if (0 == strcmp(file_path, source_file.name)) {
            return (hbk_source_id)i;
        }
    }

    hbk_string source_text = read_file_as_string(file_path);
    source_file = (hbk_source_file){
        .name = file_path,
        .text = source_text,
    };

    hbk_vector_push(state->source_files, source_file);
    hbk_source_id source_id = (hbk_source_id)(hbk_vector_count(state->source_files) - 1);

    state->source_files[source_id].tokens = hbk_read_tokens(state, source_id);
    for (int64_t i = 0; i < hbk_vector_count(state->source_files[source_id].tokens); i++) {
        hbk_token token = state->source_files[source_id].tokens[i];
        fprintf(stderr, "INVALID: %.*s\n", (int)token.string_value.count, token.string_value.data);
    }

    return source_id;
}

const char* hbk_state_get_source_name(hbk_state* state, hbk_source_id source_id) {
    HBK_ASSERT(state != NULL, "Invalid state pointer");
    HBK_ASSERT(source_id >= 0, "Invalid source id");
    return state->source_files[source_id].name;
}

const char* hbk_state_get_source_text(hbk_state* state, hbk_source_id source_id) {
    HBK_ASSERT(state != NULL, "Invalid state pointer");
    HBK_ASSERT(source_id >= 0, "Invalid source id");
    return state->source_files[source_id].text;
}

hbk_string_view hbk_state_get_source_text_as_view(hbk_state* state, hbk_source_id source_id) {
    HBK_ASSERT(state != NULL, "Invalid state pointer");
    HBK_ASSERT(source_id >= 0, "Invalid source id");
    return (hbk_string_view){
        .data = state->source_files[source_id].text,
        .count = hbk_vector_count(state->source_files[source_id].text),
    };
}
