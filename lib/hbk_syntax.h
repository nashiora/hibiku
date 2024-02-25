#ifndef HBK_PARSE_H
#define HBK_PARSE_H

#include "hbk_internal.h"
#include "hbk_lex.h"

#include <hibiku.h>
#include <stdint.h>

/// X-Macro for token kinds (https://en.wikipedia.org/wiki/X_macro)
#define HBK_SYNTAX_KINDS(X) \
    X(DECL_FUNCTION)        \
    X(DECL_VARIABLE)        \
    X(STMT_COMPOUND)        \
    X(INTEGER_LITERAL)      \
    X(FLOAT_LITERAL)        \
    X(BOOL_LITERAL)         \
    X(STRING_LITERAL)       \
    X(TYPE_INTEGER)

typedef enum hbk_syntax_kind {
    HBK_SYNTAX_INVALID,

#define SN(N) HBK_SYNTAX_##N,
    HBK_SYNTAX_KINDS(SN)
#undef SN
} hbk_syntax_kind;

typedef struct hbk_syntax_tree hbk_syntax_tree;
typedef struct hbk_syntax hbk_syntax;

struct hbk_syntax_tree {
    hbk_arena* arena;

    hbk_source_id source_id;
    hbk_vector(hbk_syntax*) syntax_nodes;
};

// If you don't know what tagged unions are: https://en.wikipedia.org/wiki/Tagged_union
struct hbk_syntax {
    hbk_syntax_kind kind;
    hbk_location location;

    union {
        struct {
            hbk_token token;
        } invalid;

        struct {
            int64_t integer_value;
            double float_value;
            bool bool_value;
            hbk_string_view string_value;
        } literal;

        struct {
            bool is_exported;
            hbk_string_view name;
            hbk_syntax* type;
            hbk_syntax* initial_value;
        } decl_variable;
    };
};

const char* hbk_syntax_kind_to_cstring(hbk_syntax_kind kind);

void hbk_syntax_tree_destroy(hbk_syntax_tree* tree);
void hbk_syntax_tree_print_to_string(hbk_state* state, hbk_syntax_tree* tree, hbk_string* out_string, bool use_color);

hbk_syntax* hbk_syntax_create(hbk_syntax_tree* tree, hbk_syntax_kind kind, hbk_location location);
void hbk_syntax_type_print_to_string(hbk_state* state, hbk_syntax* type, hbk_string* out_string, bool use_color);

hbk_syntax_tree* hbk_parse(hbk_state* state, hbk_source_id source_id);

#endif // !HBK_PARSE_H
