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
#include <memory/vaddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
    TK_NOTYPE = 256,
    TK_EQ,    // ==
    TK_NUM,   // decimal number
    TK_PLUS,  //+
    TK_MINUS, // -
    TK_MUL,   // *
    TK_DIV,   // /
    TK_BRA,   // (
    TK_KET,   // )
    TK_HEX,   // hexadecimal-number
    TK_NEQ,   //!=
    TK_AND,   //&&
    TK_REG,   // reg name
    TK_DEREF, // dereference *
    TK_OR,    //||

    /* TODO: Add more token types */

};
#define MAX_TOKEN 1000
// the expression is illegal
bool bad_expression = false;

static struct rule {
    const char *regex;
    int token_type;
} rules[] = {
    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */
    // 16进制的规则放在十进制之前
    {" +", TK_NOTYPE},           // spaces
    {"\\+", TK_PLUS},            // plus +
    {"==", TK_EQ},               // equal ==
    {"0x[0-9A-Fa-f]+", TK_HEX},  // hexadecimal-number
    {"[0-9]+", TK_NUM},          // number why \\d+ doesn't work
    {"-", TK_MINUS},             // minus -
    {"\\*", TK_MUL},             // mul *
    {"/", TK_DIV},               // div /
    {"\\(", TK_BRA},             // bra (
    {"\\)", TK_KET},             // ket )
    {"!=", TK_NEQ},              //!=
    {"&&", TK_AND},              //&&
    {"\\|\\|", TK_OR},           //||
    {"\\$[a-zA-Z0-9]+", TK_REG}, // reg name

};

#define NR_REGEX ARRLEN(rules)
// compiled result
static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg,
                  rules[i].regex);
        }
    }
}

typedef struct token {
    int type;
    char str[50000];
} Token;

static Token tokens[MAX_TOKEN] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;
    assert(nr_token <= MAX_TOKEN);

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
                pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len,
                    substr_start);

                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                switch (rules[i].token_type) {
                case TK_NUM:
                case TK_HEX:
                case TK_REG:
                    if (substr_len >= 32) {
                        Log("Token is too long!");
                        break;
                    }
                    tokens[nr_token].type = rules[i].token_type;
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    tokens[nr_token].str[substr_len] = '\0';
                    nr_token++;
                case TK_NOTYPE:
                    break;
                default:
                    tokens[nr_token].type = rules[i].token_type;
                    nr_token++;
                    break;
                }
                break; // i don't understand this break
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e,
                   position, "");
            return false;
        }
    }
    // process deref
    for (i = 0; i < nr_token; i++) {
        if (tokens[i].type == TK_MUL &&
            (i == 0 ||
             (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX &&
              tokens[i - 1].type != TK_REG && tokens[i - 1].type != TK_KET))) {
            // ugly
            tokens[i].type = TK_DEREF;
        }
    }

    return true;
}
static bool check_parentheses(int p, int q)
{
    if (tokens[p].type != TK_BRA || tokens[q].type != TK_KET)
        return false;
    int top = 0;
    int i = 0;
    bool result = true;
    for (i = p; i < q; i++) {
        if (tokens[i].type == TK_BRA) {
            top++;
        } else if (tokens[i].type == TK_KET) {
            top--;
        } else
            continue;
        if (top < 0) { //"(4 + 3)) * ((2 - 1)"
            bad_expression = true;
            return false;
        }
        if (top == 0) //"(4 + 3) * (2 - 1)"?
            result = false;
    }
    return result;
}
static int op_priority(int op)
{
    enum {
        or, and, eq_neq, plus_minus, mul_div, deref,
    }; // priority of operators
    switch (op) {
    case TK_OR:
        return or ;
    case TK_AND:
        return and;
    case TK_PLUS:
    case TK_MINUS:
        return plus_minus;
    case TK_MUL:
    case TK_DIV:
        return mul_div;
    case TK_DEREF:
        return deref;
    default:
        printf("Undefined operator %d\n", op);
        return -1;
    }
}
static int find_main_op(int p, int q)
{
    // return the index of main operator
    int bra_count = 0;
    int ket_count = 0;
    int i = 0;
    int main_op = -1;
    int pre_priority = INT32_MAX;
    int current_priority = 0;

    for (i = p; i <= q; i++) {
        switch (tokens[i].type) {
        case TK_NUM:
            break;
        case TK_BRA:
            bra_count++;
            break;
        case TK_KET:
            ket_count++;
            break;
        default:
            current_priority = op_priority(tokens[i].type);
            if (bra_count == ket_count && current_priority <= pre_priority) {
                // main operator shuld not in a pair of parentheses
                main_op = i;
                pre_priority = current_priority;
            }
            break;
        }
    }
    return main_op;
}
static word_t eval(int p, int q)
{
    // assume all result are uint32_t
    // (1
    // obviously it's a bad expression, but it will cause stack-overflow
    // maybe call eval too much
    if (p > q) {
        Log("Bad expression!\n");
        bad_expression = true;
        return 0;
    } else if (p == q) {
        word_t num;
        if (tokens[p].type == TK_NUM) { // 十进制
            sscanf(tokens[p].str, "%u", &num);
        } else if (tokens[p].type == TK_HEX) { // 16进制
            assert(tokens[p].str[0] == '0' && tokens[p].str[1] == 'x');
            sscanf(tokens[p].str + 2, "%x", &num);
        } else if (tokens[p].type == TK_REG) { // 寄存器
            assert(tokens[p].str[0] == '$');
            bool success = false;
            num = isa_reg_str2val(tokens[p].str + 1, &success);
            if (!success) {
                Log("reg %s doesn't exsit!", tokens[p].str + 1);
                // bad_expression = true;//没搞懂
            }
        } else
            assert(0);
        return num;
    } else if (check_parentheses(p, q) == true) {
        return eval(p + 1, q - 1);
    } else if (tokens[p].type == TK_DEREF) {
        word_t addr = eval(p + 1, q);
        return vaddr_read(addr, sizeof(word_t));
    } else {
        int main_op_index = find_main_op(p, q);
        word_t val1, val2;
        if (!bad_expression) {
            val1 = eval(p, main_op_index - 1);
            val2 = eval(main_op_index + 1, q);
        } else {
            val1 = 1, val2 = 1;
        }
        switch (tokens[main_op_index].type) {
        case TK_PLUS:
            return val1 + val2;
        case TK_MINUS:
            return val1 - val2;
        case TK_MUL:
            return val1 * val2;
        case TK_DIV:
            assert(val2 != 0);
            return val1 / val2;
        case TK_AND:
            return val1 && val2;
        case TK_OR:
            return val1 || val2;
        case TK_EQ:
            return (val1 == val2);
        case TK_NEQ:
            return (val1 != val2);
        default:
            assert(0);
        }
    }
}
/*
 *expr(string,&bool)
 * if success, return result of expression
 * else let *bool=false and return 1
 */
word_t expr(char *e, bool *success)
{
    if (!make_token(e)) {
        *success = false;
        return 0;
    }
    // if (!check_parentheses(0, nr_token - 1)) {
    //     *success = false;
    //     Log("Bad expression! The brackets do not match.\n");
    //     return 0;
    // }

    /* TODO: Insert codes to evaluate the expression. */
    word_t ans = eval(0, nr_token - 1);

    if (!bad_expression) {
        *success = true;
        return ans;
    } else {
        Log("Bad expression!");
        *success = false;
        return 1;
    }
}
