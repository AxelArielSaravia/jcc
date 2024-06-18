//Standard Syntax c11
#ifndef JCC_
#define JCC_

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

//#define JCC_LOGS_
#define JCC_VERSION     "0.0.1"
#define JCC_COMPILER    "gcc"

#define JCC_NAME_FULL   "Just Compile my C"
#ifndef JCC_NAME
    #define JCC_NAME    "jcc"
#endif

typedef struct jcc_cmds jcc_cmds;
struct jcc_cmds {
    size_t cap;
    size_t len;
    char** buf;
};

#define JCC_BUF_INIT_CAP 32


void jcc_cmds_append_item(jcc_cmds pcmds[static 1], char item[static 1]) {
    jcc_cmds cmds = *pcmds;
    // + a nullptr item that must exist at the end of the buffer
    if (cmds.len + 2 > cmds.cap) {
        cmds.cap = cmds.cap << 1;
        cmds.buf = realloc(cmds.buf, cmds.cap * sizeof (char*));
        assert(cmds.buf != (void*)0 && "error you need more ram\n");
    }
    cmds.buf[cmds.len] = item;
    cmds.len += 1;
    *pcmds = cmds;
}

void jcc_cmds_append_buf(
    jcc_cmds pcmds[static 1],
    size_t const append_len,
    char const*const append[append_len]
) {
    if (append_len > 0) {
        jcc_cmds cmds = *pcmds;
        // + a nullptr item that must exist at the end of the buffer
        if (cmds.len + append_len + 1 > cmds.cap) {
            while (cmds.len + append_len + 1 > cmds.cap) {
                cmds.cap = cmds.cap << 1;
            }
            cmds.buf = realloc(cmds.buf, cmds.cap * sizeof (char*));
            assert(cmds.buf != (void*)0 && "error you need more ram\n");
        }
        memcpy(cmds.buf+cmds.len, append, append_len * sizeof (char*));
        cmds.len += append_len;
        *pcmds = cmds;
    }
}

void jcc_cmds_append_null(jcc_cmds pcmds[static 1]) {
    jcc_cmds cmds = *pcmds;
    assert(cmds.len+1 < cmds.cap && "ERROR bad jcc_cmds contruction");
    cmds.buf[cmds.len] = (void*)0;
}

jcc_cmds jcc_cmds_create(size_t cmds_len, char const*const cmds[cmds_len]) {
    jcc_cmds c = {
        .cap = JCC_BUF_INIT_CAP,
    };
    c.buf = calloc(JCC_BUF_INIT_CAP, sizeof (char*));
    assert(c.buf != (void*)0 && "error you need more ram\n");
    jcc_cmds_append_item(&c, JCC_COMPILER);
    jcc_cmds_append_buf(&c, cmds_len, cmds);
    return c;
}

void jcc_cmds_free(jcc_cmds* cmds) {
    if (cmds->buf != (void*)0) {
        free(cmds->buf);
    }
    *cmds = (jcc_cmds){0};
}

typedef enum jcc_cmd_type jcc_cmd_type;
enum jcc_cmd_type {
    //Shows the help messages over stdout
    JCC_CMDT_HELP,
    //Build the executable
    JCC_CMDT_BUILD,
    //Create object files
    JCC_CMDT_OBJ,
    //Build the executables and run the program
    JCC_CMDT_BRUN,
    //Build the executables, run the program and delete the executables
    JCC_CMDT_RUN,
    //Show the defaults
    JCC_CMDT_DEFAULTS,
    JCC_CMDT_VERSION,
    JCC_CMDT_LEN
};

char*const jcc_strcmds[JCC_CMDT_LEN] = {
    [JCC_CMDT_HELP]     = "help",
    [JCC_CMDT_BUILD]    = "build",
    [JCC_CMDT_OBJ]      = "obj",
    [JCC_CMDT_BRUN]     = "brun",
    [JCC_CMDT_RUN]      = "run",
    [JCC_CMDT_DEFAULTS] = "defaults",
    [JCC_CMDT_VERSION]  = "version"
};

