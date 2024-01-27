#ifndef HKB_LEX_H
#define HKB_LEX_H

#include "hbk_api.h"

#include <hibiku.h>
#include <stdint.h>

/// X-Macro for token kinds (https://en.wikipedia.org/wiki/X_macro)
#define HBK_TOKEN_KINDS(TK, KW) \
    TK(PLUSPLUS)                \
    TK(MINUSMINUS)              \
    TK(LESSLESS)                \
    TK(GREATERGREATER)          \
    TK(EQUALEQUAL)              \
    TK(BANGEQUAL)               \
    TK(PLUSEQUAL)               \
    TK(MINUSEQUAL)              \
    TK(SLASHEQUAL)              \
    TK(STAREQUAL)               \
    TK(PERCENTEQUAL)            \
    TK(LESSEQUAL)               \
    TK(GREATEREQUAL)            \
    TK(AMPERSANDEQUAL)          \
    TK(PIPEEQUAL)               \
    TK(TILDEEQUAL)              \
    TK(LESSLESSEQUAL)           \
    TK(GREATERGREATEREQUAL)     \
    TK(EQUALGREATER)            \
    TK(STRING_LITERAL)          \
    TK(CHARACTER_LITERAL)       \
    TK(INTEGER_LITERAL)         \
    TK(IDENTIFIER)              \
    KW(AND, "and")              \
    KW(OR, "or")                \
    KW(NOT, "not")              \
    KW(TRUE, "true")            \
    KW(FALSE, "false")          \
    KW(NIL, "nil")              \
    KW(IMPORT, "import")        \
    KW(FUNCTION, "function")    \
    KW(CONST, "const")          \
    KW(LOCAL, "local")          \
    KW(IF, "if")                \
    KW(ELSE, "else")            \
    KW(WHILE, "while")          \
    KW(FOR, "for")              \
    KW(RETURN, "return")        \
    KW(BREAK, "break")          \
    KW(CONTINUE, "continue")    \
    KW(INT, "int")              \
    KW(STRING, "string")        \
    KW(BOOL, "bool")            \
    KW(FLOAT, "float")

typedef enum hbk_token_kind {
    HBK_TOKEN_INVALID = 0,

    __HBK_TOKEN_MULTIBYTE_START__ = 255,
#define TK(N)      HBK_TOKEN_##N,
#define KW(N, ...) HBK_TOKEN_##N,
    HBK_TOKEN_KINDS(TK, KW)
#undef KW
#undef TK
} hbk_token_kind;

typedef struct hbk_token {
    hbk_token_kind kind;
    hbk_location location;
    int64_t integer_value;
    hbk_string_view string_value;
} hbk_token;

/// @brief Get a constant C string name for the token kind.
const char* hbk_token_kind_to_cstring(hbk_token_kind kind);

/// @brief Reads all of the tokens from the source text into a vector.
/// For simplicity in implementing other parts of this compiler,
/// we don't support reading individual tokens at a time.
hbk_vector(hbk_token) hbk_read_tokens(hbk_state* state, hbk_source_id source_id);

#endif // !HKB_LEX_H
