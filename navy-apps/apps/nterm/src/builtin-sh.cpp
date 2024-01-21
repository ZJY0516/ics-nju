#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

char handle_key(SDL_Event *ev);
#define MaxArgv 30

static void sh_printf(const char *format, ...)
{
    static char buf[256] = {};
    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(buf, 256, format, ap);
    va_end(ap);
    term->write(buf, len);
}

static void sh_banner()
{
    sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() { sh_printf("sh> "); }

static void sh_handle_cmd(const char *cmd)
{
    // need enhancement!
    char *t = (char *)malloc(strlen(cmd) + 1);
    strcpy(t, cmd);
    if (*t == '\n') {
        return;
    } else {
        int i = 0;
        while (t[++i] != '\n') {
            ;
        }
        t[i] = '\0';
    }
    char *argv[MaxArgv];
    int argc = 0;
    char *token = strtok(t, " ");
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    char *path = getenv("PATH");
    char *path_cpy =
        (char *)memcpy(malloc(strlen(path) + 1), path, strlen(path) + 1);
    printf("path: %s\n", path_cpy);
    char *p = strtok(path_cpy, ":");
    while (p != NULL) {
        setenv("PATH", p, 1);
        printf("p: %s\n", p);
        printf("path: %s\n", getenv("PATH"));
        if (execvp(argv[0], argv) >= 0) {
            setenv("PATH", path_cpy, 1);
            return;
        }
        p = strtok(NULL, ":");
    }
    setenv("PATH", path_cpy, 1);
    // execvp(argv[0], argv);
    return;
}

void builtin_sh_run()
{
    sh_banner();
    sh_prompt();
    setenv("PATH", "/bin/:/usr/bin/", 0);

    while (1) {
        SDL_Event ev;
        if (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
                const char *res = term->keypress(handle_key(&ev));
                if (res) {
                    sh_handle_cmd(res);
                    sh_prompt();
                }
            }
        }
        refresh_terminal();
    }
}
