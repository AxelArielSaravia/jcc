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
            if (cmds.cap < JCC_BUF_INIT_CAP) {
                cmds.cap = JCC_BUF_INIT_CAP;
            }
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
    //Build the executables and run the program
    JCC_CMDT_BRUN,
    //Build the executables, run the program and delete the executables
    JCC_CMDT_RUN,
    //Create object files
    JCC_CMDT_OBJECT,
    //make libraries
    JCC_CMDT_LIBRARY,
    //Show the defaults
    JCC_CMDT_DEFAULTS,
    JCC_CMDT_VERSION,
    JCC_CMDT_LEN
};

char*const jcc_strcmds[JCC_CMDT_LEN] = {
    [JCC_CMDT_HELP]      = "help",
    [JCC_CMDT_BUILD]     = "build",
    [JCC_CMDT_BRUN]      = "brun",
    [JCC_CMDT_RUN]       = "run",
    [JCC_CMDT_OBJECT]    = "obj",
    [JCC_CMDT_LIBRARY]   = "lib",
    [JCC_CMDT_DEFAULTS]  = "defaults",
    [JCC_CMDT_VERSION]   = "version"
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
\tlib       create a shared or static library\n\
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
\t%s build [<name>] <files> [-L <library_path>] [-l <library>]\n\
\n\
Options:\n\
\n\
\t<name>            Define an output name. If the 'name' is not declared, the output name is the first file name.\n\
\n\
\t-l <library>      Same as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\t-L <library_path> Same as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\n\
", JCC_NAME)
//#end
#define JCC_log_help_brun() fprintf(stdout, "\
Compile the file(s) and run the executable (the same as 'run' but keeps the executable file)\n\
Usage:\n\
\t%s brun [<name>] <files> [-L <library_path>] [-l <library>]\n\
\n\
Options:\n\
\n\
\t<name>            Define an output name. If the 'name' is not declared, the output name is the first file name.\n\
\n\
\t-l <library>      Same as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\t-L <library_path> Same as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\n\
", JCC_NAME)
//#end
#define JCC_log_help_run() fprintf(stdout, "\
Compile the file(s), run the executable, and remove the executable file\n\
Usage:\n\
\t%s run [<name>] <files> [-L <library_path>] [-l <library>]\n\
\n\
Options:\n\
\n\
\t<name>            Define an output name. If the 'name' is not declared, the output name is the first file name.\n\
\n\
\t-l <library>      Same as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\t-L <library_path> Same as gcc. Can only be declared after <files>, and can be declared 0 or more times.\n\
\n\
\n\
", JCC_NAME)
//#end
#define JCC_log_help_object() fprintf(stdout, "\
Create object files\n\
Usage:\n\
\t%s %s <files>\n\
\n\
", JCC_NAME, jcc_strcmds[JCC_CMDT_OBJECT])
//#end
#define JCC_log_help_library() fprintf(stdout, "\
Create a shared or static libary\n\
Usage:\n\
\t%s lib shared|static [<name>] <files>\n\
\n\
Options:\n\
\n\
\t<name>    Define an output name. If the 'name' is not declared, the output name is the first file name.\n\
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
        \"-std=c23\",\n\
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
    JCC_HELPT_BRUN,
    JCC_HELPT_RUN,
    JCC_HELPT_OBJECT,
    JCC_HELPT_LIBRARY,
    JCC_HELPT_DEFAULTS,
    JCC_HELPT_VERSION,
    JCC_HELPT_TEMPLATE,
    JCC_HELPT_LEN
};

char*const jcc_strhelp[JCC_HELPT_LEN] = {
    [JCC_HELPT_BUILD]     = jcc_strcmds[JCC_CMDT_BUILD],
    [JCC_HELPT_BRUN]      = jcc_strcmds[JCC_CMDT_BRUN],
    [JCC_HELPT_RUN]       = jcc_strcmds[JCC_CMDT_RUN],
    [JCC_HELPT_OBJECT]    = jcc_strcmds[JCC_CMDT_OBJECT],
    [JCC_HELPT_LIBRARY]   = jcc_strcmds[JCC_CMDT_LIBRARY],
    [JCC_HELPT_DEFAULTS]  = "defaults",
    [JCC_HELPT_VERSION]   = "version",
    [JCC_HELPT_TEMPLATE]  = "template",
};

