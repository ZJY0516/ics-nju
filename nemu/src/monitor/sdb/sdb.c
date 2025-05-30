/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void show_wp();
int add_new_wp(char *s);
int delete_wp(int n);

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets()
{
    static char *line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read) {
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char *args)
{
    cpu_exec(-1);
    return 0;
}

static int cmd_q(char *args)
{
    nemu_state.state = NEMU_QUIT;
    return -1;
}

static int cmd_si(char *args)
{
    /* extract the first argument */
    char *arg = strtok(NULL, " ");

    if (arg == NULL) {
        /* no argument given, n=1*/
        cpu_exec(1);
    } else {
        uint64_t n = strtoul(arg, NULL, 10);
        cpu_exec(n);
    }
    return 0;
}

static int cmd_info(char *args)
{
    char *arg = strtok(NULL, " "); // arg or args?
    if (arg == NULL) {
        printf("The commad info need a subcommand\n");
    } else if (strcmp(arg, "r") == 0) {
        isa_reg_display();
    } else if (strcmp(arg, "w") == 0) {
        show_wp();
    } else {
        printf("Subcommand is wrong!\n");
    }
    return 0;
}

static int cmd_x(char *args)
{
    char *arg = strtok(NULL, " "); // arg or args?
    if (arg == NULL) {
        printf("The commad info need a subcommand\n");
    } else {
        char *str_addr = strtok(NULL, " ");
        if (str_addr == NULL) {
            printf("We need a memory address\n");
        } else {
            int n = atoi(arg);
            if (n <= 0) {
                printf("N should be a positive number!\n");
                return 0;
            }
            word_t addr = strtoul(str_addr, NULL, 16);
            // 强制类型转换？
            for (int i = 0; i < n; i++) {
                // how to test
                printf(FMT_PADDR "%#20x\t\t%u\n", addr + i * 4,
                       vaddr_read(addr + i * 4, 4),
                       vaddr_read(addr + i * 4, 4));
            }
        }
    }
    return 0;
}

static int cmd_p(char *args)
{
    bool success;
    word_t result = expr(args, &success);
    if (success) {
        printf("%u\n", result);
        return 0;
    } else
        return 1;
}

static int cmd_w(char *args)
{
    char *arg = strtok(NULL, " ");
    if (arg == NULL) {
        printf("The commad info need a subcommand\n");
        return 1;
    }
    if (add_new_wp(arg)) {
        Log("Error\n");
        return 1;
    }
    return 0;
}

static int cmd_d(char *args)
{
    char *arg = strtok(NULL, " ");
    if (arg == NULL) {
        printf("The commad info need a subcommand\n");
        return 1;
    } else {
        int n;
        sscanf(arg, "%d", &n);
        delete_wp(n);
    }
    return 0;
}

static int cmd_test_expr(char *args)
{
    FILE *file = fopen("/home/zjy/code/ics2023/nemu/tools/gen-expr/input", "r");
    if (file == NULL) {
        perror("can not open file\n");
        return 1;
    }
    char line[50000];
    while (fgets(line, sizeof(line), file)) {
        // 删除换行符（如果存在）
        size_t line_length = strlen(line);
        if (line_length > 0 && line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
        char *result_str = strtok(line, " ");
        char *expression_str = strtok(NULL, " ");
        bool success;
        char str_my_result[12] = "\0";
        if (result_str != NULL && expression_str != NULL) {
            word_t my_result = expr(expression_str, &success);
            sprintf(str_my_result, "%u", my_result);
            if (strcmp(result_str, str_my_result) == 0)
                printf("pass\n");
            else {
                printf("fail!  %s\n", expression_str);
            }
        }
    }
    fclose(file);

    return 0;
}

static int cmd_help(char *args);

static struct {
    const char *name;
    const char *description;
    int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "Stop after N steps", cmd_si},
    {"info", "print state of program", cmd_info},
    {"x", "display memory", cmd_x},
    {"p", "eval", cmd_p},
    {"w", "set watch point", cmd_w},
    {"d", "delete watch point", cmd_d},
    {"test", "test expression", cmd_test_expr},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
    /* extract the first argument */
    char *arg = strtok(NULL, " ");
    int i;

    if (arg == NULL) {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++) {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    } else {
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name,
                       cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop()
{
    if (is_batch_mode) {
        cmd_c(NULL);
        return;
    }

    for (char *str; (str = rl_gets()) != NULL;) {
        char *str_end = str + strlen(str);

        /* extract the first token as the command */
        char *cmd = strtok(str, " ");
        if (cmd == NULL) {
            continue;
        }

        /* treat the remaining string as the arguments,
         * which may need further parsing
         */
        char *args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }

#ifdef CONFIG_DEVICE
        extern void sdl_clear_event_queue();
        sdl_clear_event_queue();
#endif

        int i;
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                if (cmd_table[i].handler(args) < 0) {
                    return;
                }
                break;
            }
        }

        if (i == NR_CMD) {
            printf("Unknown command '%s'\n", cmd);
        }
    }
}

void init_sdb()
{
    /* Compile the regular expressions. */
    init_regex();

    /* Initialize the watchpoint pool. */
    init_wp_pool();
}
