#define JCC_NAME "jcc"

#include "./jcc.c"


int main(int argc, char* argv[argc]) {
    char const*const defaults[] = {
        "-std=c23",
        "-O2",
        "-Wall",
        "-Wextra",
        "-D_FORTIFY_SOURCE=2",
        0, //the null pointer terminator is a must
    };
    return jcc_init(defaults, argc, argv);
}
