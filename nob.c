#define NOB_IMPLEMENTATION
#include "nob.h"

#if defined(__GNUC__)
#    define CC "gcc"
#elif defined(__clang__)
#    define CC "clang"
#else
#     #define CC "cc"
#endif

void cflags(Nob_Cmd* cmd) {
#if defined(__GNUC__)
    nob_cmd_append(cmd, "-std=gnu2x");
#elif defined(__clang__)
    nob_cmd_append(cmd, "-std=c23");
#endif
}

static bool cstring_ends_with(const char* cs, const char* end) {
    size_t cslen = strlen(cs);
    size_t endlen = strlen(end);

    if (cslen < endlen) {
        return false;
    }

    return 0 == strncmp(cs + cslen - endlen, end, endlen);
}

void hibiku_files(Nob_Cmd* cmd) {
    Nob_File_Paths files = {};
    nob_read_entire_dir("./lib", &files);

    for (size_t i = 0; i < files.count; i++) {
        const char* file = files.items[i];
        if (0 == strcmp(".", file) || 0 == strcmp("..", file)) {
            continue;
        }

        if (!cstring_ends_with(file, ".c")) {
            continue;
        }

        nob_cmd_append(cmd, nob_temp_sprintf("./lib/%s", file));
    }
}

bool build_hibiku_exe() {
    int result = true;

    Nob_Cmd cmd = {};
    nob_cmd_append(&cmd, CC);
    cflags(&cmd);
    nob_cmd_append(&cmd, "-o", "hibiku", "./src/hibiku.c");
    hibiku_files(&cmd);
    
    if (!nob_cmd_run_sync(cmd)) {
        nob_return_defer(false);
    }
    
defer:
    nob_cmd_free(cmd);
    return result;
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    int result = 0;
    if (!build_hibiku_exe()) {
        nob_return_defer(1);
    }

defer:
    return result;
}
