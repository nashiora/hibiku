#include "hbk_syntax.h"

typedef struct hbk_parser {
    hbk_state* state;
    hbk_source_id source_id;
    hbk_string_view source_text;

    hbk_vector(hbk_token) tokens;
    int64_t current_index;

    hbk_syntax_tree* tree;
} hbk_parser;

const char* hbk_syntax_kind_to_cstring(hbk_syntax_kind kind) {
    switch (kind) {
        case HBK_SYNTAX_INVALID: return "INVALID";

#define SN(N) \
    case HBK_SYNTAX_##N: return #N;
        HBK_SYNTAX_KINDS(SN)
#undef SN

        default: HBK_UNREACHABLE; return NULL;
    }
}

hbk_syntax* hbk_parse_decl(hbk_parser* p);
hbk_syntax* hbk_parse_decl_function(hbk_parser* p, hbk_token export_token);
hbk_syntax* hbk_parse_decl_variable(hbk_parser* p, hbk_token decl_token);

hbk_syntax* hbk_parse_stmt(hbk_parser* p);
hbk_syntax* hbk_parse_stmt_compound(hbk_parser* p);

hbk_syntax* hbk_parse_type(hbk_parser* p);

hbk_syntax* hbk_parse_expr(hbk_parser* p);
hbk_syntax* hbk_parse_expr_primary(hbk_parser* p);

void hbk_parser_advance(hbk_parser* p) {
    HBK_ASSERT(p != NULL, "invalid parser pointer");
    p->current_index++;
}

hbk_token hbk_parser_peek(hbk_parser* p, int offset) {
    HBK_ASSERT(p != NULL, "invalid parser pointer");

    int64_t peek_index = p->current_index + offset;
    if (peek_index < 0 || peek_index >= hbk_vector_count(p->tokens)) {
        return (hbk_token){
            .location = hbk_location_create(p->source_id, p->source_text.count, 0),
            .kind = HBK_TOKEN_EOF,
        };
    }

    return p->tokens[peek_index];
}

hbk_token hbk_parser_token(hbk_parser* p) {
    return hbk_parser_peek(p, 0);
}

hbk_location hbk_parser_location(hbk_parser* p) {
    return hbk_parser_token(p).location;
}

bool hbk_parser_at(hbk_parser* p, hbk_token_kind kind) {
    return hbk_parser_token(p).kind == kind;
}

bool hbk_parser_peek_at(hbk_parser* p, hbk_token_kind kind, int offset) {
    return hbk_parser_peek(p, offset).kind == kind;
}

bool hbk_parser_consume(hbk_parser* p, hbk_token_kind kind) {
    if (hbk_parser_at(p, kind)) {
        hbk_parser_advance(p);
        return true;
    }

    return false;
}

void hbk_parser_expect(hbk_parser* p, hbk_token_kind kind, hbk_token* out_token) {
    if (out_token != NULL) {
        *out_token = hbk_parser_token(p);
    }

    if (!hbk_parser_consume(p, kind)) {
        if (kind < 256) {
            hbk_diagnostic_create_format(p->state, HBK_DIAG_ERROR, hbk_parser_token(p).location, "Expected '%c'.", kind);
        } else {
            const char* suffix = NULL;
            switch (kind) {
                default: HBK_ASSERT(false, "Unexpected token kind when calling hbk_parser_expect");
                case HBK_TOKEN_IDENTIFIER: {
                    suffix = "an identifier";
                } break;
            }

            HBK_ASSERT(suffix != NULL, "error message 'suffix' was not set");
            hbk_diagnostic_create_format(p->state, HBK_DIAG_ERROR, hbk_parser_token(p).location, "Expected %s.", suffix);
        }
    }
}

void hbk_parser_expect_semi(hbk_parser* p) {
    if (!hbk_parser_consume(p, ';')) {
        hbk_diagnostic_create(p->state, HBK_DIAG_ERROR, hbk_parser_token(p).location, "Expected ';'.");
    }
}

