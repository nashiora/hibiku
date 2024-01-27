#if defined(__clang__)
#    define CC "clang"
#    define NOB_REBUILD_URSELF(binary_path, source_path) "clang", "-o", binary_path, source_path
#elif defined(__GNUC__)
#    define CC "gcc"
#    define NOB_REBUILD_URSELF(binary_path, source_path) "gcc", "-o", binary_path, source_path
#elif defined(_MSC_VER)
#    define CC "cl.exe"
#    define NOB_REBUILD_URSELF(binary_path, source_path) "cl.exe", source_path
#else
#    define CC "cc"
#    define NOB_REBUILD_URSELF(binary_path, source_path) "cc", "-o", binary_path, source_path
#endif

#define NOB_IMPLEMENTATION
#include "nob.h"

void cflags(Nob_Cmd* cmd) {
#if defined(__clang__)
    nob_cmd_append(cmd, "-std=c23");
    nob_cmd_append(cmd, "-fsanitize=address");
#elif defined(__GNUC__)
    nob_cmd_append(cmd, "-std=gnu2x");
#endif

    nob_cmd_append(cmd, "-I", "include");
    nob_cmd_append(cmd, "-ggdb");
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
