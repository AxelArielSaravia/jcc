# JCC - Just Compile my C
This is **my own** wrapper to compile C with the GCC compiler.

The idea is to have a set of defaults (warnings, security, and standards)
and a set of commands to simplify actions. You can compile at any time with
different defaults, making it flexible and easy to adapt to various projects.


This project is inspired by [nobuild](https://github.com/tsoding/nobuild).


## Features
- **Default Compilation Flags:** Predefined set of compilation flags for
better security and standards compliance.

- **Simplified Commands:** Easy-to-use commands for common tasks like
building, running, and creating object files.


## Commands
* **build:** Compile all the files with the previously set defaults.
* **run:** Compile and run the files, then remove the executable.
* **obj:** Create only object files.


## Installation
1. Get the `jcc.c` file.
2. Create a `jcc_init.c` file (or any other name).
Example: `jcc_init.c`
```c
#include "./jcc.c"

int main(int argc, char* argv[argc]) {
    jcc_cmds defaults = jcc_cmds_create(5, (char const* const[]){
        "-std=c23",
        "-O2",
        "-Wall",
        "-Wextra",
        "-D_FORTIFY_SOURCE=2"
    });
    return jcc_init(&defaults, argc, argv);
}
```
3. Compile the `jcc_init.c` file. The project uses some C11 features, so it
must be compiled with `-std=c11`. The last GCC compiler's default standard is C11
(I think), so you do not need to specify the `-std=` option.

4. Then you can create an alias or add it to your PATH in your system.

**Voilà**

You only need these two functions: `jcc_cmds_create()` where you put all the
default options, and `jcc_init()` for the magic.


## Customization
You can change the command name that appears on the console prompts and errors
by defining the JCC_NAME header.

Example 'c99.c':
```c
#define JCC_NAME "c99"

#include "./jcc.c"

int main(int argc, char* argv[argc]) {
    jcc_cmds defaults = jcc_cmds_create(1, (char const* const[]){
        "-std=c99",
    });
    return jcc_init(&defaults, argc, argv);
}
```
Now when you call `c99 help [commands]` *c99* appears as the name of the
command.


## Note

This project was created with some Linux headers and obviously depends on GCC.
It has only been tested on my own computer.