hbk_syntax_tree* hbk_syntax_tree_create() {
    hbk_arena* tree_arena = hbk_arena_create();
    HBK_ASSERT(tree_arena != NULL, "buy more ram");

    hbk_syntax_tree* tree = hbk_arena_alloc(tree_arena, sizeof *tree);
    HBK_ASSERT(tree != NULL, "buy more ram");
    tree->arena = tree_arena;

    return tree;
}

void hbk_syntax_tree_destroy(hbk_syntax_tree* tree) {
    if (tree == NULL) return;
    hbk_arena_destroy(tree->arena);
}

hbk_syntax* hbk_syntax_create(hbk_syntax_tree* tree, hbk_syntax_kind kind, hbk_location location) {
    HBK_ASSERT(tree != NULL, "invalid tree pointer");
    HBK_ASSERT(tree->arena != NULL, "invalid arena pointer");

    hbk_arena* arena = tree->arena;
    hbk_syntax* syntax = hbk_arena_alloc(arena, sizeof *syntax);
    HBK_ASSERT(syntax != NULL, "invalid syntax pointer");
    syntax->kind = kind;
    syntax->location = location;

    return syntax;
}

hbk_syntax* hbk_parse_decl(hbk_parser* p) {
    hbk_token token = hbk_parser_token(p);
    switch (token.kind) {
        case HBK_TOKEN_EXPORT: {
            hbk_parser_advance(p);
            if (hbk_parser_at(p, HBK_TOKEN_FUNCTION)) {
                return hbk_parse_decl_function(p, token);
            }

            if (!hbk_parser_at(p, HBK_TOKEN_IDENTIFIER)) {
                // TODO(local): Turn token kinds into more human-readable strings, not just the internal enum representation
                hbk_diagnostic_create_format(p->state, HBK_DIAG_ERROR, hbk_parser_location(p), "Expected an identifier to name this exported variable, but got %s.", hbk_token_kind_to_cstring(token.kind));
                hbk_syntax* invalid = hbk_syntax_create(p->tree, HBK_SYNTAX_INVALID, token.location);
                invalid->invalid.token = token;
                return invalid;
            }

            return hbk_parse_decl_variable(p, token);
        }

        case HBK_TOKEN_FUNCTION: {
            return hbk_parse_decl_function(p, (hbk_token){0});
        } break;

        case HBK_TOKEN_LOCAL: {
            hbk_parser_advance(p);

            if (!hbk_parser_at(p, HBK_TOKEN_IDENTIFIER)) {
                // TODO(local): Turn token kinds into more human-readable strings, not just the internal enum representation
                hbk_diagnostic_create_format(p->state, HBK_DIAG_ERROR, hbk_parser_location(p), "Expected an identifier to name this local variable, but got %s.", hbk_token_kind_to_cstring(token.kind));
                hbk_syntax* invalid = hbk_syntax_create(p->tree, HBK_SYNTAX_INVALID, token.location);
                invalid->invalid.token = token;
                return invalid;
            }

            return hbk_parse_decl_variable(p, token);
        }

        default: {
            // TODO(local): Turn token kinds into more human-readable strings, not just the internal enum representation
            hbk_diagnostic_create_format(p->state, HBK_DIAG_ERROR, hbk_parser_location(p), "Expected a declaration, but got %s.", hbk_token_kind_to_cstring(token.kind));
            hbk_parser_advance(p);

            hbk_syntax* invalid = hbk_syntax_create(p->tree, HBK_SYNTAX_INVALID, token.location);
            invalid->invalid.token = token;
            return invalid;
        }
    }
}

hbk_syntax* hbk_parse_decl_parameter(hbk_parser* p) {
    HBK_ASSERT(p != NULL, "invalid parser pointer");
    
    hbk_syntax* param_node = hbk_syntax_create(p->tree, HBK_SYNTAX_DECL_PARAMETER, hbk_parser_token(p).location);
    hbk_parser_expect(p, HBK_TOKEN_IDENTIFIER, &param_node->decl_parameter.name);

    if (hbk_parser_consume(p, ':')) {
        param_node->decl_parameter.type = hbk_parse_type(p);
        HBK_ASSERT(param_node->decl_parameter.type != NULL, "failed to parse type for parameter");
    }

    return param_node;
}

