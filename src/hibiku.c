#include <hibiku.h>
#include <stdio.h>

#ifdef _WIN32
#    define NOMINMAX
#    include <io.h>
#    include <Windows.h>
#    define isatty _isatty
#endif

#ifdef __linux__
#    include <execinfo.h>
#    include <unistd.h>
#endif

bool stdout_isatty();
bool stderr_isatty();

int main(int argc, char** argv) {
    fprintf(stderr, "Hello, %s!\n", HBK_VERSION);

    hbk_state* state = hbk_state_create();
    hbk_state_set_enable_color(state, stderr_isatty());

    hbk_source_id source_id = hbk_state_add_source_from_file(state, "./examples/hello.hibiku");
    auto test_diag = hbk_diagnostic_create_format(state, HBK_DIAG_ERROR, hbk_location_create(source_id, 0, 1), "Test %d", 10);

    hbk_string render_target = NULL;
    hbk_diagnostic_render_to_string(state, test_diag, &render_target);
    fprintf(stderr, "%s", render_target);
    hbk_vector_free(render_target);

    hbk_state_destroy(state);
    return 0;
}

bool stdout_isatty() {
    return isatty(fileno(stdout));
}

bool stderr_isatty() {
    return isatty(fileno(stderr));
}
