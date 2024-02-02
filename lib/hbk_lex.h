#ifndef HKB_LEX_H
#define HKB_LEX_H

#include "hbk_internal.h"

#include <hibiku.h>
#include <stdint.h>

/// X-Macro for token kinds (https://en.wikipedia.org/wiki/X_macro)
#define HBK_TOKEN_KW_KINDS(X) \
    X(AND, "and")             \
    X(OR, "or")               \
    X(NOT, "not")             \
    X(TRUE, "true")           \
    X(FALSE, "false")         \
    X(NIL, "nil")             \
    X(IMPORT, "import")       \
    X(FUNCTION, "function")   \
    X(CONST, "const")         \
    X(LOCAL, "local")         \
    X(IF, "if")               \
    X(ELSE, "else")           \
    X(WHILE, "while")         \
    X(FOR, "for")             \
    X(RETURN, "return")       \
    X(BREAK, "break")         \
    X(CONTINUE, "continue")   \
    X(INT, "int")             \
    X(STRING, "string")       \
    X(BOOL, "bool")           \
    X(FLOAT, "float")

#define HBK_TOKEN_KINDS(X) \
    X(PLUSPLUS)            \
    X(MINUSMINUS)          \
    X(LESSLESS)            \
    X(GREATERGREATER)      \
    X(EQUALEQUAL)          \
    X(BANGEQUAL)           \
    X(PLUSEQUAL)           \
    X(MINUSEQUAL)          \
    X(SLASHEQUAL)          \
    X(STAREQUAL)           \
    X(PERCENTEQUAL)        \
    X(LESSEQUAL)           \
    X(GREATEREQUAL)        \
    X(AMPERSANDEQUAL)      \
    X(PIPEEQUAL)           \
    X(TILDEEQUAL)          \
    X(LESSLESSEQUAL)       \
    X(GREATERGREATEREQUAL) \
    X(EQUALGREATER)        \
    X(STRING_LITERAL)      \
    X(CHARACTER_LITERAL)   \
    X(INTEGER_LITERAL)     \
    X(IDENTIFIER)          \
    HBK_TOKEN_KW_KINDS(X)

typedef enum hbk_token_kind {
    HBK_TOKEN_INVALID = 0,
    HBK_TOKEN_EOF = 1,

    __HBK_TOKEN_MULTIBYTE_START__ = 255,
#define TK(N, ...) HBK_TOKEN_##N,
    HBK_TOKEN_KINDS(TK)
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
hbk_vector(hbk_token) hbk_lex(hbk_state* state, hbk_source_id source_id);

#endif // !HKB_LEX_H