hbk_syntax* hbk_parse_decl_function(hbk_parser* p, hbk_token export_token) {
    // <decl-function> ::= <attribs> [ EXPORT ] FUNCTION IDENTIFIER "(" ")" [ ":" <type> ] <function-body>

    HBK_ASSERT(p != NULL, "invalid parser pointer");
    HBK_ASSERT(hbk_parser_token(p).kind == HBK_TOKEN_FUNCTION, "hbk_parse_decl_function must be called with the parser positioned at the 'function' keyword");

    hbk_syntax* func_node = hbk_syntax_create(p->tree, HBK_SYNTAX_DECL_FUNCTION, hbk_parser_token(p).location);
    hbk_parser_advance(p);

    hbk_parser_expect(p, HBK_TOKEN_IDENTIFIER, &func_node->decl_function.name);

    hbk_parser_expect(p, '(', NULL);
    while (!hbk_parser_at(p, HBK_TOKEN_EOF) && !hbk_parser_at(p, ')')) {
        hbk_syntax* param = hbk_parse_decl_parameter(p);
        HBK_ASSERT(param != NULL, "did not parse param ig");

        hbk_vector_push(func_node->decl_function.parameter_declarations, param);

        if (!hbk_parser_consume(p, ',')) {
            break;
        }
    }

    hbk_parser_expect(p, ')', NULL);

    if (hbk_parser_consume(p, ':')) {
        func_node->decl_function.return_type = hbk_parse_type(p);
        HBK_ASSERT(func_node->decl_function.return_type != NULL, "failed to parse type for parameter");
    }

    if (hbk_parser_consume(p, HBK_TOKEN_EQUALGREATER)) {
        hbk_syntax* return_value = hbk_parse_expr(p);
        HBK_ASSERT(return_value != NULL, "return value was null");

        func_node->decl_function.body = hbk_syntax_create(p->tree, HBK_SYNTAX_STMT_ARROW, return_value->location);
        func_node->decl_function.body->stmt_arrow.value = return_value;
    }

    hbk_parser_expect_semi(p);

    return func_node;
}

hbk_syntax* hbk_parse_decl_variable(hbk_parser* p, hbk_token decl_token) {
    HBK_ASSERT(p != NULL, "invalid parser pointer");
    HBK_ASSERT(decl_token.kind == HBK_TOKEN_LOCAL || decl_token.kind == HBK_TOKEN_EXPORT, "hibiku variable must start with either `local` or `export`");
    HBK_ASSERT(hbk_parser_at(p, HBK_TOKEN_IDENTIFIER), "hbk_parse_decl_variable expected to be at the variable name");

    hbk_token variable_token = hbk_parser_token(p);
    hbk_parser_advance(p);

    hbk_syntax* var_node = hbk_syntax_create(p->tree, HBK_SYNTAX_DECL_VARIABLE, variable_token.location);
    var_node->decl_variable.name = variable_token;

    if (hbk_parser_consume(p, ':')) {
        var_node->decl_variable.type = hbk_parse_type(p);
    }

    if (hbk_parser_consume(p, '=')) {
        var_node->decl_variable.default_value = hbk_parse_expr(p);
    }

    hbk_parser_expect_semi(p);
    return var_node;
}

hbk_syntax* hbk_parse_type(hbk_parser* p) {
    hbk_token token = hbk_parser_token(p);
    switch (token.kind) {
        case HBK_TOKEN_INT: {
            hbk_parser_advance(p);
            hbk_syntax* invalid = hbk_syntax_create(p->tree, HBK_SYNTAX_TYPE_INTEGER, token.location);
            return invalid;
        }

        default: {
            // TODO(local): Turn token kinds into more human-readable strings, not just the internal enum representation
            hbk_diagnostic_create_format(p->state, HBK_DIAG_ERROR, hbk_parser_location(p), "Expected a type, but got %s.", hbk_token_kind_to_cstring(token.kind));
            hbk_parser_advance(p);

            hbk_syntax* invalid = hbk_syntax_create(p->tree, HBK_SYNTAX_INVALID, token.location);
            invalid->invalid.token = token;
            return invalid;
        }
    }
}

