#include "hbk_api.h"
#include "hbk_lex.h"

#include <stddef.h>

typedef struct hbk_lexer {
    hbk_state* state;
    hbk_source_id source_id;

    /// @brief The source text being read by the lexer.
    hbk_string_view text;
    /// @brief The current character index within the source text.
    int64_t position;
} hbk_lexer;

/// @brief Return the character at the current position of this lexer.
/// @return The current character if not at the end of the file, otherwise 0.
static int hbk_lexer_current_char(hbk_lexer* l) {
    HBK_ASSERT(l != NULL, "Invalid lexer pointer");

    if (l->position < 0 || l->position >= l->text.count) {
        return 0;
    }

    return l->text.data[l->position];
}

/// @brief Moves the position of this lexer to the next character.
static void hbk_lexer_advance(hbk_lexer* l) {
    HBK_ASSERT(l != NULL, "Invalid lexer pointer");

    if (l->position >= l->text.count) {
        l->position = l->text.count;
    } else {
        l->position++;
    }
}

static bool hbk_lexer_is_eof(hbk_lexer* l) {
    HBK_ASSERT(l != NULL, "Invalid lexer pointer");
    return l->position >= l->text.count;
}

static hbk_string_view hbk_lexer_view_from_location(hbk_lexer* l, hbk_location location) {
    return (hbk_string_view) {
        .data = l->text.data + location.offset,
        .count = location.length,
    };
}

static hbk_token hbk_lexer_read_token(hbk_lexer* l) {
    HBK_ASSERT(l != NULL, "Invalid lexer pointer");

    hbk_token token = {
        .location = hbk_location_create(l->source_id, l->position, 1),
    };

    switch (hbk_lexer_current_char(l)) {
        default: {
            hbk_lexer_advance(l);
            token.string_value = hbk_lexer_view_from_location(l, token.location);
            // TODO(local): report invalid character with diagnostic API
        } break;
    }

    return token;
}

hbk_vector(hbk_token) hbk_read_tokens(hbk_state* state, hbk_source_id source_id) {
    hbk_lexer lexer = {
        .state = state,
        .source_id = source_id,
    };

    lexer.text = hbk_state_get_source_text_as_view(state, source_id);
    HBK_ASSERT(lexer.text.data != NULL, "Invalid lexer source text");
    HBK_ASSERT(lexer.text.data[lexer.text.count] != 0, "Invalid lexer source text (not NUL-terminated)");

    hbk_vector(hbk_token) tokens = NULL;
    while (!hbk_lexer_is_eof(&lexer)) {
        hbk_token token = hbk_lexer_read_token(&lexer);
        hbk_vector_push(tokens, token);
    }

    return tokens;
}
