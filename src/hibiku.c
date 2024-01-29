#include <hibiku.h>
#include <stdio.h>

int main(int argc, char** argv) {
    fprintf(stderr, "Hello, %s!\n", HBK_VERSION);

    hbk_state* state = hbk_state_create();
    hbk_source_id source_id = hbk_state_add_source_from_file(state, "./examples/hello.hibiku");
    hbk_diagnostic test_diag = hbk_diagnostic_create_format(state, HBK_DIAG_ERROR, hbk_location_create(source_id, 0, 1), "Test %d", 10);

    fprintf(
        stderr,
        "%.*s:%ld:%ld: %d: %.*s\n",
        HBK_SV_EXPAND(hbk_state_get_source_name(state, test_diag.location.source_id)),
        test_diag.location.offset,
        test_diag.location.length,
        (int)test_diag.kind,
        HBK_SV_EXPAND(test_diag.message)
    );

    hbk_state_destroy(state);
    return 0;
}