hbk_syntax* hbk_parse_expr(hbk_parser* p) {
    return hbk_parse_expr_primary(p);
}

hbk_syntax* hbk_parse_expr_primary(hbk_parser* p) {
    HBK_ASSERT(p != NULL, "invalid parser pointer");

    hbk_token token = hbk_parser_token(p);
    switch (token.kind) {
        default: {
            // TODO(local): Turn token kinds into more human-readable strings, not just the internal enum representation
            hbk_diagnostic_create_format(p->state, HBK_DIAG_ERROR, hbk_parser_location(p), "Expected an expression, but got %s.", hbk_token_kind_to_cstring(token.kind));
            hbk_parser_advance(p);

            hbk_syntax* invalid = hbk_syntax_create(p->tree, HBK_SYNTAX_INVALID, token.location);
            invalid->invalid.token = token;
            return invalid;
        }

        case HBK_TOKEN_IDENTIFIER: {
            hbk_parser_advance(p);
            hbk_syntax* primary = hbk_syntax_create(p->tree, HBK_SYNTAX_IDENTIFIER, token.location);
            primary->identifier.name = token;
            return primary;
        }

        case HBK_TOKEN_INTEGER_LITERAL: {
            hbk_parser_advance(p);
            hbk_syntax* primary = hbk_syntax_create(p->tree, HBK_SYNTAX_INTEGER_LITERAL, token.location);
            primary->literal.integer_value = token.integer_value;
            return primary;
        }

        case HBK_TOKEN_CHARACTER_LITERAL: {
            hbk_parser_advance(p);
            hbk_syntax* primary = hbk_syntax_create(p->tree, HBK_SYNTAX_INTEGER_LITERAL, token.location);
            primary->literal.integer_value = token.integer_value;
            return primary;
        }

        case HBK_TOKEN_STRING_LITERAL: {
            hbk_parser_advance(p);
            hbk_syntax* primary = hbk_syntax_create(p->tree, HBK_SYNTAX_STRING_LITERAL, token.location);
            primary->literal.string_value = token.string_value;
            return primary;
        }

        case HBK_TOKEN_TRUE:
        case HBK_TOKEN_FALSE: {
            hbk_parser_advance(p);
            hbk_syntax* primary = hbk_syntax_create(p->tree, HBK_SYNTAX_BOOL_LITERAL, token.location);
            primary->literal.bool_value = token.kind == HBK_TOKEN_TRUE;
            return primary;
        }
    }
}

hbk_syntax_tree* hbk_parse(hbk_state* state, hbk_source_id source_id) {
    hbk_vector(hbk_token) tokens = hbk_lex(state, source_id);
    hbk_string_view source_text = hbk_state_get_source_text(state, source_id);

    hbk_syntax_tree* tree = hbk_syntax_tree_create();
    tree->source_id = source_id;

    hbk_parser parser = {
        .state = state,
        .source_id = source_id,
        .source_text = source_text,
        .tokens = tokens,
        .tree = tree,
    };

    while (!hbk_parser_at(&parser, HBK_TOKEN_EOF)) {
        int64_t last_parser_index = parser.current_index;
        hbk_syntax* parsed_syntax = hbk_parse_decl(&parser);
        HBK_ASSERT(parsed_syntax != NULL, "all parser routines must return a valid syntax node");
        HBK_ASSERT(last_parser_index < parser.current_index, "all parser routines must advance the token index by at least one");
        hbk_vector_push(tree->syntax_nodes, parsed_syntax);
    }

    return tree;
}

// ===== debug printing stuff =====

#define COL_TREE     MAGENTA
#define COL_ADDRESS  BRIGHT_BLACK
#define COL_LOCATION RED
#define COL_NAME     BLUE
#define COL_LITERAL  YELLOW
#define COL_KEYWORD  BRIGHT_MAGENTA