void jcc_log_cmds(FILE f[static 1], jcc_cmds pcmds[static 1]) {
    jcc_cmds cmds = *pcmds;
    for (size_t i = 0; i < cmds.len; i += 1) {
        char const*const cmd = cmds.buf[i];
        if (strchr(cmd, ' ')) {
            fprintf(f, "'%s' ", cmd);
        } else {
            fprintf(f, "%s ", cmd);
        }
    }
    fprintf(f, "\n");
}

#define JCC_log_defaults_template() fprintf(stdout, "\
Here is an example/template of 'jcc_init.c':\n\
#include \"./jcc.c\"\n\
\n\
int main(int argc, char* argv[argc]) {\n\
    char const*const defaults[] = {\n\
        \"-std=c\23\",\n\
        \"-O2\",\n\
        \"-Wall\",\n\
        \"-Wextra\",\n\
        \"-D_FORTIFY_SOURCE=2\"\n\
    };\n\
    return jcc_init(defaults, argc, argv);\n\
}\n\
\n")
//#end

#define JCC_log_help() fprintf(stdout,"\
%s - %s \n\
A gcc wrapper.\n\
\n\
Usage:\n\
\t%s <command> ...\n\
\n\
Commands:\n\
\n\
\tbuild     compile the file or list of files\n\
\tbrun      compile the file(s) and run the executable\n\
\trun       compile the file(s), run the executable, and remove it\n\
\tobj       create object files\n\
\thelp      this text\n\
\n\
Use %s help <command> for more information.\n\
Use %s help template for a file.c template.\n\
\n\
", JCC_NAME_FULL, JCC_NAME, JCC_NAME, JCC_NAME, JCC_NAME)
//#end

#define JCC_log_help_build() fprintf(stdout,"\
Compile the file or list of files\n\
Usage:\n\
\t%s build <files> [-o name] [-L library_path] [-l library]\n\
\n\
Options:\n\
\n\
\t-o name\n\
\t\tdefine an output name. It can be declared 0 or 1 times. If the '-o' option is not declared, the output name is gets from the first file.\n\
\n\
\t-l library\n\
\t\tsame as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\t-L library_path\n\
\t\tsame as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\n\
", JCC_NAME)
//#end
#define JCC_log_help_brun() fprintf(stdout, "\
Compile the file(s) and run the executable (the same as 'run' but keeps the executable file)\n\
Usage:\n\
\t%s brun <files> [-o name] [-L library_path] [-l library]\n\
\n\
Options:\n\
\n\
\t-o name\n\
\t\tdefine an output name. It can be declared 0 or 1 times. If the '-o' option is not declared, the output name is gets from the first file.\n\
\n\
\t-l library\n\
\t\tsame as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\t-L library_path\n\
\t\tsame as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\n\
", JCC_NAME)
//#end
#define JCC_log_help_run() fprintf(stdout, "\
Compile the file(s), run the executable, and remove the executable file\n\
Usage:\n\
\t%s run <files> [-o name] [-L library_path] [-l library]\n\
\n\
Options:\n\
\n\
\t-o name\n\
\t\tdefine an output name. It can be declared 0 or 1 times. If the '-o' option is not declared, the output name is gets from the first file.\n\
\n\
\t-l library\n\
\t\tsame as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\t-L library_path\n\
\t\tsame as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\n\
", JCC_NAME)
//#end
#define JCC_log_help_obj() fprintf(stdout, "\
Create object files\n\
Usage:\n\
\t%s obj <files> [-L library_path] [-l library]\n\
\n\
Options:\n\
\n\
\t-l library\n\
\t\tsame as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\t-L library_path\n\
\t\tsame as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\n\
", JCC_NAME)
//#end
#define JCC_log_help_defaults() \
    fprintf(stdout, "%s defaults\n", JCC_NAME)
//#end
#define JCC_log_help_version() \
    fprintf(stdout, "%s version\n", JCC_NAME)
//#end
#define JCC_log_help_template() fprintf(stdout, "\
Here is an example/template of 'jcc_init.c':\n\
\n\
#include \"./jcc.c\"\n\
\n\
int main(int argc, char* argv[argc]) {\n\
    jcc_cmds defaults = jcc_cmds_create(5, (char const*const[]){\n\
        \"-std=c\",\n\
        \"-O2\",\n\
        \"-Wall\",\n\
        \"-Wextra\",\n\
        \"-D_FORTIFY_SOURCE=2\"\n\
    });\n\
    return jcc_init(&defaults, argc, argv);\n\
}\n\
\n")
//#end

_Bool jcc_wait_procs(pid_t procs) {
    if (procs == -1) {
        return (_Bool)0;
    }
    for (;;) {
        int wstatus = 0;
        if (waitpid(procs, &wstatus, 0) < 0) {
            fprintf(stderr, "could not wait on command (pid %d): %s\n", procs, strerror(errno));
            return (_Bool)0;
        }
        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                fprintf(stderr, "command exited with exit code %d\n", exit_status);
                return (_Bool)0;
            }

            break;
        }
        if (WIFSIGNALED(wstatus)) {
            fprintf(stderr, "command process was terminated\n");
            return (_Bool)0;
        }
    }
    return (_Bool)1;
}

typedef enum jcc_exec_error jcc_exec_error;
enum jcc_exec_error {
    JCC_EERROR_NONE,
    JCC_EERROR_FORK,
    JCC_EERROR_EXEC
};
typedef struct jcc_exec_res jcc_exec_res;
struct jcc_exec_res {
    int error;
    int errn;
    pid_t pid;
};

void jcc_exec_log_error(jcc_exec_res res, jcc_cmds cmds[static 1]) {
    switch (res.error) {
        case JCC_EERROR_NONE: break;
        case JCC_EERROR_FORK: {
            fprintf(stderr, "ERROR Could not fork process: %s\n", strerror(errno));
        } break;
        case JCC_EERROR_EXEC: {
            fprintf(stderr, "ERROR Could not exec:\n\t");
            jcc_log_cmds(stderr, cmds);
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
        } break;
        default: assert(0 && "unreachable");
    }
}

jcc_exec_res _jcc_exec_cmds(jcc_cmds cmds[static 1]) {
    assert(cmds->buf != (void*)0 && "ERROR Check cmds buffer");
    pid_t pid = fork();
    if (pid < 0) {
        return (jcc_exec_res){
            .error = JCC_EERROR_FORK,
            .pid = pid,
        };
    }
    if (pid == 0) {
        char** args = cmds->buf;
        if (execvp(args[0], args) < 0) {
            return (jcc_exec_res){
                .error = JCC_EERROR_EXEC,
                .pid = pid,
            };
        }
        assert(0 && "unreachable");
    }
    return (jcc_exec_res){
        .error = JCC_EERROR_NONE,
        .pid = pid,
    };
}

_Bool jcc_exec_cmds(jcc_cmds cmds[static 1]) {
    jcc_exec_res exec_res = _jcc_exec_cmds(cmds);
    if (exec_res.error != JCC_EERROR_NONE) {
        jcc_exec_log_error(exec_res, cmds);
        return (_Bool)0;
    }
    if (!jcc_wait_procs(exec_res.pid)) {
        return (_Bool)0;
    }
    return (_Bool)1;
}

_Bool jcc_file_exist(char const* file_path) {
    struct stat statbuf;
    errno = 0;
    if (stat(file_path, &statbuf) >= 0 && S_ISREG(statbuf.st_mode)) {
        return 1;
    }
    if (errno != ENOENT) {
        fprintf(
            stderr,
            "Could not check if file %s exists: %s",
            file_path,
            strerror(errno)
        );
    }
    return 0;
}

_Bool jcc_is_ext(char const* file_path, char const* ext) {
    size_t file_len = strlen(file_path);
    if (file_len <= strlen(ext)) {
        return 0;
    }
    return (strcmp(&file_path[file_len-2], ext) == 0);
}

typedef enum jcc_help_types jcc_help_types;
enum jcc_help_types {
    JCC_HELPT_BUILD,
    JCC_HELPT_OBJ,
    JCC_HELPT_BRUN,
    JCC_HELPT_RUN,
    JCC_HELPT_DEFAULTS,
    JCC_HELPT_VERSION,
    JCC_HELPT_TEMPLATE,
    JCC_HELPT_LEN
};

char*const jcc_strhelp[JCC_HELPT_LEN] = {
    [JCC_HELPT_BUILD]    = "build",
    [JCC_HELPT_OBJ]      = "obj",
    [JCC_HELPT_BRUN]     = "brun",
    [JCC_HELPT_RUN]      = "run",
    [JCC_HELPT_DEFAULTS] = "defaults",
    [JCC_HELPT_VERSION]  = "version",
    [JCC_HELPT_TEMPLATE] = "template",
};

void jcc_help(size_t argc, char*const argv[argc]) {
    assert(argc > 0);
    char const*const help = argv[0];
    for (size_t i = JCC_HELPT_BUILD; i < JCC_HELPT_LEN; i += 1) {
        if (strcmp(help, jcc_strhelp[i]) == 0) {
            if (i == JCC_HELPT_BUILD) {
                JCC_log_help_build();
                return;
            } else if (i == JCC_HELPT_OBJ) {
                JCC_log_help_obj();
                return;
            } else if (i == JCC_HELPT_BRUN) {
                JCC_log_help_brun();
                return;
            } else if (i == JCC_HELPT_RUN) {
                JCC_log_help_run();
                return;
            } else if (i == JCC_HELPT_DEFAULTS) {
                JCC_log_help_defaults();
                return;
            } else if (i == JCC_HELPT_VERSION) {
                JCC_log_help_version();
                return;
            } else if (i == JCC_HELPT_TEMPLATE) {
                JCC_log_help_template();
                return;
            }
            JCC_log_help();
        }
    }
    JCC_log_help();
}

typedef enum jcc_berror_type jcc_berror_type;
enum jcc_berror_type {
    JCC_BERROR_NONE,
    JCC_BERROR_OUTOPT_MORE,
    JCC_BERROR_OUTOPT_EMPTY,
    JCC_BERROR_LPOPT_BEFORE,
    JCC_BERROR_LPOPT_EMPTY,
    JCC_BERROR_LOPT_BEFORE,
    JCC_BERROR_LOPT_EMPTY,
    JCC_BERROR_FILE_OUTPOS,
    JCC_BERROR_FILE_EXT,
    JCC_BERROR_FILE_NOEXIST,
    JCC_BERROR_FILES_EMPTY,
    JCC_BERROR_NAME_TOOLARGE,
    JCC_BERROR_LEN
};

typedef struct jcc_berror jcc_berror;
struct jcc_berror {
    jcc_berror_type type;
    int argv_i;
};

void jcc_log_berror(
    jcc_berror const berr,
    jcc_cmd_type cmd,
    char*const* argv
) {
    switch (berr.type) {
        case JCC_BERROR_NONE: break;
        case JCC_BERROR_OUTOPT_MORE: {
            fprintf(
                stderr,
                "%s %s: ERROR Must not set more than ones the option '-o'\nSee '%s help %s'\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                JCC_NAME,
                jcc_strcmds[cmd]
            );
        } break;
        case JCC_BERROR_OUTOPT_EMPTY: {
            fprintf(
                stderr,
                "%s %s: ERROR It founds an empty '-o' option at the end.\nSee '%s help %s'\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                JCC_NAME,
                jcc_strcmds[cmd]
            );
        } break;
        case JCC_BERROR_LOPT_BEFORE: {
            fprintf(
                stderr,
                "%s %s: ERROR The option '-l' must be set after <files>.\nSee '%s help %s'\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                JCC_NAME,
                jcc_strcmds[cmd]
            );
        } break;
        case JCC_BERROR_LOPT_EMPTY: {
            fprintf(
                stderr,
                "%s %s: ERROR It founds an empty '-l' option at the end.\nSee '%s help %s'\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                JCC_NAME,
                jcc_strcmds[cmd]
            );
        } break;
        case JCC_BERROR_LPOPT_BEFORE: {
            fprintf(
                stderr,
                "%s %s: ERROR The option '-L' must be set after <files>.\nSee '%s help %s'\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                JCC_NAME,
                jcc_strcmds[cmd]
            );
        } break;
        case JCC_BERROR_LPOPT_EMPTY: {
            fprintf(
                stderr,
                "%s %s: ERROR It found an empty '-L' option at the end.\nSee '%s help %s'\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                JCC_NAME,
                jcc_strcmds[cmd]
            );
        } break;
        case JCC_BERROR_FILE_OUTPOS: {
            fprintf(
                stderr,
                "%s %s: ERROR Its found '%s', thats maybe is a <file>, outside <files> section.\nSee '%s help %s'\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                argv[berr.argv_i],
                JCC_NAME,
                jcc_strcmds[cmd]
            );
        } break;
        case JCC_BERROR_FILE_EXT: {
            fprintf(
                stderr,
                "%s %s: ERROR <file> '%s' not ends with '.c' or '.o' extension\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                argv[berr.argv_i]
            );

        } break;
        case JCC_BERROR_FILE_NOEXIST: {
            fprintf(
                stderr,
                "%s %s: ERROR <file> '%s' does not exist\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                argv[berr.argv_i]
            );
        } break;
        case JCC_BERROR_FILES_EMPTY: {
            fprintf(
                stderr,
                "%s %s: ERROR Could not found <files>\nSee '%s help %s'\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                JCC_NAME,
                jcc_strcmds[cmd]
            );
        } break;
        case JCC_BERROR_NAME_TOOLARGE: {
            fprintf(
                stderr,
                "%s %s: ERROR The file name '%s' is too large for the ouptu name\n",
                JCC_NAME,
                jcc_strcmds[cmd],
                argv[berr.argv_i]
            );
        } break;
        default: assert(0 && "unreachable");
    }
}

#define JCC_BUILD_NAME_LEN 256

//Append all args to the command dynamic array
//General format:
//  SOFT_NAME COMMAND [-o name] <files> [-l library][-L library_path]
//The rules are:
//  - The option -o can appears in any position, 0 or 1 times. The next arg
//    must be the 'output_name'. If -o does not exist, the first <file> becomes
//    the 'output_name'.
//  - The option -l and -L must appears at the end, 0 or more times.
//  - The <files> must be a section, that can appears after or before the
//    option -o, but never after the options -l and -L.
jcc_berror jcc_general_cmds_build(
    char jcc_bname[static 1],
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    _Bool start_files = 0;
    _Bool end_files = 0;
    _Bool option_output = 0;

    for (int i = 0; i < argc; i += 1) {
        char*const arg = argv[i];
        if (strcmp(arg, "-o") == 0) {
            if (option_output) {
                return (jcc_berror){
                    .type=JCC_BERROR_OUTOPT_MORE,
                    .argv_i=i
                };
            }
            if (i + 1 == argc) {
                return (jcc_berror){
                    .type=JCC_BERROR_OUTOPT_EMPTY,
                    .argv_i=i
                };
            }
            i += 1;
            option_output = 1;
            if (strlen(argv[i]) < JCC_BUILD_NAME_LEN - 1) {
                strcpy(jcc_bname, argv[i]);
            } else {
                return (jcc_berror){
                    .type=JCC_BERROR_NAME_TOOLARGE,
                    .argv_i=i,
                };
            }
            jcc_cmds_append_buf(cmds, 2, (char const*[]){"-o", jcc_bname});
            if (start_files) {
                end_files = 1;
            }
        } else if (strncmp(arg, "-L", 2) == 0) {
            if (!start_files) {
                return (jcc_berror){
                    .type=JCC_BERROR_LPOPT_BEFORE,
                    .argv_i=i
                };
            }
            end_files = 1;
            if (strlen(arg) > 2) {
                jcc_cmds_append_item(cmds, arg);
            } else {
                if (i + 1 == argc) {
                    return (jcc_berror){
                        .type=JCC_BERROR_LPOPT_EMPTY,
                        .argv_i=i
                    };
                }
                i += 1;
                jcc_cmds_append_buf(cmds, 2, (char const*[]){"-L", argv[i]});
            }

        } else if (strncmp(arg, "-l", 2) == 0) {
            if (!start_files) {
                return (jcc_berror){
                    .type=JCC_BERROR_LOPT_BEFORE,
                    .argv_i=i
                };
            }
            end_files = 1;
            if (strlen(arg) > 2) {
                jcc_cmds_append_item(cmds, arg);
            } else {
                if (i + 1 == argc) {
                    return (jcc_berror){
                        .type=JCC_BERROR_LOPT_EMPTY,
                        .argv_i=i
                    };
                }
                i += 1;
                jcc_cmds_append_buf(cmds, 2, (char const*[]){"-l", argv[i]});
            }
        } else {
            if (end_files) {
                return (jcc_berror){
                    .type=JCC_BERROR_FILE_OUTPOS,
                    .argv_i=i
                };
            }
            if (!jcc_is_ext(arg, ".c") && !jcc_is_ext(arg, ".o")) {
                return (jcc_berror){
                    .type=JCC_BERROR_FILE_EXT,
                    .argv_i=i
                };
            }
            if (!jcc_file_exist(arg)) {
                return (jcc_berror){
                    .type=JCC_BERROR_FILE_NOEXIST,
                    .argv_i=i
                };
            }

            jcc_cmds_append_item(cmds, arg);
            start_files = 1;

            if (!option_output && jcc_bname[0] == '\0') {
                size_t arg_len = strlen(arg);
                if (arg_len-1 > JCC_BUILD_NAME_LEN) {
                    return (jcc_berror){
                        .type=JCC_BERROR_NAME_TOOLARGE,
                        .argv_i=i
                    };
                }
                strncpy(jcc_bname, arg, arg_len - 2);
            }
        }
    }
    if (!start_files) {
        return (jcc_berror){.type=JCC_BERROR_FILES_EMPTY, .argv_i=-1};
    }
    if (!option_output) {
        assert(jcc_bname[0] != '\0' && "ERROR The program do not catch any output name");
        jcc_cmds_append_buf(cmds, 2, (char const*[]){"-o", jcc_bname});
    }
    return (jcc_berror){.type=JCC_BERROR_NONE, .argv_i=-1};
}

jcc_berror jcc_obj_cmds_build(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    jcc_cmds_append_item(cmds, "-c");

    _Bool start_files = 0;
    _Bool end_files = 0;

    for (int i = 0; i < argc; i += 1) {
        char*const arg = argv[i];
        if (strncmp(arg, "-L", 2) == 0) {
            if (!start_files) {
                return (jcc_berror){
                    .type=JCC_BERROR_LPOPT_BEFORE,
                    .argv_i=i
                };
            }
            end_files = 1;
            if (strlen(arg) > 2) {
                jcc_cmds_append_item(cmds, arg);
            } else {
                if (i + 1 == argc) {
                    return (jcc_berror){
                        .type=JCC_BERROR_LPOPT_EMPTY,
                        .argv_i=i
                    };
                }
                i += 1;
                jcc_cmds_append_buf(cmds, 2, (char const*[]){"-L", argv[i]});
            }

        } else if (strncmp(arg, "-l", 2) == 0) {
            if (!start_files) {
                return (jcc_berror){
                    .type=JCC_BERROR_LOPT_BEFORE,
                    .argv_i=i
                };
            }
            end_files = 1;
            if (strlen(arg) > 2) {
                jcc_cmds_append_item(cmds, arg);
            } else {
                if (i + 1 == argc) {
                    return (jcc_berror){
                        .type=JCC_BERROR_LOPT_EMPTY,
                        .argv_i=i
                    };
                }
                i += 1;
                jcc_cmds_append_buf(cmds, 2, (char const*[]){"-l", argv[i]});
            }
        } else {
            if (end_files) {
                return (jcc_berror){
                    .type=JCC_BERROR_FILE_OUTPOS,
                    .argv_i=i
                };
            }
            if (!jcc_is_ext(arg, ".c") && !jcc_is_ext(arg, ".o")) {
                return (jcc_berror){
                    .type=JCC_BERROR_FILE_EXT,
                    .argv_i=i
                };
            }
            if (!jcc_file_exist(arg)) {
                return (jcc_berror){
                    .type=JCC_BERROR_FILE_NOEXIST,
                    .argv_i=i
                };
            }

            jcc_cmds_append_item(cmds, arg);
            start_files = 1;
        }
    }
    if (!start_files) {
        return (jcc_berror){
            .type=JCC_BERROR_FILES_EMPTY,
            .argv_i=-1
        };
    }
    return (jcc_berror){
        .type=JCC_BERROR_NONE,
        .argv_i=-1
    };
}

size_t jcc_get_defaults_len(char const*const defaults[static 1]) {
    size_t acc = 0;
    while (defaults) {
        defaults += sizeof *defaults;
        acc += 1;
    }
    return acc;
}

//returns EXIT_SUCCESS or EXIT_FAILURE
int jcc_build(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    assert(argc > 0);
    char jcc_bname[JCC_BUILD_NAME_LEN] = {0};
    jcc_berror berr = jcc_general_cmds_build(jcc_bname, cmds, argc, argv);
    if (berr.type != JCC_BERROR_NONE) {
        #ifdef JCC_LOGS_
            jcc_log_cmds(stdout, cmds);
        #endif
        jcc_log_berror(berr, JCC_CMDT_BUILD, argv);
        jcc_cmds_free(cmds);
        return EXIT_FAILURE;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        return EXIT_FAILURE;
    }
    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;
}

//returns EXIT_SUCCESS or EXIT_FAILURE
int jcc_brun(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    assert(argc > 0);

    char jcc_bname[JCC_BUILD_NAME_LEN] = {0};
    jcc_berror berr = jcc_general_cmds_build(jcc_bname, cmds, argc, argv);
    if (berr.type != JCC_BERROR_NONE) {
        #ifdef JCC_LOGS_
            jcc_log_cmds(stdout, cmds);
        #endif
        jcc_log_berror(berr, JCC_CMDT_BUILD, argv);
        jcc_cmds_free(cmds);
        return EXIT_FAILURE;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        return EXIT_FAILURE;
    }

    if (!strchr(jcc_bname, '/')) {
        size_t jcc_bname_len = strlen(jcc_bname);
        // jcc_name_len + len"./" + null > JCC_BUILD_NAME_LEN
        if (jcc_bname_len + 3 > JCC_BUILD_NAME_LEN) {
            jcc_log_berror(
                (jcc_berror){.type=JCC_BERROR_NAME_TOOLARGE},
                JCC_CMDT_BUILD,
                argv
            );
            return EXIT_FAILURE;
        }
        memmove(&jcc_bname[2], jcc_bname, sizeof *jcc_bname + jcc_bname_len);
        jcc_bname[0] = '.';
        jcc_bname[1] = '/';
    }
    cmds->len = 0;
    jcc_cmds_append_item(cmds, jcc_bname);
    jcc_cmds_append_null(cmds);

    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
        putchar('\n');
    #endif

    if (!jcc_exec_cmds(cmds)) {
        return EXIT_FAILURE;
    }
    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;
}

//returns EXIT_SUCCESS or EXIT_FAILURE
int jcc_run(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    assert(argc > 0);
    char jcc_bname[JCC_BUILD_NAME_LEN] = {0};

    jcc_berror berr = jcc_general_cmds_build(jcc_bname, cmds, argc, argv);
    if (berr.type != JCC_BERROR_NONE) {
        #ifdef JCC_LOGS_
            jcc_log_cmds(stdout, cmds);
        #endif
        jcc_log_berror(berr, JCC_CMDT_BUILD, argv);
        jcc_cmds_free(cmds);
        exit(1);
        return EXIT_FAILURE;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        return EXIT_FAILURE;
    }

    if (!strchr(jcc_bname, '/')) {
        size_t jcc_bname_len = strlen(jcc_bname);
        // jcc_name_len + len"./" + null > JCC_BUILD_NAME_LEN
        if (jcc_bname_len + 3 > JCC_BUILD_NAME_LEN) {
            jcc_log_berror(
                (jcc_berror){.type=JCC_BERROR_NAME_TOOLARGE},
                JCC_CMDT_BUILD,
                argv
            );
            return EXIT_FAILURE;
        }
        memmove(&jcc_bname[2], jcc_bname, sizeof *jcc_bname + jcc_bname_len);
        jcc_bname[0] = '.';
        jcc_bname[1] = '/';
    }
    cmds->len = 0;
    jcc_cmds_append_item(cmds, jcc_bname);
    jcc_cmds_append_null(cmds);

    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
        putchar('\n');
    #endif

    if (!jcc_exec_cmds(cmds)) {
        return EXIT_FAILURE;
    }

    cmds->len = 0;
    jcc_cmds_append_buf(cmds, 2, (char const*const[]){"rm", jcc_bname});
    jcc_cmds_append_null(cmds);

    #ifdef JCC_LOGS_
        fprintf(stdout, "\n\n");
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        return EXIT_FAILURE;
    }
    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;
}

