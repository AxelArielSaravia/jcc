#include "./jcc.c"

int main(int argc, char* argv[argc]) {
    jcc_cmds defaults = jcc_cmds_create(5, (char const*const[]){
        "-std=c23",
        "-O2",
        "-Wall",
        "-Wextra",
        "-D_FORTIFY_SOURCE=2"
    });
    return jcc_init(&defaults, argc, argv);
}