typedef struct hbk_syntax_print_context {
    hbk_state* state;
    hbk_string* output;
    hbk_string indents;
    bool use_color;
} hbk_syntax_print_context;

void hbk_syntax_print(hbk_syntax_print_context* print_context, hbk_syntax* node);

void hbk_syntax_print_children(hbk_syntax_print_context* print_context, hbk_vector(hbk_syntax*) children) {
    HBK_ASSERT(print_context != NULL, "invalid print context pointer");

    bool use_color = print_context->use_color;

    for (int64_t i = 0, count = hbk_vector_count(children); i < count; i++) {
        bool is_last = i == count - 1;

        hbk_syntax* child = children[i];
        int64_t old_indents_count = hbk_vector_count(print_context->indents);

        const char* next_leader = is_last ? "└─" : "├─";
        hbk_string_append_format(print_context->output, "%s%s%s", COL(COL_TREE), (print_context->indents ? print_context->indents : ""), next_leader);

        hbk_string_append_format(&print_context->indents, "%s", is_last ? "  " : "│ ");
        hbk_syntax_print(print_context, child);

        hbk_vector_set_count(print_context->indents, old_indents_count);
        print_context->indents[old_indents_count] = 0;
    }
}

void hbk_syntax_print(hbk_syntax_print_context* print_context, hbk_syntax* node) {
    HBK_ASSERT(print_context != NULL, "invalid print context pointer");
    HBK_ASSERT(node != NULL, "invalid hibiku syntax node pointer");

    bool use_color = print_context->use_color;

    hbk_string_append_format(
        print_context->output,
        "%s%s %s<%llX> %s[%lld:%lld]%s",
        COL(COL_TREE),
        hbk_syntax_kind_to_cstring(node->kind),
        COL(COL_ADDRESS),
        (size_t)node,
        COL(COL_LOCATION),
        (long long)node->location.offset,
        (long long)node->location.length,
        COL(RESET)
    );

    hbk_vector(hbk_syntax*) children = NULL;

    switch (node->kind) {
        default: break;

        case HBK_SYNTAX_INVALID: {
            hbk_string_view source_text = hbk_state_get_source_text(print_context->state, node->location.source_id);
            hbk_string_append_format(print_context->output, " %s%.*s", COL(RED), (int)node->location.length, source_text.data + node->location.offset);
        } break;

        case HBK_SYNTAX_DECL_FUNCTION: {
            hbk_string_append_format(print_context->output, " %s%.*s%s(", COL(COL_NAME), HBK_SV_EXPAND(node->decl_function.name.string_value), COL(RESET));
            for (int64_t i = 0; i < hbk_vector_count(node->decl_function.parameter_declarations); i++) {
                if (i > 0) {
                    hbk_string_append_format(print_context->output, "%s, ", COL(RESET));
                }

                hbk_syntax* parameter_syntax = node->decl_function.parameter_declarations[i];
                hbk_vector_push(children, parameter_syntax);

                hbk_string_append_format(print_context->output, "%s%.*s", COL(COL_NAME), HBK_SV_EXPAND(parameter_syntax->decl_parameter.name.string_value), COL(RESET));
                if (parameter_syntax->decl_parameter.type != NULL) {
                    hbk_string_append_format(print_context->output, " %s: ", COL(RESET));
                    hbk_syntax_type_print_to_string(print_context->state, parameter_syntax->decl_parameter.type, print_context->output, print_context->use_color);
                }
            }

            hbk_string_append_format(print_context->output, "%s)", COL(RESET));
            if (node->decl_function.return_type != NULL) {
                hbk_string_append_format(print_context->output, " %s: ", COL(RESET));
                hbk_syntax_type_print_to_string(print_context->state, node->decl_function.return_type, print_context->output, print_context->use_color);
            }

            if (node->decl_function.body != NULL) {
                hbk_vector_push(children, node->decl_function.body);
            }
        } break;

        case HBK_SYNTAX_DECL_PARAMETER: {
            hbk_string_append_format(print_context->output, " %s%.*s", COL(COL_NAME), HBK_SV_EXPAND(node->decl_parameter.name.string_value));
            if (node->decl_parameter.type != NULL) {
                hbk_string_append_format(print_context->output, " %s: ", COL(RESET));
                hbk_syntax_type_print_to_string(print_context->state, node->decl_parameter.type, print_context->output, print_context->use_color);
            }

            if (node->decl_parameter.default_value != NULL) {
                hbk_vector_push(children, node->decl_parameter.default_value);
            }
        } break;

        case HBK_SYNTAX_DECL_VARIABLE: {
            hbk_string_append_format(print_context->output, " %s%.*s", COL(COL_NAME), HBK_SV_EXPAND(node->decl_variable.name.string_value));
            if (node->decl_variable.type != NULL) {
                hbk_string_append_format(print_context->output, " %s: ", COL(RESET));
                hbk_syntax_type_print_to_string(print_context->state, node->decl_variable.type, print_context->output, print_context->use_color);
            }

            if (node->decl_variable.default_value != NULL) {
                hbk_vector_push(children, node->decl_variable.default_value);
            }
        } break;

        case HBK_SYNTAX_STMT_ARROW: {
            hbk_vector_push(children, node->stmt_arrow.value);
        } break;

        case HBK_SYNTAX_IDENTIFIER: {
            hbk_string_append_format(print_context->output, " %s%.*s", COL(COL_NAME), HBK_SV_EXPAND(node->identifier.name.string_value));
        } break;

        case HBK_SYNTAX_INTEGER_LITERAL: {
            hbk_string_append_format(print_context->output, " %s%lld", COL(COL_LITERAL), node->literal.integer_value);
        } break;

        case HBK_SYNTAX_STRING_LITERAL: {
            // TODO(local): print the escaped version of the literal
            hbk_string_append_format(print_context->output, " %s\"%.*s\"", COL(COL_LITERAL), HBK_SV_EXPAND(node->literal.string_value));
        } break;

        case HBK_SYNTAX_BOOL_LITERAL: {
            hbk_string_append_format(print_context->output, " %s%s", COL(COL_LITERAL), node->literal.bool_value ? "true" : "false");
        } break;
    }

    hbk_string_append_format(print_context->output, "%s\n", COL(RESET));

    if (children != NULL) {
        hbk_syntax_print_children(print_context, children);
        hbk_vector_free(children);
    }
}

