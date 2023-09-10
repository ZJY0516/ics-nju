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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {'\0'};
int max_length = 65530;
int len = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format = "#include <stdio.h>\n"
                           "int main() { "
                           "  unsigned result = %s; "
                           "  printf(\"%%u\", result); "
                           "  return 0; "
                           "}";

void gen_rand_op()
{
    char ops[] = {'+', '-', '*', '/'};
    sprintf(buf + +strlen(buf), "%c", ops[rand() % 4]);
    len++;
}

uint32_t gen_rand_uint32() { return rand() % UINT32_MAX; }

int choose(int n) { return rand() % n; }

void gen_num()
{
    if (len < max_length) {
        int t = sprintf(buf + strlen(buf), "%uu", gen_rand_uint32());
        len += t;
    }
}

static void gen_rand_expr()
{
    // buf[0] = '\0';
    switch (choose(3)) {
    case 0:
        gen_num();
        break;
    case 1:
        strcat(buf, "(");
        len++;
        gen_rand_expr();
        strcat(buf, ")");
        len++;
        break;
    default:
        gen_rand_expr();
        gen_rand_op();
        gen_rand_expr();
        break;
    }
}

int main(int argc, char *argv[])
{
    int seed = time(0);
    srand(seed);
    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }
    int i;
    for (i = 0; i < loop; i++) {
        gen_rand_expr();

        sprintf(code_buf, code_format, buf);

        FILE *fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        int ret = system("gcc /tmp/.code.c -o /tmp/.expr -O2 -Wall -Werror");
        if (ret != 0)
            continue;

        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        int result;
        ret = fscanf(fp, "%d", &result);
        pclose(fp);

        printf("%u %s\n", result, buf);
    }
    return 0;
}
