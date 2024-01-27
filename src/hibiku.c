#include <hibiku.h>

#include <stdio.h>

int main(int argc, char** argv) {
    fprintf(stderr, "Hello, Hibiku v%s!\n", HBK_VERSION_STRING);
    return 0;
}
