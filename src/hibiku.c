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
    hbk_state_render_diagnostics_to_file(state, stderr);

    hbk_state_destroy(state);
    return 0;
}

bool stdout_isatty() {
    return isatty(fileno(stdout));
}

bool stderr_isatty() {
    return isatty(fileno(stderr));
}