//returns EXIT_SUCCESS or EXIT_FAILURE
int jcc_obj(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    assert(argc > 0);
    jcc_berror berr = jcc_obj_cmds_build(cmds, argc, argv);
    if (berr.type != JCC_BERROR_NONE) {
        #ifdef JCC_LOGS_
            jcc_log_cmds(stdout, cmds);
        #endif
        jcc_log_berror(berr, JCC_CMDT_BUILD, argv);
        jcc_cmds_free(cmds);
        return EXIT_SUCCESS;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif
    if (!jcc_exec_cmds(cmds)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//returns EXIT_SUCCESS or EXIT_FAILURE
int jcc_init(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    if (argc < 2) {
        JCC_log_help();
        return EXIT_SUCCESS;
    }
    char const*const cmd = argv[1];
    for (int i = JCC_CMDT_HELP; i < JCC_CMDT_LEN; ++i) {
        if (strcmp(cmd, jcc_strcmds[i]) == 0) {
            if (JCC_CMDT_HELP == i) {
                if (argc - 2 < 1) {
                    JCC_log_help();
                } else {
                    jcc_help(argc - 2, &argv[2]);
                }
                return EXIT_SUCCESS;
            } else if (JCC_CMDT_BUILD == i) {
                if (argc - 2 < 1) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_build();
                    return EXIT_FAILURE;
                }
                return jcc_build(cmds, argc - 2, &argv[2]);
            } else if (JCC_CMDT_BRUN == i) {
                if (argc - 2 < 1) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_brun();
                    return EXIT_FAILURE;
                }
                return jcc_brun(cmds, argc - 2, &argv[2]);
            } else if (JCC_CMDT_RUN == i) {
                if (argc - 2 < 1) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_run();
                    return EXIT_FAILURE;
                }
                return jcc_run(cmds, argc - 2, &argv[2]);
            } else if (JCC_CMDT_OBJ == i) {
                if (argc - 2 < 1) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_obj();
                    return EXIT_FAILURE;
                }
                return jcc_obj(cmds, argc - 2, &argv[2]);
            } else if (JCC_CMDT_DEFAULTS == i) {
                fprintf(stdout, "%s %s: ", JCC_NAME, jcc_strcmds[JCC_CMDT_DEFAULTS]);
                jcc_log_cmds(stdout, cmds);
                return 0;
            } else if (JCC_CMDT_VERSION == i) {
                fprintf(
                    stdout,
                    "%s version: %s\n",
                    JCC_NAME,
                    JCC_VERSION
                );
                return EXIT_SUCCESS;
            }
            assert(0 && "unreachable");
        }
    }
    fprintf(stderr, "WARNING Could not found command %s\n\n", cmd);
    JCC_log_help();
    return EXIT_SUCCESS;
}
#endif