void jcc_help(size_t argc, char*const argv[argc]) {
    assert(argc > 0);
    char const*const help = argv[0];
    for (size_t i = JCC_HELPT_BUILD; i < JCC_HELPT_LEN; i += 1) {
        if (strcmp(help, jcc_strhelp[i]) == 0) {
            if (i == JCC_HELPT_BUILD) {
                JCC_log_help_build();
                return;
            } else if (i == JCC_HELPT_OBJECT) {
                JCC_log_help_object();
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
            } else if (i == JCC_HELPT_LIBRARY) {
                JCC_log_help_library();
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
    JCC_BERROR_LPOPT_EMPTY,
    JCC_BERROR_LOPT_EMPTY,
    JCC_BERROR_FILE_EXT,
    JCC_BERROR_FILE_NOEXIST,
    JCC_BERROR_NOFILES,
    JCC_BERROR_NAME_TOOLARGE,
    JCC_BERROR_OPTION_INVALID,
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
        case JCC_BERROR_NOFILES: {
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
        case JCC_BERROR_OPTION_INVALID: {
            fprintf(
                stderr,
                "%s %s: ERROR Option '%s' is not allow\n", 
                JCC_NAME,
                jcc_strcmds[JCC_CMDT_OBJECT],
                argv[berr.argv_i]
            );
        } break;

        default: assert(0 && "unreachable");
    }
}

#define JCC_BUILD_NAME_LEN 256

typedef int set_bname_res;
#define JCC_BNAME_FILE              1
#define JCC_BNAME_EXIST             0
#define JCC_BNAME_ERR_NOFILES       -1
#define JCC_BNAME_ERR_TOOLARGE      -2
#define JCC_BNAME_ERR_FILENOEXIST   -3

set_bname_res jcc_set_bname(char const name[static 1], char bname[static 1]) {
    if (strncmp(name, "-", 1) == 0) {
        return JCC_BNAME_ERR_NOFILES;
    }
    if (!jcc_is_ext(name, ".c") && !jcc_is_ext(name, ".o")) {
        if (strlen(name)+1 > JCC_BUILD_NAME_LEN) {
            return JCC_BNAME_ERR_TOOLARGE;
        }
        strcpy(bname, name);
        return JCC_BNAME_EXIST;
    }
    if (!jcc_file_exist(name)) {
        return JCC_BNAME_ERR_FILENOEXIST;
    }
    size_t name_len = strlen(name);
    if (name_len+1 > JCC_BUILD_NAME_LEN) {
        return JCC_BNAME_ERR_TOOLARGE;
    }
    strncpy(bname, name, name_len - 2);
    return JCC_BNAME_FILE;
}

jcc_berror jcc_bname_error(int bnstatus) {
    switch (bnstatus) {
        case JCC_BNAME_ERR_NOFILES:
            return (jcc_berror){.type=JCC_BERROR_NOFILES};
        case JCC_BNAME_ERR_TOOLARGE:
            return (jcc_berror){
                .type=JCC_BERROR_NAME_TOOLARGE,
                .argv_i=0
            };
        case JCC_BNAME_ERR_FILENOEXIST:
            return (jcc_berror){
                .type=JCC_BERROR_FILE_NOEXIST,
                .argv_i=0
            };
        default: assert(0 && "unrechable");
    }
}

//Append all args to the command dynamic array
//General format:
//  SOFT_NAME COMMAND [name] <files> [-l library][-L library_path]
//The rules are:
//  - The option 'name' is the first argument. If 'name' does not exist, the
//    first <file> becomes the 'output_name'.
//  - The option -l and -L must appears at the end, 0 or more times.
//  - The <files> must be a section, that can never appears after the options -l and -L.
jcc_berror jcc_general_cmds_build(
    char jcc_bname[static 1],
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    //name
    int bnstatus = jcc_set_bname(argv[0], jcc_bname);
    if (bnstatus < 0) {
        return jcc_bname_error(bnstatus);
    }
    int i = 0;
    if (bnstatus == JCC_BNAME_EXIST) {
        i += 1;
    }
    jcc_cmds_append_buf(cmds, 2, (char const*[]){"-o", jcc_bname});
    //files
    _Bool files_exist = (_Bool)0;
    while (i < argc && strncmp(argv[i], "-", 1) != 0) {
        char*const arg = argv[i];
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
        files_exist = (_Bool)1;

        i += 1;
    }
    if (!files_exist) {
        return (jcc_berror){.type=JCC_BERROR_NOFILES};
    }

    //options -L, -l
    while (i < argc) {
        char*const arg = argv[i];
        if (strncmp(arg, "-L", 2) == 0) {
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
            i += 1;

        } else if (strncmp(arg, "-l", 2) == 0) {
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
            i += 1;
        }
        return (jcc_berror){
            .type=JCC_BERROR_OPTION_INVALID,
            .argv_i=i,
        };
    }

    return (jcc_berror){.type=JCC_BERROR_NONE, .argv_i=-1};
}

jcc_berror jcc_obj_cmds_build(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    _Bool files_exist = (_Bool)0;
    jcc_cmds_append_item(cmds, "-c");

    for (int i = 0; i < argc; i += 1) {
        char*const arg = argv[i];
        if (strncmp(arg, "-", 1) == 0) {
            return (jcc_berror){
                .type=JCC_BERROR_OPTION_INVALID,
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
        files_exist = (_Bool)1;
        jcc_cmds_append_item(cmds, arg);
    }
    if (!files_exist) {
        return (jcc_berror){.type=JCC_BERROR_NOFILES};
    }
    return (jcc_berror){.type=JCC_BERROR_NONE};
}

size_t jcc_get_defaults_len(char const*const defaults[static 1]) {
    size_t acc = 0;
    while (defaults) {
        defaults += sizeof *defaults;
        acc += 1;
    }
    return acc;
}

//we asume that there is at least 1 args
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
        goto defer;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        goto defer;
    }
    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;

    defer:
    jcc_cmds_free(cmds);
    return EXIT_FAILURE;
}

//we asume that there is at least 1 args
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
        goto defer;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        goto defer;
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
            goto defer;
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
        goto defer;
    }
    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;

    defer:
    jcc_cmds_free(cmds);
    return EXIT_FAILURE;
}

//we asume that there is at least 1 args
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
        goto defer;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        goto defer;
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
            goto defer;
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
        goto defer;
    }

    cmds->len = 0;
    jcc_cmds_append_buf(cmds, 2, (char const*const[]){"rm", jcc_bname});
    jcc_cmds_append_null(cmds);

    #ifdef JCC_LOGS_
        fprintf(stdout, "\n\n");
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        goto defer;
    }
    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;

    defer:
    jcc_cmds_free(cmds);
    return EXIT_FAILURE;

}

