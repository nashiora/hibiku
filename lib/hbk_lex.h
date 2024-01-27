#ifndef HKB_LEX_H
#define HKB_LEX_H

#include <hibiku.h>

#include "hbk_api.h"

#include <stdint.h>

typedef enum hbk_token_kind {
    HBK_TOKEN_INVALID = 0,
} hbk_token_kind;

typedef struct hbk_token {
    hbk_token_kind kind;
    hbk_location location;
    int64_t integer_value;
    hbk_string_view string_value;
} hbk_token;

/// Reads all of the tokens from the source text into a vector.
/// For simplicity in implementing other parts of this compiler,
/// we don't support reading individual tokens at a time.
hbk_vector(hbk_token) hbk_read_tokens(hbk_state* state, hbk_source_id source_id);

#endif // !HKB_LEX_H
