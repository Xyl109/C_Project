#ifndef BLACKLIST_H
#define BLACKLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*拉入黑名单所需累计超时次数：逾期达到该次数即禁止借阅*/
#define MAX_OVERDUE_TIMES 2

/*黑名单节点：记录某借阅人的累计超时次数*/
typedef struct BlacklistNode {
    char userName[30];      /*借阅人姓名*/
    int overdueCount;       /*累计超时次数*/
    struct BlacklistNode *next;
} BlacklistNode;

/*黑名单链表头指针类型*/
typedef BlacklistNode *Blacklist;

/*创建空黑名单链表，返回NULL*/
Blacklist createBlacklist(void);

/*按借阅人姓名查找黑名单节点：O(n)线性扫描，找到返回节点指针，否则返回NULL*/
BlacklistNode *findBlacklistByName(Blacklist head, const char *userName);

/*判断某借阅人是否已被拉入黑名单：累计超时达到 MAX_OVERDUE_TIMES 返回1，否则返回0*/
int isBlacklisted(Blacklist head, const char *userName);

/*累计某借阅人一次超时：已有节点则计数+1，无则新建节点，返回1；内存不足返回0*/
int recordOverdue(Blacklist *head, const char *userName);

/*查询某借阅人当前累计超时次数：O(n)线性扫描，未记录返回0*/
int overdueCountOf(Blacklist head, const char *userName);

/*释放整条黑名单链表，防止内存泄漏*/
void freeBlacklist(Blacklist head);

#endif