//we asume that there is at least 1 args
//returns EXIT_SUCCESS or EXIT_FAILURE
int jcc_object(
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
        goto defer;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif

    if (!jcc_exec_cmds(cmds)) {
        goto defer;
    }

    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;

    defer:
    jcc_cmds_free(cmds);
    return EXIT_FAILURE;
}

//returns EXIT_SUCCESS or EXIT_FAILURE
int jcc_libstatic(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    assert(argc > 0);
    char jcc_bname[JCC_BUILD_NAME_LEN] = {0};
    int bnstatus = jcc_set_bname(argv[0], jcc_bname);
    if (bnstatus < 0) {
        #ifdef JCC_LOGS_
            jcc_log_cmds(stdout, cmds);
        #endif
        jcc_log_berror(
            jcc_bname_error(bnstatus),
            JCC_CMDT_LIBRARY,
            argv
        );
        goto defer;
    }
    //set the '.a' at the end
    if (strlen(jcc_bname) + 3 > JCC_BUILD_NAME_LEN) {
        jcc_log_berror(
            jcc_bname_error(JCC_BNAME_ERR_TOOLARGE),
            JCC_CMDT_LIBRARY,
            argv
        );
    }
    strcat(jcc_bname, ".a");

    char** args = (char**)argv;
    int args_len = argc;
    jcc_berror berr = {0};
    if (bnstatus == JCC_BNAME_EXIST) {
        if (argc - 1 < 1) {
            #ifdef JCC_LOGS_
                jcc_log_cmds(stdout, cmds);
            #endif
            berr = (jcc_berror){.type = JCC_BERROR_NOFILES};
            jcc_log_berror(berr, JCC_CMDT_LIBRARY, argv);
            goto defer;
        }
        args_len -= 1;
        args = (char**)&argv[1];
    }

    berr = jcc_obj_cmds_build(cmds, args_len, args);
    if (berr.type != JCC_BERROR_NONE) {
        #ifdef JCC_LOGS_
            jcc_log_cmds(stdout, cmds);
        #endif
        jcc_log_berror(berr, JCC_CMDT_LIBRARY, argv);
        goto defer;
    }

    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif
    if (!jcc_exec_cmds(cmds)) {
        goto defer;
    }

    for (int i = 0; i < args_len; i += 1) {
        char* arg = args[i];
        if (jcc_is_ext(arg, ".c")) {
            size_t len = strlen(arg);
            arg[len-1] = 'o';
        }
    }

    cmds->len = 0;
    jcc_cmds_append_buf(cmds, 3, (char const*const[]){"ar", "rcs", jcc_bname});
    jcc_cmds_append_buf(cmds, args_len, (char const*const*)args);
    jcc_cmds_append_null(cmds);
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif
    if (!jcc_exec_cmds(cmds)) {
        goto defer;
    }

    cmds->len = 0;
    jcc_cmds_append_item(cmds, "rm");
    jcc_cmds_append_buf(cmds, args_len, (char const*const*)args);
    jcc_cmds_append_null(cmds);
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif
    if (!jcc_exec_cmds(cmds)) {
        goto defer;
    }

    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;

    defer:
    jcc_cmds_free(cmds);
    return EXIT_FAILURE;
}

