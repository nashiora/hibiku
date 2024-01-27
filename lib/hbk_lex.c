#include "hbk_api.h"
#include "hbk_lex.h"

#include <stddef.h>

typedef struct hbk_lexer {
    const char* file_name;
    /// @brief The source text being read by the lexer.
    hbk_string_view text;
    /// @brief The current character index within the source text.
    int64_t position;
} hbk_lexer;

/// @brief Return the character at the current position of this lexer.
/// @return The current character if not at the end of the file, otherwise 0.
static int hbk_lexer_current_char(hbk_lexer* l) {
    HBK_ASSERT(l != NULL, "Invalid lexer pointer");
    HBK_ASSERT(l->text.data != NULL, "Invalid lexer source text");

    if (l->position < 0 || l->position >= l->text.count) {
        return 0;
    }

    return l->text.data[l->position];
}

hbk_vector(hbk_token) hbk_read_tokens(const char* file_path) {
    hbk_lexer lexer = {
        .file_name = file_path,
    };

    hbk_vector(hbk_token) tokens = NULL;
    return tokens;
}
