#include "hbk_api.h"

hbk_location hbk_location_create(hbk_source_id source_id, int64_t offset, int64_t length) {
    return (hbk_location){
        .source_id = source_id,
        .offset = offset,
        .length = length,
    };
}