//returns EXIT_SUCCESS or EXIT_FAILURE
int jcc_libshared(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    assert(argc > 0);
    size_t def_cmds_len = cmds->len;

    char jcc_bname[JCC_BUILD_NAME_LEN] = {0};
    int bnstatus = jcc_set_bname(argv[0], jcc_bname);
    if (bnstatus < 0) {
        #ifdef JCC_LOGS_
            jcc_log_cmds(stdout, cmds);
        #endif
        jcc_log_berror(
            jcc_bname_error(bnstatus),
            JCC_CMDT_LIBRARY,
            argv
        );
        goto defer;
    }
    //set the '.so' at the end
    if (strlen(jcc_bname) + 4 > JCC_BUILD_NAME_LEN) {
        jcc_log_berror(
            jcc_bname_error(JCC_BNAME_ERR_TOOLARGE),
            JCC_CMDT_LIBRARY,
            argv
        );
    }
    strcat(jcc_bname, ".so");

    char** args = (char**)argv;
    int args_len = argc;
    jcc_berror berr = {0};
    if (bnstatus == JCC_BNAME_EXIST) {
        if (argc - 1 < 1) {
            #ifdef JCC_LOGS_
                jcc_log_cmds(stdout, cmds);
            #endif
            berr = (jcc_berror){.type = JCC_BERROR_NOFILES};
            jcc_log_berror(berr, JCC_CMDT_LIBRARY, argv);
            goto defer;
        }
        args_len -= 1;
        args = (char**)&argv[1];
    }

    jcc_cmds_append_item(cmds, "-fpic");
    berr = jcc_obj_cmds_build(cmds, args_len, args);
    if (berr.type != JCC_BERROR_NONE) {
        #ifdef JCC_LOGS_
            jcc_log_cmds(stdout, cmds);
        #endif
        jcc_log_berror(berr, JCC_CMDT_LIBRARY, argv);
        goto defer;
    }
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, cmds);
    #endif
    if (!jcc_exec_cmds(cmds)) {
        goto defer;
    }

    for (int i = 0; i < args_len; i += 1) {
        char* arg = args[i];
        if (jcc_is_ext(arg, ".c")) {
            size_t len = strlen(arg);
            arg[len-1] = 'o';
        }
    }
    cmds->len = def_cmds_len;
    jcc_cmds_append_buf(cmds, 3, (char const*const[]){
        "-shared",
        "-o",
        jcc_bname
    });
    jcc_cmds_append_buf(cmds, args_len, (char const*const*)args);
    jcc_cmds_append_null(cmds);
    #ifdef JCC_LOGS_
        jcc_log_cmds(stdout, &cmds_cpy);
    #endif
    if (!jcc_exec_cmds(cmds)) {
        goto defer;
    }

    jcc_cmds_free(cmds);
    return EXIT_SUCCESS;

    defer:
    jcc_cmds_free(cmds);
    return EXIT_FAILURE;
}

//we asume that there is at least 2 args
int jcc_library(
    jcc_cmds cmds[static 1],
    int argc,
    char*const argv[argc]
) {
    char const*const arg = argv[0];
    if (strcmp(arg, "shared") == 0) {
        return jcc_libshared(cmds, argc-1, &argv[1]);
    } else if (strcmp(arg, "static") == 0) {
        return jcc_libstatic(cmds, argc-1, &argv[1]);
    }
    fprintf(stderr, "ERROR Bad command format\n");
    JCC_log_help_library();
    return EXIT_FAILURE;
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
                if (argc - 3 < 0) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_build();
                    return EXIT_FAILURE;
                }
                return jcc_build(cmds, argc - 2, &argv[2]);

            } else if (JCC_CMDT_BRUN == i) {
                if (argc - 3 < 0) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_brun();
                    return EXIT_FAILURE;
                }
                return jcc_brun(cmds, argc - 2, &argv[2]);

            } else if (JCC_CMDT_RUN == i) {
                if (argc - 3 < 0) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_run();
                    return EXIT_FAILURE;
                }
                return jcc_run(cmds, argc - 2, &argv[2]);

            } else if (JCC_CMDT_OBJECT == i) {
                if (argc - 3 < 0) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_object();
                    return EXIT_FAILURE;
                }
                return jcc_object(cmds, argc - 2, &argv[2]);

            } else if (JCC_CMDT_LIBRARY) {
                if (argc - 4 < 0) {
                    fprintf(stderr, "ERROR Bad command format\n");
                    JCC_log_help_library();
                    return EXIT_FAILURE;
                }
                return jcc_library(cmds, argc - 2, &argv[2]);

            } else if (JCC_CMDT_DEFAULTS == i) {
                fprintf(stdout, "%s %s: ", JCC_NAME, jcc_strcmds[JCC_CMDT_DEFAULTS]);
                jcc_log_cmds(stdout, cmds);
                return EXIT_SUCCESS;

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
