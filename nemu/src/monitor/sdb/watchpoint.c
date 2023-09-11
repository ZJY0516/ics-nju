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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
    int NO;
    word_t result;
    struct watchpoint *next;

    /* TODO: Add more members if necessary */
    char expr[128];
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool()
{
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    }

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
/* return a pointer of watch point in the head of free pool*/
static WP *new_wp()
{
    assert(free_ != NULL);
    WP *tmp;
    tmp = free_;
    free_ = tmp->next;
    return tmp;
}
/*add the input pointer to the head of free pool*/
static void free_wp(WP *wp)
{
    wp->next = free_;
    free_ = wp;
}
/*print info of watch point*/
void show_wp()
{
    if (head == NULL) {
        Log("No watch point now");
        return;
    }
    WP *ptr = head;
    printf("%sid  value\texpression%s\n", ANSI_FG_GREEN, ANSI_NONE);
    while (ptr) {
        printf("%-4d%-12u%s\n", ptr->NO, ptr->result, ptr->expr);
        ptr = ptr->next;
    }
}
/*
 * add a new watch point with expression
 * return 0 represents add successfully
 * return -1 represents error
 */
int add_new_wp(char *s)
{
    if (strlen(s) >= 128) {
        Log("The expression is too long\n");
        return -1;
    }
    assert(s);
    word_t value;
    bool success = false;
    value = expr(s, &success);
    if (success) {
        WP *ptr = new_wp();
        strcpy(ptr->expr, s);
        ptr->result = value;
        ptr->next = head;
        head = ptr;
        return 0;
    } else {
        Log("The expression is invalid");
        return -1;
    }
}
/*
 * delete a watch point of given id
 * return 0 represents add successfully
 * return -1 represents error
 */
int delete_wp(int n)
{
    WP *current = head;
    WP *prev = NULL;
    if (!head) {
        Log("No watch point to delete");
        return -1;
    }
    while (current && current->NO != n) {
        prev = current;
        current = current->next;
    }
    if (current != NULL) {
        if (prev == NULL) {
            head = current->next;
        } else {
            prev->next = current->next;
        }
        free_wp(current);
    }
    return 0;
}