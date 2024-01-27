#include <hibiku.h>

#include <stdio.h>

int main(int argc, char** argv) {
    fprintf(stderr, "Hello, %s!\n", HBK_VERSION);

    hbk_state* state = hbk_state_create();
    hbk_source_id source_id = hbk_state_add_source_from_file(state, "./examples/hello.hibiku");

    fprintf(stderr, "%s\n%s\n", hbk_state_get_source_name(state, source_id), hbk_state_get_source_text(state, source_id));

    return 0;
}