void hbk_syntax_tree_print_to_string(hbk_state* state, hbk_syntax_tree* tree, hbk_string* out_string, bool use_color) {
    HBK_ASSERT(state != NULL, "invalid state pointer");
    HBK_ASSERT(tree != NULL, "invalid tree pointer");
    HBK_ASSERT(out_string != NULL, "invalid (output) string pointer");

    hbk_syntax_print_context print_context = {
        .state = state,
        .indents = NULL,
        .output = out_string,
        .use_color = use_color,
    };

    for (int64_t i = 0; i < hbk_vector_count(tree->syntax_nodes); i++) {
        hbk_syntax_print(&print_context, tree->syntax_nodes[i]);
    }
}

void hbk_syntax_type_print_to_string(hbk_state* state, hbk_syntax* type, hbk_string* out_string, bool use_color) {
    HBK_ASSERT(state != NULL, "invalid state pointer");
    HBK_ASSERT(type != NULL, "invalid syntax node pointer");
    HBK_ASSERT(out_string != NULL, "invalid (output) string pointer");

    switch (type->kind) {
        default: {
            hbk_string_append_format(out_string, "%s<unknown type %s>", COL(RED), hbk_syntax_kind_to_cstring(type->kind));
        } break;

        case HBK_SYNTAX_INVALID: {
            hbk_string_append_format(out_string, "%s<invalid>", COL(RED));
        } break;

        case HBK_SYNTAX_TYPE_INTEGER: {
            hbk_string_append_format(out_string, "%sint", COL(COL_KEYWORD));
        } break;
    }
}
