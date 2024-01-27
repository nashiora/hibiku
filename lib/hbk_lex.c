#include "hbk_lex.h"

#include "hbk_api.h"

#include <stddef.h>
#include <string.h>

const char* hbk_token_kind_to_cstring(hbk_token_kind kind) {
    switch (kind) {
        case HBK_TOKEN_INVALID: return "INVALID";

#define X(N) \
    case HBK_TOKEN_##N: return #N;
            HBK_TOKEN_KINDS(X)
#undef X

        default: {
            HBK_ASSERT(kind > 0 && kind < 256, "Invalid/unknown hbk_token_kind, cannot stringify it");

            static bool characters_initialized = false;
            static char characters[256 * 2] = {};

            if (!characters_initialized) {
                characters_initialized = true;
                for (int i = 0; i < 256; i++) {
                    characters[i * 2] = (char)i;
                }
            }

            return &characters[(int)kind * 2];
        }
    }
}

typedef struct keyword_info {
    hbk_token_kind kind;
    const char* keyword_image;
} keyword_info;

static keyword_info keyword_infos[] = {
    {HBK_TOKEN_FUNCTION, "function"},
    {HBK_TOKEN_RETURN, "return"},
    {HBK_TOKEN_INT, "int"},
    {HBK_TOKEN_LOCAL, "local"},
    {0, NULL}
};

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
    return (hbk_string_view){
        .data = l->text.data + location.offset,
        .count = location.length,
    };
}

static bool is_identifier_start(int c) {
    /// We know the ranges of ASCII values for these characters, and they are continuous.
    /// 'a' is always the first lowercase letter, and 'z' is always the last. This is
    /// true for capital letters as well as numbers.
    /// Our identifiers in Hibiku can only start with these three character ranges.
    /// (pretend that the underscore is a range of length 1, I guess).
    /// Optionally, we've also said any non-ASCII character is a valid identifier character, too.
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c >= 256;
}

static bool is_identifier_part(int c) {
    /// The same as identifier start, but including digits.
    /// Identifiers cannot start with numbers, but they may contain them.
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c >= 256;
}

static bool is_digit(int c) {
    return (c >= '0' && c <= '9');
}

static bool is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v';
}

static void hbk_lexer_skip_whitespace(hbk_lexer* l) {
    while (!hbk_lexer_is_eof(l)) {
        if (is_space(hbk_lexer_current_char(l))) {
            hbk_lexer_advance(l);
        } else {
            break;
        }
    }
}

static hbk_token hbk_lexer_read_token(hbk_lexer* l) {
    HBK_ASSERT(l != NULL, "Invalid lexer pointer");

    hbk_token token = {
        .location = hbk_location_create(l->source_id, l->position, 1),
    };

    switch (hbk_lexer_current_char(l)) {
        case '~':
        case '%':
        case '&':
        case '*':
        case '(':
        case ')':
        case '-':
        case '+':
        case '=':
        case '[':
        case ']':
        case '{':
        case '}':
        case '|':
        case ',':
        case '.':
        case '<':
        case '>':
        case ';': {
            token.kind = (hbk_token_kind)hbk_lexer_current_char(l);
            hbk_lexer_advance(l);
        } break;

        default: {
            if (is_identifier_start(hbk_lexer_current_char(l))) {
                token.location.length = 0;
                while (is_identifier_part(hbk_lexer_current_char(l))) {
                    hbk_lexer_advance(l);
                    token.location.length++;
                }

                token.kind = HBK_TOKEN_IDENTIFIER;
                token.string_value = hbk_lexer_view_from_location(l, token.location);

                for (int64_t i = 0; keyword_infos[i].kind != 0 && token.kind == HBK_TOKEN_IDENTIFIER; i++) {
                    if (0 == strncmp(keyword_infos[i].keyword_image, token.string_value.data, token.string_value.count)) {
                        token.kind = keyword_infos[i].kind;
                        token.string_value = (hbk_string_view){};
                    }
                }
            } else if (is_digit(hbk_lexer_current_char(l))) {
                token.location.length = 0;
                while (is_digit(hbk_lexer_current_char(l))) {
                    /// get the digit value. We know this character is an ASCII digit, so just get the
                    /// difference between the two. '0' - '0' = 0, '1' - '0' = 1, etc.
                    int digit_value = hbk_lexer_current_char(l) - '0';
                    token.integer_value *= 10;
                    token.integer_value += digit_value;

                    hbk_lexer_advance(l);
                    token.location.length++;
                }

                token.kind = HBK_TOKEN_INTEGER_LITERAL;
            } else {
                hbk_lexer_advance(l);
                token.string_value = hbk_lexer_view_from_location(l, token.location);
                // TODO(local): report invalid character with diagnostic API
            }
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

    hbk_lexer_skip_whitespace(&lexer);
    while (!hbk_lexer_is_eof(&lexer)) {
        hbk_token token = hbk_lexer_read_token(&lexer);
        hbk_lexer_skip_whitespace(&lexer);

        hbk_vector_push(tokens, token);
    }

    return tokens;
}
