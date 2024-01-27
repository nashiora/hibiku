#include <hibiku.h>

#include <stdio.h>

int main(int argc, char** argv) {
    fprintf(stderr, "Hello, %s!\n", HBK_VERSION);

    hbk_state* state = hbk_state_create();
    hbk_source_id source_id = hbk_state_add_source_from_file(state, "./examples/hello.hibiku");

    return 0;
}
